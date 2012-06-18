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

static int _coreaudio_isCAF(const AudioFileID*file) {
  /* trying to read format (is it caf?) */
  UInt32 myformat=0;
  UInt32 formatSize=sizeof(myformat);

  OSStatus err=AudioFileGetProperty (
   *file,
   kAudioFilePropertyFileFormat,
   &formatSize,
   &myformat
  );
  return (kAudioFileCAFType==myformat);
}

static int
read_uuidchunk(ambix_t*ax) {
  /* read UUID */
  UInt32 index=0; 
  OSStatus err = noErr;
  char*data=NULL;

  for(index=0; ; index++) {
    UInt32 size=0;
    UInt32 datasize=0;
    uint32_t chunkver=0;

    if(data) free(data);
    data=NULL;

    err = AudioFileGetUserDataSize (
     PRIVATE(ax)->file,
     'uuid',
     index,
     &size);
    if(noErr!=err)
	break;
    if(0==size)
	continue;

    datasize=size;
    data=calloc(datasize, sizeof(char));
    err = AudioFileGetUserData (
     PRIVATE(ax)->file,
     'uuid',
     index,
     &datasize, data);

    if(datasize<16)
	continue;

    chunkver=_ambix_checkUUID(data);
    switch(chunkver) {
      case(1):
        if(_ambix_uuid1_to_matrix(data+16, datasize-16, &ax->matrix, ax->byteswap)) {
          if(data) free(data) ;
          return AMBIX_ERR_SUCCESS;
        }
	break;
      default:
	break;
    }
  }

  if(data) free(data);
  data=NULL;
  return AMBIX_ERR_UNKNOWN;
}

static void
ambix2coreaudio_info(const ambix_info_t*axinfo, AudioStreamBasicDescription*format) {
  UInt32 flags=0;
  UInt32 samplesize=3;
  UInt32 channels=(UInt32)(axinfo->ambichannels+axinfo->extrachannels);

  memset(format, 0, sizeof(*format));

  format->mSampleRate=(Float64)axinfo->samplerate;
  format->mChannelsPerFrame=channels;
  format->mFormatID=kAudioFormatLinearPCM;
  format->mFramesPerPacket=1;
  format->mBytesPerPacket=1;

  switch(axinfo->sampleformat) {
      case AMBIX_SAMPLEFORMAT_PCM16:
	samplesize=2;
        flags|=kAudioFormatFlagIsSignedInteger;
	break;
      default: case AMBIX_SAMPLEFORMAT_PCM24:
	samplesize=3;
        flags|=kAudioFormatFlagIsSignedInteger;
	break;
      case AMBIX_SAMPLEFORMAT_PCM32:
	samplesize=4;
        flags|=kAudioFormatFlagIsSignedInteger;
	break;
      case AMBIX_SAMPLEFORMAT_FLOAT32:
	samplesize=4;
        flags|=kAudioFormatFlagIsFloat;
	break;
  }

  format->mFormatFlags=flags;

  format->mBytesPerFrame=channels*samplesize;
  format->mBitsPerChannel=8*samplesize;
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

static ambix_err_t
coreaudio2ambix_info(const ExtAudioFileRef cainfo, ambix_info_t*axinfo) {
  AudioStreamBasicDescription format;
  UInt32 datasize=0;
  SInt64 frames;

  datasize=sizeof(SInt64);
  if(noErr == ExtAudioFileGetProperty(cainfo, kExtAudioFileProperty_FileLengthFrames, &datasize, &frames)) {
    axinfo->frames=(uint64_t)frames;
  }

  datasize=sizeof(format);
  memset(&format, 0, sizeof(format));
  if(noErr == ExtAudioFileGetProperty(cainfo, kExtAudioFileProperty_FileDataFormat, &datasize, &format)) {
    int samplesize=0;
    int isFloat=(format.mFormatFlags & kAudioFormatFlagIsFloat);
    int isInt  =(format.mFormatFlags & kAudioFormatFlagIsSignedInteger);

    samplesize=format.mBitsPerChannel;

    if(0) {}
    else if(isFloat && 32==samplesize) {axinfo->sampleformat = AMBIX_SAMPLEFORMAT_FLOAT32;}
    else if(isInt && 32==samplesize) {axinfo->sampleformat = AMBIX_SAMPLEFORMAT_PCM32;}
    else if(isInt && 24==samplesize) {axinfo->sampleformat = AMBIX_SAMPLEFORMAT_PCM24;}
    else if(isInt && 16==samplesize) {axinfo->sampleformat = AMBIX_SAMPLEFORMAT_PCM16;}
    else return AMBIX_ERR_INVALID_FORMAT;

    axinfo->samplerate = (double)format.mSampleRate;
    axinfo->extrachannels = format.mChannelsPerFrame;
  }
  return AMBIX_ERR_SUCCESS;
}


ambix_err_t _ambix_open_read(ambix_t*ambix, const char *path, const ambix_info_t*ambixinfo) {
  OSStatus err = noErr;
  ambixcoreaudio_private_t*priv=0;
  ambix->private_data=calloc(1, sizeof(ambixcoreaudio_private_t));
  priv=(ambixcoreaudio_private_t*)ambix->private_data;
  priv->pool = [[NSAutoreleasePool alloc] init];

  priv->data = [[AmbixData alloc] init];

  PRIVATE(ambix)->file=NULL;
  PRIVATE(ambix)->xfile=NULL;


  NSURL *inURL = [NSURL fileURLWithPath:[NSString stringWithUTF8String:path]];

  err = AudioFileOpenURL((CFURLRef)inURL, 
			0x01, /* kAudioFileReadPermission, */
			0,
			&(PRIVATE(ambix)->file));
  if(noErr!=err) {
    _ambix_close(ambix);
    return AMBIX_ERR_INVALID_FILE;
  }

  err = ExtAudioFileWrapAudioFileID (
			(PRIVATE(ambix)->file),
			false,
			&(PRIVATE(ambix)->xfile));
  if(noErr!=err) {
    _ambix_close(ambix);
    return AMBIX_ERR_INVALID_FILE;
  }
  if(NULL==(PRIVATE(ambix)->xfile)) {
    _ambix_close(ambix);
    return AMBIX_ERR_INVALID_FILE;
  }
  memset(&ambix->realinfo, 0, sizeof(*ambixinfo));
  if(AMBIX_ERR_SUCCESS!=coreaudio2ambix_info(PRIVATE(ambix)->xfile, &ambix->realinfo)) {
    _ambix_close(ambix);
    return AMBIX_ERR_INVALID_FILE;
  }

  ambix->byteswap = 0; /* FIXXXME: assuming wrong defaults */
  ambix->channels = ambix->realinfo.extrachannels; /* FIXXXME: realinfo is a bad vehicle */

  int caf=_coreaudio_isCAF(&PRIVATE(ambix)->file);
  int is_ambix=0;
  if(caf) {
    is_ambix=1;
    if(AMBIX_ERR_SUCCESS == read_uuidchunk(ambix)) {
       ambix->format=AMBIX_EXTENDED;
    } else {
       ambix->format=AMBIX_BASIC;
    }
  } else {
    ambix->format=AMBIX_NONE;
  }
  ambix->is_AMBIX=is_ambix;

  return AMBIX_ERR_SUCCESS;
}
ambix_err_t _ambix_open_write(ambix_t*ambix, const char *path, const ambix_info_t*ambixinfo) {
  OSStatus err = noErr;
  ambixcoreaudio_private_t*priv=0;
  AudioStreamBasicDescription format;
  ambix2coreaudio_info(ambixinfo, &format);

  ambix->private_data=calloc(1, sizeof(ambixcoreaudio_private_t));
  priv=(ambixcoreaudio_private_t*)ambix->private_data;
  priv->pool = [[NSAutoreleasePool alloc] init];

  priv->data = [[AmbixData alloc] init];

  PRIVATE(ambix)->file=NULL;
  PRIVATE(ambix)->xfile=NULL;

  NSURL *inURL = [NSURL fileURLWithPath:[NSString stringWithUTF8String:path]];

  err = AudioFileCreateWithURL((CFURLRef)inURL,
                        kAudioFileCAFType,
			&format,
                        0,
                        &(PRIVATE(ambix)->file));
  if(noErr!=err) {
    _ambix_close(ambix);
    return AMBIX_ERR_INVALID_FILE;
  }

  err = ExtAudioFileWrapAudioFileID (
                        (PRIVATE(ambix)->file),
                        true,
                        &(PRIVATE(ambix)->xfile));
  if(noErr!=err) {
    _ambix_close(ambix);
    return AMBIX_ERR_INVALID_FILE;
  }
  if(NULL==(PRIVATE(ambix)->xfile)) {
    _ambix_close(ambix);
    return AMBIX_ERR_INVALID_FILE;
  }
  
  return AMBIX_ERR_INVALID_FILE;
}
ambix_err_t _ambix_open	(ambix_t*ambix, const char *path, const ambix_filemode_t mode, const ambix_info_t*ambixinfo) {
  if((mode & AMBIX_READ) & (mode & AMBIX_WRITE))
    return AMBIX_ERR_INVALID_FILE;
  else if (mode & AMBIX_WRITE)
    return _ambix_open_write(ambix, path, ambixinfo);
  else if (mode & AMBIX_READ)
    return _ambix_open_read(ambix, path, ambixinfo);

  return AMBIX_ERR_INVALID_FILE;
}

ambix_err_t	_ambix_close	(ambix_t*ambix) {
  if(ambix&&ambix->private_data) {
    ambix_err_t err=AMBIX_ERR_SUCCESS;
    ambixcoreaudio_private_t*priv=(ambixcoreaudio_private_t*)ambix->private_data;
    /* FIRST close the file itself */
    if(PRIVATE(ambix)) {
      if(PRIVATE(ambix)->xfile)
        ExtAudioFileDispose(PRIVATE(ambix)->xfile);
      PRIVATE(ambix)->xfile=NULL;

      if(PRIVATE(ambix)->file)
        AudioFileClose(PRIVATE(ambix)->file);
      PRIVATE(ambix)->file=NULL;

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


