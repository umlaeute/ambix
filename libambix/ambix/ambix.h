/** 
 * @file	ambix.h
 * @brief	Ambisonics Xchange Library Interface
 * @detail This file is part of libambix
 * @author IOhannes m zmölnig <zmoelnig@iem.at>
 * @date 2012
 * @copyright (C) 2012 IOhannes m zmölnig <zmoelnig@iem.at>, Institute of Electronic Music and Acoustics (IEM), University of Music and Performing Arts Graz (KUG)
 *   libambix is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as
 *   published by the Free Software Foundation; either version 2.1 of
 *   the License, or (at your option) any later version.
 *   .
 *   Libgcrypt is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *   .
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef AMBIX_AMBIX_H
#define AMBIX_AMBIX_H

#include "exportdedfs.h"

#ifdef __cplusplus
//extern "C" {
#endif	/* __cplusplus */

typedef struct ambix_ ambix_t;



typedef enum {
  AMBIX_READ  = (1<<0),
  AMBIX_WRITE = (1<<1)
} ambix_filemode_t;

typedef enum {
  AMBIX_NONE     = 0, /* file is no ambix file */
  AMBIX_SIMPLE   = 1, /* simple ambix file   (w/ pre-multiplication matrix) */
  AMBIX_EXTENDED = 2  /* extended ambix file (w pre-multiplication matrix ) */
} ambix_filetype_t;



typedef struct ambixinfo_ {
  size_t  frames;
  int			samplerate;

	int			format;
	int			sections;
	int			seekable;

  ambix_filetype_t ambixformat;
	int			ambichannels;
	int			otherchannels;
} ambixinfo_t;

/** @brief Open an ambix file
 *
 * Opens a soundfilek for reading/writing
 *
 * @param path filename of the file to open
 * @param mode whether to open the file for reading and/or writing (AMBIX_READ, AMBIX_WRITE, AMBIX_READ | AMBIX_WRITE)
 * @param ambixinfo 
 * @return A handle to the opened file (or NULL on failure)
 */
ambix_t* 	ambix_open	(const char *path, const ambix_filemode_t mode, ambixinfo_t*ambixinfo) ;

/** @brief Close an ambix handle
 *
 * Closes an ambix handle and cleans up all memory allocations associated with it.
 *
 * @param ambix The handle to an ambix file
 * @return an error code (0 == sucess)
 */
int	ambix_close	(ambix_t*ambix);






#ifdef __cplusplus
//}		/* extern "C" */
#endif	/* __cplusplus */
#endif /* AMBIX_AMBIX_H */
