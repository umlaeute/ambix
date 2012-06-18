/* coreaudio.c -  CoreAudio backend support              -*- c -*-

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

@interface AmbixData : NSObject
{
@public AudioFileID file;
@public ExtAudioFileRef xfile;
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
#endif
}

static void print_caformat(AudioStreamBasicDescription*format) {
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

  datasize=sizeof(SInt64);
  if(noErr == ExtAudioFileGetProperty(cainfo, kExtAudioFileProperty_FileLengthFrames, &datasize, &frames)) {
    printf("frames(%d bytes): %d\n", (int)datasize, (int)frames);
    axinfo->frames=(uint64_t)frames;
  }

  datasize=sizeof(format);
  memset(&format, 0, sizeof(format));
  if(noErr == ExtAudioFileGetProperty(cainfo, kExtAudioFileProperty_FileDataFormat, &datasize, &format)) {
    axinfo->samplerate = (double)format.mSampleRate;
    axinfo->extrachannels = format.mChannelsPerFrame;
    axinfo->sampleformat = AMBIX_SAMPLEFORMAT_PCM24; /* FIXXXME: this is only a dummy */
  }
  print_caformat(&format);

  datasize=sizeof(format);
  memset(&format, 0, sizeof(format));
  if(noErr == ExtAudioFileGetProperty(cainfo, 'UUID', &datasize, &format)) {
    printf("UUIDUUIDUUIDUUIDUUID\n");
  } else printf("no uuid\n");
}


ambix_err_t _ambix_open	(ambix_t*ambix, const char *path, const ambix_filemode_t mode, const ambix_info_t*ambixinfo) {
  OSStatus err = noErr;
  ExtAudioFileRef inputAudioFileRef = NULL;
  ambixcoreaudio_private_t*priv=0;
  ambix->private_data=calloc(1, sizeof(ambixcoreaudio_private_t));
  priv=(ambixcoreaudio_private_t*)ambix->private_data;
  priv->pool = [[NSAutoreleasePool alloc] init];

  priv->data = [[AmbixData alloc] init];

  PRIVATE(ambix)->file=NULL;
  PRIVATE(ambix)->xfile=NULL;


  NSURL *inURL = [NSURL fileURLWithPath:[NSString stringWithUTF8String:path]];

MARK();
//  err = ExtAudioFileOpenURL((CFURLRef)inURL, &inputAudioFileRef);
MARK();
  err = AudioFileOpenURL((CFURLRef)inURL, 
			0x01, // kAudioFileReadPermission,
			0,
			&(PRIVATE(ambix)->file));
  if(noErr!=err) {
    _ambix_close(ambix);
    return AMBIX_ERR_INVALID_FILE;
  }
  MARK();

  err = ExtAudioFileWrapAudioFileID (
			(PRIVATE(ambix)->file),
			false,
			&(PRIVATE(ambix)->xfile));
  if(noErr!=err) {
    _ambix_close(ambix);
    return AMBIX_ERR_INVALID_FILE;
  }
  MARK();

//  if(!inputAudioFileRef) {
  if(NULL==(PRIVATE(ambix)->xfile)) {
    _ambix_close(ambix);
    return AMBIX_ERR_INVALID_FILE;
  }
MARK();
  coreaudio2ambix_info(PRIVATE(ambix)->xfile, &ambix->realinfo);
  _ambix_print_info(&ambix->realinfo);
MARK();

  ambix->byteswap = 0; /* FIXXXME: assuming wrong defaults */
  ambix->channels = ambix->realinfo.extrachannels; /* FIXXXME: realinfo is a bad vehicle */
  ambix->format = AMBIX_BASIC; /* FIXXXME: assuming wrong defaults */





  _ambix_close(ambix);
  return AMBIX_ERR_INVALID_FILE;
}

ambix_err_t	_ambix_close	(ambix_t*ambix) {
  if(ambix&&ambix->private_data) {
    ambix_err_t err=AMBIX_ERR_SUCCESS;
    ambixcoreaudio_private_t*priv=(ambixcoreaudio_private_t*)ambix->private_data;
    /* FIRST close the file itself */
    if(PRIVATE(ambix)) {
MARK();
      if(PRIVATE(ambix)->xfile)
        ExtAudioFileDispose(PRIVATE(ambix)->xfile);
      PRIVATE(ambix)->xfile=NULL;

MARK();
      if(PRIVATE(ambix)->file)
        AudioFileClose(PRIVATE(ambix)->file);
      PRIVATE(ambix)->file=NULL;

MARK();
        [priv->data release];
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


