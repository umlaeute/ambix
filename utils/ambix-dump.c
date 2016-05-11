/* ambix_dump -  create an ambix file              -*- c -*-

   Copyright © 2012-2014 IOhannes m zmölnig <zmoelnig@iem.at>.
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

/**
 * @brief ambix_dump - merge several (multi-channel) audio files into a single ambix file
 *
 * ambix_dump -o <outfile> [-O <order>] [-X <matrixfile>] <infile1> [<infile2> ...]
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "replacement/strndup.h"

#define MARK() printf("%s:%d[%s]\n", __FILE__, __LINE__, __FUNCTION__)

typedef struct ai_t {
  ambix_t*ambix;
  ambix_info_t info;

  char*filename;

  const ambix_matrix_t*matrix;

  ambix_fileformat_t format;

  int dumpRaw;
  int dumpCooked;
  int dumpXtra;

  uint32_t blocksize;
#define DEFAULT_BLOCKSIZE 1024
#define MAX_FILENAMESIZE 1024
} ai_t;
static void print_usage(const char*path);
static void print_version(const char*path);
static ai_t*ai_close(ai_t*ai);


static ai_t*ai_cmdline(const char*name, int argc, char**argv) {
  ai_t*ai=(ai_t*)calloc(1, sizeof(ai_t));
  uint32_t blocksize=0;
  int dumpRaw=0, dumpCooked=0, dumpXtra=0;
  ambix_fileformat_t format=0;

  while(argc) {
    if(!strcmp(argv[0], "-h") || !strcmp(argv[0], "--help")) {
      print_usage(name);
      exit(0);
    }
    if(!strcmp(argv[0], "-V") || !strcmp(argv[0], "--version")) {
      print_version(name);
      exit(0);
    }
    if(!strcmp(argv[0], "-r") || !strcmp(argv[0], "--raw")) {
      dumpRaw=1;
      argv++; argc--;
      continue;
    }
    if(!strcmp(argv[0], "-a") || !strcmp(argv[0], "--ambisonics")) {
      dumpCooked=1;
      argv++; argc--;
      continue;
    }
    if(!strcmp(argv[0], "-x") || !strcmp(argv[0], "--extra")) {
      dumpXtra=1;
      argv++; argc--;
      continue;
    }
    if(!strcmp(argv[0], "-B") || !strcmp(argv[0], "--basic")) {
      format=AMBIX_BASIC;
      argv++; argc--;
      continue;
    }
    if(!strcmp(argv[0], "-X") || !strcmp(argv[0], "--extended")) {
      format=AMBIX_EXTENDED;
      argv++; argc--;
      continue;
    }
    if(!strcmp(argv[0], "-b") || !strcmp(argv[0], "--blocksize")) {
      if(argc>1) {
        blocksize=atoi(argv[1]);
        argv+=2;
        argc-=2;
        continue;
      }
      return ai_close(ai);
    }
    if(argc) {
      ai->filename=strndup(argv[0], 1024);
    }
    break;
  }

  if(!ai->filename) {
    return ai_close(ai);
  }

  if(blocksize>0)
    ai->blocksize=blocksize;
  else
    ai->blocksize=DEFAULT_BLOCKSIZE;


  if(!dumpRaw && !dumpCooked && !dumpXtra) {
    dumpCooked=1;
  }

  if(format>0)
    ai->format = format;
  else
    ai->format = AMBIX_EXTENDED;

  if(format == AMBIX_BASIC)
    dumpRaw=dumpRaw || dumpCooked;

  ai->dumpRaw=dumpRaw;
  ai->dumpCooked=dumpCooked;
  ai->dumpXtra=dumpXtra;


  return ai;
}

static ai_t*ai_close(ai_t*ai) {
  if(!ai)return NULL;

  //  printf("closing %d outhandles %p\n", ai->numOuts, ai->outhandles);
  if(ai->ambix) {
    ambix_close(ai->ambix);
  }
  free(ai->filename); ai->filename=NULL;

  ai->ambix=NULL;

  free(ai); ai=NULL;
  return ai;
}

static ai_t*ai_open_input(ai_t*ai) {
  if(!ai)return ai;
  if(!ai->ambix) {
    ai->info.fileformat=ai->format;

    ai->ambix=ambix_open(ai->filename, AMBIX_READ, &ai->info);
  }
  if(!ai->ambix) {
    return ai_close(ai);
  }

  if((ai->info.ambichannels < 1) && (ai->info.extrachannels < 1)) {
    return ai_close(ai);
  }

  ai->matrix=ambix_get_adaptormatrix(ai->ambix);

  return ai;
}

static void printf_block(const char*prefix, float*data, uint32_t channels, uint32_t frames) {
  printf("%s:*[%dx%d]@%p\n", prefix, frames, channels, data);

  if(data) {
    uint32_t c, f;
    for(f=0; f<frames; f++) {
      printf("%s[%05d]:", prefix, f);
      for(c=0; c<channels; c++) {
        printf(" %05g", *data);
        data++;
      }
      printf("\n");
    }
  }
}
static ai_t*ai_dump_block(ai_t*ai,
                          float*rawdata,
                          float*cookeddata,
                          float*extradata,
                          float*dumpbuffer,
                          uint64_t frames) {
  uint32_t ambichannels, fullambichannels, extrachannels;

  const ambix_matrix_t*matrix;
  //printf("rawdata=%p\tcookeddata=%p\textradata=%p\n", rawdata, cookeddata, extradata);

  if(!ai)return ai;
  ambichannels=ai->info.ambichannels;
  extrachannels=ai->info.extrachannels;

  matrix=ai->matrix;
  if(matrix) {
    uint32_t rows=matrix->rows;
    uint32_t cols=matrix->cols;

    fullambichannels=rows;
    if(cols!=ambichannels) {
      printf("columns do not match ambichannels %d!=%d\n", cols, ambichannels);
    }
  } else {
    fullambichannels=ambichannels;
  }

  /* read the raw data */
  if(frames!=ambix_readf_float32(ai->ambix,
                                 rawdata,
                                 extradata,
                                 frames)) {
    return ai_close(ai);
  }

  if(ai->dumpRaw) {
    printf_block("RAW", rawdata, ambichannels, frames);
  }

  /* decode the ambisonics data */
  //  printf("reading ambidata %p & %p\n", rawdata, cookeddata);
  if(rawdata && cookeddata && matrix) {
    if(AMBIX_ERR_SUCCESS!=ambix_matrix_multiply_float32(cookeddata, matrix, rawdata, frames)) {
      printf("failed decoding\n");
      return ai_close(ai);
    }
  }
  if(ai->dumpCooked) {
    printf_block("AMB", cookeddata, fullambichannels, frames);
  }

  /* dump the extra data */
  //  printf("reading extradata %p\n", extradata);
  if(ai->dumpXtra) {
    printf_block("XTR", extradata, extrachannels, frames);
  }

  //printf("reading done\n");
  return ai;
}

static ai_t*ai_dodump(ai_t*ai) {
  uint64_t blocksize=0, blocks=0;
  uint64_t frames=0;
  float32_t*rawdata=NULL, *cookeddata=NULL, *extradata=NULL,*dumpbuf=NULL;
  uint64_t size=0;
  if(!ai)return ai;

  blocksize=ai->blocksize;
  if(blocksize<1)
    blocksize=DEFAULT_BLOCKSIZE;
  frames=ai->info.frames;

  if(ai->info.ambichannels) {
    const ambix_matrix_t*matrix=ai->matrix;
    if(!matrix) {
      printf("no adaptor matrix founde...\n");
      //      return ai_close(ai);
    }
    size=(ai->info.ambichannels)*blocksize;
    rawdata=(float32_t*)malloc(sizeof(float32_t)*size);
    if(matrix) {
      cookeddata=(float32_t*)malloc(sizeof(float32_t)*(matrix->rows)*blocksize);
      if((matrix->rows)*blocksize > size)
        size=(matrix->rows)*blocksize;
    }

  }

  if(ai->info.extrachannels) {
    extradata=(float32_t*)malloc(sizeof(float32_t)*(ai->info.extrachannels)*blocksize);
    if((ai->info.extrachannels)*blocksize > size)
      size=(ai->info.extrachannels)*blocksize;
  }
  dumpbuf=(float32_t*)malloc(sizeof(float32_t)*size);

  while(frames>blocksize) {
    blocks++;
    if(!ai_dump_block(ai, rawdata, cookeddata, extradata, dumpbuf, blocksize)) {
      ai=NULL;
      break;
    }
    frames-=blocksize;
  }
  if(ai && !ai_dump_block(ai, rawdata, cookeddata, extradata, dumpbuf, frames)) {
    ai=NULL;
  }

  free(rawdata);
  free(cookeddata);
  free(extradata);
  free(dumpbuf);

  //  printf("reading really done %p\n", ai);
  return ai;
}



static int ambix_dump(ai_t*ai) {
  ai_t*result=ai_open_input(ai);
  //  if(result)printf("success @ %d!\n", __LINE__);
  result=ai_dodump(result);
  //if(result)printf("success @ %d!\n", __LINE__);

  if(result)
    ai_close(result);
  //if(result)printf("success @ %d!\n", __LINE__);
  //  printf("dump done %p\n", result);
  return (result!=NULL);
}




int main(int argc, char**argv) {
  ai_t*ai=ai_cmdline(argv[0], argc-1, argv+1);
  if(!ai) {
    print_usage(argv[0]);//"ambix_dump");
    return 1;
  }

  return ambix_dump(ai);
}
void print_usage(const char*name) {
  printf("\n");
  printf("Usage: %s [options] infile\n", name);
  printf("Print sample values of an ambix file to the stdout\n");

  printf("\n");
  printf("Options:\n");
  printf("  -h, --help                       print this help\n");
  printf("  -V, --version                    print version info\n");
  printf("  -r, --raw                        dump raw ambisonics data\n");
  printf("  -a, --ambisonics                 dump cooked ambisonics data\n");
  printf("  -x, --extra                      dump additional audio data\n");

  printf("  -B, --basic                       open as AMBIX_BASIC\n");
  printf("  -X, --extended                    open as AMBIX_EXTENDED\n");

  printf("\n");

  printf(
         "\nDumps the contents of an ambix file to the stderr in human readable form:"
         "\neach frame is output in a separate line, with the channels separated by space"
         "\n"
         );

  printf("\n");
#ifdef PACKAGE_BUGREPORT
  printf("Report bugs to: %s\n\n", PACKAGE_BUGREPORT);
#endif
#ifdef PACKAGE_URL
  printf("Home page: %s\n", PACKAGE_URL);
#endif
}
void print_version(const char*name) {
#ifdef PACKAGE_VERSION
  printf("%s %s\n", name, PACKAGE_VERSION);
#endif
  printf("\n");
  printf("Copyright (C) 2012 Institute of Electronic Music and Acoustics (IEM), University of Music and Dramatic Arts (KUG), Graz, Austria.\n");
  printf("\n");
  printf("License LGPLv2.1: GNU Lesser GPL version 2.1 or later <http://gnu.org/licenses/lgpl.html>\n");
  printf("This is free software: you are free to change and redistribute it.\n");
  printf("There is NO WARRANTY, to the extent permitted by law.\n");
  printf("\n");
  printf("Written by IOhannes m zmoelnig <zmoelnig@iem.at>\n");
}
