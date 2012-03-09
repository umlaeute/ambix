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

/** 32bit floating point number */
typedef float float32_t;

#ifdef HAVE_STDINT_H
# include <stdint.h>
#else
/** 32bit signed integer  */
typedef signed int int32_t;
/** 32bit unsigned integer  */
typedef unsigned int uint32_t;
/** 64bit signed integer  */
typedef signed long int64_t;
/** 64bit unsigned integer  */
typedef unsigned long uint64_t;
#endif

/** opaque handle to an ambix file */
typedef struct ambix_t ambix_t;


/** error codes returned by functions */
typedef enum
{	
  /** no error encountered */
  AMBIX_ERR_SUCCESS			= 0,
  /** an invalid ambix handle was passed to the function */
  AMBIX_ERR_INVALID_HANDLE,
  /** the file in question is invalid (e.g. doesn't contain audio) */
  AMBIX_ERR_INVALID_FILE,

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
} ambix_fileformat_t;

/** ambix sample formats */
typedef enum {
/** unknown (or illegal) sample formats */
  AMBIX_SAMPLEFORMAT_NONE=0,
  /** signed 16 bit integer */
  AMBIX_SAMPLEFORMAT_PCM16,
  /** signed 24 bit integer */
  AMBIX_SAMPLEFORMAT_PCM24,
  /** signed 32 bit integer */
  AMBIX_SAMPLEFORMAT_PCM32,
  /** 32 bit floating point */
  AMBIX_SAMPLEFORMAT_FLOAT32,
} ambix_sampleformat_t;


/** a 2-dimensional floating point matrix */
typedef struct ambixmatrix_t {
  /** number of rows */
  uint32_t rows;
  /** number of columns */
  uint32_t cols;
  /** matrix data (as vector (length: rows) of row-vectors (length: cols)) */
  float32_t**data;
} ambixmatrix_t;

/** @brief Initialize a matrix
 *
 * Allocates memory for matrix-data of given dimensions
 *
 * @param rows number of rows in the newly initialized matrix
 * @param cols number of columns in the newly initialized matrix
 * @param mtx pointer to a matrix object; if NULL a new matrix object will be created, else the given matrix object will be re-initialized
 * @return pointer to a newly initialized (and/or allocated) matrix, or NULL on error
 */
AMBIX_API
ambixmatrix_t*ambix_matrix_init(uint32_t rows, uint32_t cols, ambixmatrix_t*mtx);
/** @brief De-initialize a matrix
 *
 * Frees associated ressources and sets rows/columns to 0
 *
 * @param mtx matrix object to deinitialize
 */
AMBIX_API
void ambix_matrix_deinit(ambixmatrix_t*mtx);
/** @brief Fill a matrix with values
 *
 * Fill data into a properly initialized matrix
 *
 * @param mtx initialized matrix object to copy data into
 * @param data pointer to at least (mtx->rows*mtx->cols) values; data is ordered row-by-row with no padding (A[0,0], A[0,1], .., A[0,cols-1],  A[1, 0], .. A[rows-1, cols-1])
 * @return an error code indicating success
 */
AMBIX_API
int ambix_matrix_fill(ambixmatrix_t*mtx, float32_t*data);


/** @brief Calculate the number of channels for a full 3d ambisonics set of a given order
 *
 * @param order the order of the full set
 * @return the number of channels of the full set
 */
AMBIX_API
uint32_t ambix_order2channels(uint32_t order);
/** @brief Calculate the order of a full 3d ambisonics set fora gien number of channels
 *
 * @param channels the number of channels of the full set
 * @return the order of the full set, or -1 if the channels don't form a full set
 */
AMBIX_API
int32_t ambix_channels2order(uint32_t channels);
/** @brief Checks whether the channel can form a full 3 ambisonics set
 *
 * @param channels the number of channels supposed to form a full set
 * @return TRUE if the channels can form full set, FALSE otherwise
 */
AMBIX_API
int ambix_isFullSet(uint32_t channels);



/** this is for passing data about the opened ambix file between the host application and the library */
typedef struct ambixinfo_t {
  /** number of frames in the file */
  uint64_t  frames;
  /** samplerate in Hz */
  double			samplerate;
  /** type of the ambix file */
  ambix_sampleformat_t sampleformat;

  /** type of the ambix file */
  ambix_fileformat_t ambixfileformat;
  /** number of (raw) ambisonics channels present in the file
   * if the file contains a full set of ambisonics channels (always true if ambixformat==AMBIX_SIMPLE),
   * then ambichannels=(ambiorder+1)^2;
   * if the file contains a reduced set (ambichannels<(ambiorder+1)^2) you can reconstruct the full set by
   * multiplying the reduced set with the reconstruction matrix */
	uint32_t			ambichannels;
  /** number of non-ambisonics channels in the file */
	uint32_t			otherchannels;
} ambixinfo_t;

/** @brief Open an ambix file
 *
 * Opens a soundfile for reading/writing
 *
 * @param path filename of the file to open
 * @param mode whether to open the file for reading and/or writing (AMBIX_READ, AMBIX_WRITE, AMBIX_READ | AMBIX_WRITE)
 * @param ambixinfo pointer to a valid ambixinfo_t structure
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
AMBIX_API
ambix_t* 	ambix_open	(const char *path, const ambix_filemode_t mode, ambixinfo_t*ambixinfo) ;

/** @brief Close an ambix handle
 *
 * Closes an ambix handle and cleans up all memory allocations associated with it.
 *
 * @param ambix The handle to an ambix file
 * @return an error code indicating success
 */
AMBIX_API
ambix_err_t	ambix_close	(ambix_t*ambix);


/**
 * typedef from libsndfile
 * @private
 */
typedef struct SNDFILE_tag SNDFILE;

/** @brief get the libsndfile handle associated with the ambix handle
 *
 * If possible, require an SNDFILE handle if possible; 
 * if the ambix handle is not asociated with SNDFILE (e.g. because libambix is compiled without libsndfile support),
 * NULL is returned
 *
 * @param ambix The handle to an ambix file
 * @return A libsndfile handle or NULL
 */
AMBIX_API
SNDFILE*ambix_get_sndfile	(ambix_t*ambix);






#ifdef __cplusplus
//}		/* extern "C" */
#endif	/* __cplusplus */
#endif /* AMBIX_AMBIX_H */
