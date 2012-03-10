/* utils.c -  various utilities              -*- c -*-

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

#include "private.h"
#include <math.h>

uint32_t ambix_order2channels(uint32_t order) {
  /* L=(N+1)^2 */
  return (order+1)*(order+1);
}
int32_t ambix_channels2order(uint32_t channels) {
  /* L=(N+1)^2 */
  int32_t order1=(int32_t)sqrt((double)channels);
  
  if(order1*order1==channels) { /* expanded set must be a full set */
    return order1-1;
  }

  return -1;
}

int ambix_isFullSet(uint32_t channels) {
  return (ambix_order2channels(ambix_channels2order(channels))==channels);
}


void _ambix_print_info(const ambixinfo_t*info) {
  printf("AMBIX_INFO 0x%X\n", info);
  if(!info)return;
  printf("  frames\t: %d\n", info->frames);
  printf("  samplerate\t: %f\n", info->samplerate);
  printf("  sampleformat\t: %d\n", info->sampleformat);
  printf("  ambixfileformat\t: %d\n", info->ambixfileformat);
  printf("  ambichannels\t: %d\n", info->ambichannels);
  printf("  otherchannels\t: %d\n", info->otherchannels);
}


void _ambix_swap4array(uint32_t*data, uint64_t datasize) {
  uint64_t i;
  for(i=0; i<datasize; i++) {
    uint32_t v=*data;
    *data++=swap4(v);
  }
}
