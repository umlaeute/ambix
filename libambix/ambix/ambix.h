/* ambix/ambix.h -  Ambisonics Xchange Library Interface              -*- c -*-

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
 * @file	ambix/ambix.h
 * @brief	Ambisonics Xchange Library Interface
 * @details This file is part of libambix
 * @author IOhannes m zmölnig <zmoelnig@iem.at>
 * @date 2012
 * @copyright LGPL-2.1
 */
#ifndef AMBIX_AMBIX_H
#define AMBIX_AMBIX_H

#include "exportdefs.h"

#ifdef __cplusplus
//extern "C" {
#endif	/* __cplusplus */

/** opaque handle to an ambix file */
typedef struct ambix_t ambix_t;


/** error codes returned by functions */
typedef enum
{	
  /** no error encountered */
  AMBIX_ERR_SUCCESS			= 0,
  /** an invalid ambix handle was passed to the function */
  AMBIX_ERR_INVALID_HANDLE,
} ambix_err_t;


/** error codes returned by functions */
typedef enum {
  /** open file for reading */
  AMBIX_READ  = (1<<0),
  /** open file for writing  */
  AMBIX_WRITE = (1<<1)
} ambix_filemode_t;


/** ambix file types */
typedef enum {
  /** file is not an ambix file (or unknown) */
  AMBIX_NONE     = 0, 
  /** simple ambix file   (w/ pre-multiplication matrix) */
  AMBIX_SIMPLE   = 1,
  /** extended ambix file (w pre-multiplication matrix ) */
  AMBIX_EXTENDED = 2
} ambix_filetype_t;


/** this is for passing data about the opened ambix file between the host application and the library */
typedef struct ambixinfo_t {
  /** number of frames in the file */
  unsigned long  frames;
  /** samplerate in Hz */
  int			samplerate;

  /** type of the ambix file */
  ambix_filetype_t ambixformat;
  /** number of (raw) ambisonics channels present in the file
   * if the file contains a full set of ambisonics channels (always true if ambixformat==AMBIX_SIMPLE),
   * then ambichannels=(ambiorder+1)^2;
   * if the file contains a reduced set (ambichannels<(ambiorder+1)^2) you can reconstruct the full set by
   * multiplying the reduced set with the reconstruction matrix */
	int			ambichannels;
  /** number of non-ambisonics channels in the file */
	int			otherchannels;
} ambixinfo_t;

/** @brief Open an ambix file
 *
 * Opens a soundfile for reading/writing
 *
 * @param path filename of the file to open
 * @param mode whether to open the file for reading and/or writing (AMBIX_READ, AMBIX_WRITE, AMBIX_READ | AMBIX_WRITE)
 * @param ambixinfo pointer to a valid ambixinfo_t structure;
 * @remark
 *    when opening a file for reading, the structure should be initialized to zero before calling ambix_open():
 *    the fields will be set by the library; if you set the ambixinfo_t.ambixformat field to something else than AMBIX_NONE, 
 *    the library will present the data as if the was written in this format (e.g. if you set ambixinfo_t.ambixformat:=AMBIX_SIMPLE
 *    but the file really is AMBIX_EXTENDED, the library will automatically pre-multiply the reconstruction matrix to
 *    give you the full ambisonics set.
 * @remark
 *   when opening a file for writing, the caller must set the fields; if ambixinfo_t.ambixformat is AMBIX_NONE, than ambixinfo_t.ambixchannels must be 0,
 *   else ambixinfo_t.ambichannels must be >0; if ambixinfo_t.ambixformat is AMBIX_SIMPLE, then ambixinfo_t.ambichannels must be (ambiorder+1)^2
 * @return A handle to the opened file (or NULL on failure)
 */
ambix_t* 	ambix_open	(const char *path, const ambix_filemode_t mode, ambixinfo_t*ambixinfo) ;

/** @brief Close an ambix handle
 *
 * Closes an ambix handle and cleans up all memory allocations associated with it.
 *
 * @param ambix The handle to an ambix file
 * @return an error code indicating success
 */
ambix_err_t	ambix_close	(ambix_t*ambix);






#ifdef __cplusplus
//}		/* extern "C" */
#endif	/* __cplusplus */
#endif /* AMBIX_AMBIX_H */
