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
  @public ambix_sampleformat_t sampleformat;
}
@end

typedef struct ambixcoreaudio_private_struct {
  NSAutoreleasePool * pool;
  AmbixData*data;
} ambixcoreaudio_private_t;

static inline AmbixData*PRIVATE(ambix_t*ax) { 
  ambixcoreaudio_private_t*priv=(ambixcoreaudio_private_t*)(ax->private_data);
  return priv->data;
}

@implementation AmbixData
@end

static void print_error(OSStatus err) {
  union {
    char s[5];
    OSStatus err;
  } u;
  u.err=err;
  u.s[4]=0;
  printf("ERR: %x='%s'\n", err, u.s);
}

static void print_caf_formatID(UInt32 format) {
  union {
    char s[4];
    UInt32 format;
  } u;
  u.format=format;
  printf("'%c%c%c%c'\n", u.s[0], u.s[1], u.s[2], u.s[3]);
}
#define str(x) #x
#define xstr(x) str(x)
#define PRINTFLAG(y) do {if (flags&y) {printf("" xstr(y)"|"); flags^=y;}}while(0)
static void print_caf_flags(UInt32 flags) {
   PRINTFLAG(kAudioFormatFlagIsBigEndian);
   PRINTFLAG(kAudioFormatFlagIsPacked);
   PRINTFLAG(kAudioFormatFlagIsAlignedHigh);
   PRINTFLAG(kAudioFormatFlagIsNonInterleaved);
   PRINTFLAG(kAudioFormatFlagIsNonMixable);
   PRINTFLAG(kAudioFormatFlagsAreAllClear);

   PRINTFLAG(kAudioFormatFlagIsFloat);
   PRINTFLAG(kAudioFormatFlagIsSignedInteger);
   printf("%d\n", flags);
}
static void print_caformat(const AudioStreamBasicDescription*format) {
  printf("	SampleRate=%f\n", (float)format->mSampleRate);
  printf("	FormatID="); print_caf_formatID(format->mFormatID);
  printf("	FormatFlags="); print_caf_flags(format->mFormatFlags);
  printf("	BytesPerPacket=%lu\n", (unsigned long)format->mBytesPerPacket);
  printf("	FramesPerPacket=%lu\n", (unsigned long)format->mFramesPerPacket);
  printf("	BytesPerFrame=%lu\n", (unsigned long)format->mBytesPerFrame);
  printf("	ChannelsPerFrame=%lu\n", (unsigned long)format->mChannelsPerFrame);
  printf("	BitsPerChannel=%lu\n", (unsigned long)format->mBitsPerChannel);
  printf("	Reserved=%lu\n", (unsigned long)format->mReserved);
}

UInt32 coreaudio_doGetFlags (
   UInt32 inValidBitsPerChannel,
   UInt32 inTotalBitsPerChannel,
   bool inIsFloat,
   bool inIsBigEndian,
   bool inIsNonInterleaved
) {
   return
   (inIsFloat ? kAudioFormatFlagIsFloat : kAudioFormatFlagIsSignedInteger) |
   (inIsBigEndian ? ((UInt32)kAudioFormatFlagIsBigEndian) : 0)             |
   ((!inIsFloat && (inValidBitsPerChannel == inTotalBitsPerChannel)) ?
   kAudioFormatFlagIsPacked : kAudioFormatFlagIsAlignedHigh)           |
   (inIsNonInterleaved ? ((UInt32)kAudioFormatFlagIsNonInterleaved) : 0);
}
UInt32 coreaudio_getFlags (UInt32 samplebits, bool isFloat, bool isBigEndian) {
  return coreaudio_doGetFlags(samplebits, samplebits, isFloat, isBigEndian, false);
}



static int _coreaudio_isNativeEndian(const ExtAudioFileRef cainfo) {
  AudioStreamBasicDescription f;
  UInt32 datasize=sizeof(f);
  memset(&f, 0, sizeof(f));
  if(noErr == ExtAudioFileGetProperty(cainfo, kExtAudioFileProperty_FileDataFormat, &datasize, &f)) {
    return ((f.mFormatID == kAudioFormatLinearPCM) 
            && ((f.mFormatFlags & kLinearPCMFormatFlagIsBigEndian) == kAudioFormatFlagsNativeEndian));
  }

  return 0;
}

static int _coreaudio_isCAF(const AudioFileID*file) {
  /* trying to read format (is it caf?) */
  UInt32 myformat=0;
  UInt32 formatSize=sizeof(myformat);

  OSStatus err=AudioFileGetProperty (*file,
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

    err = AudioFileGetUserDataSize (PRIVATE(ax)->file,
                                    'uuid',
                                    index,
                                    &size);
    if(noErr!=err)
      break;
    if(0==size)
      continue;

    datasize=size;
    data=calloc(datasize, sizeof(char));
    if(!data)continue;
    err = AudioFileGetUserData (PRIVATE(ax)->file,
                                'uuid',
                                index,
                                &datasize, data);
    if(noErr!=err)
      break;

    if(datasize<16)
      continue;

    chunkver=_ambix_checkUUID(data);
    switch(chunkver) {
    case(1):
      if(_ambix_uuid1_to_matrix(data+16, datasize-16, &ax->matrix, ax->byteswap)) {
        if(data) free(data) ; data=NULL;
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
static ambix_sampleformat_t coreaudio_getSampleformat(AudioStreamBasicDescription*format) {
  int isFloat=(format->mFormatFlags & kLinearPCMFormatFlagIsFloat);
  int isInt  =(format->mFormatFlags & kLinearPCMFormatFlagIsSignedInteger);
  int samplesize=format->mBitsPerChannel;

  if(format->mFormatFlags & kLinearPCMFormatFlagIsFloat) {
    if(32==samplesize)return AMBIX_SAMPLEFORMAT_FLOAT32;
  } else if (format->mFormatFlags & kLinearPCMFormatFlagIsSignedInteger) {
    switch(samplesize) {
    case 16: return AMBIX_SAMPLEFORMAT_PCM16;
    case 24: return AMBIX_SAMPLEFORMAT_PCM24;
    case 32: return AMBIX_SAMPLEFORMAT_PCM32;
    default: break;
    }
  }
  return AMBIX_SAMPLEFORMAT_NONE;
}
static ambix_sampleformat_t coreaudio_setSampleformat(ambix_sampleformat_t sampleformat, AudioStreamBasicDescription*format, bool nativeEndian) {
  bool bigEndian=true;
  if(nativeEndian) bigEndian=( kAudioFormatFlagIsBigEndian == kAudioFormatFlagsNativeEndian);
  switch(sampleformat) {
  case(AMBIX_SAMPLEFORMAT_PCM16):
    format->mBitsPerChannel = 16;
    format->mFormatID=kAudioFormatLinearPCM;
    format->mFramesPerPacket=1;
    format->mBytesPerFrame=(format->mBitsPerChannel/8)*format->mChannelsPerFrame; format->mBytesPerPacket=format->mBytesPerFrame;
    format->mFormatFlags = coreaudio_getFlags (format->mBitsPerChannel, false, bigEndian);
    return sampleformat;
  case(AMBIX_SAMPLEFORMAT_PCM24):
    format->mBitsPerChannel = 24;
    format->mFormatID=kAudioFormatLinearPCM;
    format->mFramesPerPacket=1;
    format->mBytesPerFrame=(format->mBitsPerChannel/8)*format->mChannelsPerFrame; format->mBytesPerPacket=format->mBytesPerFrame;
    format->mFormatFlags = coreaudio_getFlags (format->mBitsPerChannel, false, bigEndian);
    return sampleformat;
  case(AMBIX_SAMPLEFORMAT_PCM32):
    format->mBitsPerChannel = 32;
    format->mFormatID=kAudioFormatLinearPCM;
    format->mFramesPerPacket=1;
    format->mBytesPerFrame=(format->mBitsPerChannel/8)*format->mChannelsPerFrame; format->mBytesPerPacket=format->mBytesPerFrame;
    format->mFormatFlags = coreaudio_getFlags (format->mBitsPerChannel, false, bigEndian);
    return sampleformat;
  case(AMBIX_SAMPLEFORMAT_FLOAT32):
    format->mBitsPerChannel = 32;
    format->mFormatID=kAudioFormatLinearPCM;
    format->mFramesPerPacket=1;
    format->mBytesPerFrame=(format->mBitsPerChannel/8)*format->mChannelsPerFrame; format->mBytesPerPacket=format->mBytesPerFrame;
    format->mFormatFlags = coreaudio_getFlags (format->mBitsPerChannel, true, bigEndian);
    return sampleformat;
  }
  return AMBIX_SAMPLEFORMAT_NONE;
}

static ambix_sampleformat_t coreaudio_setClientFormat(ambix_t*axinfo, ambix_sampleformat_t sampleformat) {
  OSStatus err = noErr;
  AudioStreamBasicDescription format;
  UInt32 formatsize=sizeof(format);
  if(sampleformat == PRIVATE(axinfo)->sampleformat)
    return sampleformat;

  err =  ExtAudioFileGetProperty(PRIVATE(axinfo)->xfile, kExtAudioFileProperty_FileDataFormat,
                                 &formatsize, &format);
  if(noErr != err)
    return AMBIX_SAMPLEFORMAT_NONE;

  sampleformat = coreaudio_setSampleformat(sampleformat, &format, true);
  if(AMBIX_SAMPLEFORMAT_NONE == sampleformat)
    return AMBIX_SAMPLEFORMAT_NONE;

  err =  ExtAudioFileSetProperty(PRIVATE(axinfo)->xfile, kExtAudioFileProperty_ClientDataFormat,
                                 sizeof(format), &format);
  if(noErr != err)
    return AMBIX_SAMPLEFORMAT_NONE;

  PRIVATE(axinfo)->sampleformat = sampleformat;

  return sampleformat;
}
static void
ambix2coreaudio_info(const ambix_info_t*axinfo, AudioStreamBasicDescription*format, bool nativeEndian) {
  UInt32 channels=(UInt32)(axinfo->ambichannels+axinfo->extrachannels);

  memset(format, 0, sizeof(*format));
  format->mChannelsPerFrame=channels;
  format->mSampleRate=(Float64)axinfo->samplerate;
  coreaudio_setSampleformat(axinfo->sampleformat, format, nativeEndian);
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
    ambix_sampleformat_t sampleformat=coreaudio_getSampleformat(&format);
    if(AMBIX_SAMPLEFORMAT_NONE==sampleformat)
      return AMBIX_ERR_INVALID_FORMAT;
    axinfo->sampleformat = sampleformat;

    axinfo->samplerate = (double)format.mSampleRate;
    axinfo->extrachannels = format.mChannelsPerFrame;
  }
  return AMBIX_ERR_SUCCESS;
}


ambix_err_t _ambix_open_read(ambix_t*ambix, const char *path, const ambix_info_t*ambixinfo) {
  OSStatus err = noErr;
  ambixcoreaudio_private_t*priv=0;
  ambix->private_data=calloc(1, sizeof(ambixcoreaudio_private_t));
  if(!ambix->private_data)return AMBIX_ERR_UNKNOWN;
  priv=(ambixcoreaudio_private_t*)ambix->private_data;
  priv->pool = [[NSAutoreleasePool alloc] init];
  priv->data = [[AmbixData alloc] init];

  if(!(priv->pool) || !(priv->data)) {
    _ambix_close(ambix);
    return AMBIX_ERR_UNKNOWN;
  }
 
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

  ambix->byteswap = !_coreaudio_isNativeEndian(PRIVATE(ambix)->xfile);
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
  int is_ambix=0;
  OSStatus err = noErr;
  ambixcoreaudio_private_t*priv=0;
  AudioStreamBasicDescription format;
  ambix2coreaudio_info(ambixinfo, &format, false);
  ambix->private_data=calloc(1, sizeof(ambixcoreaudio_private_t));
  if(!ambix->private_data)return AMBIX_ERR_UNKNOWN;

  priv=(ambixcoreaudio_private_t*)ambix->private_data;
  priv->pool = [[NSAutoreleasePool alloc] init];
  priv->data = [[AmbixData alloc] init];
  if(!(priv->pool) || !(priv->data)) {
    _ambix_close(ambix);
    return AMBIX_ERR_UNKNOWN;
  }

  PRIVATE(ambix)->file=NULL;
  PRIVATE(ambix)->xfile=NULL;
  PRIVATE(ambix)->sampleformat=ambixinfo->sampleformat;

  if(ambixinfo->ambichannels>0) is_ambix=1;

  NSURL *inURL = [NSURL fileURLWithPath:[NSString stringWithUTF8String:path]];

  err = AudioFileCreateWithURL((CFURLRef)inURL,
                               kAudioFileCAFType,
                               &format,
                               kAudioFileFlags_EraseFile, /* FIXME?: 0 */
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
  
  ambix->is_AMBIX=is_ambix;
  ambix->byteswap = !_coreaudio_isNativeEndian(PRIVATE(ambix)->xfile);
  ambix->channels = format.mChannelsPerFrame;

  return AMBIX_ERR_SUCCESS;
}
ambix_err_t _ambix_open	(ambix_t*ambix, const char *path, const ambix_filemode_t mode, const ambix_info_t*ambixinfo) {
//printf("AMBIX: CoreAudio support\n");
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

int64_t coreaudio_readf(ambix_t*ambix, void*data, int64_t frames, ambix_sampleformat_t sampleformat, UInt32 bytespersample) {
  if(AMBIX_SAMPLEFORMAT_NONE == coreaudio_setClientFormat(ambix, sampleformat)) return -1;

  UInt32 readframes=(UInt32)frames;
  UInt32 channels = ambix->channels;

  AudioBufferList fillBufList;
  fillBufList.mNumberBuffers = 1;
  fillBufList.mBuffers[0].mNumberChannels = channels;
  fillBufList.mBuffers[0].mDataByteSize = frames * bytespersample * channels;
  fillBufList.mBuffers[0].mData = data;

  OSStatus err =  ExtAudioFileRead (PRIVATE(ambix)->xfile, &readframes, &fillBufList);
  if(noErr != err)return -1;
  return (int64_t)readframes;
}
int64_t _ambix_readf_int16   (ambix_t*ambix, int16_t*data, int64_t frames) {
  return coreaudio_readf(ambix, data, frames, AMBIX_SAMPLEFORMAT_PCM16, 2);
}
int64_t _ambix_readf_int32   (ambix_t*ambix, int32_t*data, int64_t frames) {
  return coreaudio_readf(ambix, data, frames, AMBIX_SAMPLEFORMAT_PCM32, 4);
}
int64_t _ambix_readf_float32   (ambix_t*ambix, float32_t*data, int64_t frames) {
  return coreaudio_readf(ambix, data, frames, AMBIX_SAMPLEFORMAT_FLOAT32, 4);
}
int64_t _ambix_readf_float64   (ambix_t*ambix, float64_t*data, int64_t frames) {
  return coreaudio_readf(ambix, data, frames, AMBIX_SAMPLEFORMAT_FLOAT64, 8);
}

int64_t coreaudio_writef(ambix_t*ambix, const void*data, int64_t frames, ambix_sampleformat_t sampleformat, UInt32 bytespersample) {
 //printf("info:\n");_ambix_print_info(&ambix->info);
 //printf("realinfo:\n");_ambix_print_info(&ambix->realinfo);
  if(AMBIX_SAMPLEFORMAT_NONE == coreaudio_setClientFormat(ambix, sampleformat)) return -1;
  UInt32 writeframes=(UInt32)frames;
  UInt32 channels = ambix->channels;
  OSStatus err = noErr;

  AudioBufferList fillBufList;
  fillBufList.mNumberBuffers = 1;
  fillBufList.mBuffers[0].mNumberChannels = channels;
  fillBufList.mBuffers[0].mDataByteSize = frames * bytespersample * channels;
  fillBufList.mBuffers[0].mData = (void*)data;

  //printf("ambix_write: %d/%d channels\n", (int)ambix->info.ambichannels, (int)ambix->info.extrachannels);
  //printf("writing %d frames of %d channels (%d) in %p\n", (int)frames, (int)channels, (int)(fillBufList.mBuffers[0].mDataByteSize), data);
  err=ExtAudioFileWrite (PRIVATE(ambix)->xfile, writeframes, &fillBufList);
  if(noErr != err)return -1;
  return (int64_t)writeframes;
}
int64_t _ambix_writef_int16   (ambix_t*ambix, const int16_t*data, int64_t frames) {
  return coreaudio_writef(ambix, data, frames, AMBIX_SAMPLEFORMAT_PCM16, 2);
}
int64_t _ambix_writef_int32   (ambix_t*ambix, const int32_t*data, int64_t frames) {
  return coreaudio_writef(ambix, data, frames, AMBIX_SAMPLEFORMAT_PCM32, 4);
}
int64_t _ambix_writef_float32   (ambix_t*ambix, const float32_t*data, int64_t frames) {
//printf("_ambix_writef_float32(%p, %p, %lu)\n", ambix, data);
  return coreaudio_writef(ambix, data, frames, AMBIX_SAMPLEFORMAT_FLOAT32, 4);
}
int64_t _ambix_writef_float64   (ambix_t*ambix, const float64_t*data, int64_t frames) {
  return coreaudio_writef(ambix, data, frames, AMBIX_SAMPLEFORMAT_FLOAT64, 8);
}
ambix_err_t _ambix_write_uuidchunk_at(ambix_t*ax, UInt32 index, const void*data, int64_t datasize) {
  OSStatus  err = AudioFileSetUserData (
                                        PRIVATE(ax)->file,
                                        'uuid',
                                        index,
                                        datasize, data);
  if(noErr!=err)
    return AMBIX_ERR_UNKNOWN;

  return AMBIX_ERR_SUCCESS;
}
ambix_err_t _ambix_write_uuidchunk(ambix_t*ax, const void*data, int64_t datasize) {
  /* find a free slot and write UUID */
  UInt32 index=0;
  OSStatus err = noErr;
  char*readdata=NULL;

  for(index=0; ; index++) {
    UInt32 size=0;
    UInt32 readdatasize=0;
    uint32_t chunkver=0;

    if(readdata) free(readdata);
    readdata=NULL;

    err = AudioFileGetUserDataSize (
                                    PRIVATE(ax)->file,
                                    'uuid',
                                    index,
                                    &size);
    if(noErr!=err) {
      if(readdata) free(readdata) ; readdata=NULL;
      /* there is no uuid-chunk[index], so we use it: */
      return _ambix_write_uuidchunk_at(ax, index, data, datasize);
    }
    if(0==size)
      continue;

    readdatasize=size;
    data=calloc(readdatasize, sizeof(char));
    if(!data)continue;
    err = AudioFileGetUserData (
                                PRIVATE(ax)->file,
                                'uuid',
                                index,
                                &readdatasize, readdata);
    if(noErr!=err)
      break;

    if(readdatasize<16)
      continue;

    chunkver=_ambix_checkUUID(readdata);
    switch(chunkver) {
    case(1):
      /* there is a valid ambix uuid-chunk[index], so we overwrite it: */
      if(readdata) free(readdata) ; readdata=NULL;
      return _ambix_write_uuidchunk_at(ax, index, data, datasize);
    default:
      /* there is a uuid-chunk[index] but it's not ambix, so skip it */
      break;
    }
  }

  if(readdata) free(readdata);
  data=NULL;
  return AMBIX_ERR_UNKNOWN;
}

static int64_t _ambix_tell(ambix_t*ambix) {
  SInt64 pos=0;
  OSStatus err = ExtAudioFileTell (PRIVATE(ambix)->xfile, &pos);
  if(noErr==err)
    return pos;
  else
    return -1;
}

int64_t _ambix_seek (ambix_t* ambix, int64_t frames, int whence) {
  OSStatus err=noErr;
  SInt64           inFrameOffset;
  int64_t offset=0;
  switch(whence) {
  case SEEK_SET:
    offset=0;
    break;
  case SEEK_END:
    offset=ambix->realinfo.frames;
    break;
  case SEEK_CUR:
    offset=_ambix_tell(ambix);
    break;
  default:
    return -1;
  }
  if(offset<0)
    return -1;

  inFrameOffset=frames+offset;

  err=ExtAudioFileSeek (
                        PRIVATE(ambix)->xfile, 
                        inFrameOffset);

  return _ambix_tell(ambix);
}

