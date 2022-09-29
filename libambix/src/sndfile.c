/* sndfile.c -  libsndfile support              -*- c -*-

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


   SF_UUID_ code is:
   Copyright © 2012 Christian Nachbar <christian.nachbar@student.kug.ac.at>,
         Institute of Electronic Music and Acoustics (IEM),
         University of Music and Dramatic Arts, Graz

*/

#include "private.h"

#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif /* HAVE_STDLIB_H */
#ifdef HAVE_STRING_H
# include <string.h>
#endif /* HAVE_STRING_H */
#ifdef HAVE_SNDFILE_H
# include <sndfile.h>
#endif /* HAVE_SNDFILE_H */

#ifdef _MSC_VER
# define snprintf _snprintf
#endif /* _MSC_VER */

//#define DEBUG_SFINFO


typedef struct ambixsndfile_private_t {
  /** handle to the libsndfile object */
  SNDFILE*sf_file;
  /** libsndfile info as returned by sf_open() */
  SF_INFO sf_info;
#if defined HAVE_SF_CHUNK_INFO
  /** for writing uuid chunks */
  SF_CHUNK_INFO sf_chunk;
  /** for writing other kind of chunks */
  SF_CHUNK_INFO *sf_otherchunks;
  uint32_t sf_numchunks;
#elif defined HAVE_SF_UUID_INFO
#endif
}ambixsndfile_private_t;
static inline ambixsndfile_private_t*PRIVATE(ambix_t*ax) { return ((ambixsndfile_private_t*)(ax->private_data)); }


static  ambix_sampleformat_t
sndfile2ambix_sampleformat(int sformat) {
  switch(sformat) {
  case(SF_FORMAT_PCM_16): return AMBIX_SAMPLEFORMAT_PCM16  ;
  case(SF_FORMAT_PCM_24): return AMBIX_SAMPLEFORMAT_PCM24  ;
  case(SF_FORMAT_PCM_32): return AMBIX_SAMPLEFORMAT_PCM32  ;
  case(SF_FORMAT_FLOAT ): return AMBIX_SAMPLEFORMAT_FLOAT32;
  case(SF_FORMAT_DOUBLE): return AMBIX_SAMPLEFORMAT_FLOAT64;
  }
  return AMBIX_SAMPLEFORMAT_NONE;
}

static  int
ambix2sndfile_sampleformat(ambix_sampleformat_t asformat) {
  switch(asformat) {
  case(AMBIX_SAMPLEFORMAT_PCM16):   return SF_FORMAT_PCM_16;
  case(AMBIX_SAMPLEFORMAT_PCM24):   return SF_FORMAT_PCM_24;
  case(AMBIX_SAMPLEFORMAT_PCM32):   return SF_FORMAT_PCM_32;
  case(AMBIX_SAMPLEFORMAT_FLOAT32): return SF_FORMAT_FLOAT;
  case(AMBIX_SAMPLEFORMAT_FLOAT64): return SF_FORMAT_DOUBLE;
  default:break;
  }
  return SF_FORMAT_PCM_24;
}
#ifdef DEBUG_SFINFO
static void print_sfinfo(SF_INFO*info) {
  printf("SF_INFO %p\n", info);
  printf("  frames\t: %d\n", (int)(info->frames));
  printf("  samplerate\t: %d\n", info->samplerate);
  printf("  channels\t: %d\n", info->channels);
  printf("  format\t: %d\n", info->format);
  printf("  sections\t: %d\n", info->sections);
  printf("  seekable\t: %d\n", info->seekable);
}
#endif

static void
ambix2sndfile_info(const ambix_info_t*axinfo, SF_INFO *sfinfo) {
  sfinfo->frames=axinfo->frames;
  sfinfo->samplerate=(int)axinfo->samplerate;
  sfinfo->channels=axinfo->ambichannels+axinfo->extrachannels;
  sfinfo->format=SF_FORMAT_CAF | ambix2sndfile_sampleformat(axinfo->sampleformat);
  sfinfo->sections=0;
  sfinfo->seekable=0;
}
static void
sndfile2ambix_info(const SF_INFO*sfinfo, ambix_info_t*axinfo) {
  axinfo->frames=(uint64_t)sfinfo->frames;
  axinfo->samplerate=(double)(sfinfo->samplerate);
  axinfo->extrachannels=sfinfo->channels;
  axinfo->sampleformat=sndfile2ambix_sampleformat(sfinfo->format & SF_FORMAT_SUBMASK);
}

static int
read_uuidchunk(ambix_t*ax) {
#if defined HAVE_SF_GET_CHUNK_ITERATOR && defined (HAVE_SF_CHUNK_INFO)
  int                           err ;
  SF_CHUNK_INFO chunk_info ;
  SNDFILE*file=PRIVATE(ax)->sf_file;
  SF_CHUNK_ITERATOR *iterator;
  uint32_t chunkver=0;

  const char*id="uuid";
  memset (&chunk_info, 0, sizeof (chunk_info)) ;
  snprintf (chunk_info.id, sizeof (chunk_info.id), "%s", id) ;
  chunk_info.id_size = 4 ;

  for(iterator = sf_get_chunk_iterator (file, &chunk_info); NULL!=iterator; iterator=sf_next_chunk_iterator (iterator)) {
    if(chunk_info.data)
      free(chunk_info.data);
    chunk_info.data=NULL;

    err = sf_get_chunk_size (iterator, &chunk_info) ;
    if(err != SF_ERR_NO_ERROR) {
      continue;
    }

    if(chunk_info.datalen<16) {
      continue;
    }

    chunk_info.data = malloc (chunk_info.datalen) ;
    if(!chunk_info.data) {
      return AMBIX_ERR_UNKNOWN;
    }
    err = sf_get_chunk_data (iterator, &chunk_info) ;
    if(err != SF_ERR_NO_ERROR) {
      continue;
    }

    chunkver=_ambix_checkUUID((const char*)chunk_info.data);
    if(1==chunkver) {
      if(_ambix_uuid1_to_matrix(((const char*)chunk_info.data+16), chunk_info.datalen-16, &ax->matrix, ax->byteswap)) {
        free(chunk_info.data);
        return AMBIX_ERR_SUCCESS;
      }
    } else
      continue;
  }

  if(chunk_info.data)
    free(chunk_info.data) ;
  return AMBIX_ERR_UNKNOWN;

#elif defined HAVE_SF_UUID_INFO
  SF_UUID_INFO uuid;
  SNDFILE*file=PRIVATE(ax)->sf_file;

  memset(&uuid, 0, sizeof(uuid));
  strncpy(uuid.id, _ambix_getUUID(1), 16);
  if ( !sf_command(file, SFC_GET_UUID, &uuid, sizeof(uuid)) )   {
    // extended
    if(_ambix_uuid1_to_matrix(uuid.data, uuid.data_size, &ax->matrix, ax->byteswap)) {
      return AMBIX_ERR_SUCCESS;
    }
  }
#endif
  return AMBIX_ERR_UNKNOWN;
}




ambix_err_t _ambix_open (ambix_t*ambix, const char *path, const ambix_filemode_t mode, const ambix_info_t*ambixinfo) {
  int sfmode=0;
  int caf=0;
  int is_ambix=0;

  ambix->private_data=calloc(1, sizeof(ambixsndfile_private_t));
  ambix2sndfile_info(ambixinfo, &PRIVATE(ambix)->sf_info);

  if((mode & AMBIX_READ) && (mode & AMBIX_WRITE))
    sfmode=     SFM_RDWR;
  else if (mode & AMBIX_WRITE)
    sfmode=     SFM_WRITE;
  else if (mode & AMBIX_READ)
    sfmode=     SFM_READ;

  PRIVATE(ambix)->sf_file=sf_open(path, sfmode, &PRIVATE(ambix)->sf_info) ;
  if(!PRIVATE(ambix)->sf_file)
    return AMBIX_ERR_INVALID_FILE;

  memset(&ambix->realinfo, 0, sizeof(*ambixinfo));
  sndfile2ambix_info(&PRIVATE(ambix)->sf_info, &ambix->realinfo);

  ambix->byteswap=(sf_command(PRIVATE(ambix)->sf_file, SFC_RAW_DATA_NEEDS_ENDSWAP, NULL, 0) == SF_TRUE);
  ambix->channels = PRIVATE(ambix)->sf_info.channels;

  caf=((SF_FORMAT_CAF == (SF_FORMAT_TYPEMASK & PRIVATE(ambix)->sf_info.format)) != 0);
  if(caf) {
    is_ambix=1;

    if(read_uuidchunk(ambix) == AMBIX_ERR_SUCCESS) {
      ambix->format=AMBIX_EXTENDED;
    } else {
      ambix->format=AMBIX_BASIC;
    }
  } else {
    // check whether this is an .amb file or the like...
    if(0) {
    } else if(sf_command(PRIVATE(ambix)->sf_file, SFC_WAVEX_GET_AMBISONIC, NULL, 0) == SF_AMBISONIC_B_FORMAT) {
      /*
        The four B-format signals are interleaved for each sample frame in the order W,X,Y,Z.

        If the extended six-channel B-Format is used, the U and V signals will occupy the
        fifth and sixth slots: W,X,Y,Z,U,V.

        If horizontal-only B-format  is to be represented, a three or five-channel file
        will suffice, with signals interleaved as W,X,Y (First Order), or W,X,Y,U,V (Second-order).
        However, four and -six-channel files are also acceptable, with the Z channel empty.
        Higher-order configurations are possible in theory, but are not addressed here.
        A decoder program should either 'degrade gracefully', or reject formats it cannot handle.

        For all B-format configurations, the dwChannelMask field should be set to zero.

        Though strictly speaking an optional chunk, it is recommended that the PEAK chunk be
        used for all B-Format files. Apart from its general utility, it has the special virtue
        for B-format in that applications can determine from the peak value for the Z channel
        whether the file is indeed full periphonic B-format (with height information), or
        'Horizontal-only' (Z channel present but empty).
      */
      switch(ambix->channels) {
      case  3: /* h   = 1st order 2-D */
        ambix_matrix_init(ambix_order2channels(1), ambix->channels, &ambix->matrix);
        break;
      case  4: /* f   = 1st order 3-D */
        ambix_matrix_init(ambix_order2channels(1), ambix->channels, &ambix->matrix);
        break;
      case  5: /* hh  = 2nd order 2-D */
        ambix_matrix_init(ambix_order2channels(2), ambix->channels, &ambix->matrix);
        break;
      case  6: /* fh  = 2nd order 2-D + 1st order 3-D (formerly called 2.5 order) */
        ambix_matrix_init(ambix_order2channels(2), ambix->channels, &ambix->matrix);
        break;
      case  7: /* hhh = 3rd order 2-D */
        ambix_matrix_init(ambix_order2channels(3), ambix->channels, &ambix->matrix);
        break;
      case  8: /* fhh = 3rd order 2-D + 1st order 3-D */
        ambix_matrix_init(ambix_order2channels(3), ambix->channels, &ambix->matrix);
        break;
      case  9: /* ff  = 2nd order 3-D */
        ambix_matrix_init(ambix_order2channels(2), ambix->channels, &ambix->matrix);
        break;
      case 11: /* ffh = 3rd order 2-D + 2nd order 3-D */
        ambix_matrix_init(ambix_order2channels(3), ambix->channels, &ambix->matrix);
        break;
      case 16: /* fff = 3rd order 3-D */
        ambix_matrix_init(ambix_order2channels(3), ambix->channels, &ambix->matrix);
        break;
      }
      if(NULL != ambix_matrix_fill(&ambix->matrix, AMBIX_MATRIX_FUMA)) {
        is_ambix=1;
        ambix->format=AMBIX_EXTENDED;
      } else {
        is_ambix=0;
        ambix->format=AMBIX_NONE;
      }
    } else {
      is_ambix=0;
      ambix->format=AMBIX_NONE;
    }
  }

  ambix->is_AMBIX=is_ambix;
#ifdef DEBUG_SFINFO
    print_sfinfo( &PRIVATE(ambix)->sf_info);
#endif

  return AMBIX_ERR_SUCCESS;
}

ambix_err_t     _ambix_close    (ambix_t*ambix) {
  int i;
  if(PRIVATE(ambix)->sf_file)
    sf_close(PRIVATE(ambix)->sf_file);
  PRIVATE(ambix)->sf_file=NULL;

#if defined HAVE_SF_SET_CHUNK && defined (HAVE_SF_CHUNK_INFO)
  if((PRIVATE(ambix)->sf_chunk).data)
    free((PRIVATE(ambix)->sf_chunk).data);
  for (i=0;i<PRIVATE(ambix)->sf_numchunks;i++) {
    if (PRIVATE(ambix)->sf_otherchunks[i].data)
      free(PRIVATE(ambix)->sf_otherchunks[i].data);
  }
  free(PRIVATE(ambix)->sf_otherchunks);
#endif

  free(PRIVATE(ambix));
  return AMBIX_ERR_SUCCESS;
}

int64_t _ambix_seek (ambix_t* ambix, int64_t frames, int bias) {
  int whence=0;
  if(bias & AMBIX_RDRW) {
    whence |= SFM_RDWR;
  } else {
    whence |= (bias & AMBIX_READ )?SFM_READ:0;
    whence |= (bias & AMBIX_WRITE)?SFM_WRITE:0;
  }

  whence |= (bias & SEEK_SET);
  whence |= (bias & SEEK_CUR);
  whence |= (bias & SEEK_END);

  if(PRIVATE(ambix)->sf_file)
    return (int64_t)sf_seek(PRIVATE(ambix)->sf_file, (sf_count_t)frames, whence);
  return -1;
}

void*_ambix_get_sndfile      (ambix_t*ambix) {
  return PRIVATE(ambix)->sf_file;
}
int64_t _ambix_readf_int16   (ambix_t*ambix, int16_t*data, int64_t frames) {
  return (int64_t)sf_readf_short(PRIVATE(ambix)->sf_file, (short*)data, frames) ;
}
int64_t _ambix_readf_int32   (ambix_t*ambix, int32_t*data, int64_t frames) {
  return (int64_t)sf_readf_int(PRIVATE(ambix)->sf_file, (int*)data, frames) ;
}
int64_t _ambix_readf_float32   (ambix_t*ambix, float32_t*data, int64_t frames) {
  return (int64_t)sf_readf_float(PRIVATE(ambix)->sf_file, (float*)data, frames) ;
}
int64_t _ambix_readf_float64   (ambix_t*ambix, float64_t*data, int64_t frames) {
  return (int64_t)sf_readf_double(PRIVATE(ambix)->sf_file, (double*)data, frames) ;
}

int64_t _ambix_writef_int16   (ambix_t*ambix, const int16_t*data, int64_t frames) {
  return (int64_t)sf_writef_short(PRIVATE(ambix)->sf_file, (short*)data, frames) ;
}
int64_t _ambix_writef_int32   (ambix_t*ambix, const int32_t*data, int64_t frames) {
  return (int64_t)sf_writef_int(PRIVATE(ambix)->sf_file, (int*)data, frames) ;
}
int64_t _ambix_writef_float32   (ambix_t*ambix, const float32_t*data, int64_t frames) {
  return (int64_t)sf_writef_float(PRIVATE(ambix)->sf_file, (float*)data, frames) ;
}
int64_t _ambix_writef_float64   (ambix_t*ambix, const float64_t*data, int64_t frames) {
  return (int64_t)sf_writef_double(PRIVATE(ambix)->sf_file, (double*)data, frames) ;
}
ambix_err_t _ambix_write_uuidchunk(ambix_t*ax, const void*data, int64_t datasize) {
#if defined HAVE_SF_SET_CHUNK && defined (HAVE_SF_CHUNK_INFO)
  int                           err ;
  SF_CHUNK_INFO*chunk=&PRIVATE(ax)->sf_chunk;
  int64_t datasize4 = datasize>>2;

  if(datasize4*4 < datasize)
    datasize4++;
  memset (chunk, 0, sizeof (*chunk)) ;
  snprintf (chunk->id, sizeof (chunk->id), "uuid") ;
  chunk->id_size = 4 ;
  if(chunk->data)
    free(chunk->data);

  chunk->data = calloc(datasize4, 4);

  memcpy(chunk->data, data, datasize);
  chunk->datalen = datasize ;
  err = sf_set_chunk (PRIVATE(ax)->sf_file, chunk) ;
  if(SF_ERR_NO_ERROR != err)
    return AMBIX_ERR_UNKNOWN;
  return AMBIX_ERR_SUCCESS;
#elif defined HAVE_SF_UUID_INFO
  SF_UUID_INFO uuid;
  memcpy(uuid.id, data, 16);
  uuid.data=(void*)(data+16);
  uuid.data_size=datasize-16;
  if(!sf_command(PRIVATE(ax)->sf_file, SFC_SET_UUID, &uuid, sizeof(uuid))) {
    return  AMBIX_ERR_SUCCESS;
  }
#endif
  return  AMBIX_ERR_UNKNOWN;
}
ambix_err_t _ambix_write_chunk(ambix_t*ax, uint32_t id, const void*data, int64_t datasize) {
#if defined (HAVE_SF_SET_CHUNK)
  int err ;
  int64_t datasize4 = datasize>>2;

  SF_CHUNK_INFO *chunks = NULL;
  SF_CHUNK_INFO *chunk  = NULL;
  unsigned int numchunks=(PRIVATE(ax)->sf_numchunks);

  if(datasize4*4 < datasize)
    datasize4++;

  /* (re-)allocate memory for the new chunk - data has to be kept until file closes! */
  if (0 == numchunks) {
    chunks = (SF_CHUNK_INFO*)calloc(1, sizeof(*chunks));
  } else {
    chunks = (SF_CHUNK_INFO*)realloc(PRIVATE(ax)->sf_otherchunks, (numchunks+1)*sizeof(*chunks));
  }
  if (NULL == chunks)
    return AMBIX_ERR_UNKNOWN;
  chunk = chunks + numchunks;
  memset (chunk, 0, sizeof (*chunk)) ;

  //snprintf (chunk->id, 4, id) ;
  memcpy(chunk->id, &id, 4);
  chunk->id_size = 4 ;
  if(chunk->data)
    free(chunk->data);

  chunk->data = calloc(datasize4, 4);

  memcpy(chunk->data, data, datasize);
  chunk->datalen = datasize ;
  err = sf_set_chunk (PRIVATE(ax)->sf_file, chunk);

  PRIVATE(ax)->sf_numchunks += 1;
  PRIVATE(ax)->sf_otherchunks = chunks;

  if(SF_ERR_NO_ERROR != err)
    return AMBIX_ERR_UNKNOWN;
  return AMBIX_ERR_SUCCESS;
#endif
  return  AMBIX_ERR_UNKNOWN;
}
void* _ambix_read_chunk(ambix_t*ax, uint32_t id, uint32_t chunk_it, int64_t *datasize) {
#if defined HAVE_SF_GET_CHUNK_ITERATOR
  int err;
  SF_CHUNK_INFO	chunk_info;
  SF_CHUNK_ITERATOR * iterator;
  int i;
  memset (&chunk_info, 0, sizeof (chunk_info));
  memcpy(chunk_info.id, &id, 4);
  chunk_info.id_size = 4;
  iterator = sf_get_chunk_iterator (PRIVATE(ax)->sf_file, &chunk_info);
  if (!iterator) {
    *datasize = 0;
    return NULL;
  }
  // jump to wanted iterator
  for (i=0; i<chunk_it; i++) {
    iterator = sf_next_chunk_iterator (iterator) ;
    if (!iterator) {
      *datasize = 0;
      return NULL;
    }
  }
  memset (&chunk_info, 0, sizeof (chunk_info));
  err = sf_get_chunk_size (iterator, &chunk_info);
  if (!chunk_info.datalen) {
    *datasize = 0;
    return NULL;
  }
  chunk_info.data = malloc (chunk_info.datalen); // has to be freed later by the caller!
  err = sf_get_chunk_data (iterator, &chunk_info);

  // free (chunk_info.data);
  *datasize = chunk_info.datalen;
  return chunk_info.data;
#endif
  /* coverity[unreachable]: reachable in the case of sndfile without set_chunk */
  *datasize = 0;
  return NULL;
}
