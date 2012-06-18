/* null.c -  dummy backend support              -*- c -*-

   Copyright © 2012 IOhannes m zmölnig <zmoelnig@iem.at>.
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

#import <Foundation/Foundation.h>
#import <AudioToolbox/ExtendedAudioFile.h>

@interface AmbixData
{
@public ExtAudioFileRef file;
}
@end

typedef struct ambixcoreaudio_private_struct {
  NSAutoreleasePool * pool;
  AmbixData*data;
}ambixcoreaudio_private_t;
static inline AmbixData*PRIVATE(ambix_t*ax) { 
 ambixcoreaudio_private_t*priv=(ambixcoreaudio_private_t*)(ax->private_data);
 return priv->data;
}


@implementation AmbixData
@end


static void
ambix2coreaudio_info(const ambix_info_t*axinfo, ExtAudioFileRef cainfo) {
#if 0
  cainfo->frames=axinfo->frames;
  cainfo->samplerate=(int)axinfo->samplerate;
  cainfo->channels=axinfo->ambichannels+axinfo->extrachannels;
  cainfo->format=SF_FORMAT_CAF | ambix2coreaudio_sampleformat(axinfo->sampleformat);
  cainfo->sections=0;
  cainfo->seekable=0;
#endif
}

static void print_coreaudioformat(AudioStreamBasicDescription*format) {
printf("	SampleRate=%f\n", (float)format->mSampleRate);
printf("	FormatID=%ul\n", (unsigned long)format->mFormatID);
printf("	FormatFlags=%ul\n", (unsigned long)format->mFormatFlags);
printf("	BytesPerPacket=%ul\n", (unsigned long)format->mBytesPerPacket);
printf("	FramesPerPacket=%ul\n", (unsigned long)format->mFramesPerPacket);
printf("	BytesPerFrame=%ul\n", (unsigned long)format->mBytesPerFrame);
printf("	ChannelsPerFrame=%ul\n", (unsigned long)format->mChannelsPerFrame);
printf("	BitsPerChannel=%ul\n", (unsigned long)format->mBitsPerChannel);
printf("	Reserved=%ul\n", (unsigned long)format->mReserved);
}

static void
coreaudio2ambix_info(const ExtAudioFileRef cainfo, ambix_info_t*axinfo) {
  AudioStreamBasicDescription format;
  UInt32 datasize=0;
  SInt64 frames;

MARK();
  datasize=sizeof(SInt64);
  if(noErr == ExtAudioFileGetProperty(cainfo, kExtAudioFileProperty_FileLengthFrames, &datasize, &frames)) {
    printf("frames(%d bytes): %d\n", (int)datasize, (int)frames);
    axinfo->frames=(uint64_t)frames;
  }
MARK();

  datasize=sizeof(format);
  memset(&format, 0, sizeof(format));
  if(noErr == ExtAudioFileGetProperty(cainfo, kExtAudioFileProperty_FileDataFormat, &datasize, &format)) {
    axinfo->samplerate = (double)format.mSampleRate;
    axinfo->extrachannels = format.mChannelsPerFrame;
    axinfo->sampleformat = 0;
  }
MARK();
  printf("frames=%d\nsamplerate=%f\nxchannels=%d\nformat=%d\n", (int)axinfo->frames, (float)axinfo->samplerate, (int)axinfo->extrachannels, (int)axinfo->sampleformat);
}


ambix_err_t _ambix_open	(ambix_t*ambix, const char *path, const ambix_filemode_t mode, const ambix_info_t*ambixinfo) {
  ambixcoreaudio_private_t*priv=0;
  ambix->private_data=calloc(1, sizeof(ambixcoreaudio_private_t));
  priv=(ambixcoreaudio_private_t*)ambix->private_data;
  priv->pool = [[NSAutoreleasePool alloc] init];

  OSStatus err = noErr;
  ExtAudioFileRef inputAudioFileRef = NULL;

  NSURL *inURL = [NSURL fileURLWithPath:[NSString stringWithUTF8String:path]];

MARK();
  err = ExtAudioFileOpenURL((CFURLRef)inURL, &inputAudioFileRef);
  if(noErr!=err) {
    _ambix_close(ambix);
    return AMBIX_ERR_INVALID_FILE;
  }
  if(!inputAudioFileRef) {
    _ambix_close(ambix);
    return AMBIX_ERR_INVALID_FILE;
  }
MARK();
//  PRIVATE(ambix)->file=inputAudioFileRef;
MARK();
  //coreaudio2ambix_info(PRIVATE(ambix)->file, &ambix->realinfo);
  coreaudio2ambix_info(inputAudioFileRef, &ambix->realinfo);
MARK();
  






  _ambix_close(ambix);
  return AMBIX_ERR_INVALID_FILE;
}

ambix_err_t	_ambix_close	(ambix_t*ambix) {
  if(ambix&&ambix->private_data) {
    ambix_err_t err=AMBIX_ERR_SUCCESS;
    ambixcoreaudio_private_t*priv=(ambixcoreaudio_private_t*)ambix->private_data;
    /* FIRST close the file itself */
    if(PRIVATE(ambix)) {
      if(PRIVATE(ambix)->file)
        ExtAudioFileDispose(PRIVATE(ambix)->file);


    } else {
      err=AMBIX_ERR_INVALID_FILE;
    }
    priv->data=NULL;
    /* SECOND release the autorelease pool... */
    if(priv->pool) {
      [priv->pool release];
    } else {
      err=AMBIX_ERR_INVALID_FILE;
    }
    priv->pool=NULL;

    /* LAST free the private data */
    free(ambix->private_data);
    ambix->private_data=NULL;
    return err;
  }
  return AMBIX_ERR_INVALID_FILE;
}

int64_t _ambix_readf_int16   (ambix_t*ambix, int16_t*data, int64_t frames) {
  return -1;
}
int64_t _ambix_readf_int32   (ambix_t*ambix, int32_t*data, int64_t frames) {
  return -1;
}
int64_t _ambix_readf_float32   (ambix_t*ambix, float32_t*data, int64_t frames) {
  return -1;
}

int64_t _ambix_writef_int16   (ambix_t*ambix, const int16_t*data, int64_t frames) {
  return -1;
}
int64_t _ambix_writef_int32   (ambix_t*ambix, const int32_t*data, int64_t frames) {
  return -1;
}
int64_t _ambix_writef_float32   (ambix_t*ambix, const float32_t*data, int64_t frames) {
  return -1;
}
ambix_err_t _ambix_write_uuidchunk(ambix_t*ax, const void*data, int64_t datasize) {
  return  AMBIX_ERR_UNKNOWN;
}
int64_t _ambix_seek (ambix_t* ambix, int64_t frames, int whence) {
  return -1;
}


/* no sndfile when using CoreAudio */
struct SNDFILE_tag*_ambix_get_sndfile	(ambix_t*ambix) {
  return 0;
}


