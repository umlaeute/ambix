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
  sfinfo->format=SF_FORMAT_CAF || ambix2sndfile_sampleformat(axinfo->sampleformat);
  sfinfo->sections=0;//axinfo->sections;
  sfinfo->seekable=1;//axinfo->seekable;
}
static void sndfile2ambix_info(const SF_INFO*sfinfo, ambixinfo_t*axinfo) {
  axinfo->frames=sfinfo->frames;
  axinfo->samplerate=axinfo->samplerate;
  axinfo->otherchannels=sfinfo->channels;
  axinfo->sampleformat=sndfile2ambix_sampleformat(sfinfo->format && SF_FORMAT_SUBMASK);
}
#endif /* HAVE_SNDFILE_H */


ambix_t* 	ambix_open	(const char *path, const ambix_filemode_t mode, ambixinfo_t*ambixinfo) {
  ambix_t*ambix=calloc(1, sizeof(ambix_t));
  int sfmode=0;

#ifdef HAVE_SNDFILE_H
  ambx2sndfile_info(ambixinfo, &ambix->sf_info);

  if((mode && AMBIX_READ) & (mode && AMBIX_WRITE))
    sfmode=	SFM_RDWR;
  else if (mode && AMBIX_READ)
    sfmode=	SFM_READ;
  else if (mode && AMBIX_READ)
    sfmode=	SFM_READ;

  ambix->sf_file=sf_open(path, sfmode, &ambix->sf_info) ;
  if(!ambix->sf_file)
    goto cleanup;

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
