/* private.h -  Ambisonics Xchange Library Private Interface              -*- c -*-

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
 * @file	private.h
 * @brief	Ambisonics Xchange Library Private Interface
 * @details This file is part of libambix
 * @author IOhannes m zmölnig <zmoelnig@iem.at>
 * @date 2012
 * @copyright LGPL-2.1
 * @internal
 */

#ifndef AMBIX_INTERNAL
# error private.h must only be used from within libambix
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

  /** reconstruction matrix */
  ambixmatrix_t matrix;
} ambix_t;


/** @brief Do open an ambix file
 *
 * this is implemented by the various backends (currently only libsndfile)
 *
 * @param ambix a pointer to an allocated ambix structure, that get's filled by this call
 * @param path filename of the file to open
 * @param mode open read/write
 * @param ambixinfo struct to a valid ambixinfo structure
 * @return errorcode indicating success
 */
ambix_err_t	_ambix_open	(ambix_t*ambix, const char *path, const ambix_filemode_t mode, ambixinfo_t*ambixinfo);
/** @brief Do close an ambix file
 *
 * this is implemented by the various backends (currently only libsndfile)
 *
 * @param ambix a pointer to a valid ambix structure (has to be freed outside)
 * @return errorcode indicating success
 */
ambix_err_t	_ambix_close	(ambix_t*ambix);
/** @brief Do get an libsndfile handle
 *
 * this is implemented by the various backends (currently only libsndfile)
 *
 * @param ambix a pointer to a valid ambix structure
 * @return an SNDFILE handle or NULL if not possible
 */

SNDFILE*_ambix_get_sndfile	(ambix_t*ambix);



/** @brief Check data for ambix UUID
 * @param data Array holding the UUID
 * @return ambix-version this UUID is associated with, or 0 on failure
 * @remark currently only one UUID is defined for the ambix format (version 1)
 *         future versions of the standard might add additional UUIDs
 */
uint32_t _ambix_parseuuid(const char UUID[16]);

/** @brief extract matrix from ambix UUID-chunk (v1)
 * @param data Array holding the payload data (excluding the UUID itself)
 * @param datasize size of data
 * @param mtx pointer to a matrix object that should be filled (if NULL, this function will allocate a matrix object for you)
 * @return a pointer to the filled matrix or NULL on failure
 * @remark only use data from a uuid-chunk for which _ambix_parseuuid() that returned '1' 
 */
ambixmatrix_t*_ambix_uuid1_to_matrix(const void*data, uint64_t datasize, ambixmatrix_t*mtx);
