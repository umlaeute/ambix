
/* ambix/ambix.h -  Ambisonics Xchange Library Interface              -*- c -*-

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
#include "types.h"
#include "matrix.h"
#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @brief Open an ambix file
 *
 * Opens a soundfile for reading/writing
 *
 * @param path filename of the file to open
 *
 * @param mode whether to open the file for reading and/or writing (AMBIX_READ,
 * AMBIX_WRITE, AMBIX_READ | AMBIX_WRITE)
 *
 * @param ambixinfo pointer to a valid ambix_info_t structure
 *
 * @remark when opening a file for reading, the structure should be initialized
 * to zero before calling ambix_open(): the fields will be set by the library;
 * if you set the ambixinfo.ambixformat field to something else than AMBIX_NONE,
 * the library will present the data as if the was written in this format (e.g.
 * if you set ambixinfo.ambixformat to AMBIX_BASIC but the file really is
 * AMBIX_EXTENDED, the library will automatically pre-multiply the
 * reconstruction matrix to give you the full ambisonics set.
 *
 * @remark when opening a file for writing, the caller must set the fields; if
 * ambixinfo.ambixformat is AMBIX_NONE, than ambixinfo.ambichannels must be 0,
 * else ambixinfo.ambichannels must be >0; if ambixinfo.ambixformat is
 * AMBIX_BASIC, then ambixinfo.ambichannels must be @f$(order_{ambi}+1)^2@f$
 *
 * @return A handle to the opened file (or NULL on failure)
 */
AMBIX_API
ambix_t* ambix_open (const char* path, const ambix_filemode_t mode, ambix_info_t* ambixinfo) ;

/** @brief Close an ambix handle
 *
 * Closes an ambix handle and cleans up all memory allocations associated with
 * it.
 *
 * @param ambix The handle to an ambix file
 *
 * @return an error code indicating success
 */
AMBIX_API
ambix_err_t ambix_close (ambix_t* ambix);

/** @brief Read samples (as 16bit signed integer values) from the ambix file
 * @ingroup ambix_readf
 */
AMBIX_API
int64_t ambix_readf_int16 (ambix_t* ambix, int16_t* ambidata, int16_t* otherdata, int64_t frames) ;
/** @brief Read samples (as 32bit signed integer values) from the ambix file
 * @ingroup ambix_readf
 */
AMBIX_API
int64_t ambix_readf_int32 (ambix_t* ambix, int32_t* ambidata, int32_t* otherdata, int64_t frames) ;
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
 * @return the number of sample frames sucessfully read
 */
/** @brief Read samples (as single prevision floating point values) from the
 * ambix file
 * @ingroup ambix_readf
 */
AMBIX_API
int64_t ambix_readf_float32 (ambix_t* ambix, float32_t* ambidata, float32_t* otherdata, int64_t frames) ;

/** @brief Write (16bit signed integer) samples to the ambix file
 * @ingroup ambix_writef
 */
AMBIX_API
int64_t ambix_writef_int16 (ambix_t* ambix, const int16_t* ambidata, const int16_t* otherdata, int64_t frames) ;
/** @brief Write (32bit signed integer) samples to the ambix file
 * @ingroup ambix_writef
 */
AMBIX_API
int64_t ambix_writef_int32 (ambix_t* ambix, const int32_t* ambidata, const int32_t* otherdata, int64_t frames) ;
/** @brief Write samples to the ambix file
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
 * @return the number of sample frames sucessfully written
 */
/** @brief Write (32bit floating point) samples to the ambix file
 * @ingroup ambix_writef
 */
AMBIX_API
int64_t ambix_writef_float32 (ambix_t* ambix, const float32_t* ambidata, const float32_t* otherdata, int64_t frames) ;


/** @brief Get the libsndfile handle associated with the ambix handle
 *
 * If possible, require an SNDFILE handle if possible; if the ambix handle is
 * not asociated with SNDFILE (e.g. because libambix is compiled without
 * libsndfile support), NULL is returned.
 *
 * @param ambix The handle to an ambix file
 *
 * @return A libsndfile handle or NULL
 */
AMBIX_API
SNDFILE* ambix_get_sndfile (ambix_t* ambix);

/** @brief Get the adaptor matrix
 *
 * The ambix extended fileformat comes with a adaptor matrix, that can be used
 * to reconstruct a full 3d ambisonics set from the channels stored in ambix
 * file. In the ambix BASIC format no adaptor matrix is present, the file
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
 */
AMBIX_API
const ambix_matrix_t* ambix_get_adaptormatrix (ambix_t* ambix);

/** @brief Set a matrix to be pre-multiplied
 *
 * Adds an (additional) adaptor matrix to the processing. Depending on the mode
 * of operation this canhave different meanings! When READing an ambix 'BASIC'
 * file, this tells the library to do an (additional) matrix-multiplication When
 * reconstructing the full ambisonics set; you can use use this to get the
 * ambisonics channels in a format other than SN3D/ACN (e.g. using an ambix to
 * Furse-Malham adaptor matrix) or getting the loudspeaker feeds directly (by
 * supplying a decoder matrix); in this case, the matrix MUST have
 * ambix->ambichannels columns. When WRITEing an ambix 'EXTENDED' file, this
 * tells the library to store the matrix as the adaptor matrix within the file.
 *
 * @param ambix The handle to an ambix file
 *
 * @param matrix a matrix that will be pre-multiplied to the
 * reconstruction-matrix; can be freed after this call.
 *
 * @return an errorcode indicating success
 *
 * @remark using this on ambix handles other than READ/BASIC or WRITE/EXTENDED
 * is an error.
 */
AMBIX_API
ambix_err_t ambix_set_adaptormatrix (ambix_t* ambix, const ambix_matrix_t* matrix);


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */
#endif /* AMBIX_AMBIX_H */
