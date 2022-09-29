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

ambix_err_t _ambix_open (ambix_t*ambix, const char *path, const ambix_filemode_t mode, const ambix_info_t*ambixinfo) {
  return AMBIX_ERR_INVALID_FILE;
}

ambix_err_t     _ambix_close    (ambix_t*ambix) {
  return AMBIX_ERR_INVALID_FILE;
}

SNDFILE*_ambix_get_sndfile   (ambix_t*ambix) {
  return 0;
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
int64_t _ambix_readf_float64   (ambix_t*ambix, float64_t*data, int64_t frames) {
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
int64_t _ambix_writef_float64   (ambix_t*ambix, const float64_t*data, int64_t frames) {
  return -1;
}
ambix_err_t _ambix_write_uuidchunk(ambix_t*ax, const void*data, int64_t datasize) {
  return  AMBIX_ERR_UNKNOWN;
}
int64_t _ambix_seek (ambix_t* ambix, int64_t frames, int whence) {
  return -1;
}
