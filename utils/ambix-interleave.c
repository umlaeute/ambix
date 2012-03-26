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
 * by default this will write an 'ambix basic' file (only full sets are accepted)
 * eventually files are written as 'ambix extended' file with adaptor matrix set to unity
 * if 'order' is specified, all inchannels not needed for the full set are written as 'extrachannels'
 * 'matrixfile' is a soundfile/octavefile that is interpreted as matrix: each channel is a row, sampleframes are columns
 * if 'matrix' is specified it must construct a full-set (it must satisfy rows=(O+1)^2)
 * if 'matrix' is specified, all inchannels not needed to reconstruct to a full set are 'extrachannels'
 * if both 'order' and 'matrix' are specified they must match
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include "ambix/ambix.h"
#include "sndfile.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define AMBIX_INTERLEAVE_VERSION "0.1"

#define MARK() printf("%s:%d[%s]\n", __FILE__, __LINE__, __FUNCTION__)


typedef struct ai_t {
  ambix_info_t info;

  char**infilenames;
  SNDFILE**inhandles;
  SF_INFO *ininfo;
  uint32_t numIns;

  char*outfilename;
  ambix_t*outhandle;

  ambix_matrix_t*matrix;
  uint32_t channels;

  uint32_t blocksize;
#define DEFAULT_BLOCKSIZE 1024
} ai_t;
static void print_usage(const char*path);
static void print_version(const char*path);

static ai_t*ai_matrix(ai_t*ai, const char*path) {
  SF_INFO info;
  uint32_t rows, cols;
  float*data=NULL;
  ai_t*result=NULL;
  ambix_matrix_t*mtx=NULL;
  uint32_t frames;

  memset(&info, 0, sizeof(info));
  SNDFILE*file=sf_open(path, SFM_READ, &info);

  if(!file) {
    fprintf(stderr, "ambix_interleave: matrix open failed '%s'\n", path);
    return NULL;
  }
  rows=info.channels;
  cols=info.frames;
  data=malloc(rows*cols*sizeof(float));
  frames=sf_readf_float(file, data, cols);
  if(cols!=frames) {
    fprintf(stderr, "ambix_interleave: matrix reading %d frames returned %d\n", frames, cols);
    goto cleanup;
  }

  mtx=ambix_matrix_init(cols, rows, NULL);
  if(mtx && (AMBIX_ERR_SUCCESS==ambix_matrix_fill_data(mtx, data))) {
    uint32_t r, c;
    ai->matrix=ambix_matrix_init(rows, cols, NULL);

    for(r=0; r<rows; r++)
      for(c=0; c<cols; c++)
        ai->matrix->data[r][c]=mtx->data[c][r];
  }

  result=ai;

  //  fprintf(stderr, "ambix_interleave: matrices not yet supported\n");
  cleanup:
  if(mtx)
    ambix_matrix_destroy(mtx);
  sf_close(file);
  free(data);
  return result;
}

static ai_t*ai_cmdline(const char*name, int argc, char**argv) {
  ai_t*ai=calloc(1, sizeof(ai_t));
  uint32_t channels=0;
  uint32_t order=0;
  uint32_t blocksize=0;
  while(argc) {
    if(!strcmp(argv[0], "-h") || !strcmp(argv[0], "--help")) {
      print_usage(name);
      exit(0);
    }
    if(!strcmp(argv[0], "-V") || !strcmp(argv[0], "--version")) {
      print_version(name);
      exit(0);
    }
    if(!strcmp(argv[0], "-o") || !strcmp(argv[0], "--output")) {
      if(argc>1) {
        ai->outfilename=strdup(argv[1]);
        argv+=2;
        argc-=2;
        continue;
      }
      fprintf(stderr, "no output file specified\n");
      return NULL;
    }
    if(!strcmp(argv[0], "-O") || !strcmp(argv[0], "--order")) {
      if(argc>1) {
        order=atoi(argv[1]);
        channels=ambix_order2channels(order);
        argv+=2;
        argc-=2;
        continue;
      }
      fprintf(stderr, "no ambisonics order specified\n");
      return NULL;
    }
    if(!strcmp(argv[0], "-b") || !strcmp(argv[0], "--blocksize")) {
      if(argc>1) {
        blocksize=atoi(argv[1]);
        argv+=2;
        argc-=2;
        continue;
      }
      fprintf(stderr, "no blocksize specified\n");
      return NULL;
    }
    if(!strcmp(argv[0], "-X") || !strcmp(argv[0], "--matrix")) {
      if(argc>1) {
        if(!ai_matrix(ai, argv[1])) {
          fprintf(stderr, "Couldn't read matrix '%s'\n", argv[1]);
          return NULL;
        }
        argv+=2;
        argc-=2;
        continue;
      }
      fprintf(stderr, "no matrix file specified\n");
      return NULL;
    }
    ai->infilenames=argv;
    ai->numIns=argc;
    break;
  }

  if(!ai->infilenames) {
    fprintf(stderr, "no input files specified\n");
    return NULL;
  }

  if(!ai->outfilename) {
    fprintf(stderr, "no output filename specified\n");
    return NULL;
  }

  if(channels>0) {
    ai->channels = channels;
  }
  if(blocksize>0)
    ai->blocksize=blocksize;
  else
    ai->blocksize=DEFAULT_BLOCKSIZE;

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
    ai->info.fileformat=AMBIX_EXTENDED;
    ai->info.ambichannels=ai->matrix->cols;
    ai->info.extrachannels=channels-ai->matrix->cols;
  } else if (ai->channels > 0) {
    if(ai->channels < channels) {
      ai->info.fileformat=AMBIX_EXTENDED;
      ai->info.ambichannels=(ai->channels);
      ai->info.extrachannels=channels-(ai->channels);

      ai->matrix=ambix_matrix_init(ai->channels, ai->channels, NULL);
      ai->matrix=ambix_matrix_fill (ai->matrix, AMBIX_MATRIX_IDENTITY);
    } else if (ai->channels == channels) {
      /* basic format */
      ai->info.fileformat=AMBIX_BASIC;
      ai->info.ambichannels=channels;
      ai->info.extrachannels=0;
    } else {
      return ai_close(ai);
    }
  } else {
    if(!ambix_is_fullset(channels)) {
      return ai_close(ai);
    }
    /* basic format */
    ai->info.fileformat=AMBIX_BASIC;
    ai->info.ambichannels=channels;
    ai->info.extrachannels=0;
  }

  printf("format: %s\n", (ai->info.fileformat==AMBIX_BASIC)?"basic":"extended");
  printf("got %d input channels each %d frames\n", channels, ai->info.frames);
  printf("ambichannels: %d\n", ai->info.ambichannels);
  printf("extrachannels: %d\n", ai->info.extrachannels);
  if(ai->matrix)
    printf("matrix: [%dx%d]\n", ai->matrix->rows, ai->matrix->cols);
  else
    printf("matrix: NONE\n");

  if((ai->info.ambichannels < 1) && (ai->info.extrachannels < 1)) {
    fprintf(stderr, "no output channels defined\n");
    return ai_close(ai);
  }


  return ai;
}

static ai_t*ai_open_output(ai_t*ai) {
  ambix_info_t info;
  if(!ai)return ai;
  memcpy(&info, &ai->info, sizeof(info));
  ai->outhandle=ambix_open(ai->outfilename, AMBIX_WRITE, &info);

  if(!ai) return ai_close(ai);
  if(AMBIX_EXTENDED==ai->info.fileformat) {
    ambix_err_t err=ambix_set_adaptormatrix(ai->outhandle, ai->matrix);
    if(err!=AMBIX_ERR_SUCCESS) {
      fprintf(stderr, "setting adapator matrix [%dx%d]=%d returned %d\n", ai->matrix->rows, ai->matrix->cols, ambix_is_fullset(ai->matrix->rows), err);
      return ai_close(ai);
    }
  }
  return ai;
}


static interleaver(float*dest, const float*source, uint64_t frames, uint32_t channels) {
  uint64_t frame;


  for(frame=0; frame<frames; frame++) {
    uint64_t channel;
    for(channel=0; channel<channels; channel++) {
      //      dest[channel*frames+frame] = *source++;
      *dest++ = source[channel*frames+frame];
    }
  }
}

static ai_t*ai_copy_block(ai_t*ai,
                          float*ambidata,
                          float*extradata,
                          float*interleavedata,
                          uint64_t frames) {
  uint32_t i;
  uint64_t channels=0;

  if(!ai)return ai;
  for(i=0; i<ai->numIns; i++) {
    uint64_t offset=channels*frames;
    SNDFILE*in=ai->inhandles[i];
    if(in) {
      //printf("reading %d frames from[%d] at %p+%d\n", (int)frames, (int)i, interleavedata, (int)offset);
      if(frames!=sf_readf_float(in, interleavedata+offset, frames)) {
        return ai_close(ai);
      }
      channels+=ai->ininfo[i].channels;
    }
  }

  if(ambidata)
    interleaver(ambidata, interleavedata, frames, ai->info.ambichannels);

  if(extradata)
    interleaver(extradata, interleavedata+frames*ai->info.ambichannels, frames, ai->info.extrachannels);

  //printf("writing %d frames to  %p & %p\n", (int)frames,  ambidata, extradata);

  if(frames!=ambix_writef_float32(ai->outhandle, ambidata, extradata, frames)) {
    return ai_close(ai);
  }

  return ai;
}

static ai_t*ai_copy(ai_t*ai) {
  uint64_t blocksize=0, blocks=0;
  uint64_t f, frames=0, channels=0;
  float32_t*ambidata=NULL,*extradata=NULL,*interleavebuffer=NULL;
  if(!ai)return ai;
  blocksize=ai->blocksize;
  if(blocksize<1)
    blocksize=DEFAULT_BLOCKSIZE;
  frames=ai->info.frames;
  channels=(ai->info.ambichannels+ai->info.extrachannels);

  if(ai->info.ambichannels>0)
    ambidata =malloc(sizeof(float32_t)*ai->info.ambichannels *blocksize);
  if(ai->info.extrachannels>0)
  extradata=malloc(sizeof(float32_t)*ai->info.extrachannels*blocksize);

  interleavebuffer=malloc(sizeof(float32_t)*channels*blocksize);

  while(frames>blocksize) {
    blocks++;
    if(!ai_copy_block(ai, ambidata, extradata, interleavebuffer, blocksize)) {
      return ai_close(ai);
    }
    frames-=blocksize;
  }
  if(!ai_copy_block(ai, ambidata, extradata, interleavebuffer, frames))
    return ai_close(ai);


  free(ambidata);
  free(extradata);
  free(interleavebuffer);
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
  ai_t*ai=ai_cmdline(argv[0], argc-1, argv+1);
  if(!ai) {
    print_usage(argv[0]);//"ambix_interleave");
    return 1;
  }

  return ambix_interleave(ai);
}
void print_usage(const char*name) {
  printf("\n");
  printf("Usage: %s -o outfile [options] infile1...\n", name);
  printf("Merges several audio files into an ambix file\n");

  printf("\n");
  printf("Options:\n");
  printf("  -h, --help                       print this help\n");
  printf("  -o, --output                     output filename\n");
  printf("  -O, --order                      force ambisonics order (default: autodetect)\n");
  printf("  -X, --matrix                     specify adaptor matrix file\n");
  printf("  -b, --blocksize                  blocksize for copying (default: %d)\n", DEFAULT_BLOCKSIZE);
  printf("\n");

  printf(
         "\nIn it's simplest form, you pass it a number of input files,"
         "\nthe accumulated channels of which form a full 3d ambisonics set"
         "\n(channel #0 becomes the W-channel,...)."
         "\nThe ambisonics order is calculated from the number of channels using N=(O+1)^2."
         "\n"
         "\nIf you specify the 'order', then all extranous channels will be stored as 'extra' channels."
         "\n"
         "\nYou can also write extended ambix files, by specifying an adaptor matrix"
         "\n(a soundfile where the channels are read as rows and the frames as columns),"
         "\nin which case the columns of the matrix define the number of (reduced) ambisonics channels read from the input"
         "\nand the rows of the matrix must form a full 3d ambisonics set."
         "\nInput channels exceeding the number of matrix columns are stored as 'extra' channels."
         "\n"
         );

  printf("Report bugs to: zmoelnig@iem.at\n\n");
  printf("Home page: http://ambisonics.iem.at/xchange/products/libambix\n", name);
}
void print_version(const char*name) {
  printf("%s %s\n", name, AMBIX_INTERLEAVE_VERSION);
  printf("\n");
  printf("Copyright (C) 2012 Institute of Electronic Music and Acoustics (IEM), University of Music and Dramatic Arts (KUG), Graz, Austria.\n");
  printf("\n");
  printf("License LGPLv2.1: GNU Lesser GPL version 2.1 or later <http://gnu.org/licenses/lgpl.html>\n");
  printf("This is free software: you are free to change and redistribute it.\n");
  printf("There is NO WARRANTY, to the extent permitted by law.\n");
  printf("\n");
  printf("Written by IOhannes m zmoelnig <zmoelnig@iem.at>\n");
}
