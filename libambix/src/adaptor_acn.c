/* adapator_acn.c -  adaptors to/from ACN channel ordering           -*- c -*-

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

int _matrix_sid2acn(float32_t*data, uint32_t count) {
  float32_t*datap=data;
  int32_t order=ambix_channels2order(count);
  int32_t o;
  if(order<0)return 0;

  for(o=0; o<=order; o++) {
    uint32_t offset=o>0?ambix_order2channels(o-1):0;
    int32_t maxindex=ambix_order2channels(o)-offset;

    int32_t index;

    for(index=1; index<maxindex; index+=2) {
      *datap++=(float32_t)(index+offset);
    }
    for(index=maxindex-1; index>=0; index-=2) {
      *datap++=(float32_t)(index+offset);
    }
  }
  return 1;
}
