/* libambix.c -  AMBIsonics eXchange Library              -*- c -*-

   Copyright © 2012-2016 IOhannes m zmölnig <zmoelnig@iem.at>.
         Institute of Electronic Music and Acoustics (IEM),
         University of Music and Dramatic Arts, Graz

   This file is part of libambix

   libambix is free software; you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of
   the License, or (at your option) any later version.

   libambix is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this program; if not, see <http://www.gnu.org/licenses/>.

*/

#include "private.h"

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */
#ifdef HAVE_STRING_H
# include <string.h>
#endif /* HAVE_STRING_H */

/* forward declarations */
ambix_err_t     _ambix_write_header     (ambix_t*ambix);


static ambix_err_t _check_write_ambixinfo(ambix_info_t*info) {
  /* FIXXME: rather than failing, we could force the values to be correct */
  switch(info->fileformat) {
  case AMBIX_NONE:
    if(info->ambichannels>0)
      return AMBIX_ERR_INVALID_FORMAT;
    break;
  case AMBIX_BASIC:
    if(info->extrachannels>0)
      return AMBIX_ERR_INVALID_FORMAT;
    if(!ambix_is_fullset(info->ambichannels))
      return AMBIX_ERR_INVALID_FORMAT;
    break;
  default:
    break;
  }

  return AMBIX_ERR_SUCCESS;
}

static void _ambix_info_set(ambix_t*ambix
                            , ambix_fileformat_t format
                            , int32_t otherchannels
                            , int32_t ambichannels
                            , int32_t fullambichannels
                            ) {
  switch(format) {
  case AMBIX_NONE:
    ambichannels=fullambichannels=0;
    break;
  case AMBIX_BASIC:
    otherchannels=0;
    break;
  default:
    break;
  }
  ambix->realinfo.fileformat=format;
  ambix->realinfo.ambichannels=ambichannels;
  ambix->realinfo.extrachannels=otherchannels;
  ambix->ambisonics_order=(fullambichannels>0)?ambix_channels2order(fullambichannels):0;
}

ambix_t*        ambix_open      (const char *path, const ambix_filemode_t mode, ambix_info_t*ambixinfo) {
  ambix_t*ambix=NULL;
  ambix_err_t err = AMBIX_ERR_UNKNOWN;
  int32_t ambichannels=0, otherchannels=0;
  int basic2extended = 0; /* writing extended file as basic */

  if((AMBIX_WRITE & mode) && (AMBIX_READ & mode)) {
    /* RDRW not yet implemented */
    return NULL;
  }

  if(AMBIX_WRITE & mode) {
    err=_check_write_ambixinfo(ambixinfo);
    if(err!=AMBIX_ERR_SUCCESS) {
      if (AMBIX_BASIC == ambixinfo->fileformat) {
        /* the user might actually try to write an EXTENDED file as BASIC:
         * - ambichannels do not form a full set
         * - otherchannels>0
         * LATER add better checks whether this is really the case
         */
        ambixinfo->fileformat = AMBIX_EXTENDED;
        err=_check_write_ambixinfo(ambixinfo);
        if(err!=AMBIX_ERR_SUCCESS)
          return NULL;
        basic2extended = 1;
      } else
        return NULL;
    }
    ambichannels=ambixinfo->ambichannels;
    otherchannels=ambixinfo->extrachannels;
  }

  ambix=(ambix_t*)calloc(1, sizeof(ambix_t));
  if(AMBIX_ERR_SUCCESS == _ambix_open(ambix, path, mode, ambixinfo)) {
    const ambix_fileformat_t wantformat=basic2extended?AMBIX_BASIC:ambixinfo->fileformat;
    ambix_fileformat_t haveformat;
    uint32_t channels = ambix->channels;
    /* successfully opened, initialize common stuff... */
    if(ambix->is_AMBIX) {
      if(AMBIX_WRITE & mode) {
        switch(ambixinfo->fileformat) {
        case(AMBIX_NONE):
          _ambix_info_set(ambix, AMBIX_NONE, channels, 0, 0);
          break;
        case(AMBIX_BASIC):
          _ambix_info_set(ambix, AMBIX_BASIC, 0, channels, channels);
          break;
        case(AMBIX_EXTENDED):
          /* the number of full channels is not clear yet!
           * the user MUST call set_adaptormatrix() first */
          _ambix_info_set(ambix, AMBIX_EXTENDED, otherchannels, ambichannels, 0);
          break;
        }
      } else {
        if(ambix->format>AMBIX_BASIC) {
          /* check whether channels are (N+1)^2
           * if so, we have a simple-ambix file, else it is just an ordinary caf
           */
          if(ambix->matrix.cols <= channels &&  /* reduced set must be fully present */
             ambix_is_fullset(ambix->matrix.rows)) { /* expanded set must be a full set */
            /* it's a simple AMBIX! */
            _ambix_info_set(ambix, AMBIX_EXTENDED, channels-ambix->matrix.cols, ambix->matrix.cols, ambix->matrix.rows);
          } else {
            /* ouch! matrix is not valid! */
            _ambix_info_set(ambix, AMBIX_NONE, channels, 0, 0);
          }
        } else {
          /* no uuid chunk found, it's probably a BASIC ambix file */

          /* check whether channels are (N+1)^2
           * if so, we have a simple-ambix file, else it is just an ordinary caf
           */
          if(ambix_is_fullset(channels)) { /* expanded set must be a full set */
            /* it's a simple AMBIX! */
            _ambix_info_set(ambix, AMBIX_BASIC, 0, channels, channels);
          } else {
            /* it's an ordinary CAF file */
            _ambix_info_set(ambix, AMBIX_NONE, channels, 0, 0);
          }
        }
      }
      /* retrieve markers/regions/strings*/
      _ambix_read_markersregions(ambix);
    } else {
      /* it's not a CAF file.... */
      _ambix_info_set(ambix, AMBIX_NONE, channels, 0, 0);
    }

    haveformat=ambix->realinfo.fileformat;

    ambix->filemode=mode;
    memcpy(&ambix->info, &ambix->realinfo, sizeof(ambix->info));

    if(basic2extended) {
      /* write EXTENDED files as BASIC */
#if 0
      ambix_matrix_init(ambix->info.ambichannels, ambix->realinfo.ambichannels, &ambix->matrix);
      ambix_matrix_fill(&ambix->matrix, AMBIX_MATRIX_IDENTITY);
      _ambix_matrix_pinv(&ambix->matrix, &ambix->matrix2);
#endif
    }
    if(0) {
    } else if(AMBIX_BASIC==wantformat && AMBIX_EXTENDED==haveformat) {
      ambix->info.fileformat=AMBIX_BASIC;
      ambix->use_matrix=1;
      ambix->info.ambichannels=ambix->matrix.rows;
    } else if(AMBIX_EXTENDED==wantformat && AMBIX_BASIC==haveformat) {
      ambix_matrix_init(ambix->realinfo.ambichannels, ambix->realinfo.ambichannels, &ambix->matrix);
      ambix_matrix_fill(&ambix->matrix, AMBIX_MATRIX_IDENTITY);
      ambix->info.fileformat=AMBIX_EXTENDED;
      ambix->use_matrix=0;
    }

    memcpy(ambixinfo, &ambix->info, sizeof(ambix->info));

    if(_ambix_adaptorbuffer_resize(ambix, DEFAULT_ADAPTORBUFFER_SIZE, sizeof(float32_t)) == AMBIX_ERR_SUCCESS)
      return ambix;
  }

  ambix_close(ambix);
  return NULL;
}

ambix_err_t     ambix_close     (ambix_t*ambix) {
  ambix_err_t res=AMBIX_ERR_SUCCESS;
  if(NULL==ambix) {
    return AMBIX_ERR_INVALID_HANDLE;
  }

  if((ambix->filemode & AMBIX_WRITE) && ambix->pendingHeaders) {
    _ambix_write_header(ambix);
  }

  res=_ambix_close(ambix);

  _ambix_adaptorbuffer_destroy(ambix);
  ambix_matrix_deinit(&ambix->matrix);
  ambix_matrix_deinit(&ambix->matrix2);

  ambix_delete_markers(ambix);
  ambix_delete_regions(ambix);

  free(ambix);
  ambix=NULL;
  return res;
}

int64_t ambix_seek (ambix_t* ambix, int64_t frames, int whence) {
  return _ambix_seek(ambix, frames, whence);
}

void*ambix_get_sndfile    (ambix_t*ambix) {
#ifdef HAVE_SNDFILE_H
  return _ambix_get_sndfile(ambix);
#endif
  return NULL;
}

uint32_t ambix_get_num_markers (ambix_t*ambix) {
  return ambix->num_markers;
}
uint32_t ambix_get_num_regions(ambix_t *ambix) {
  return ambix->num_regions;
}
ambix_marker_t *ambix_get_marker(ambix_t *ambix, uint32_t id) {
  if (id < ambix->num_markers)
    return &ambix->markers[id];
  else
    return NULL;
}
ambix_region_t *ambix_get_region(ambix_t *ambix, uint32_t id) {
  if (id < ambix->num_regions)
    return &ambix->regions[id];
  else
    return NULL;
}
ambix_err_t ambix_add_marker(ambix_t *ambix, ambix_marker_t *marker) {
  if(ambix->startedWriting)
    return AMBIX_ERR_UNKNOWN;

  if (!marker)
    return AMBIX_ERR_UNKNOWN;

  if (ambix->num_markers > 0)
    if (ambix->markers)
      ambix->markers = (ambix_marker_t*)realloc(ambix->markers, (ambix->num_markers+1)*sizeof(ambix_marker_t));
    else
      return AMBIX_ERR_UNKNOWN;
  else
    ambix->markers = (ambix_marker_t*)calloc(1, sizeof(ambix_marker_t));

  memcpy(&ambix->markers[ambix->num_markers], marker, sizeof(ambix_marker_t));
  ambix->num_markers += 1;

  ambix->pendingHeaders = 1;
  return AMBIX_ERR_SUCCESS;
}
ambix_err_t ambix_add_region(ambix_t *ambix, ambix_region_t *region) {
  if(ambix->startedWriting)
    return AMBIX_ERR_UNKNOWN;

  if (!region)
    return AMBIX_ERR_UNKNOWN;

  if (ambix->num_regions > 0)
    if (ambix->regions)
      ambix->regions = (ambix_region_t*)realloc(ambix->regions, (ambix->num_regions+1)*sizeof(ambix_region_t));
    else
      return AMBIX_ERR_UNKNOWN;
  else
    ambix->regions = (ambix_region_t*)calloc(1, sizeof(ambix_region_t));

  memcpy(&ambix->regions[ambix->num_regions], region, sizeof(ambix_region_t));
  ambix->num_regions += 1;

  ambix->pendingHeaders = 1;
  return AMBIX_ERR_SUCCESS;
}
ambix_err_t ambix_delete_markers(ambix_t *ambix) {
  if (ambix->num_markers > 0) {
    if (ambix->markers)
      free (ambix->markers);
    else
      return AMBIX_ERR_UNKNOWN;

    ambix->num_markers = 0;
    return AMBIX_ERR_SUCCESS;
  }

  return AMBIX_ERR_UNKNOWN;
}
ambix_err_t ambix_delete_regions(ambix_t *ambix) {
  if (ambix->num_regions > 0) {
    if (ambix->regions)
      free (ambix->regions);
    else
      return AMBIX_ERR_UNKNOWN;

    ambix->num_regions = 0;
    return AMBIX_ERR_SUCCESS;
  }
  return AMBIX_ERR_UNKNOWN;
}

const ambix_matrix_t*ambix_get_adaptormatrix    (ambix_t*ambix) {
  if(AMBIX_EXTENDED==ambix->info.fileformat)
    return &(ambix->matrix);
  return NULL;
}
ambix_err_t ambix_set_adaptormatrix     (ambix_t*ambix, const ambix_matrix_t*matrix) {
  if(0) {
  } else if((ambix->filemode & AMBIX_READ ) && (AMBIX_BASIC   == ambix->info.fileformat)) {
    ambix_matrix_t*mtx=NULL;
    /* multiply the matrix with the previous adaptor matrix */
    if(AMBIX_EXTENDED == ambix->realinfo.fileformat) {
      mtx=_ambix_matrix_multiply(matrix, &ambix->matrix, &ambix->matrix2);
      if(mtx != &ambix->matrix2)
        return AMBIX_ERR_UNKNOWN;
      ambix->use_matrix=2;
      return AMBIX_ERR_SUCCESS;
    } else {
      if(matrix->cols != ambix->realinfo.ambichannels) {
        return AMBIX_ERR_INVALID_DIMENSION;
      }
      mtx=ambix_matrix_copy(matrix, &ambix->matrix2);
      if(mtx) {
        ambix->use_matrix=2;
      } else {
        return AMBIX_ERR_UNKNOWN;
      }
    }
  } else if(ambix->filemode & AMBIX_WRITE) {
    int basic2extended = AMBIX_BASIC == ambix->info.fileformat;
    if ((AMBIX_EXTENDED != ambix->info.fileformat) && (AMBIX_BASIC != ambix->info.fileformat))
      return AMBIX_ERR_UNKNOWN;
    /* too late, writing started already */
    if(ambix->startedWriting)
      return AMBIX_ERR_UNKNOWN;
    /* check whether the matrix will expand to a full set */
    if(!ambix_is_fullset(matrix->rows))
      return AMBIX_ERR_INVALID_DIMENSION;

    if(basic2extended) {
      ambix_matrix_t*pinv=NULL;
      /* user requested AMBIX_BASIC, but now sets a matrix...
       * so we write an ambix-extended format, and create the ambix-channels by
       * multiplying the full-set with pinv(matrix)
       */

      /* check whether the reduced set has the same number of channels as we created our file for */
      if(ambix->realinfo.ambichannels != matrix->cols)
        return AMBIX_ERR_INVALID_DIMENSION;

      /* check whether the matrix is actually invertible */
      pinv=_ambix_matrix_pinv(matrix, pinv);
      if(!pinv)
        return AMBIX_ERR_INVALID_MATRIX;

      if(!ambix_matrix_copy(pinv, &ambix->matrix2))
        return AMBIX_ERR_UNKNOWN;

      ambix_matrix_destroy(pinv);
    }

    if(!ambix_matrix_copy(matrix, &ambix->matrix))
      return AMBIX_ERR_UNKNOWN;

    if(basic2extended) {
      ambix->realinfo.fileformat=AMBIX_EXTENDED;
      ambix->info.ambichannels=matrix->rows;
      ambix->use_matrix=2;
    }

    /* ready to write it to file */
    ambix->pendingHeaders=1;
    return AMBIX_ERR_SUCCESS;
  }

  return AMBIX_ERR_UNKNOWN;
}

ambix_err_t     _ambix_write_header     (ambix_t*ambix) {
  void*data=NULL;
  if(ambix->filemode & AMBIX_WRITE) {
    if((AMBIX_EXTENDED == ambix->realinfo.fileformat)) {
      _ambix_write_markersregions(ambix); // this need to be done in a more elegant way...!
      ambix_err_t res;
      /* generate UUID-chunk */
      uint64_t datalen=_ambix_matrix_to_uuid1(&ambix->matrix, NULL, ambix->byteswap);
      uint64_t usedlen=1+datalen/sizeof(float32_t);

      if(datalen<1)
        return AMBIX_ERR_UNKNOWN;

      data=calloc(usedlen, sizeof(float32_t));
      if(_ambix_matrix_to_uuid1(&ambix->matrix, data, ambix->byteswap)!=datalen)
        goto cleanup;

      /* and write it to file */
      res=_ambix_write_uuidchunk(ambix, data, datalen);
      if(data)
        free(data);

      if(AMBIX_ERR_SUCCESS==res)
        ambix->pendingHeaders=0;

      return res;
    } else {
      ambix_err_t res;
      res = _ambix_write_markersregions(ambix); // this need to be done in a more elegant way...!
      if(AMBIX_ERR_SUCCESS==res)
         ambix->pendingHeaders=0;
      return res;
    }
  } else
    return AMBIX_ERR_INVALID_FILE;

 cleanup:
  if(data)
    free(data);
  return AMBIX_ERR_UNKNOWN;
}

static ambix_err_t _ambix_check_write(ambix_t*ambix, const void*ambidata, const void*otherdata, int64_t frames) {
  /* TODO: add some checks whether writing is feasible
   * e.g. format=extended but no (or wrong) matrix present */
  if((ambix->realinfo.fileformat==AMBIX_EXTENDED) && !ambix_is_fullset(ambix->matrix.rows))
    return AMBIX_ERR_INVALID_DIMENSION;

  /* write out any headers if we haven't done so yet */
  if(ambix->pendingHeaders) {
    ambix_err_t res=_ambix_write_header(ambix);
    if(AMBIX_ERR_SUCCESS!=res)
      return res;
  }

  ambix->startedWriting=1;
  return AMBIX_ERR_SUCCESS;
}

static ambix_err_t _ambix_check_read(ambix_t*ambix, const void*ambidata, const void*otherdata, int64_t frames) {
  /* TODO: add some checks whether reading is feasible
   * e.g. format=extended but no (or wrong) matrix present */
  ambix->startedReading=1;
  return AMBIX_ERR_SUCCESS;
}


#define AMBIX_READF(type)                                               \
  int64_t ambix_readf_##type (ambix_t*ambix, type##_t*ambidata, type##_t*otherdata, int64_t frames) { \
    int64_t realframes;                                                 \
    type##_t*adaptorbuffer;                                             \
    ambix_err_t err= _ambix_check_read(ambix, (const void*)ambidata, (const void*)otherdata, frames); \
    if(AMBIX_ERR_SUCCESS != err) { return (err>0)?-err:err;}            \
    err=_ambix_adaptorbuffer_resize(ambix, frames, sizeof(type##_t));   \
    if(AMBIX_ERR_SUCCESS != err) { return (err>0)?-err:err;}            \
    adaptorbuffer=(type##_t*)ambix->adaptorbuffer;                      \
    realframes=_ambix_readf_##type(ambix, adaptorbuffer, frames);       \
    switch(ambix->use_matrix) {                                         \
    case 1:                                                             \
      _ambix_splitAdaptormatrix_##type(adaptorbuffer, ambix->realinfo.ambichannels+ambix->realinfo.extrachannels, &ambix->matrix          , ambidata, otherdata, realframes); \
      break;                                                            \
    case 2:                                                             \
      _ambix_splitAdaptormatrix_##type(adaptorbuffer, ambix->realinfo.ambichannels+ambix->realinfo.extrachannels, &ambix->matrix2         , ambidata, otherdata, realframes); \
      break;                                                            \
    default:                                                            \
      _ambix_splitAdaptor_##type      (adaptorbuffer, ambix->realinfo.ambichannels+ambix->realinfo.extrachannels, ambix->realinfo.ambichannels, ambidata, otherdata, realframes); \
    };                                                                  \
    return realframes;                                                  \
  }

#define AMBIX_WRITEF(type)                                              \
  int64_t ambix_writef_##type (ambix_t*ambix, const type##_t *ambidata, const type##_t*otherdata, int64_t frames) { \
    type##_t*adaptorbuffer;                                             \
    ambix_err_t err= _ambix_check_write(ambix, (const void*)ambidata, (const void*)otherdata, frames); \
    if(AMBIX_ERR_SUCCESS != err) { return (err>0)?-err:err;}            \
    err=_ambix_adaptorbuffer_resize(ambix, frames, sizeof(type##_t));   \
    if(AMBIX_ERR_SUCCESS != err) { return (err>0)?-err:err;}            \
    adaptorbuffer=(type##_t*)ambix->adaptorbuffer;                      \
    switch(ambix->use_matrix) {                                         \
    case 1:                                                             \
      _ambix_mergeAdaptormatrix_##type(ambidata, &ambix->matrix , otherdata, ambix->info.extrachannels, adaptorbuffer, frames); \
      break;                                                            \
    case 2:                                                             \
      _ambix_mergeAdaptormatrix_##type(ambidata, &ambix->matrix2, otherdata, ambix->info.extrachannels, adaptorbuffer, frames); \
      break;                                                            \
    default:                                                            \
      _ambix_mergeAdaptor_##type(ambidata, ambix->info.ambichannels, otherdata, ambix->info.extrachannels, adaptorbuffer, frames); \
    };                                                                  \
    return _ambix_writef_##type(ambix, adaptorbuffer, frames);          \
  }

AMBIX_READF(int16);
AMBIX_READF(int32);
AMBIX_READF(float32);
AMBIX_READF(float64);

AMBIX_WRITEF(int16);
AMBIX_WRITEF(int32);
AMBIX_WRITEF(float32);
AMBIX_WRITEF(float64);
