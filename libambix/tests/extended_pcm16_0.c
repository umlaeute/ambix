/* extended_pcm16_0 - test ambix extended (PCM16, blocksize 0)

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

#include "common.h"
#include <unistd.h>
#include <string.h>


void check_create_extended(const char*path, ambix_sampleformat_t format, uint32_t chunksize, float32_t eps);

int main(int argc, char**argv) {
  check_create_extended("test2-pcm16.caf",  AMBIX_SAMPLEFORMAT_PCM16, 1024, 1./20000.);

  pass();
  return 0;
}
