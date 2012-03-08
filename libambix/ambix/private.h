/* ambix/private.h -  Ambisonics Xchange Library Private Interface              -*- c -*-

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

/** 
 * @file	ambix/private.h
 * @brief	Ambisonics Xchange Library Private Interface
 * @details This file is part of libambix
 * @author IOhannes m zmölnig <zmoelnig@iem.at>
 * @date 2012
 * @copyright LGPL-2.1
 * @internal
 */

#ifndef AMBIX_INTERNAL
# error ambix/private.h must only be used from within libambix
#endif


#include "ambix/ambix.h"
#ifdef HAVE_SNDFILE_H
# include <sndfile.h>
#endif /* HAVE_SNDFILE_H */

/** this is for passing data about the opened ambix file between the host application and the library */
typedef struct ambix_t {
#ifdef HAVE_SNDFILE_H
  /** handle to the libsndfile object */
  SNDFILE*sf_file;
  /** libsndfile info as returned by sf_open() */
  SF_INFO sf_info;
#endif /* HAVE_SNDFILE_H */

  /** ambisonics info chunk */
  ambixinfo_t info;

  /** ambisonics order of the full set */
  uint32_t ambisonics_order;

  /** matrix rows (number of channels in full set; always a square number: matrix_rows=(ambisonics_order+1)^2) */
  uint32_t matrix_rows;
  /** matrix columns (number of channels in reduced set) */
  uint32_t matrix_cols;
  /** reconstruction matrix (vector of row-vectors) */
  float32_t**matrix;

  
} ambix_t;

