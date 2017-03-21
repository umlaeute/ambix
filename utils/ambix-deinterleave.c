/* ambix_deinterleave -  create an ambix file              -*- c -*-

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
 * @brief ambix_deinterleave - merge several (multi-channel) audio files into a single ambix file
 *
 * ambix_deinterleave -o <outfile> [-O <order>] [-X <matrixfile>] <infile1> [<infile2> ...]
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

#ifdef _MSC_VER
# define strdup _strdup
# define snprintf _snprintf
#endif

#define MARK() printf("%s:%d[%s]\n", __FILE__, __LINE__, __FUNCTION__)

typedef struct ai_t {
  ambix_info_t info;

  char*infilename;
  ambix_t*inhandle;

  char*prefix,*suffix;

  SNDFILE**outhandles;
  SF_INFO *outinfo;
  uint32_t numOuts;

  ambix_matrix_t matrix;

  uint32_t blocksize;
  int format;
#define DEFAULT_BLOCKSIZE 1024
#define MAX_FILENAMESIZE 1024
} ai_t;
static void print_usage(const char*path);
static void print_version(const char*path);
static ai_t*ai_close(ai_t*ai);

#define DEFAULT_SUFFIX ".wav"

static char*ai_prefix(const char*filename) {
  char*result=NULL;
  const char*last=strrchr(filename, '.');
  if(last) {
    int length=last-filename;
    result=(char*)calloc(sizeof(char), strlen(filename));
    strncpy(result, filename, length);
    result[length]='-';
    result[length+1]='\0';
    return result;
  }

  result=strdup("outfile-");
  return result;
}
static char*ai_suffix(const char*suffix, int format) {
  unsigned int length = strnlen(suffix, 1024)+6;
  char*tmp=calloc(length, sizeof(char));
  char*result=NULL;
  switch (format) {
  case SF_FORMAT_WAV:
    snprintf(tmp, length-1, "%s.wav", suffix);
    break;
  case SF_FORMAT_CAF:
    snprintf(tmp, length-1, "%s.caf", suffix);
    break;
  default:
    snprintf(tmp, length-1, "%s", suffix);
  }
  result = strdup(tmp);
  free(tmp);
  return result;
}


static ai_t*ai_cmdline(const char*name, int argc, char**argv) {
  ai_t*ai=(ai_t*)calloc(1, sizeof(ai_t));
  uint32_t blocksize=0;
  const char*suffix="";
  const char*prefix=NULL;
  ai->format=SF_FORMAT_WAV;

  while(argc) {
    if(!strcmp(argv[0], "-h") || !strcmp(argv[0], "--help")) {
      print_usage(name);
      exit(0);
    }
    if(!strcmp(argv[0], "-V") || !strcmp(argv[0], "--version")) {
      print_version(name);
      exit(0);
    }
    if(!strcmp(argv[0], "-p") || !strcmp(argv[0], "--prefix")) {
      if(argc>1) {
        //printf("prefix: '%s'\n", argv[1]);
        prefix=argv[1];
        argv+=2;
        argc-=2;
        continue;
      }
      return ai_close(ai);
    }
    if(!strcmp(argv[0], "-s") || !strcmp(argv[0], "--suffix")) {
      if(argc>1) {
        suffix = argv[1];
        argv+=2;
        argc-=2;
        continue;
      }
      return ai_close(ai);
    }
    if(!strcmp(argv[0], "-f") || !strcmp(argv[0], "--format")) {
      if(argc<2)
        return ai_close(ai);
      if(!strncmp(argv[1], "caf", 3) || !strncmp(argv[1], "CAF", 3))
        ai->format=SF_FORMAT_CAF;
      else if(!strncmp(argv[1], "wav", 3) || !strncmp(argv[1], "WAV", 3))
        ai->format=SF_FORMAT_WAV;
      else {
        fprintf(stderr, "unknown format '%s'\n", argv[1]);
        return ai_close(ai);
      }
      argv+=2;
      argc-=2;
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
      ai->infilename=strdup(argv[0]);
    }
    break;
  }


  if(!ai->infilename)
      return ai_close(ai);

  if(prefix)
    ai->prefix=strdup(prefix);
  else
    ai->prefix=ai_prefix(ai->infilename);
  ai->suffix=ai_suffix(suffix, ai->format);

  if(blocksize>0)
    ai->blocksize=blocksize;
  else
    ai->blocksize=DEFAULT_BLOCKSIZE;

  return ai;
}

static ai_t*ai_close(ai_t*ai) {
  uint32_t i;
  if(!ai)return NULL;

  //  printf("closing %d outhandles %p\n", ai->numOuts, ai->outhandles);
  if(ai->outhandles) {
    for(i=0; i<ai->numOuts; i++) {
      SNDFILE*outhandle=ai->outhandles[i];
      if(outhandle) {
        int err=sf_close(outhandle);
        if(err!=0) {
          ;
          //    printf("closing outhandle[%d] returned %d\n", i, err);
        }
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


  if(ai->infilename)
    free(ai->infilename);
  ai->infilename=NULL;

  ambix_matrix_deinit (&ai->matrix);

  free(ai);

  return NULL;
}

static ai_t*ai_open_input(ai_t*ai) {
  const ambix_matrix_t*matrix=NULL;
  if(!ai)return ai;
  if(!ai->inhandle) {
    ai->info.fileformat=AMBIX_EXTENDED;

    ai->inhandle=ambix_open(ai->infilename, AMBIX_READ, &ai->info);
  }
  if(!ai->inhandle) {
    return ai_close(ai);
  }

  if((ai->info.ambichannels < 1) && (ai->info.extrachannels < 1)) {
    return ai_close(ai);
  }

  if(ai->info.ambichannels>0) {
    matrix=ambix_get_adaptormatrix(ai->inhandle);
    if(!matrix) {
      return ai_close(ai);
    }
    ambix_matrix_copy(matrix, &ai->matrix);
  } else {
    ambix_matrix_deinit(&ai->matrix);
  }

  return ai;
}

static ai_t*ai_open_output(ai_t*ai) {
  SF_INFO info;
  int format=0;
  int32_t chan, channel, ambichannels, extrachannels;
  if(!ai)return ai;

  ambichannels=ai->matrix.rows;
  extrachannels=ai->info.extrachannels;

  if(!ai->outhandles) {
    ai->numOuts=ambichannels+extrachannels;
    ai->outhandles=(SNDFILE**)calloc(ai->numOuts, sizeof(SNDFILE*));
    ai->outinfo   =(SF_INFO*)calloc(ai->numOuts, sizeof(SF_INFO));
  }
  if(!ai->outhandles) {
    return ai_close(ai);
  }
  if(!ai->outinfo) {
    return ai_close(ai);
  }
  switch(ai->info.sampleformat) {
  case(AMBIX_SAMPLEFORMAT_PCM16)  : format |= SF_FORMAT_PCM_16; break;
  case(AMBIX_SAMPLEFORMAT_PCM24)  : format |= SF_FORMAT_PCM_24; break;
  case(AMBIX_SAMPLEFORMAT_PCM32)  : format |= SF_FORMAT_PCM_32; break;
  default:
  case(AMBIX_SAMPLEFORMAT_FLOAT32): format |= SF_FORMAT_FLOAT ; break;
  }

  info.format = format | ai->format;
  info.frames = ai->info.frames;

  info.samplerate = (int)(ai->info.samplerate);
  info.channels = 1;

  if(!sf_format_check(&info)) {
    printf("output format invalid!\n");
    printf(" format = 0x%x\n", info.format);
    printf(" rate = %d\n", info.samplerate);
    printf(" channels = %d\n", info.channels);
  }

  channel=0;
  //  printf("creating outfiles for %d/%d\n", ambichannels, extrachannels);
  for(chan=0; chan<ambichannels; chan++) {
    char filename[MAX_FILENAMESIZE];
    snprintf(filename, MAX_FILENAMESIZE, "%sambi%03d%s", ai->prefix, chan, ai->suffix);
    filename[MAX_FILENAMESIZE-1]=0;
    //    printf("ambifile%d=%s\n", chan, filename);

    memcpy(&ai->outinfo[channel], &info, sizeof(info));

    ai->outhandles[channel]=sf_open(filename, SFM_WRITE, &ai->outinfo[channel]);
    //    printf("created outhandle[%d] %p\n", channel, ai->outhandles[channel]);
    if(!ai->outhandles[channel]) {
      return ai_close(ai);
    }

    channel++;
  }

  for(chan=0; chan<extrachannels; chan++) {
    char filename[MAX_FILENAMESIZE];
    snprintf(filename, MAX_FILENAMESIZE, "%sextra%03d%s", ai->prefix, chan, ai->suffix);
    filename[MAX_FILENAMESIZE-1]=0;
    //printf("extrafile%d=%s\n", chan, filename);
    memcpy(&ai->outinfo[channel], &info, sizeof(info));

    ai->outhandles[channel]=sf_open(filename, SFM_WRITE, &ai->outinfo[channel]);

    if(!ai->outhandles[channel]) {
      return ai_close(ai);
    }

    channel++;
  }


  return ai;
}


static void
deinterleaver(float*dest, const float*source, uint64_t frames, uint32_t channels) {
  uint64_t frame;
  for(frame=0; frame<frames; frame++) {
    uint64_t channel;
    for(channel=0; channel<channels; channel++) {
      dest[channel*frames+frame] = *source++;
    }
  }
}

static ai_t*ai_copy_block(ai_t*ai,
                          float*rawdata,
                          float*cookeddata,
                          float*extradata,
                          float*deinterleavebuffer,
                          uint64_t frames) {
  uint32_t ambichannels, fullambichannels, extrachannels;
  uint64_t channel, c;
  sf_count_t framed;

  const ambix_matrix_t*matrix;
  uint32_t rows, cols;

  //printf("rawdata=%p\tcookeddata=%p\textradata=%p\n", rawdata, cookeddata, extradata);

  if(!ai)return ai;
  matrix=&ai->matrix;
  rows=matrix->rows;
  cols=matrix->cols;

  ambichannels=ai->info.ambichannels;
  extrachannels=ai->info.extrachannels;
  fullambichannels=rows;
  if(cols!=ambichannels) {
    printf("columns do not match ambichannels %d!=%d\n", cols, ambichannels);
  }

  /* read the raw data */
  framed=ambix_readf_float32(ai->inhandle,
                             rawdata,
                             extradata,
                             frames);

  if(frames!=framed) {
    printf("failed reading %d frames (got %d)\n", (int)frames, (int)framed);
    return ai_close(ai);
  }

  /* decode the ambisonics data */
  //  printf("reading ambidata %p & %p\n", rawdata, cookeddata);
  channel=0;

  if(rawdata && cookeddata) {

    if(AMBIX_ERR_SUCCESS!=ambix_matrix_multiply_float32(cookeddata, matrix, rawdata, frames)) {
      printf("failed decoding\n");
      return ai_close(ai);
    }

    /* deinterleave the buffer */
    deinterleaver(deinterleavebuffer, cookeddata, frames, fullambichannels);

    /* store the ambisonics data */
    for(c=0; c<fullambichannels; c++) {
      framed=sf_writef_float(ai->outhandles[channel], deinterleavebuffer+c*frames, frames);
      if(frames!=framed) {
        printf("failed writing %d ambiframes to %d (got %d)\n", (int)frames, (int)channel, (int)framed);
        return ai_close(ai);
      }
      channel++;
    }
  }
  /* store the extra data */
  //  printf("reading extradata %p\n", extradata);
  if(extradata) {
    deinterleaver(deinterleavebuffer, extradata, frames, extrachannels);
    for(c=0; c<extrachannels; c++) {
      framed=sf_writef_float(ai->outhandles[channel], deinterleavebuffer+c*frames, frames);
      if(frames!=framed) {
        printf("failed writing %d extraframes to %d (got %d)\n", (int)frames, (int)channel, (int)framed);
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
  uint64_t frames=0;
  float32_t*rawdata=NULL, *cookeddata=NULL, *extradata=NULL,*deinterleavebuf=NULL;
  uint64_t size=0;
  if(!ai)return ai;
  blocksize=ai->blocksize;
  if(blocksize<1)
    blocksize=DEFAULT_BLOCKSIZE;
  frames=ai->info.frames;

  if(ai->info.ambichannels) {
    const ambix_matrix_t*matrix=&ai->matrix;
    if(!matrix) {
      printf("no adaptor matrix found...\n");
      return ai_close(ai);
    }
    size=(ai->info.ambichannels)*blocksize;
    rawdata=(float32_t*)malloc(sizeof(float32_t)*size);
    cookeddata=(float32_t*)malloc(sizeof(float32_t)*(matrix->rows)*blocksize);
    if((matrix->rows)*blocksize > size)
      size=(matrix->rows)*blocksize;

  }
  if(ai->info.extrachannels) {
    extradata=(float32_t*)malloc(sizeof(float32_t)*(ai->info.extrachannels)*blocksize);
    if((ai->info.extrachannels)*blocksize > size)
      size=(ai->info.extrachannels)*blocksize;
  }
  deinterleavebuf=(float32_t*)malloc(sizeof(float32_t)*size);
  if(NULL==deinterleavebuf) {
    ai=ai_close(ai);
    goto done;
  }

  while(frames>blocksize) {
    blocks++;
    ai = ai_copy_block(ai, rawdata, cookeddata, extradata, deinterleavebuf, blocksize);
    if(!ai) {
      goto done;
    }
    frames-=blocksize;
  }

  ai = ai_copy_block(ai, rawdata, cookeddata, extradata, deinterleavebuf, frames);

 done:
  free(rawdata);
  free(cookeddata);
  free(extradata);
  free(deinterleavebuf);

  //  printf("reading really done %p\n", ai);
  return ai;
}



static int ambix_deinterleave(ai_t*ai) {
  ai_t*result=ai_open_input(ai);
  //  if(result)printf("success @ %d!\n", __LINE__);
  result=ai_open_output(result);
  //if(result)printf("success @ %d!\n", __LINE__);
  result=ai_copy(result);
  //if(result)printf("success @ %d!\n", __LINE__);

  if(result) {
    printf("Deinterleaving '%s' to %d files (%s*%s)\n", ai->infilename, ai->numOuts, ai->prefix, ai->suffix);
    ai_close(result);
  }

  //if(result)printf("success @ %d!\n", __LINE__);
  //  printf("deinterleave done %p\n", result);
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
  fprintf(stderr, "\n");
  fprintf(stderr, "Usage: %s [options] infile\n", name);
  fprintf(stderr, "Split an ambix file into several mono files\n");

  fprintf(stderr, "\n");
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "  -h, --help                       print this help\n");
  fprintf(stderr, "  -v, --version                    print version info\n");
  fprintf(stderr, "  -p, --prefix <PREFIX>            output prefix (default: '<infilename>-')\n");
  fprintf(stderr, "  -s, --suffix <SUFFIX>            output suffix (default: '')\n");
  fprintf(stderr, "  -f, --format <FMT>               output format (WAV, CAFF)\n");
  fprintf(stderr, "\n");

  fprintf(stderr,
          "\nThis splits an ambix file into several mono files,"
          "\nnaming them according to prefix, suffix and format:"
          "\nIf infile is 'FieldRecording.caf', this will extract audio data into"
          "\n  - 'FieldRecording-ambi000.wav'"
          "\n  - 'FieldRecording-ambi001.wav'"
          "\n  - ..."
          "\n  - 'FieldRecording-extra000.wav'"
          "\n  - ..."
          "\n"
         );

  fprintf(stderr, "\n");
#ifdef PACKAGE_URL
  fprintf(stderr, "Home page: %s\n", PACKAGE_URL);
#endif
#ifdef PACKAGE_BUGREPORT
  fprintf(stderr, "Report bugs at: %s\n", PACKAGE_BUGREPORT);
#endif
}
void print_version(const char*name) {
#ifdef PACKAGE_VERSION
  fprintf(stderr, "%s %s\n", name, PACKAGE_VERSION);
#endif
  fprintf(stderr, "\n");
  fprintf(stderr, "Copyright (C) 2012-2017 Institute of Electronic Music and Acoustics (IEM),\n"
          "                        University of Music and Dramatic Arts (KUG),\n"
          "                        Graz, Austria.\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "License LGPLv2.1: GNU Lesser GPL version 2.1 or later\n"
                  "                  <http://gnu.org/licenses/lgpl.html>\n");
  fprintf(stderr, "This is free software: you are free to change and redistribute it.\n");
  fprintf(stderr, "There is NO WARRANTY, to the extent permitted by law.\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Written by IOhannes m zmoelnig <zmoelnig@iem.at>\n");
}
