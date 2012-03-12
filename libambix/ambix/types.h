/* ambix/types.h -  Ambisonics Xchange - Type Definitions              -*- c -*-

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
 * @file	ambix/types.h
 * @brief	Ambisonics Xchange Type Definitions
 * @details This file is part of libambix
 * @author IOhannes m zmölnig <zmoelnig@iem.at>
 * @date 2012
 * @copyright LGPL-2.1
 */

#ifndef AMBIX_AMBIX_H
# error please dont include <ambix/types.h>...use <ambix/ambix.h> instead!
#endif

#ifndef AMBIX_TYPES_H
#define AMBIX_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

/** 32bit floating point number */
typedef float float32_t;

#ifdef HAVE_STDINT_H
# include <stdint.h>
#else
/** 16bit signed integer  */
typedef signed short int16_t;
/** 16bit unsigned integer  */
typedef unsigned short uint16_t;
/** 32bit signed integer  */
typedef signed int int32_t;
/** 32bit unsigned integer  */
typedef unsigned int uint32_t;
/** 64bit signed integer  */
typedef signed long int64_t;
/** 64bit unsigned integer  */
typedef unsigned long uint64_t;
#endif

/** a 32bit number (either float or int), useful for endianess operations */
typedef union {
  /** 32bit floating point */
  float32_t f;
  /** 32bit signed integer */
  int32_t   i;
  /** 32bit unsigned integer */
  uint32_t  u;
} number32_t;


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
  /** matrix dimension mismatch */
  AMBIX_ERR_INVALID_DIMENSION,
  /** the ambix handle is in a format that does not allow the function (e.g. setting a premultiply matrix for a format other than AMBIX_SIMPLE) */
  AMBIX_ERR_INVALID_FORMAT,
  /** an unknown error */
  AMBIX_ERR_UNKNOWN=-1

} ambix_err_t;


/** error codes returned by functions */
typedef enum {
  /** open file for reading */
  AMBIX_READ  = (1<<0),
  /** open file for writing  */
  AMBIX_WRITE = (1<<1),
  /** open file for reading&writing  */
  AMBIX_RRW = (AMBIX_READ|AMBIX_WRITE)

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

/**
 * typedef from libsndfile
 * @private
 */
typedef struct SNDFILE_tag SNDFILE;

#ifdef __cplusplus
}		/* extern "C" */
#endif	/* __cplusplus */
#endif /* AMBIX_TYPES_H */
