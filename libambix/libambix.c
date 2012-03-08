/* libambix.c -  Ambisonics Xchange Library              -*- c -*-

   Copyright © 2012 IOhannes m zmölnig <zmoelnig@iem.at>.
         Institute of Electronic Music and Acoustics (IEM),
         University of Music and Dramatic Arts, Graz

   This file is part of libambix

   libambix is free software; you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of
   the License, or (at your option) any later version.

   Libgcrypt is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this program; if not, see <http://www.gnu.org/licenses/>.

*/


#include "ambix/private.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

static int
ambix_checkuuid(char*data) {
  const char uuid[]="IEM.AT/AMBIX/XML";
  unsigned int i;
  for(i=0; i<16; i++)
    if(uuid[i]!=data[i])
      return 0;

  return 1;
}

static int
ambix_matrix_destroy(ambix_t*ax) {
  uint32_t r;
  for(r=0; r<ax->matrix_rows; r++) {
    if(ax->matrix[r])
      free(ax->matrix[r]);
    ax->matrix[r]=NULL;
  }
  free(ax->matrix);
  ax->matrix=NULL;
  ax->matrix_rows=0;
  ax->matrix_cols=0;
}
static int
ambix_matrix_create(ambix_t*ax, uint32_t rows, uint32_t cols) {
  uint32_t r;
  ambix_clearmatrix(ax);
  if(rows<1 || cols<1)
    return 0;

  ax->matrix_rows=rows;
  ax->matrix_cols=cols;
  ax->matrix=(float32_t**)malloc(sizeof(float32_t*)*rows);
  for(r=0; r<rows; r++) {
    ax->matrix[r]=(float32_t*)malloc(sizeof(float32_t)*cols);
  }
  return 1;
}
static int
ambix_matrix_fill(ambix_t*ax, float32_t*data) {
  float32_t**matrix=ax->matrix;
  uint32_t rows=ax->matrix_rows;
  uint32_t cols=ax->matrix_cols;
  uint32_t r;
  for(r=0; r<rows; r++) {
    uint32_t c;
    for(c=0; c<cols; c++) {
      matrix[r][c]=*data++;
    }
  }
  return 0;
}


static int
ambix_readmatrix(ambix_t*ax, void*data, uint64_t datasize) {
  uint32_t rows;
  uint32_t cols;
  uint64_t size;
  uint32_t index;
  ambix_matrix_destroy(ax);
  if(datasize<(sizeof(rows)+sizeof(cols)))
    return 0;

  index = 0;

  memcpy(&rows, data+index, sizeof(uint32_t));	
  index += sizeof(uint32_t);
			
  memcpy(&cols, data+index, sizeof(uint32_t));	
  index += sizeof(uint32_t);

  size=rows*cols;
  if(size*sizeof(float32_t) > datasize) {
    return 0;
  }

  if(!ambix_matrix_create(ax, rows, cols))
    return 0;

  if(!ambix_matrix_fill(ax, (float32_t*)(data+index)))
     return 0;

  return 1;
}


#ifdef HAVE_SNDFILE_H

static  ambix_sampleformat_t
sndfile2ambix_sampleformat(int sformat) {
  switch(sformat) {
  case(SF_FORMAT_PCM_16): return AMBIX_SAMPLEFORMAT_PCM16  ;
  case(SF_FORMAT_PCM_24): return AMBIX_SAMPLEFORMAT_PCM24  ;
  case(SF_FORMAT_PCM_32): return AMBIX_SAMPLEFORMAT_PCM32  ;
  case(SF_FORMAT_FLOAT ): return AMBIX_SAMPLEFORMAT_FLOAT32;
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
  }
  return SF_FORMAT_PCM_24;
}
static void ambix2sndfile_info(const ambixinfo_t*axinfo, SF_INFO *sfinfo) {
  sfinfo->frames=axinfo->frames;
  sfinfo->samplerate=axinfo->samplerate;
  sfinfo->channels=axinfo->ambichannels+axinfo->otherchannels;
  sfinfo->format=SF_FORMAT_CAF | ambix2sndfile_sampleformat(axinfo->sampleformat);
  sfinfo->sections=0;//axinfo->sections;
  sfinfo->seekable=1;//axinfo->seekable;
}
static void sndfile2ambix_info(const SF_INFO*sfinfo, ambixinfo_t*axinfo) {
  axinfo->frames=sfinfo->frames;
  axinfo->samplerate=axinfo->samplerate;
  axinfo->otherchannels=sfinfo->channels;
  axinfo->sampleformat=sndfile2ambix_sampleformat(sfinfo->format & SF_FORMAT_SUBMASK);
}

static int ambix_read_uuidchunk(ambix_t*ax) {
	int				err ;
  int result=0;
	SF_CHUNK_INFO	chunk_info ;
  SNDFILE*file=ax->sf_file;
  const char*id="uuid";
	memset (&chunk_info, 0, sizeof (chunk_info)) ;
	snprintf (chunk_info.id, sizeof (chunk_info.id), id) ;
	chunk_info.id_size = 4 ;
	err = sf_get_chunk_size (file, &chunk_info) ;
  if(err != SF_ERR_NO_ERROR) {
    result=__LINE__;goto cleanup;
  }
  if(chunk_info.datalen<16) {
    result=__LINE__;goto cleanup;
  }

	chunk_info.data = malloc (chunk_info.datalen) ;
	err = sf_get_chunk_data (file, &chunk_info) ;
  if(err != SF_ERR_NO_ERROR) {
    /* FIXXME: no data chunk, can only be AMBIX_SIMPLE */
    result=__LINE__;goto simple;
  }
  
  if(!ambix_checkuuid(chunk_info.data)) {
    result=__LINE__;goto simple;
  }
  if(!ambix_readmatrix(ax, chunk_info.data+16, chunk_info.datalen-16)) {
    result=__LINE__;goto simple;
  }

  if(chunk_info.data)
    free(chunk_info.data) ;
  return 0;

 simple:

  if(chunk_info.data)
    free(chunk_info.data) ;
  return result;

 cleanup:

  if(chunk_info.data)
    free(chunk_info.data) ;
  return result;
}


#endif /* HAVE_SNDFILE_H */


ambix_t* 	ambix_open	(const char *path, const ambix_filemode_t mode, ambixinfo_t*ambixinfo) {
  ambix_t*ambix=calloc(1, sizeof(ambix_t));
  int sfmode=0;
  uint32_t channels=0;
  int isCAF=0;

#ifdef HAVE_SNDFILE_H
  ambx2sndfile_info(ambixinfo, &ambix->sf_info);

  if((mode & AMBIX_READ) & (mode & AMBIX_WRITE))
    sfmode=	SFM_RDWR;
  else if (mode & AMBIX_READ)
    sfmode=	SFM_READ;
  else if (mode & AMBIX_READ)
    sfmode=	SFM_READ;

  ambix->sf_file=sf_open(path, sfmode, &ambix->sf_info) ;
  if(!ambix->sf_file)
    goto cleanup;

  channels=ambix->sf_info.channels;
  isCAF=(SF_FORMAT_CAF & ambix->sf_info.format);

  if(isCAF) {
    if(ambix_read_uuidchunk(ambix) == 0) {
      /* check whether channels are (N+1)^2
       * if so, we have a simple-ambix file, else it is just an ordinary caf
       */
      uint32_t order1=(uint32_t)sqrt((double)ambix->matrix_rows);
      if(order1*order1==ambix->matrix_rows) {
        /* it's a simple AMBIX! */
        ambix->ambisonics_order=order1-1;
        ambix->info.ambixfileformat=AMBIX_EXTENDED;
        ambix->info.ambichannels=ambix->matrix_cols;
        ambix->info.otherchannels=channels-ambix->matrix_cols;
      } else {
        /* ouch! matrix is not valid! */
        ambix->ambisonics_order=0;
        ambix->info.ambixfileformat=AMBIX_NONE;
        ambix->info.ambichannels=0;
        ambix->info.otherchannels=channels;
      }
    } else {
      /* no uuid chunk found, it's probably a SIMPLE ambix file */

      /* check whether channels are (N+1)^2
       * if so, we have a simple-ambix file, else it is just an ordinary caf
       */
      uint32_t order1=(uint32_t)sqrt((double)channels);
      if(order1*order1==channels) {
        /* it's a simple AMBIX! */
        ambix->ambisonics_order=order1-1;
        ambix->info.ambixfileformat=AMBIX_SIMPLE;
        ambix->info.ambichannels=channels;
        ambix->info.otherchannels=0;
      } else {
        /* it's an ordinary CAF file */
        ambix->ambisonics_order=0;
        ambix->info.ambixfileformat=AMBIX_NONE;
        ambix->info.ambichannels=0;
        ambix->info.otherchannels=channels;
      }
    }
  } else {
    /* it's not a CAF file.... */
    ambix->ambisonics_order=0;
    ambix->info.ambixfileformat=AMBIX_NONE;
    ambix->info.ambichannels=0;
    ambix->info.otherchannels=channels;
  }

#else
  goto cleanup;
#endif

  return ambix;

 cleanup:
  ambix_close(ambix);
  return NULL;
}

ambix_err_t	ambix_close	(ambix_t*ambix) {
  if(NULL==ambix) {
    return AMBIX_ERR_INVALID_HANDLE;
  }
#ifdef HAVE_SNDFILE_H
  if(ambix->sf_file)
    sf_close(ambix->sf_file);
  ambix->sf_file=NULL;

#endif
  free(ambix);
  ambix=NULL;
  return AMBIX_ERR_SUCCESS;
}

SNDFILE*ambix_get_sndfile	(ambix_t*ambix) {
#ifdef HAVE_SNDFILE_H
  return ambix->sf_file;
#endif /* HAVE_SNDFILE_H */
  return NULL;
}
