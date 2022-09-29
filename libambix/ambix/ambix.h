/* ambix/ambix.h -  AMBIsonics eXchange Library Interface              -*- c -*-

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
 * @file        ambix/ambix.h
 * @brief       AMBIsonics eXchange Library Interface
 * @details This file is part of libambix
 * @author IOhannes m zmölnig <zmoelnig@iem.at>
 * @date 2012
 * @copyright LGPL-2.1
 */
#ifndef AMBIX_AMBIX_H
#define AMBIX_AMBIX_H

#include "exportdefs.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** 32bit floating point number */
typedef float float32_t;
    
/** 64bit floating point number */
typedef double float64_t;

#ifdef _MSC_VER
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
#else
# include <stdint.h>
#endif

/** @cond skip */
  /* ugly hack to provide to allow the use type 'SNDFILE*'
   * regardless of whether <sndfile.h> is included or not
   */
#ifndef HAVE_PUSH_MACRO
# define HAVE_PUSH_MACRO 1
#endif
#if HAVE_PUSH_MACRO
# pragma push_macro("SNDFILE")
# ifdef SNDFILE
#  undef SNDFILE
# endif
# define SNDFILE void
#elif !defined(SNDFILE_1)
# define SNDFILE void
#endif
/** @endcond */


/** a 32bit number (either float or int), useful for endianness operations */
typedef union {
  /** 32bit floating point */
  float32_t f;
  /** 32bit signed integer */
  int32_t   i;
  /** 32bit unsigned integer */
  uint32_t  u;
} number32_t;

/** opaque handle to an ambix file */
typedef struct ambix_t_struct ambix_t;

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
  /** the ambix handle is in a format that does not allow the function (e.g.
   * setting a premultiply matrix for a format other than AMBIX_BASIC)
   */
  AMBIX_ERR_INVALID_FORMAT,

  /** you specified an invalid matrix */
  AMBIX_ERR_INVALID_MATRIX,

} ambix_err_t;

/** error codes returned by functions */
typedef enum {
  /** open file for reading */
  AMBIX_READ  = (1 << 4),
  /** open file for writing */
  AMBIX_WRITE = (1 << 5),
  /** open file for reading&writing */
  AMBIX_RDRW = (AMBIX_READ|AMBIX_WRITE)

} ambix_filemode_t;

/** ambix file types */
typedef enum {
  /** file is not an ambix file (or unknown) */
  AMBIX_NONE     = 0,
  /** basic ambix file (w/ pre-multiplication matrix) */
  AMBIX_BASIC   = 1,
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
  /** 64 bit floating point */
  AMBIX_SAMPLEFORMAT_FLOAT64,
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

  /** matrices with the 0x8000 bit set convert between ambix and other ambisonics formats:
   * if the 0x4000 bit is set to 0, the matrix converts to ambix,
   * if the 0x4000 bit is set to 1, the matrix converts from ambix.
   * @remark some of the following matrixes might not be implemented yet
   * @remark AMBIX_MATRIX_AMBIX converts from ambix to ambix (and is quite useless by itself)
   */
  AMBIX_MATRIX_AMBIX = 0x8000,
  /** conversion matrix N3D -> SN3D */
  AMBIX_MATRIX_N3D  =  1 | AMBIX_MATRIX_AMBIX,
  /** conversion matrix SID -> ACN */
  AMBIX_MATRIX_SID  =  2 | AMBIX_MATRIX_AMBIX,
  /** conversion matrix Furse-Malham -> ambix */
  AMBIX_MATRIX_FUMA =  3 | AMBIX_MATRIX_AMBIX,

  /** back conversion matrix ambix -> ambix */
  AMBIX_MATRIX_TO_AMBIX = 0x4000 | AMBIX_MATRIX_AMBIX,

  /** conversion matrix SN3D -> N3D */
  AMBIX_MATRIX_TO_N3D  = AMBIX_MATRIX_TO_AMBIX | AMBIX_MATRIX_N3D,
  /** conversion matrix ACN -> SID */
  AMBIX_MATRIX_TO_SID  = AMBIX_MATRIX_TO_AMBIX | AMBIX_MATRIX_SID,
  /** conversion matrix ambix -> Furse-Malham */
  AMBIX_MATRIX_TO_FUMA = AMBIX_MATRIX_TO_AMBIX | AMBIX_MATRIX_FUMA,

} ambix_matrixtype_t;

/** a 2-dimensional floating point matrix */
typedef struct ambix_matrix_t {
  /** number of rows */
  uint32_t rows;
  /** number of columns */
  uint32_t cols;
  /** matrix data (as vector (length: rows) of row-vectors (length: cols)) */
  float32_t **data;
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

  /** layout type of the ambix file
   *
   * When opening a file, this format specifies the format from the
   * user-perspective. This is not necessarily the same as the actual
   * format of the file on disk.
   * E.g. when setting this to @ref AMBIX_BASIC to read an @ref AMBIX_EXTENDED
   * file, the library will automatically convert the reduced channel set
   * to the full set (using the embedded adaptor matrix).
   * Similarly, when setting this to @ref AMBIX_BASIC for writing a file,
   * and then setting an adaptor matrix (using @ref ambix_set_adaptormatrix())
   * the actual file will be @ref AMBIX_EXTENDED, but the user has to provide
   * the full set (and the library will store the reduced set).
   */
  ambix_fileformat_t fileformat;

  /** number of non-ambisonics channels in the file
   * @remark think of a better name, like 'uncodedchannels'
   */
  uint32_t extrachannels;

  /** number of (raw) ambisonics channels present in the file.
   *
   * If the file contains a full set of ambisonics channels (format==@ref AMBIX_BASIC),
   * then \f$ambichannel=(order_{ambi}+1)^2\f$; if the file contains an adaptor
   * matrix, it has to be used to reconstruct the full set by multiplying the
   * adaptor matrix with the channels present.
   *
   * @remark when opening for WRITING an @ref AMBIX_EXTENDED file as
   * @ref AMBIX_BASIC (by specifying an adaptor matrix via @ref ambix_set_adaptormatrix()),
   * this value must contain the reduced numer of channels (as stored on disk).
   */
  uint32_t ambichannels;
} ambix_info_t;

/** struct for holding a marker */
typedef struct ambix_marker_t {
  /** position in samples */
  float64_t position;
  /** name: NULL terminated string with maximum length of 255 */
  char name[256];
} ambix_marker_t;

/** struct for holding a region */
typedef struct ambix_region_t {
  /** start position in samples */
  float64_t start_position;
  /** end position in samples */
  float64_t end_position;
  /** name: NULL terminated string with maximum length of 255 */
  char name[256];
} ambix_region_t;

/*
 * @section api_main Main Interface
 */
/** @defgroup ambix ambix
 *
 * @brief handling AmbiX files
 */

/** @brief Open an ambix file
 *
 * Opens a soundfile for reading/writing
 *
 * @param path filename of the file to open
 *
 * @param mode whether to open the file for reading and/or writing (@ref AMBIX_READ,
 * @ref AMBIX_WRITE, @ref AMBIX_RDRW)
 *
 * @param ambixinfo pointer to a valid ambix_info_t structure
 *
 * @remark when opening a file for reading, the structure should be initialized
 * to zero before calling ambix_open(): the fields will be set by the library;
 * if you set the ambixinfo.ambixformat field to something else than @ref AMBIX_NONE,
 * the library will present the data as if the was written in this format (e.g.
 * if you set ambixinfo.ambixformat to @ref AMBIX_BASIC but the file really is
 * @ref AMBIX_EXTENDED, the library will automatically pre-multiply the
 * reconstruction matrix to give you the full ambisonics set.
 *
 * @remark when opening a file for writing, the caller must set the fields; if
 * ambixinfo.ambixformat is @ref AMBIX_NONE, than ambixinfo.ambichannels must be 0,
 * else ambixinfo.ambichannels must be >0; if ambixinfo.ambixformat is
 * @ref AMBIX_BASIC, then ambixinfo.ambichannels must be @f$(order_{ambi}+1)^2@f$
 *
 * @return A handle to the opened file (or NULL on failure)
 *
 * @ingroup ambix
 */
AMBIX_API
ambix_t *ambix_open (const char *path, const ambix_filemode_t mode, ambix_info_t *ambixinfo) ;

/** @brief Close an ambix handle
 *
 * Closes an ambix handle and cleans up all memory allocations associated with
 * it.
 *
 * @param ambix The handle to an ambix file
 *
 * @return an error code indicating success
 *
 * @ingroup ambix
 */
AMBIX_API
ambix_err_t ambix_close (ambix_t *ambix) ;

/** @brief Reposition the file pointer
 *
 * Reposition the file read (and/or write) pointer to a new offset.
 * Consecutive calls to @ref ambix_readf (resp. @ref ambix_writef) will read
 * (resp. write) from the new position.
 *
 * @param ambix The handle to an ambix file
 * @param frames frame offset from the position given in whence
 * @param whence location from where to seek; if whence is set to SEEK_SET,
 *        SEEK_CUR, or SEEK_END, the offset is relative to the start of the file,
 *        the current position indicator, or end-of-file, respectively.
 *
 * @return the offset in (multichannel) frames from the start of the audio data
 * or -1 if an error occurred.
 *
 * @ingroup ambix
 */
AMBIX_API
int64_t ambix_seek (ambix_t *ambix, int64_t frames, int whence) ;

/** @brief Read samples from the ambix file
 * @defgroup ambix_readf ambix_readf()
 *
 * Reads samples from an ambix file, possibly expanding a reduced channel set to
 * a full ambisonics set (when reading an 'ambix extended' file as 'ambix
 * basic')
 *
 * @param ambix The handle to an ambix file
 *
 * @param ambidata pointer to user allocated array to retrieve ambisonics
 * channels into; must be large enough to hold at least
 * (frames*ambix->info.ambichannels) samples, OR if you are reading the file as
 * 'ambix basic' and you successfully added an adaptor matrix using
 * ambix_set_adaptormatrix() the array must be large enough to hold at least
 * (frames * adaptormatrix.rows) samples.
 *
 * @param otherdata pointer to user allocated array to retrieve non-ambisonics
 * channels into must be large enough to hold at least
 * (frames*ambix->info.otherchannels) samples.
 *
 * @param frames number of sample frames you want to read
 *
 * @return the number of sample frames successfully read
 *
 * @ingroup ambix
 */
/** @brief Read samples (as 16bit signed integer values) from the ambix file
 * @ingroup ambix_readf
 */
AMBIX_API
int64_t ambix_readf_int16 (ambix_t *ambix, int16_t *ambidata, int16_t *otherdata, int64_t frames) ;
/** @brief Read samples (as 32bit signed integer values) from the ambix file
 * @ingroup ambix_readf
 */
AMBIX_API
int64_t ambix_readf_int32 (ambix_t *ambix, int32_t *ambidata, int32_t *otherdata, int64_t frames) ;
/** @brief Read samples (as single precision floating point values) from the
 * ambix file
 * @ingroup ambix_readf
 */
AMBIX_API
int64_t ambix_readf_float32 (ambix_t *ambix, float32_t *ambidata, float32_t *otherdata, int64_t frames) ;

/** @brief Read samples (as double precision floating point values) from the
* ambix file
* @ingroup ambix_readf
*/
AMBIX_API
int64_t ambix_readf_float64 (ambix_t *ambix, float64_t *ambidata, float64_t *otherdata, int64_t frames) ;

/** @brief Write samples to the ambix file.
 * @defgroup ambix_writef ambix_writef()
 *
 * Writes samples (as single precision floating point values) to an ambix file,
 * possibly expanding a reduced channel set to a full ambisonics set (when
 * writing an 'ambix extended' file as 'ambix basic').
 *
 * Data will be stored on harddisk in the format specified when opening the file
 * for writing which need not be float32, in which case the data is
 * automatically converted by the library to the appropriate format.
 *
 * @param ambix The handle to an ambix file.
 *
 * @param ambidata pointer to user allocated array to retrieve ambisonics
 * channels from; must be large enough to hold (frames*ambix->info.ambichannels)
 * samples.
 *
 * @param otherdata pointer to user allocated array to retrieve non-ambisonics
 * channels into must be large enough to hold (frames*ambix->info.otherchannels)
 * samples.
 *
 * @param frames number of sample frames you want to write
 *
 * @return the number of sample frames successfully written
 *
 * @ingroup ambix
 */
/** @brief Write (16bit signed integer) samples to the ambix file
 * @ingroup ambix_writef
 */
AMBIX_API
int64_t ambix_writef_int16 (ambix_t *ambix, const int16_t *ambidata, const int16_t *otherdata, int64_t frames) ;
/** @brief Write (32bit signed integer) samples to the ambix file
 * @ingroup ambix_writef
 */
AMBIX_API
int64_t ambix_writef_int32 (ambix_t *ambix, const int32_t *ambidata, const int32_t *otherdata, int64_t frames) ;
/** @brief Write (32bit floating point) samples to the ambix file
 * @ingroup ambix_writef
 */
AMBIX_API
int64_t ambix_writef_float32 (ambix_t *ambix, const float32_t *ambidata, const float32_t *otherdata, int64_t frames) ;
/** @brief Write (64bit floating point) samples to the ambix file
 * @ingroup ambix_writef
 */
AMBIX_API
int64_t ambix_writef_float64 (ambix_t *ambix, const float64_t *ambidata, const float64_t *otherdata, int64_t frames) ;

/** @brief Get the libsndfile handle associated with the ambix handle
 *
 * If possible, require an SNDFILE handle; if the ambix handle is
 * not associated with SNDFILE (e.g. because libambix is compiled without
 * libsndfile support), NULL is returned.
 * If you need this, you probably should include <sndfile.h> before
 * including <ambix/ambix.h>
 *
 * @param ambix The handle to an ambix file
 *
 * @return A libsndfile handle or NULL
 *
 * @ingroup ambix
 */
AMBIX_API
SNDFILE *ambix_get_sndfile (ambix_t *ambix) ;

#pragma pop_macro("SNDFILE")

/** @brief Get the number of stored markers within the ambix file.
 *
 * @return number of markers.
 *
 * @ingroup ambix
 */
AMBIX_API
uint32_t ambix_get_num_markers(ambix_t *ambix) ;
/** @brief Get the number of stored regions within the ambix file.
 *
 * @param ambix The handle to an ambix file
 *
 * @return Number of regions.
 *
 * @ingroup ambix
 */
AMBIX_API
uint32_t ambix_get_num_regions(ambix_t *ambix) ;
/** @brief Get one marker.
 *
 * @param ambix The handle to an ambix file
 *
 * @param id The id of the marker to retrieve.
 *
 * @return The marker requested or NULL in case the marker does not exist.
 *
 * @ingroup ambix
 */
AMBIX_API
ambix_marker_t *ambix_get_marker(ambix_t *ambix, uint32_t id) ;
/** @brief Get one region.
 *
 * @param ambix The handle to an ambix file
 *
 * @param id The id of the region to retrieve.
 *
 * @return The region requested or NULL in case the region does not exist.
 *
 * @ingroup ambix
 */
AMBIX_API
ambix_region_t *ambix_get_region(ambix_t *ambix, uint32_t id) ;
/** @brief Add a new marker to the ambix file.
 *
 * @remark Markers have to be set before sample data is written!
 *
 * @param ambix The handle to an ambix file
 *
 * @param marker A valid marker that should be added to the ambix file.
 *
 * @return an errorcode indicating success.
 *
 * @ingroup ambix
 */
AMBIX_API
ambix_err_t ambix_add_marker(ambix_t *ambix, ambix_marker_t *marker) ;  // returns id
/** @brief Add a new region to the ambix file.
 *
 * @remark Regions have to be set before sample data is written!
 *
 * @param ambix The handle to an ambix file
 *
 * @param region A valid region that should be added to the ambix file.
 *
 * @return an errorcode indicating success.
 *
 * @ingroup ambix
 */
AMBIX_API
ambix_err_t ambix_add_region(ambix_t *ambix, ambix_region_t *region) ; // returns id
/** @brief Deletes all markers in the ambix file.
 *
 * @param ambix The handle to an ambix file
 *
 * @return an errorcode indicating success.
 *
 * @ingroup ambix
 */
AMBIX_API
ambix_err_t ambix_delete_markers(ambix_t *ambix) ;
/** @brief Deletes all regions in the ambix file.
 *
 * @param ambix The handle to an ambix file
 *
 * @return an errorcode indicating success.
 *
 * @ingroup ambix
 */
AMBIX_API
ambix_err_t ambix_delete_regions(ambix_t *ambix) ;
/** @brief Various utilities to handle matrices
 * @defgroup ambix_matrix ambix_matrix
 */
/** @brief Get the adaptor matrix
 *
 * The ambix extended fileformat comes with a adaptor matrix, that can be used
 * to reconstruct a full 3d ambisonics set from the channels stored in ambix
 * file. In the ambix @ref AMBIX_BASIC format no adaptor matrix is present, the file
 * always contains the full set.
 *
 * @remark the adaptor matrix can only be obtained for a ambix extended file; if
 * you have opened an ambix extended file as "ambix basic", the adaptor will be
 * done by the library; in this case, you will not be able to fetch the adaptor
 * matrix.
 *
 * @remark if you have opened an ambix basic file as ambix extended, this will
 * return a unity matrix.
 *
 * @param ambix The handle to an ambix file
 *
 * @return the adaptor matrix to restore the full ambisonics set from the
 * reduced set, or NULL if there is no such matrix; the memory is owned by the
 * library and must neither be freed nor used after calling ambix_close().
 *
 * @ingroup ambix
 */
AMBIX_API
const ambix_matrix_t *ambix_get_adaptormatrix (ambix_t *ambix) ;

/** @brief Set a matrix to be pre-multiplied
 *
 * Adds an (additional) adaptor matrix to the processing. Depending on the mode
 * of operation this can have different meanings! When READing an ambix '@ref
 * AMBIX_BASIC' file, this tells the library to do an (additional)
 * matrix-multiplication When reconstructing the full ambisonics set; you can
 * use this to get the ambisonics channels in a format other than SN3D/ACN
 * (e.g. using an ambix to Furse-Malham adaptor matrix) or getting the
 * loudspeaker feeds directly (by supplying a decoder matrix); in this case, the
 * matrix MUST have ambix->ambichannels columns. When WRITEing an ambix '@ref
 * AMBIX_EXTENDED' file, this tells the library to store the matrix as the
 * adaptor matrix within the file.
 *
 * @param ambix The handle to an ambix file
 *
 * @param matrix a matrix that will be pre-multiplied to the
 * reconstruction-matrix; can be freed after this call.
 *
 * @return an errorcode indicating success
 *
 * @remark using this on ambix handles other than @ref AMBIX_READ/@ref
 * AMBIX_BASIC or @ref AMBIX_WRITE/@ref AMBIX_EXTENDED
 * is an error.
 *
 * @ingroup ambix
 */
AMBIX_API
ambix_err_t ambix_set_adaptormatrix (ambix_t *ambix, const ambix_matrix_t *matrix) ;

/*
 * @section api_matrix matrix utility functions
 */

/** @brief Create a matrix
 *
 * Allocates a new (empty) matrix object.
 * It's equivalent to calling ambix_matrix_init(0, 0, NULL) ;
 *
 * @return a new matrix object or NULL
 *
 * @ingroup ambix_matrix
 */
AMBIX_API
ambix_matrix_t *ambix_matrix_create (void) ;

/** @brief Destroy a matrix
 *
 * Frees all resources allocated for the matrix object.
 * It's a shortcut for ambix_matrix_deinit(mtx), free(mtx)
 *
 * @param mtx matrix object to destroy
 *
 * @ingroup ambix_matrix
 */
AMBIX_API
void ambix_matrix_destroy (ambix_matrix_t *mtx) ;

/** @brief Initialize a matrix
 *
 * Allocates memory for matrix-data of given dimensions
 *
 * @param rows number of rows in the newly initialized matrix
 *
 * @param cols number of columns in the newly initialized matrix
 *
 * @param mtx pointer to a matrix object; if NULL a new matrix object will be
 * created, else the given matrix object will be re-initialized.
 *
 * @return pointer to a newly initialized (and/or allocated) matrix, or NULL on
 * error.
 *
 * @ingroup ambix_matrix
 */
AMBIX_API
ambix_matrix_t *ambix_matrix_init (uint32_t rows, uint32_t cols, ambix_matrix_t *mtx) ;

/** @brief De-initialize a matrix
 *
 * Frees associated resources and sets rows/columns to 0
 *
 * @param mtx matrix object to deinitialize
 *
 * @ingroup ambix_matrix
 */
AMBIX_API
void ambix_matrix_deinit (ambix_matrix_t *mtx) ;

/** @brief Fill a matrix according to specs
 *
 * Fill a properly initialized matrix according to type. You can use this to
 * generate standard matrices (e.g. using AMBIX_MATRIX_ONE will set all elements
 * of the matrix to 1.0). Since this call will not change the matrix layout
 * (e.g. the dimension), it is the responsibility of the caller to ensure that
 * the matrix has a proper layout for the requested type (e.g. it is an error to
 * fill a Furse-Malham matrix into a matrix that holds more than 3rd order
 * ambisonics).
 *
 * @param matrix initialized matrix object to fill
 *
 * @param type data specification
 *
 * @return pointer to the matrix object, or NULL if the type was not valid (for the
 * input matrix)
 *
 * @ingroup ambix_matrix
 */
AMBIX_API
ambix_matrix_t *ambix_matrix_fill (ambix_matrix_t *matrix, ambix_matrixtype_t type) ;

/** @brief Fill a matrix with values
 *
 * Fill data into a properly initialized matrix
 *
 * @param mtx initialized matrix object to copy data into
 *
 * @param data pointer to at least (mtx->rows*mtx->cols) values; data is ordered
 * row-by-row with no padding (A[0,0], A[0,1], .., A[0,cols-1],  A[1, 0], ..
 * A[rows-1, cols-1])
 *
 * @return an error code indicating success
 *
 * @ingroup ambix_matrix
 */
AMBIX_API
ambix_err_t ambix_matrix_fill_data (ambix_matrix_t *mtx, const float32_t *data) ;

/** @brief Copy a matrix to another matrix
 *
 * Copy a matrix, possibly resizing or creating the destination
 *
 * @param src the source matrix to copy the data from
 *
 * @param dest the destination matrix (if NULL a new matrix will be created)
 *
 * @return pointer to the destination matrix
 *
 * @ingroup ambix_matrix
 */
AMBIX_API
ambix_matrix_t *ambix_matrix_copy (const ambix_matrix_t *src, ambix_matrix_t *dest) ;
/** @cond DEPRECATED */
/** @brief Multiply two matrices
 *
 * Multiply matrices dest=A*B, possibly resizing or creating the destination
 * matrix.
 *
 * @param A left-hand operator
 *
 * @param B right-hand operator
 *
 * @param result pointer to the matrix object that will hold the result or NULL
 *
 * @return pointer to the result matrix, or NULL in case the matrix
 * multiplication did not succeed.
 *
 * @remark If this returns a newly allocated matrix object (result!=return
 * value), the host has to take care of calling ambix_matrix_destroy().
 *
 * @ingroup ambix_matrix
 */
AMBIX_API AMBIX_DEPRECATED
ambix_matrix_t *ambix_matrix_multiply (const ambix_matrix_t *A, const ambix_matrix_t *B, ambix_matrix_t *result) ;
/** @brief Get the Moore-Penrose pseudoinverse of a matrix.
 *
 * Get the Moore-Penrose pseudoinverse of the matrix input and write the result
 * to pinv
 *
 * @param matrix input matrix
 *
 * @param pinv pointer to the matrix object that will hold the result or NULL
 *
 * @return pointer to the result matrix, or NULL in case the matrix
 * inversion did not succeed.
 *
 * @ingroup ambix_matrix
 */
AMBIX_API AMBIX_DEPRECATED
ambix_matrix_t* ambix_matrix_pinv(const ambix_matrix_t*matrix, ambix_matrix_t*pinv) ;
/** @endcond */
/** @brief Multiply a matrix with data
 * @defgroup ambix_matrix_multiply_data ambix_matrix_multiply_data()
 * @ingroup ambix_matrix
 *
 * Multiply a [rows*cols] matrix with an array of [cols*frames] source data to
 * get [rows*frames] dest data.
 *
 * @param dest a pointer to hold the output data; it must be large enough to
 * hold at least rows*frames samples (allocated by the user).
 *
 * @param mtx the matrix to multiply source with.
 *
 * @param source a pointer to an array that holds cols*frames samples (allocated
 * by the user).
 *
 * @param frames number of frames in source
 *
 * @return an error code indicating success
 *
 * @remark Both source and dest data are arranged column-wise (as is the default
 * for interleaved audio-data).
 */

/** @brief Multiply a matrix with (32bit floating point) data
 *
 * @ingroup ambix_matrix_multiply_data
 */
AMBIX_API
ambix_err_t ambix_matrix_multiply_float32(float32_t *dest, const ambix_matrix_t *mtx, const float32_t *source, int64_t frames) ;
/** @brief Multiply a matrix with (64bit float) data
 *
 * @ingroup ambix_matrix_multiply_data
 */
AMBIX_API
ambix_err_t ambix_matrix_multiply_float64(float64_t *dest, const ambix_matrix_t *mtx, const float64_t *source, int64_t frames) ;
/** @brief Multiply a matrix with (32bit signed integer) data
 *
 * @ingroup ambix_matrix_multiply_data
 */
AMBIX_API
ambix_err_t ambix_matrix_multiply_int32(int32_t *dest, const ambix_matrix_t *mtx, const int32_t *source, int64_t frames) ;
/** @brief Multiply a matrix with (16 bit signed integer) data
 *
 * @ingroup ambix_matrix_multiply_data
 */
AMBIX_API
ambix_err_t ambix_matrix_multiply_int16(int16_t *dest, const ambix_matrix_t *mtx, const int16_t *source, int64_t frames) ;

/**
 * @section api_utils utility functions
 */
/** @defgroup ambix_utilities ambix_utilities
 *
 * @brief utility functions
 */
/** @brief Calculate the number of channels for a full 3d ambisonics set of a
 * given order.
 *
 * @param order the order of the full set
 * @return the number of channels of the full set
 *
 * @ingroup ambix_utilities
 */
AMBIX_API
uint32_t ambix_order2channels(uint32_t order) ;

/** @brief Calculate the order of a full 3d ambisonics set for a given number of
 * channels.
 *
 * @param channels the number of channels of the full set
 * @return the order of the full set, or -1 if the channels don't form a full
 * set.
 *
 * @ingroup ambix_utilities
 */
AMBIX_API
int32_t ambix_channels2order(uint32_t channels) ;

/** @brief Checks whether the channel can form a full 3d ambisonics set.
 *
 * @param channels the number of channels supposed to form a full set.
 *
 * @return TRUE if the channels can form full set, FALSE otherwise.
 *
 * @ingroup ambix_utilities
 */
AMBIX_API
int ambix_is_fullset(uint32_t channels) ;

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#if HAVE_PUSH_MACRO
# pragma pop_macro("SNDFILE")
#elif !defined(SNDFILE_1)
# undef SNDFILE void
#endif

#endif /* AMBIX_AMBIX_H */
