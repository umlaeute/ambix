/* ambix/types.h - Ambisonics Xchange - Type Definitions	-*- c -*-

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
#endif /* __cplusplus */

/** 32bit floating point number */
typedef float float32_t;

#if defined (__linux__) || defined(__apple__)
# include <stdint.h>
#else
/** 16bit signed integer */
typedef signed short int16_t;
/** 16bit unsigned integer */
typedef unsigned short uint16_t;
/** 32bit signed integer */
typedef signed int int32_t;
/** 32bit unsigned integer */
typedef unsigned int uint32_t;
/** 64bit signed integer */
typedef signed long int64_t;
/** 64bit unsigned integer */
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
  /** an unknown error */
  AMBIX_ERR_UNKNOWN=-1,

  /** no error encountered */
  AMBIX_ERR_SUCCESS= 0,

  /** an invalid ambix handle was passed to the function */
  AMBIX_ERR_INVALID_HANDLE,
  /** the file in question is invalid (e.g. doesn't contain audio) */
  AMBIX_ERR_INVALID_FILE,
  /** matrix dimension mismatch */
  AMBIX_ERR_INVALID_DIMENSION,
  /** the ambix handle is in a format that does not allow the function (e.g. setting a premultiply matrix for a format other than AMBIX_SIMPLE) */
  AMBIX_ERR_INVALID_FORMAT,

  /** you specified an invalid matrix */
  AMBIX_ERR_INVALID_MATRIX,

} ambix_err_t;


/** error codes returned by functions */
typedef enum {
  /** open file for reading */
  AMBIX_READ = (1<<0),
  /** open file for writing */
  AMBIX_WRITE = (1<<1),
  /** open file for reading&writing */
  AMBIX_RDRW = (AMBIX_READ|AMBIX_WRITE)

} ambix_filemode_t;


/** ambix file types */
typedef enum {
  /** file is not an ambix file (or unknown) */
  AMBIX_NONE     = 0,
  /** simple ambix file (w/ pre-multiplication matrix) */
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


/** ambix matrix types */
typedef enum {
  /** invalid matrix format */
  AMBIX_MATRIX_INVALID = -1,
  /** a matrix filled with zeros */
  AMBIX_MATRIX_ZERO = 0,
  /** a matrix filled with ones */
  AMBIX_MATRIX_ONE = 1,
  /** an identity matrix (diagonal is 1, rest is 0) */
  AMBIX_MATRIX_IDENTITY,

  /** conversion matrix Furse-Malham -> ambix */
  AMBIX_MATRIX_FUMA,
} ambix_matrixtype_t;



/** a 2-dimensional floating point matrix */
typedef struct ambix_matrix_t {
  /** number of rows */
  uint32_t rows;
  /** number of columns */
  uint32_t cols;
  /** matrix data (as vector (length: rows) of row-vectors (length: cols)) */
  float32_t**data;
} ambix_matrix_t;

/** this is for passing data about the opened ambix file between the host
 * application and the library
*/
typedef struct ambix_info_t {
  /** number of frames in the file */
  uint64_t frames;
  /** samplerate in Hz */
  double samplerate;
  /** sample type of the ambix file */
  ambix_sampleformat_t sampleformat;
  /** layout type of the ambix file */
  ambix_fileformat_t fileformat;

  /** number of non-ambisonics channels in the file
   * @remark think of a better name, like 'uncodedchannels'
   */
  uint32_t extrachannels;

  /** number of (raw) ambisonics channels present in the file.
   *
   * If the file contains a full set of ambisonics channels (always true if
   * ambixformat==AMBIX_SIMPLE), then \f$ambichannel=(order_{ambi}+1)^2\f$; if
   * the file contains an adaptor matrix, it has to be used to reconstruct the
   * full set by multiplying the adaptor matrix with the channels present.
   */
  uint32_t ambichannels;
} ambix_info_t;

/**
 * typedef from libsndfile
 * @private
 */
typedef struct SNDFILE_tag SNDFILE;

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */
#endif /* AMBIX_TYPES_H */
