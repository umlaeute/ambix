/* sndfile.c -  libsndfile support              -*- c -*-

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

#include "private.h"

#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif /* HAVE_STDLIB_H */
#ifdef HAVE_STRING_H
# include <string.h>
#endif /* HAVE_STRING_H */


typedef struct ambixsndfile_private_t {
  /** handle to the libsndfile object */
  SNDFILE*sf_file;
  /** libsndfile info as returned by sf_open() */
  SF_INFO sf_info;
  /** for writing uuid chunks */
  SF_CHUNK_INFO sf_chunk;
}ambixsndfile_private_t;
static inline ambixsndfile_private_t*PRIVATE(ambix_t*ax) { return ((ambixsndfile_private_t*)(ax->private)); }


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
static void print_sfinfo(SF_INFO*info) {
  printf("SF_INFO 0x%X\n", info);
  printf("  frames\t: %d\n", info->frames);
  printf("  samplerate\t: %d\n", info->samplerate);
  printf("  channels\t: %d\n", info->channels);
  printf("  format\t: %d\n", info->format);
  printf("  sections\t: %d\n", info->sections);
  printf("  seekable\t: %d\n", info->seekable);
}

static void
ambix2sndfile_info(const ambixinfo_t*axinfo, SF_INFO *sfinfo) {
  sfinfo->frames=axinfo->frames;
  sfinfo->samplerate=axinfo->samplerate;
  sfinfo->channels=axinfo->ambichannels+axinfo->otherchannels;
  sfinfo->format=SF_FORMAT_CAF | ambix2sndfile_sampleformat(axinfo->sampleformat);
  sfinfo->sections=0;
  sfinfo->seekable=0;
}
static void
sndfile2ambix_info(const SF_INFO*sfinfo, ambixinfo_t*axinfo) {
  axinfo->frames=sfinfo->frames;
  axinfo->samplerate=(double)(sfinfo->samplerate);
  axinfo->otherchannels=sfinfo->channels;
  axinfo->sampleformat=sndfile2ambix_sampleformat(sfinfo->format & SF_FORMAT_SUBMASK);
}

static int
ambix_read_uuidchunk(ambix_t*ax) {
	int				err ;
  int result=0;
	SF_CHUNK_INFO	chunk_info ;
  SNDFILE*file=PRIVATE(ax)->sf_file;
  uint32_t chunkver=0;
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
  
  chunkver=_ambix_checkUUID(chunk_info.data);
  if(1==chunkver) {
    if(!_ambix_uuid1_to_matrix(chunk_info.data+16, chunk_info.datalen-16, &ax->matrix, ax->byteswap)) {
      result=__LINE__;goto simple;
    }
  } else {
    result=__LINE__;goto simple;
  }

  if(chunk_info.data)
    free(chunk_info.data) ;
  return AMBIX_ERR_SUCCESS;

 simple:

  if(chunk_info.data)
    free(chunk_info.data) ;
  return result;

 cleanup:

  if(chunk_info.data)
    free(chunk_info.data) ;
  return result;
}




ambix_err_t _ambix_open	(ambix_t*ambix, const char *path, const ambix_filemode_t mode, const ambixinfo_t*ambixinfo) {
  int sfmode=0;
  uint32_t channels=0;
  int isCAF=0;

  ambix->private=calloc(1, sizeof(ambixsndfile_private_t));
  ambix2sndfile_info(ambixinfo, &PRIVATE(ambix)->sf_info);

  if((mode & AMBIX_READ) & (mode & AMBIX_WRITE))
    sfmode=	SFM_RDWR;
  else if (mode & AMBIX_WRITE)
    sfmode=	SFM_WRITE;
  else if (mode & AMBIX_READ)
    sfmode=	SFM_READ;

  PRIVATE(ambix)->sf_file=sf_open(path, sfmode, &PRIVATE(ambix)->sf_info) ;
  if(!PRIVATE(ambix)->sf_file)
    return AMBIX_ERR_INVALID_FILE;

  memset(&ambix->realinfo, 0, sizeof(*ambixinfo));
  sndfile2ambix_info(&PRIVATE(ambix)->sf_info, &ambix->realinfo);

  channels=PRIVATE(ambix)->sf_info.channels;
  isCAF=(SF_FORMAT_CAF & PRIVATE(ambix)->sf_info.format);
  ambix->byteswap=(sf_command(PRIVATE(ambix)->sf_file, SFC_RAW_DATA_NEEDS_ENDSWAP, NULL, 0) == SF_TRUE);

  /* FIXXXME most of this should go into libambix.c */
  if(isCAF) {
    if(ambix_read_uuidchunk(ambix) == AMBIX_ERR_SUCCESS) {
      /* check whether channels are (N+1)^2
       * if so, we have a simple-ambix file, else it is just an ordinary caf
       */
      if(ambix->matrix.cols <= channels &&  /* reduced set must be fully present */
         ambix_isFullSet(ambix->matrix.rows)) { /* expanded set must be a full set */
        /* it's a simple AMBIX! */
        ambix->ambisonics_order=ambix_channels2order(ambix->matrix.rows);
        ambix->realinfo.ambixfileformat=AMBIX_EXTENDED;
        ambix->realinfo.ambichannels=ambix->matrix.cols;
        ambix->realinfo.otherchannels=channels-ambix->matrix.cols;
      } else {
        /* ouch! matrix is not valid! */
        ambix->ambisonics_order=0;
        ambix->realinfo.ambixfileformat=AMBIX_NONE;
        ambix->realinfo.ambichannels=0;
        ambix->realinfo.otherchannels=channels;
      }
    } else {
      /* no uuid chunk found, it's probably a SIMPLE ambix file */

      /* check whether channels are (N+1)^2
       * if so, we have a simple-ambix file, else it is just an ordinary caf
       */
      if(ambix_isFullSet(channels)) { /* expanded set must be a full set */
        /* it's a simple AMBIX! */
        ambix->ambisonics_order=ambix_channels2order(channels);
        ambix->realinfo.ambixfileformat=AMBIX_SIMPLE;
        ambix->realinfo.ambichannels=channels;
        ambix->realinfo.otherchannels=0;
      } else {
        /* it's an ordinary CAF file */
        ambix->ambisonics_order=0;
        ambix->realinfo.ambixfileformat=AMBIX_NONE;
        ambix->realinfo.ambichannels=0;
        ambix->realinfo.otherchannels=channels;
      }
    }
  } else {
    /* it's not a CAF file.... */
    ambix->ambisonics_order=0;
    ambix->realinfo.ambixfileformat=AMBIX_NONE;
    ambix->realinfo.ambichannels=0;
    ambix->realinfo.otherchannels=channels;
  }

  //print_sfinfo(&PRIVATE(ambix)->sf_info);
  //_ambix_print_info(&ambix->realinfo);


  return AMBIX_ERR_SUCCESS;
}

ambix_err_t	_ambix_close	(ambix_t*ambix) {
  if(PRIVATE(ambix)->sf_file)
    sf_close(PRIVATE(ambix)->sf_file);
  PRIVATE(ambix)->sf_file=NULL;
  free(PRIVATE(ambix));
  return AMBIX_ERR_SUCCESS;
}

SNDFILE*_ambix_get_sndfile	(ambix_t*ambix) {
  return PRIVATE(ambix)->sf_file;
}

int64_t _ambix_readf_int16   (ambix_t*ambix, int16_t*data, int64_t frames) {
  return sf_readf_short(PRIVATE(ambix)->sf_file, (short*)data, frames) ;
}
int64_t _ambix_readf_int32   (ambix_t*ambix, int32_t*data, int64_t frames) {
  return sf_readf_int(PRIVATE(ambix)->sf_file, (int*)data, frames) ;
}
int64_t _ambix_readf_float32   (ambix_t*ambix, float32_t*data, int64_t frames) {
  return sf_readf_float(PRIVATE(ambix)->sf_file, (float*)data, frames) ;
}

int64_t _ambix_writef_int16   (ambix_t*ambix, int16_t*data, int64_t frames) {
  return sf_writef_short(PRIVATE(ambix)->sf_file, (short*)data, frames) ;
}
int64_t _ambix_writef_int32   (ambix_t*ambix, int32_t*data, int64_t frames) {
  return sf_writef_int(PRIVATE(ambix)->sf_file, (int*)data, frames) ;
}
int64_t _ambix_writef_float32   (ambix_t*ambix, float32_t*data, int64_t frames) {
  return sf_writef_float(PRIVATE(ambix)->sf_file, (float*)data, frames) ;
}
ambix_err_t _ambix_write_uuidchunk(ambix_t*ax, const void*data, int64_t datasize) {
	int				err ;
  SF_CHUNK_INFO*chunk=&PRIVATE(ax)->sf_chunk;

	memset (chunk, 0, sizeof (chunk)) ;
	snprintf (chunk->id, sizeof (chunk->id), "uuid") ;
	chunk->id_size = 4 ;
  if(chunk->data)
    free(chunk->data);
	chunk->data = malloc(datasize);
  memcpy(chunk->data, data, datasize);
	chunk->datalen = datasize ;

	err = sf_set_chunk (PRIVATE(ax)->sf_file, chunk) ;

  if(SF_ERR_NO_ERROR != err)
    return AMBIX_ERR_UNKNOWN;

  return AMBIX_ERR_SUCCESS;
}
