/* marker_region_chunk.c -  read/write marker and region chunk              -*- c -*-

   Copyright © 2012-2016 IOhannes m zmölnig <zmoelnig@iem.at>.
         Institute of Electronic Music and Acoustics (IEM),
         University of Music and Dramatic Arts, Graz

   Copyright © 2016 Matthias Kronlachner <mail@matthiaskronlachner.com>

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

#ifdef HAVE_STRING_H
# include <string.h>
#endif /* HAVE_STRING_H */
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif /* HAVE_STDLIB_H */

/* dense pack all structs */
/* in a CAF file there are no pad fields to ensure correct byte alignment */
#pragma pack(push, 1)

/* ------------------------------------------------ */
/* MARKER CHUNK */

/* this is for the mSMPTETime of CAFMarker */
typedef struct {
    unsigned char   mHours;
    unsigned char   mMinutes;
    unsigned char   mSeconds;
    unsigned char   mFrames;
    uint32_t        mSubFrameSampleOffset;
} CAF_SMPTE_Time;

/* this is for the mSMPTE_TimeType of CAFMarkerChunk */
typedef enum {
    kCAF_SMPTE_TimeTypeNone     = 0,
    kCAF_SMPTE_TimeType24       = 1,
    kCAF_SMPTE_TimeType25       = 2,
    kCAF_SMPTE_TimeType30Drop   = 3,
    kCAF_SMPTE_TimeType30       = 4,
    kCAF_SMPTE_TimeType2997     = 5,
    kCAF_SMPTE_TimeType2997Drop = 6,
    kCAF_SMPTE_TimeType60       = 7,
    kCAF_SMPTE_TimeType5994     = 8
} kCAF_SMPTE_TimeType; // uint32_t

/* this is for the mType field of CAFMarker */
#define AMBIX_MAKE_MARKER(a, b, c, d)           \
  ((uint32_t) (                                 \
               (((uint32_t) (a)) << 24) |       \
               (((uint32_t) (b)) << 16) |       \
               (((uint32_t) (c)) <<  8) |       \
               (((uint32_t) (d)) <<  0)         \
               ))
typedef enum {
    kCAFMarkerType_Generic              = 0,
    kCAFMarkerType_ProgramStart         = AMBIX_MAKE_MARKER('p', 'b', 'e', 'g'),
    kCAFMarkerType_ProgramEnd           = AMBIX_MAKE_MARKER('p', 'e', 'n', 'd'),
    kCAFMarkerType_TrackStart           = AMBIX_MAKE_MARKER('t', 'b', 'e', 'g'),
    kCAFMarkerType_TrackEnd             = AMBIX_MAKE_MARKER('t', 'e', 'n', 'd'),
    kCAFMarkerType_Index                = AMBIX_MAKE_MARKER('i', 'n', 'd', 'x'),
    kCAFMarkerType_RegionStart          = AMBIX_MAKE_MARKER('r', 'b', 'e', 'g'),
    kCAFMarkerType_RegionEnd            = AMBIX_MAKE_MARKER('r', 'e', 'n', 'd'),
    kCAFMarkerType_RegionSyncPoint      = AMBIX_MAKE_MARKER('r', 's', 'y', 'c'),
    kCAFMarkerType_SelectionStart       = AMBIX_MAKE_MARKER('s', 'b', 'e', 'g'),
    kCAFMarkerType_SelectionEnd         = AMBIX_MAKE_MARKER('s', 'e', 'n', 'd'),
    kCAFMarkerType_EditSourceBegin      = AMBIX_MAKE_MARKER('c', 'b', 'e', 'g'),
    kCAFMarkerType_EditSourceEnd        = AMBIX_MAKE_MARKER('c', 'e', 'n', 'd'),
    kCAFMarkerType_EditDestinationBegin = AMBIX_MAKE_MARKER('d', 'b', 'e', 'g'),
    kCAFMarkerType_EditDestinationEnd   = AMBIX_MAKE_MARKER('d', 'e', 'n', 'd'),
    kCAFMarkerType_SustainLoopStart     = AMBIX_MAKE_MARKER('s', 'l', 'b', 'g'),
    kCAFMarkerType_SustainLoopEnd       = AMBIX_MAKE_MARKER('s', 'l', 'e', 'n'),
    kCAFMarkerType_ReleaseLoopStart     = AMBIX_MAKE_MARKER('r', 'l', 'b', 'g'),
    kCAFMarkerType_ReleaseLoopEnd       = AMBIX_MAKE_MARKER('r', 'l', 'e', 'n'),
} kCAFMarkerType; // uint32_t

/* individual marker struct */
typedef struct {
    kCAFMarkerType  mType;
    float64_t       mFramePosition;
    uint32_t        mMarkerID;        // reference to a mStringsIDs for naming
    CAF_SMPTE_Time  mSMPTETime;
    uint32_t        mChannel;
} CAFMarker;


/* chunk holding all markers */
typedef struct {
    uint32_t     mSMPTE_TimeType;
    uint32_t     mNumberMarkers;
    CAFMarker*  mMarkers;
} CAFMarkerChunk;

/* ------------------------------------------------ */
/* REGIONS CHUNK */

/* used for mFlags in CAFRegion */
typedef enum {
    kCAFRegionFlag_LoopEnable    = 1,
    kCAFRegionFlag_PlayForward   = 2,
    kCAFRegionFlag_PlayBackward  = 4
} kCAFRegionFlag; // uint32_t

typedef struct {
    uint32_t    mRegionID;
    uint32_t    mFlags;
    uint32_t    mNumberMarkers;
    CAFMarker* mMarkers;
} CAFRegion;
#define SIZEOF_CAFRegion 12

typedef struct {
    uint32_t     mSMPTE_TimeType;
    uint32_t     mNumberRegions;
    CAFRegion*   mRegions;
} CAFRegionChunk;
#define SIZEOF_CAFRegionChunk 8

/* ------------------------------------------------ */
/* STRINGS CHUNK - used as labels for Markers and Regions*/

typedef struct {
    uint32_t  mStringID;
    int64_t   mStringStartByteOffset;
} CAFStringID;

typedef struct {
    uint32_t       mNumEntries; // The number of strings in the mStrings field.
    CAFStringID*   mStringsIDs; // the marker refers to this id with mMarkerID
    unsigned char* mStrings;    // An array of null-terminated UTF8-encoded text strings.
} CAFStrings;
#define SIZEOF_CAFStrings 4

#pragma pack(pop)

union int_chars {
  uint32_t a;
  char b[4];
};

typedef struct {
  uint32_t      num_strings;
  uint32_t      *string_ids;
  unsigned char **strings;
} strings_buffer;

void swap_marker_chunk(CAFMarkerChunk* marker_chunk) {
  _ambix_swap4array(&marker_chunk->mSMPTE_TimeType, 1);
  _ambix_swap4array(&marker_chunk->mNumberMarkers, 1);
}
void swap_marker(CAFMarker* marker) {
  _ambix_swap4array(&marker->mType, 1);
  _ambix_swap8array((uint64_t *)&marker->mFramePosition, 1);
  _ambix_swap4array(&marker->mMarkerID, 1);
  _ambix_swap4array(&marker->mChannel, 1);
}
void swap_region(CAFRegion* region) {
  _ambix_swap4array(&region->mRegionID, 1);
  _ambix_swap4array(&region->mFlags, 1);
  _ambix_swap4array(&region->mNumberMarkers, 1);
}
void swap_region_chunk(CAFRegionChunk* region_chunk) {
  _ambix_swap4array(&region_chunk->mSMPTE_TimeType, 1);
  _ambix_swap4array(&region_chunk->mNumberRegions, 1);
}
void swap_stringid(CAFStringID* caf_stringid) {
  _ambix_swap4array(&caf_stringid->mStringID, 1);
  _ambix_swap8array((uint64_t *)&caf_stringid->mStringStartByteOffset, 1);
}
unsigned char* get_string_from_buffer(strings_buffer* buffer, uint32_t id) {
  if (buffer) {
    uint32_t i;
    for (i=0; i<buffer->num_strings;i++) {
      if (buffer->string_ids[i] == id)
        return buffer->strings[i];
    }
    return NULL;
  } else
    return NULL;
}

ambix_err_t _ambix_read_markersregions(ambix_t*ambix) {
  int byteswap = ambix->byteswap;
  uint32_t chunk_it = 0;
  uint32_t i;

  int64_t strings_datasize = 1;
  union int_chars strg_id;
  strings_buffer mystrings;
  memset(&mystrings, 0, sizeof(strings_buffer));

  strg_id.b[0] = 's'; strg_id.b[1] = 't'; strg_id.b[2] = 'r'; strg_id.b[3] = 'g';

  /* first parse strings and save into a struct for later usage */
  while (strings_datasize) {
    void*strings_data = _ambix_read_chunk(ambix, strg_id.a, chunk_it++, &strings_datasize);
    if (!strings_data)
      continue;
    if (strings_datasize > SIZEOF_CAFStrings) {
      uint32_t temp_num_strings = 0;
      int64_t mstrings_datasize = 0;
      char *strings_ptr = NULL;
      CAFStringID* caf_stringid = NULL;
      CAFStrings* strings_chunk = (CAFStrings*)strings_data;
      if (byteswap)
        _ambix_swap4array(&strings_chunk->mNumEntries, 1);
      if (strings_datasize < (SIZEOF_CAFStrings + strings_chunk->mNumEntries*sizeof(CAFStringID))) {
        free(strings_data);
        break;
      }
      temp_num_strings = strings_chunk->mNumEntries;
      mstrings_datasize = strings_datasize - (SIZEOF_CAFStrings + temp_num_strings*sizeof(CAFStringID));
      // allocate memory for mystrings
      if (!mystrings.string_ids) {
        mystrings.string_ids = malloc((mystrings.num_strings+temp_num_strings)*sizeof(uint32_t));
        mystrings.strings = malloc((mystrings.num_strings+temp_num_strings)*sizeof(unsigned char*));
      } else {
        mystrings.string_ids = realloc(mystrings.string_ids, (mystrings.num_strings+temp_num_strings)*sizeof(uint32_t));
        mystrings.strings = realloc(mystrings.strings, (mystrings.num_strings+temp_num_strings)*sizeof(unsigned char*));
      }
      strings_ptr = strings_data;
      caf_stringid = (CAFStringID*)(&strings_ptr[4]);
      strings_ptr += (4+temp_num_strings*sizeof(CAFStringID)); // start of mStrings
      for (i=0; i<temp_num_strings; i++) {
        unsigned char* mString = NULL;
        uint32_t mString_len = 0;
        if (byteswap)
          swap_stringid(&caf_stringid[i]);
        if (caf_stringid[i].mStringStartByteOffset >= mstrings_datasize)
          break; // invalid offset!
        mString = (unsigned char*) (strings_ptr+caf_stringid[i].mStringStartByteOffset);
        mystrings.string_ids[mystrings.num_strings] = caf_stringid[i].mStringID;
        mString_len = strlen((const char *)mString);
        mystrings.strings[mystrings.num_strings] = calloc((mString_len+1), sizeof(unsigned char));
        memcpy(mystrings.strings[mystrings.num_strings], mString, mString_len*sizeof(unsigned char));
        mystrings.num_strings++;
      }
    }
    free(strings_data);
  }

  /* parse markers */
  do {
    int64_t marker_datasize = 1;
    union int_chars mark_id;
    chunk_it = 0;
    mark_id.b[0] = 'm'; mark_id.b[1] = 'a'; mark_id.b[2] = 'r'; mark_id.b[3] = 'k';
    while (marker_datasize) {
      void*marker_data = _ambix_read_chunk(ambix, mark_id.a, chunk_it++, &marker_datasize);
      if (!marker_data)
        continue;
      if (marker_datasize > 2*sizeof(uint32_t)) {
        CAFMarkerChunk* marker_chunk = (CAFMarkerChunk*)marker_data;
        unsigned char* bytePtr = NULL;
        if (byteswap)
          swap_marker_chunk(marker_chunk);
        if (marker_datasize < (marker_chunk->mNumberMarkers*(sizeof(CAFMarker)) + 2*sizeof(uint32_t))) {
          free(marker_data);
          break;
        }
        bytePtr = (unsigned char*)marker_data;
        bytePtr += 2*sizeof(uint32_t);
        for (i=0; i<marker_chunk->mNumberMarkers; i++) {
          ambix_marker_t new_ambix_marker;
          CAFMarker *caf_marker = (CAFMarker*)bytePtr;
          unsigned char* string = NULL;
          if (byteswap)
            swap_marker(caf_marker);
          memset(&new_ambix_marker, 0, sizeof(ambix_marker_t));
          new_ambix_marker.position = caf_marker->mFramePosition;
          string = get_string_from_buffer(&mystrings, caf_marker->mMarkerID);
          if (string) {
            strncpy(new_ambix_marker.name, (const char *)string, 255);
          }
          ambix_add_marker(ambix, &new_ambix_marker);
          bytePtr += sizeof(CAFMarker);
        }
      }
      free(marker_data);
    }
  } while(0);

  /* parse regions */
  do {
    void* region_data = NULL;
    int64_t region_datasize = 1;
    union int_chars regn_id;
    regn_id.b[0] = 'r'; regn_id.b[1] = 'e'; regn_id.b[2] = 'g'; regn_id.b[3] = 'n';

    chunk_it = 0;

    while (region_datasize) {
      region_data = _ambix_read_chunk(ambix, regn_id.a, chunk_it++, &region_datasize);
      if(!region_data)
        continue;
      if (region_datasize > 2*sizeof(uint32_t)) {
        int64_t data_read = 0;
        unsigned char* bytePtr = NULL;
        CAFRegionChunk* region_chunk = (CAFRegionChunk*)region_data;
        if (byteswap)
          swap_region_chunk(region_chunk);
        if (region_datasize < (region_chunk->mNumberRegions*(SIZEOF_CAFRegion+sizeof(CAFMarker)) + SIZEOF_CAFRegionChunk)) {
          free(region_data);
          break;
        }
        bytePtr = (unsigned char*)region_data;
        data_read += 2*sizeof(uint32_t); // SIZEOF_CAFRegionChunk
        for (i=0; i<region_chunk->mNumberRegions; i++) {
          uint32_t j;
          ambix_region_t new_ambix_region;
          CAFRegion *caf_region = NULL;
          if (region_datasize < data_read + SIZEOF_CAFRegion)
            break;
          caf_region = (CAFRegion*)&bytePtr[data_read];
          if (byteswap)
            swap_region(caf_region);
          memset(&new_ambix_region, 0, sizeof(ambix_region_t));
          /* iterate over all markers and find startMarker and endMarker */
          data_read += SIZEOF_CAFRegion;
          for (j=0; j<caf_region->mNumberMarkers; j++) {
            CAFMarker *caf_marker = NULL;
            if (region_datasize < data_read + sizeof(CAFMarker))
              break;
            caf_marker = (CAFMarker*)&bytePtr[data_read];
            if (byteswap)
              swap_marker(caf_marker);
            if (caf_marker->mType == kCAFMarkerType_RegionStart) {
              unsigned char* string = NULL;
              new_ambix_region.start_position = caf_marker->mFramePosition;
              string = get_string_from_buffer(&mystrings, caf_marker->mMarkerID);
              if (string)
                strncpy(new_ambix_region.name, (const char *)string, 255);
            }
            else if (caf_marker->mType == kCAFMarkerType_RegionEnd)
              new_ambix_region.end_position = caf_marker->mFramePosition;
            data_read += sizeof(CAFMarker);
          }
          ambix_add_region(ambix, &new_ambix_region);
        }
      }
      free(region_data);
    }
  } while(0);

  /* free allocated strings data */
  if (mystrings.string_ids)
    free(mystrings.string_ids);
  for (i=0; i<mystrings.num_strings; i++) {
    if (mystrings.strings[i])
      free(mystrings.strings[i]);
  }
  if (mystrings.strings)
    free(mystrings.strings);
  memset(&mystrings, 0, sizeof(strings_buffer));

  return AMBIX_ERR_UNKNOWN;
}

void add_string_to_data(int id, unsigned char *byte_ptr_stringid, char *name, int64_t *byteoffset_strings, unsigned char *byte_ptr_strings, uint32_t *datasize_strings, int byteswap) {
  /* handle the string */
  CAFStringID* string_id = (CAFStringID*)byte_ptr_stringid;
  uint32_t name_len = strlen((const char *)name);
  string_id->mStringID = id;
  string_id->mStringStartByteOffset = *byteoffset_strings;
  memcpy(byte_ptr_strings, name, name_len*sizeof(char));
  byte_ptr_strings[name_len] = 0; // set the last char to NUL
  *byteoffset_strings += (name_len+1);
  *datasize_strings += (name_len+1);
  if (byteswap)
    swap_stringid(string_id);
}

ambix_err_t _ambix_write_markersregions(ambix_t*ambix) {
  uint32_t i;
  int byteswap = ambix->byteswap;
  uint32_t num_strings = ambix->num_markers+ambix->num_regions;

  void *strings_data = NULL;
  void *marker_data = NULL;
  void *region_data = NULL;
  uint32_t datasize_strings = 0;
  uint32_t datasize_markers = 0;
  uint32_t datasize_regions = 0;

  unsigned char* byte_ptr_strings = NULL;
  unsigned char* byte_ptr_stringid = NULL;
  int64_t byteoffset_strings = 0;
  CAFStrings *strings_chunk = NULL;

  /* reserve space for strings */
  if (!num_strings) {
    return AMBIX_ERR_SUCCESS;
  }
  datasize_strings = sizeof(uint32_t)+num_strings*sizeof(CAFStringID);
  strings_data = calloc(1, datasize_strings+256); // reserve a fixed space of 256 bytes for each string
  byte_ptr_strings = (unsigned char*)strings_data;
  byte_ptr_strings += (sizeof(uint32_t)+num_strings*(sizeof(CAFStringID)));
  byte_ptr_stringid = (unsigned char*)strings_data;
  byte_ptr_stringid += sizeof(uint32_t);
  strings_chunk = (CAFStrings*)strings_data;
  strings_chunk->mNumEntries = num_strings;
  if (byteswap)
    _ambix_swap4array(&strings_chunk->mNumEntries, 1);

  /* markers */
  if (ambix->num_markers > 0) {
    CAFMarkerChunk* marker_chunk = NULL;
    unsigned char* bytePtr = NULL;
    datasize_markers = 2*sizeof(uint32_t) + ambix->num_markers*sizeof(CAFMarker);
    marker_data = calloc(1, datasize_markers);
    bytePtr = (unsigned char*)marker_data;

    marker_chunk = (CAFMarkerChunk*)marker_data;
    marker_chunk->mSMPTE_TimeType = kCAF_SMPTE_TimeTypeNone;
    marker_chunk->mNumberMarkers = ambix->num_markers;
    if (byteswap)
      swap_marker_chunk(marker_chunk);

    // offset the data pointer by 2*uint32_t to point to start of markers
    bytePtr += 2*sizeof(uint32_t);
    for (i=0; i<ambix->num_markers;i++) {
      CAFMarker* new_marker = (CAFMarker*) bytePtr;
      new_marker->mType = kCAFMarkerType_Generic;
      new_marker->mFramePosition = ambix->markers[i].position;
      new_marker->mMarkerID = i+1; // string ID -> 1...num_markers
      new_marker->mChannel = 0; // 0 means for all channels
      if (byteswap) {
        swap_marker(new_marker);
      }
      bytePtr += sizeof(CAFMarker);

      add_string_to_data(i+1, byte_ptr_stringid, ambix->markers[i].name, &byteoffset_strings, byte_ptr_strings, &datasize_strings, byteswap);
      byte_ptr_strings += (strlen((const char*)ambix->markers[i].name)+1);
      byte_ptr_stringid += sizeof(CAFStringID);
    }
  }

  /* regions */
  if (ambix->num_regions > 0) {
    // a region consists of 2 markers (start,end) and region chunk
    CAFRegionChunk* region_chunk = NULL;
    unsigned char* byte_ptr_regions = NULL;
    datasize_regions = 2*sizeof(uint32_t) + ambix->num_regions*(SIZEOF_CAFRegion + 2*sizeof(CAFMarker));
    region_data      = calloc(1, datasize_regions);
    region_chunk     = (CAFRegionChunk*)region_data;
    byte_ptr_regions = (unsigned char*)region_data;

    region_chunk->mSMPTE_TimeType = kCAF_SMPTE_TimeTypeNone;
    region_chunk->mNumberRegions = ambix->num_regions;
    if (byteswap)
      swap_region_chunk(region_chunk);
    // offset the data pointer by 2*uint32_t to point to start of mRegions
    byte_ptr_regions += 2*sizeof(uint32_t);
    for (i=0; i<ambix->num_regions;i++) {
      CAFRegion *new_region   = NULL;
      CAFMarker *start_marker = NULL, *end_marker = NULL;

      /* region */
      new_region = (CAFRegion*) byte_ptr_regions;
      new_region->mRegionID = i+1; // does not have a connection with a string
      new_region->mFlags = 0;
      new_region->mNumberMarkers = 2; // start, end marker

      /* start region marker */
      byte_ptr_regions += SIZEOF_CAFRegion; // offset pointer to start marker
      start_marker = (CAFMarker*)byte_ptr_regions;// &((new_region->mMarkers)[0]);
      start_marker->mType = kCAFMarkerType_RegionStart;
      start_marker->mFramePosition = ambix->regions[i].start_position;
      start_marker->mMarkerID = ambix->num_markers+i+1; // string ID -> num_markers+1...num_markers+num_regions
      start_marker->mChannel = 0; // 0 means for all channels

      /* end region marker */
      byte_ptr_regions += sizeof(CAFMarker); // offset pointer to end marker
      end_marker = (CAFMarker*)byte_ptr_regions; // &new_region->mMarkers[1];
      end_marker->mType = kCAFMarkerType_RegionEnd;
      end_marker->mFramePosition = ambix->regions[i].end_position;
      end_marker->mMarkerID = ambix->num_markers+i+1; // string ID -> num_markers+1...num_markers+num_regions
      end_marker->mChannel = 0; // 0 means for all channels
      if (byteswap) {
        swap_region(new_region);
        swap_marker(start_marker);
        swap_marker(end_marker);
      }
      byte_ptr_regions += sizeof(CAFMarker);

      add_string_to_data(ambix->num_markers+i+1, byte_ptr_stringid, ambix->regions[i].name, &byteoffset_strings, byte_ptr_strings, &datasize_strings, byteswap);
      byte_ptr_strings += (strlen((const char*)ambix->regions[i].name)+1);
      byte_ptr_stringid += sizeof(CAFStringID);
    }
  }

  /* add the chunk data */
  do {
    union int_chars mark_id, regn_id, strg_id;

    mark_id.b[0] = 'm'; mark_id.b[1] = 'a'; mark_id.b[2] = 'r'; mark_id.b[3] = 'k';
    _ambix_write_chunk(ambix, mark_id.a, marker_data, datasize_markers);
    free(marker_data);

    regn_id.b[0] = 'r'; regn_id.b[1] = 'e'; regn_id.b[2] = 'g'; regn_id.b[3] = 'n';
    _ambix_write_chunk(ambix, regn_id.a, region_data, datasize_regions);
    free(region_data);

    strg_id.b[0] = 's'; strg_id.b[1] = 't'; strg_id.b[2] = 'r'; strg_id.b[3] = 'g';
    _ambix_write_chunk(ambix, strg_id.a, strings_data, datasize_strings);
    free(strings_data);
  } while(0);
  return AMBIX_ERR_SUCCESS;
}
