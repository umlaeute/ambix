/* ambix_interleave -  create an ambix file              -*- c -*-

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


/**
 * @brief ambix_interleave - merge several (multi-channel) audio files into a single ambix file
 *
 * ambix_interleave -o <outfile> [-O <order>] [-X <matrixfile>] <infile1> [<infile2> ...]
 * merge several (multi-channel) audio files into a single ambix file;
 * infile1 becomes W-channel, infile2 becomes X-channel,...
 * by default this will write an 'ambix simple' file (only full sets are accepted) 
 * eventually files are written as 'ambix extended' file with adaptor matrix set to unity
 * if 'order' is specified, all inchannels not needed for the full set are written as 'extrachannels'
 * 'matrixfile' is a soundfile/octavefile that is interpreted as matrix: each channel is a row, sampleframes are columns
 * if 'matrix' is specified it must construct a full-set (it must satisfy rows=(O+1)^2)
 * if 'matrix' is specified, all inchannels not needed to reconstruct to a full set are 'extrachannels'
 * if both 'order' and 'matrix' are specified they must match
 */
#include "ambix/ambix.h"
#include "sndfile.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define MARK() printf("%s:%d[%s]\n", __FILE__, __LINE__, __FUNCTION__)

typedef struct ai_t {
  ambixinfo_t info;

  char**infilenames;
  SNDFILE**inhandles;
  SF_INFO *ininfo;
  uint32_t numIns;

  char*outfilename;
  ambix_t*outhandle;

  ambixmatrix_t*matrix;
  uint32_t channels;
} ai_t;
static void usage(const char*path);

static ai_t*ai_matrix(ai_t*ai, const char*path) {
  fprintf(stderr, "ambix_interleave: matrices not yet supported\n");
  return NULL;
}

static ai_t*ai_cmdline(int argc, char**argv) {
  ai_t*ai=calloc(1, sizeof(ai_t));
  uint32_t channels=0;
  uint32_t order=0;
  while(argc) {
    if(!strcmp(argv[0], "-o")) {
      if(argc>1) {
        ai->outfilename=strdup(argv[1]);
        argv+=2;
        argc-=2;
        continue;
      }
      return NULL;
    }
    if(!strcmp(argv[0], "-O")) {
      if(argc>1) {
        order=atoi(argv[1]);
        channels=ambix_order2channels(order);
        argv+=2;
        argc-=2;
        continue;
      }
      return NULL;
    }
    if(!strcmp(argv[0], "-X")) {
      if(argc>1) {
        if(!ai_matrix(ai, argv[1]))
          return NULL;
        argv+=2;
        argc-=2;
      }
      return NULL;
    }
    ai->infilenames=argv;
    ai->numIns=argc;
    break;
  }

  if(!ai->infilenames)
    return NULL;

  if(channels>0) {
    ai->channels = channels;
  }

  if((channels > 0) && ai->matrix) {
    if(channels != ai->matrix->rows) {
      fprintf(stderr, "ambix_interleave: order%02d needs %d channels, not %d\n", order, channels, ai->matrix->rows);
      return NULL;
    }
  }

  return ai;
}

static ai_t*ai_close(ai_t*ai) {
  uint32_t i;
  if(!ai)return NULL;

  if(ai->inhandles) {
    for(i=0; i<ai->numIns; i++) {
      SNDFILE*inhandle=ai->inhandles[i];
      if(inhandle) {
        sf_close(inhandle);
      }
      ai->inhandles[i]=NULL;
    }
    free(ai->inhandles);
  }
  ai->inhandles=NULL;
  if(ai->ininfo)
    free(ai->ininfo);
  ai->ininfo=NULL;

  if(ai->outhandle) {
    ambix_close(ai->outhandle);
  }
  ai->outhandle=NULL;

  if(ai->matrix) {
    ambix_matrix_destroy(ai->matrix);
  }

  ai->matrix=NULL;

  return NULL;
}

static ai_t*ai_open_input(ai_t*ai) {
  uint32_t i;
  uint32_t channels=0;
  if(!ai)return ai;
  if(!ai->inhandles) {
    ai->inhandles=calloc(ai->numIns, sizeof(SNDFILE*));
    ai->ininfo   =calloc(ai->numIns, sizeof(SF_INFO));
  }


  for(i=0; i<ai->numIns; i++) {
    SNDFILE*inhandle=ai->inhandles[i];
    SF_INFO*info=&ai->ininfo[i];
    if(!inhandle) {
      inhandle=sf_open(ai->infilenames[i], SFM_READ, info);
    }
    if(ai->info.frames==0 || (ai->info.frames > (info->frames)))
      ai->info.frames=info->frames;
    if(ai->info.samplerate<1.)
      ai->info.samplerate=info->samplerate;
    if(ai->info.sampleformat==AMBIX_SAMPLEFORMAT_NONE) {
      int format=info->format;
      if((format & SF_FORMAT_FLOAT)  || (format & SF_FORMAT_DOUBLE))
        ai->info.sampleformat=AMBIX_SAMPLEFORMAT_FLOAT32;
      else if((format & SF_FORMAT_PCM_S8)  || (format & SF_FORMAT_PCM_16))
        ai->info.sampleformat=AMBIX_SAMPLEFORMAT_PCM16;
      else if((format & SF_FORMAT_PCM_24))
        ai->info.sampleformat=AMBIX_SAMPLEFORMAT_PCM24;
      else if((format & SF_FORMAT_PCM_32))
        ai->info.sampleformat=AMBIX_SAMPLEFORMAT_PCM32;
      else
        ai->info.sampleformat=AMBIX_SAMPLEFORMAT_PCM24;
    }
    channels+=info->channels;
    ai->inhandles[i]=inhandle;
  }

   /* check whether input channels form a valid full set */
  if(ai->matrix) {
    if(channels<ai->matrix->cols) {
      return ai_close(ai);
    }
    /* extended format */
    ai->info.ambixfileformat=AMBIX_EXTENDED;
    ai->info.ambichannels=ai->matrix->cols;
    ai->info.otherchannels=channels-ai->matrix->cols;
  } else if (ai->channels > 0) {
    if(ai->channels < channels) {
      ai->info.ambixfileformat=AMBIX_EXTENDED;
      ai->info.ambichannels=(ai->channels);
      ai->info.otherchannels=channels-(ai->channels);

      ai->matrix=ambix_matrix_init(ai->channels, ai->channels, NULL);
      ai->matrix=ambix_matrix_eye (ai->matrix);
    } else if (ai->channels == channels) {
      /* simple format */
      ai->info.ambixfileformat=AMBIX_SIMPLE;
      ai->info.ambichannels=channels;
      ai->info.otherchannels=0;
    } else {
      return ai_close(ai);
    }
  } else {
    if(!ambix_isFullSet(channels)) {
      return ai_close(ai);
    }
    /* simple format */
    ai->info.ambixfileformat=AMBIX_SIMPLE;
    ai->info.ambichannels=channels;
    ai->info.otherchannels=0;
  }

  printf("format: %s\n", (ai->info.ambixfileformat==AMBIX_SIMPLE)?"simple":"extended");
  printf("got %d input channels\n", channels);
  printf("ambichannels: %d\n", ai->info.ambichannels);
  printf("otherchannels: %d\n", ai->info.otherchannels);
  if(ai->matrix) 
    printf("matrix: [%dx%d]\n", ai->matrix->rows, ai->matrix->cols);
  else
    printf("matrix: NONE\n");

  if((ai->info.ambichannels < 1) && (ai->info.otherchannels < 1)) {
    return ai_close(ai);
  }


  return ai;
}

static ai_t*ai_open_output(ai_t*ai) {
  if(!ai)return ai;
  ai->outhandle=ambix_open(ai->outfilename, AMBIX_WRITE, &ai->info);

  if(!ai) return ai_close(ai);  
  if(AMBIX_EXTENDED==ai->info.ambixfileformat) {
    ambix_err_t err=ambix_setAdaptorMatrix(ai->outhandle, ai->matrix);
    if(err==AMBIX_ERR_SUCCESS) {
      ambix_write_header(ai->outhandle);
    } else {
      printf("setting adapator matrix [%dx%d]=%d returned %d\n", ai->matrix->rows, ai->matrix->cols, ambix_isFullSet(ai->matrix->rows), err);
      return ai_close(ai);
    }
  }
  return ai;
}

static ai_t*ai_copy_block(ai_t*ai, 
                          float*tempdata,
                          uint64_t frames) {
  uint32_t i;
  uint64_t channels=0;
  if(!ai)return ai;
  for(i=0; i<ai->numIns; i++) {
    SNDFILE*in=ai->inhandles[i];
    if(in) {
      if(frames!=sf_readf_float(in, tempdata+channels*frames, frames)) {
        return ai_close(ai);
      }
      channels+=ai->ininfo[i].channels;
    }
  }
  if(frames!=ambix_writef_float32(ai->outhandle, tempdata, tempdata+frames*ai->info.ambichannels, frames)) {
    return ai_close(ai);
  }

  return ai;
}

static ai_t*ai_copy(ai_t*ai) {
  const uint64_t blocksize=64;
  uint64_t f, frames=0;
  float32_t*tmpdata=NULL;
  if(!ai)return ai;
  frames=ai->info.frames;
  tmpdata=malloc(sizeof(float32_t)*(ai->info.ambichannels*ai->info.otherchannels)*blocksize);
  while(frames>blocksize) {
    if(!ai_copy_block(ai, tmpdata, blocksize))
      return ai_close(ai);
  }
  if(!ai_copy_block(ai, tmpdata, frames))
    return ai_close(ai);
  return ai;
}



static int ambix_interleave(ai_t*ai) {
  ai_t*result=ai_open_input(ai);
  //  if(result)printf("success @ %d!\n", __LINE__);
  result=ai_open_output(result);
  //if(result)printf("success @ %d!\n", __LINE__);
  result=ai_copy(result);
  //if(result)printf("success @ %d!\n", __LINE__);
  ai_close(ai);
  //if(result)printf("success @ %d!\n", __LINE__);
  return (result!=NULL);
}




int main(int argc, char**argv) {
  ai_t*ai=ai_cmdline(argc-1, argv+1);
  if(!ai) {
    usage("ambix_interleave");
    return 1;
  }

  return ambix_interleave(ai);
}
void usage(const char*path) {
  printf("%s - merge several audio files into an ambix file\n", path);
}
