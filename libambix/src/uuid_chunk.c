/* uuid_chunk.c -  parse an UUID-chunk to extract relevant data              -*- c -*-

   Copyright © 2012-2016 IOhannes m zmölnig <zmoelnig@iem.at>.
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

#ifdef HAVE_STRING_H
# include <string.h>
#endif /* HAVE_STRING_H */
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif /* HAVE_STDLIB_H */

/* on UUIDs:
 * ideally, we would use UUIDs based on an URI that indicates the format-version
 * e.g. URI="http://ambisonics.iem.at/xchange/format/1.0"
 * this can be converted to an UUID(v5), using a small perl-script like
 * @code
 #!/usr/bin/perl
 use UUID::Tiny;
 my $v5_url_UUIDs    = create_UUID_as_string(UUID_V5, UUID_NS_URL, $ARGV[0]);
 print "$v5_url_UUIDs\n";
 * @endcode
 *
 * this gives us:
 *  http://ambisonics.iem.at/xchange/format/1.0 1ad318c3-00e5-5576-be2d-0dca2460bc89
 *  http://ambisonics.iem.at/xchange/format/2.0 7c449194-05e1-58e4-9463-d0f8f2a1ed0f
 *  http://ambisonics.iem.at/xchange/format/3.0 50c26359-d96d-5a67-bc19-db124f7606d0
 */

#include <stdio.h>

#if 0
static void _ambix_printUUID4(const char data[2]) {
  unsigned char data0=data[0];
  unsigned char data1=data[1];
  printf("%02x%02x", data0, data1);
}
static void _ambix_printUUID(const char data[16]) {
  /* 8-4-4-4-12 */
  _ambix_printUUID4(data);data+=2;
  _ambix_printUUID4(data);data+=2;
  printf("-");
  _ambix_printUUID4(data);data+=2;
  printf("-");
  _ambix_printUUID4(data);data+=2;
  printf("-");
  _ambix_printUUID4(data);data+=2;
  printf("-");
  _ambix_printUUID4(data);data+=2;
  _ambix_printUUID4(data);data+=2;
  _ambix_printUUID4(data);data+=2;
}
#endif
/*
 * uarg, this is not a UUID!
 * (well it is...UUID::Tiny thinks it's a v2 (DCE security) UUID
 */
static const char _ambix_uuid_v1_[]="IEM.AT/AMBIX/XML";
/*
 * that's a better UUID, based on a SHA1-hash of "http://ambisonics.iem.at/xchange/format/1.0"
 */
static const char _ambix_uuid_v1[]={0x1a, 0xd3, 0x18, 0xc3, 0x00, 0xe5, 0x55, 0x76, 0xbe, 0x2d, 0x0d, 0xca, 0x24, 0x60, 0xbc, 0x89};
const char* _ambix_getUUID(uint32_t version) {
  switch(version) {
  default:
    break;
  case 1:
    return _ambix_uuid_v1;
  }
  return NULL;
}

uint32_t
_ambix_checkUUID(const char data[16]) {
  if(!memcmp(data, _ambix_uuid_v1, 16))
    return 1;
  /* compat mode: old AMBIXv1 UUID */
  if(!memcmp(data, _ambix_uuid_v1_, 16))
    return 1;
  return 0;
}

ambix_matrix_t*
_ambix_uuid1_to_matrix(const void*vdata, uint64_t datasize, ambix_matrix_t*orgmtx, int swap) {
  const char*cdata=(const char*)vdata;
  ambix_matrix_t*mtx=orgmtx;
  uint32_t rows;
  uint32_t cols;
  uint64_t size;
  uint32_t index;

  if(datasize<(sizeof(rows)+sizeof(cols)))
    return NULL;

  index = 0;

  memcpy(&rows, cdata+index, sizeof(uint32_t));
  index += sizeof(uint32_t);

  memcpy(&cols, cdata+index, sizeof(uint32_t));
  index += sizeof(uint32_t);

  if(swap) {
    rows=swap4(rows);
    cols=swap4(cols);
  }

  size=(uint64_t)rows*cols;

  if(rows<1 || cols<1 || size < 1)
    goto cleanup;

  if(size*sizeof(float32_t) > datasize) {
    goto cleanup;
  }

  if(!mtx) {
    mtx=(ambix_matrix_t*)calloc(1, sizeof(ambix_matrix_t));
    if(!mtx)
      goto cleanup;
  }

  if(!ambix_matrix_init(rows, cols, mtx))
    goto cleanup;

  if(swap) {
    if(_ambix_matrix_fill_data_byteswapped(mtx, (number32_t*)(cdata+index)) != AMBIX_ERR_SUCCESS)
      goto cleanup;
  } else {
    if(ambix_matrix_fill_data(mtx, (float32_t*)(cdata+index)) != AMBIX_ERR_SUCCESS)
      goto cleanup;
  }

  return mtx;

 cleanup:
  if(mtx && mtx!=orgmtx) {
    ambix_matrix_deinit(mtx);
    free(mtx);
    mtx=NULL;
  }
  return NULL;
}


uint64_t
_ambix_matrix_to_uuid1(const ambix_matrix_t*matrix, void*vdata, int swap) {
  char*cdata=(char*)vdata;
  const char*uuid=_ambix_getUUID(1);
  uint64_t index=0;
  uint64_t datasize=0;
  uint32_t rows=matrix->rows;
  uint32_t cols=matrix->cols;
  float32_t**mtx=matrix->data;
  if(!uuid)
    return 0;
  datasize+=16; /* reserved for the UUID */

  datasize+=sizeof(uint32_t); /* rows */
  datasize+=sizeof(uint32_t); /* cols */
  datasize+=(uint64_t)rows*(uint64_t)cols*sizeof(float32_t); /* data */

  if(vdata) {
    uint32_t*uidata=(uint32_t*)vdata;
    uint64_t r;
    uint32_t*swapdata;
    uint64_t elements=(uint64_t)rows*(uint64_t)cols;
    memcpy(cdata, uuid, 16);
    index+=16;

    //swapdata=cdata+index;
    swapdata=uidata+(index / sizeof(uint32_t));

    memcpy(cdata+index, &rows, sizeof(uint32_t));
    index+=sizeof(uint32_t);

    memcpy(cdata+index, &cols, sizeof(uint32_t));
    index+=sizeof(uint32_t);

    for(r=0; r<rows; r++) {
      memcpy(cdata+index, mtx[r], cols*sizeof(float32_t));
      index+=cols*sizeof(float32_t);
    }

    if(swap) {
      _ambix_swap4array(swapdata, elements+2);
    }
  }
  return datasize;
}
