/* ambix_deinterleave -  create an ambix file              -*- c -*-

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
 * @brief ambix_deinterleave - merge several (multi-channel) audio files into a single ambix file
 *
 * ambix_deinterleave -o <outfile> [-O <order>] [-X <matrixfile>] <infile1> [<infile2> ...]
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include "ambix/ambix.h"
#include "sndfile.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define AMBIX_DEINTERLEAVE_VERSION "0.1"

#define MARK() printf("%s:%d[%s]\n", __FILE__, __LINE__, __FUNCTION__)

typedef struct ai_t {
  ambixinfo_t info;

  char*infilename;
  ambix_t*inhandle;

  char*prefix,*suffix;

  SNDFILE**outhandles;
  SF_INFO *outinfo;
  uint32_t numOuts;

  ambixmatrix_t matrix;

  uint32_t blocksize;
#define DEFAULT_BLOCKSIZE 1024
#define MAX_FILENAMESIZE 1024
} ai_t;
static void print_usage(const char*path);
static void print_version(const char*path);


#define DEFAULT_SUFFIX ".wav"

static char*ai_prefix(const char*filename) {
#if 0
  char*result=calloc(1, strlen(filename)+1);
  strcat(result, filename);
  strcat(result, "-");
#else
  char*last=rindex(filename, '.');
  if(last) {
    int length=last-filename;
    char*result=calloc(1, strlen(filename));
    strncpy(result, filename, length);
    result[length]='-';
    result[length+1]='\0';
    return result;
  }

  char*result=strdup("outfile-");
#endif
  return result;
}

static ai_t*ai_cmdline(const char*name, int argc, char**argv) {
  ai_t*ai=calloc(1, sizeof(ai_t));
  uint32_t blocksize=0;
  while(argc) {
    if(!strcmp(argv[0], "-h") || !strcmp(argv[0], "--help")) {
      print_usage(name);
      exit(0);
    }
    if(!strcmp(argv[0], "-v") || !strcmp(argv[0], "--version")) {
      print_version(name);
      exit(0);
    }
    if(!strcmp(argv[0], "-p") || !strcmp(argv[0], "--prefix")) {
      if(argc>1) {
        ai->prefix=strdup(argv[1]);
        argv+=2;
        argc-=2;
        continue;
      }
      return NULL;
    }
    if(!strcmp(argv[0], "-s") || !strcmp(argv[0], "--suffix")) {
      if(argc>1) {
        ai->suffix=strdup(argv[1]);
        argv+=2;
        argc-=2;
        continue;
      }
      return NULL;
    }
    if(!strcmp(argv[0], "-b") || !strcmp(argv[0], "--blocksize")) {
      if(argc>1) {
        blocksize=atoi(argv[1]);
        argv+=2;
        argc-=2;
        continue;
      }
      return NULL;
    }
    if(argc) {
      ai->infilename=strdup(argv[0]);
    }
    break;
  }

  if(!ai->infilename)
    return NULL;

  if(!ai->prefix)
    ai->prefix=ai_prefix(ai->infilename);

  if(!ai->suffix)
    ai->suffix=strdup(DEFAULT_SUFFIX);

  if(blocksize>0)
    ai->blocksize=blocksize;
  else
    ai->blocksize=DEFAULT_BLOCKSIZE;

  return ai;
}

static ai_t*ai_close(ai_t*ai) {
  uint32_t i;
  if(!ai)return NULL;

  printf("closing %d outhandles %p\n", ai->numOuts, ai->outhandles);
  if(ai->outhandles) {
    for(i=0; i<ai->numOuts; i++) {
      SNDFILE*outhandle=ai->outhandles[i];
      if(outhandle) {
        int err=sf_close(outhandle);
        printf("closing outhandle[%d] returned %d\n", i, err);
      }
      ai->outhandles[i]=NULL;
    }
    free(ai->outhandles);
  }
  ai->outhandles=NULL;
  if(ai->outinfo)
    free(ai->outinfo);
  ai->outinfo=NULL;

  if(ai->inhandle) {
    ambix_close(ai->inhandle);
  }
  ai->inhandle=NULL;

  if(ai->prefix)
    free(ai->prefix);
  ai->prefix=NULL;

  if(ai->suffix)
    free(ai->suffix);
  ai->suffix=NULL;

  return NULL;
}

static ai_t*ai_open_input(ai_t*ai) {
  uint32_t i;
  uint32_t channels=0;
  const ambixmatrix_t*matrix=NULL;
  if(!ai)return ai;
  if(!ai->inhandle) {
    ai->info.ambixfileformat=AMBIX_EXTENDED;

    ai->inhandle=ambix_open(ai->infilename, AMBIX_READ, &ai->info);
  }
  if(!ai->inhandle) {
    return ai_close(ai);
  }

  if((ai->info.ambichannels < 1) && (ai->info.otherchannels < 1)) {
    return ai_close(ai);
  }

  matrix=ambix_getAdaptorMatrix(ai->inhandle);
  if(!matrix) {
    return ai_close(ai);
  }
  ambix_matrix_copy(matrix, &ai->matrix);

  return ai;
}

static ai_t*ai_open_output(ai_t*ai) {
  SF_INFO info, tmpinfo;
  int format=0;
  int32_t chan, channel, ambichannels, otherchannels;
  if(!ai)return ai;

  ambichannels=ai->matrix.rows;
  otherchannels=ai->info.otherchannels;

  if(!ai->outhandles) {
    ai->numOuts=ambichannels+otherchannels;
    ai->outhandles=calloc(ai->numOuts, sizeof(SNDFILE*));
    ai->outinfo   =calloc(ai->numOuts, sizeof(SF_INFO));
  }
  switch(ai->info.sampleformat) {
  case(AMBIX_SAMPLEFORMAT_PCM16)  : format |= SF_FORMAT_PCM_16; break;
  case(AMBIX_SAMPLEFORMAT_PCM24)  : format |= SF_FORMAT_PCM_24; break;
  case(AMBIX_SAMPLEFORMAT_PCM32)  : format |= SF_FORMAT_PCM_32; break;
  default:
  case(AMBIX_SAMPLEFORMAT_FLOAT32): format |= SF_FORMAT_FLOAT ; break;
  }

  info.format = format | SF_FORMAT_WAV;
  info.frames = ai->info.frames;

  info.samplerate = ai->info.samplerate;
  info.channels = 1;

  if(!sf_format_check(&info)) {
    printf("output format invalid!\n");
    printf(" format = 0x%x\n", info.format);
    printf(" rate = %d\n", info.samplerate);
    printf(" channels = %d\n", info.channels);
  }

  channel=0;
  printf("creating outfiles for %d/%d\n", ambichannels, otherchannels);
  for(chan=0; chan<ambichannels; chan++) {
    char filename[MAX_FILENAMESIZE];
    snprintf(filename, MAX_FILENAMESIZE, "%sambi%03d%s", ai->prefix, chan, ai->suffix);
    filename[MAX_FILENAMESIZE-1]=0;
    printf("ambifile%d=%s\n", chan, filename);

    memcpy(&ai->outinfo[channel], &info, sizeof(info));

    ai->outhandles[channel]=sf_open(filename, SFM_WRITE, &ai->outinfo[channel]);
    printf("created outhandle[%d] %p\n", channel, ai->outhandles[channel]);
    if(!ai->outhandles[channel]) {
      return ai_close(ai);
    }

    channel++;
  }

  for(chan=0; chan<otherchannels; chan++) {
    char filename[MAX_FILENAMESIZE];
    snprintf(filename, MAX_FILENAMESIZE, "%extra%03d%s", ai->prefix, chan, ai->suffix);
    filename[MAX_FILENAMESIZE-1]=0;
    printf("extrafile%d=%s\n", chan, filename);
    memcpy(&ai->outinfo[channel], &info, sizeof(info));

    ai->outhandles[channel]=sf_open(filename, SFM_WRITE, &ai->outinfo[channel]);

    if(!ai->outhandles) {
      return ai_close(ai);
    }

    channel++;
  }


  return ai;
}

static ai_t*ai_copy_block(ai_t*ai, 
                          float*rawdata,
                          float*cookeddata,
                          float*extradata,
                          uint64_t frames) {
  uint32_t i;
  uint32_t ambichannels, fullambichannels, otherchannels;
  uint64_t channel, f, c, channels=0;

  const ambixmatrix_t*matrix;
  uint32_t rows, cols;
  float32_t**mtx;
  float*source, *dest;

  //printf("rawdata=%p\tcookeddata=%p\textradata=%p\n", rawdata, cookeddata, extradata);

  if(!ai)return ai;
  matrix=&ai->matrix;
  rows=matrix->rows;
  cols=matrix->cols;
  mtx =matrix->data;

  ambichannels=ai->info.ambichannels;
  otherchannels=ai->info.otherchannels;
  fullambichannels=rows;
  if(cols!=ambichannels) {
    printf("columns do not match ambichannels %d!=%d\n", cols, ambichannels);
  }

  /* read the raw data */
  if(frames!=ambix_readf_float32(ai->inhandle, 
                                 rawdata,
                                 extradata,
                                 frames)) {
    return ai_close(ai);
  }

  /* decode the ambisonics data */
  if(rawdata && cookeddata) {
    source=rawdata;
    dest  =cookeddata;
    //printf("rawdata=%p\tcookeddata=%p\n", source, dest);
    //printf("reading %d frames with [%dx%d]\n", (int)frames, (int)rows, (int)cols);
    for(f=0; f<frames; f++) {
      uint32_t chan;
      source=rawdata;
      for(chan=0; chan<rows; chan++) {
        float32_t*src=source;
        float32_t sum=0.;
        uint32_t c;
        for(c=0; c<cols; c++) {
          float32_t s;
          //printf("accessing matrix at [%dx%d] for frame %d\n", chan, c, f);
          s=mtx[chan][c];
          //MARK();
          // printf("accessing source %p (%p/%p) at %d\n", src, source, rawdata, c);
          sum+=s * src[c];
        }
        *dest++=sum;
        source+=cols;
      }
    }
    /* store the ambisonics data */
    channel=0;
    for(c=0; c<fullambichannels; c++) {
      if(frames!=sf_writef_float(ai->outhandles[channel], cookeddata+c*frames, frames)) {
        printf("failed writing %d ambiframes to %d:\n", frames, channel);
        printf("  handle=%p, data=%p, frames=%d\n", ai->outhandles[channel], cookeddata+c*frames, frames);
        return ai_close(ai);
      }
      channel++;
    }
  }
  /* store the extra data */
  //printf("reading extradata %p\n", extradata);
  if(extradata) {
    for(c=0; c<otherchannels; c++) {
      if(frames!=sf_writef_float(ai->outhandles[channel], extradata+c*frames, frames)) {
        printf("failed writing %d extraframes to %d", frames, channel);
        return ai_close(ai);
      }
      channel++;
    }
  }
  //printf("reading done\n");
  return ai;
}

static ai_t*ai_copy(ai_t*ai) {
  uint64_t blocksize=0, blocks=0;
  uint64_t f, frames=0, channels=0;
  float32_t*rawdata=NULL, *cookeddata=NULL, *extradata=NULL;
  if(!ai)return ai;
  blocksize=ai->blocksize;
  if(blocksize<1)
    blocksize=DEFAULT_BLOCKSIZE;
  frames=ai->info.frames;
  channels=(ai->info.ambichannels+ai->info.otherchannels);

  if(ai->info.ambichannels) {
    const ambixmatrix_t*matrix=&ai->matrix;
    if(!matrix) {
      printf("no adaptor matrix founde...\n");
      return ai_close(ai);
    }
    rawdata=malloc(sizeof(float32_t)*(ai->info.ambichannels)*blocksize);
    printf("allocating rawdata for %dx%d samples -> %p\n", (int)(ai->info.ambichannels), (int)blocksize, rawdata);
    cookeddata=malloc(sizeof(float32_t)*(matrix->rows)*blocksize);
    printf("allocating cokdata for %dx%d samples -> %p\n", (int)(matrix->rows), (int)blocksize, cookeddata);
  }
  if(ai->info.otherchannels) {
    extradata=malloc(sizeof(float32_t)*(ai->info.otherchannels)*blocksize);
  }

  while(frames>blocksize) {
    blocks++;
    if(!ai_copy_block(ai, rawdata, cookeddata, extradata, blocksize)) {
      return ai_close(ai);
    }
    frames-=blocksize;
  }
  if(!ai_copy_block(ai, rawdata, cookeddata, extradata, frames)) {
    return ai_close(ai);
  }
  printf("reading really done %p\n", ai);
  return ai;
}



static int ambix_deinterleave(ai_t*ai) {
  ai_t*result=ai_open_input(ai);
  //  if(result)printf("success @ %d!\n", __LINE__);
  result=ai_open_output(result);
  //if(result)printf("success @ %d!\n", __LINE__);
  result=ai_copy(result);
  //if(result)printf("success @ %d!\n", __LINE__);
  ai_close(ai);
  //if(result)printf("success @ %d!\n", __LINE__);
  printf("deinterleave done %p\n", result);
  return (result!=NULL);
}




int main(int argc, char**argv) {
  ai_t*ai=ai_cmdline(argv[0], argc-1, argv+1);
  if(!ai) {
    print_usage(argv[0]);//"ambix_deinterleave");
    return 1;
  }

  return ambix_deinterleave(ai);
}
void print_usage(const char*name) {
  printf("\n");
  printf("Usage: %s [options] infile\n", name);
  printf("Split an ambix file into several mono files\n");

  printf("\n");
  printf("Options:\n");
  printf("  -h, --help                       print this help\n");
  printf("  -v, --version                    print version info\n");
  printf("  -p, --prefix                     output prefix\n");
  printf("\n");

  printf(
         "\nThis splits an ambix file into several mono files, naming them according to type:"
         "\nIf infile is 'FieldRecording.caf', this will extract audio data into"
         "\n'FieldRecording-ambi000.caf', 'FieldRecording-ambi001.caf', ..., 'FieldRecording-extra000.caf', ..."
         "\n"
         );

  printf("Report bugs to: zmoelnig@iem.at\n\n");
  printf("Home page: http://ambisonics.iem.at/xchange/products/libambix\n", name);
}
void print_version(const char*name) {
  printf("%s %s\n", name, AMBIX_DEINTERLEAVE_VERSION);
  printf("\n");
  printf("Copyright (C) 2012 Institute of Electronic Music and Acoustics (IEM), University of Music and Dramatic Arts (KUG), Graz, Austria.\n");
  printf("\n");
  printf("License LGPLv2.1: GNU Lesser GPL version 2.1 or later <http://gnu.org/licenses/lgpl.html>\n");
  printf("This is free software: you are free to change and redistribute it.\n");
  printf("There is NO WARRANTY, to the extent permitted by law.\n");
  printf("\n");
  printf("Written by IOhannes m zmoelnig <zmoelnig@iem.at>\n");
}