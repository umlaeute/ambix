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

  /** buffer for adaptor signals */
  void*adaptorbuffer;
  /** size of the adaptor buffer (in bytes) */
  int64_t adaptorbuffersize;

  /** read/write mode */
  ambix_filemode_t filemode;

  /** whether the file is byteswapped in relation to host */
  int byteswap;

  /** default adaptorbuffer size in frames */
#define DEFAULT_ADAPTORBUFFER_SIZE 64
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



/** @brief read 32bit float data from file
 * @param ambix a pointer to a valid ambix structure
 * @param data pointer to an float32_t array that can hold at least frames*channels values
 * @param frames number of sample frames to read
 * @return number of sample frames successfully read
 */
int64_t _ambix_readf_float32   (ambix_t*ambix, float32_t*data, int64_t frames);
/** @see _ambix_readf_float32
 * @remark this operates on 32bit integer data
 */
int64_t _ambix_readf_int32   (ambix_t*ambix, int32_t*data, int64_t frames);
/** @see _ambix_readf_float32
 * @remark this operates on 16bit integer data
 */
int64_t _ambix_readf_int16   (ambix_t*ambix, int16_t*data, int64_t frames);

/** @brief write 32bit float data to file
 * @param ambix a pointer to a valid ambix structure
 * @param data pointer to an float32_t array that holds frames*channels values
 * @param frames number of sample frames to write
 * @return number of sample frames successfully written
 */
int64_t _ambix_writef_float32   (ambix_t*ambix, float32_t*data, int64_t frames);
/** @see _ambix_writef_float32
 * @remark this operates on 32bit integer data
 */
int64_t _ambix_writef_int32   (ambix_t*ambix, int32_t*data, int64_t frames);
/** @see _ambix_writef_float32
 * @remark this operates on 16bit integer data
 */
int64_t _ambix_writef_int16   (ambix_t*ambix, int16_t*data, int64_t frames);

/** @brief Get UUID for ambix
 * @param ambix version
 * @return a pointer to the UUID for the given version or NULL
 * @remark currently only one UUID is defined for the ambix format (version 1)
 *         future versions of the standard might add additional UUIDs
 */
const char* _ambix_getUUID(uint32_t version);

/** @brief Check data for ambix UUID
 * @param data Array holding the UUID
 * @return ambix-version this UUID is associated with, or 0 on failure
 * @remark currently only one UUID is defined for the ambix format (version 1)
 *         future versions of the standard might add additional UUIDs
 */
uint32_t _ambix_checkUUID(const char UUID[16]);

/** @brief extract matrix from ambix UUID-chunk (v1)
 * @param data Array holding the payload data (excluding the UUID itself)
 * @param datasize size of data
 * @param mtx pointer to a matrix object that should be filled (if NULL, this function will allocate a matrix object for you)
 * @param byteswap TRUE if data has to be byteswapped (e.g. when reading BIG_ENDIAN data on LITTLE_ENDIAN machines)
 * @return a pointer to the filled matrix or NULL on failure
 * @remark only use data from a uuid-chunk for which _ambix_parseuuid() that returned '1'
 */
ambixmatrix_t*_ambix_uuid1_to_matrix(const void*data, uint64_t datasize, ambixmatrix_t*mtx, int byteswap);

/** @brief generate UUID-chunk (v1) from matrix
 * @param matrix data to store in chunk
 * @param data pointer to memory to store the UUID-chunk in (or NULL)
 * @param byteswap TRUE if data has to be byteswapped (e.g. when reading BIG_ENDIAN data on LITTLE_ENDIAN machines)
 * @return datasize needed for the UUID-chunk
 * @remark you should call this two times: first with data=NULL, which will return the datasize you need to allocate;
 *         then you allocate enough data (datasize bytes) and call the function again
 */
uint64_t _ambix_matrix_to_uuid1(const ambixmatrix_t*matrix, void*data, int byteswap);


/** @brief write UUID chunk to file
 * @param ambix valid ambix handle
 * @param data pointer to memory holding the UUID-chunk
 * @param datasize size of data
 * @return error code indicating success
 * @remark you should only call this once, after opening the file and before writing sample frames to it
 */
ambix_err_t _ambix_write_uuidchunk(ambix_t*ax, const void*data, int64_t datasize);

/** @brief byte-swap 32bit data
 * @param n a 32bit chunk in the wrong byte order
 * @return byte-swapped data
 */
static inline uint32_t swap4(uint32_t n)
{
  return (((n & 0xff) << 24) | ((n & 0xff00) << 8) |
          ((n & 0xff0000) >> 8) | ((n & 0xff000000) >> 24));
}


/** @brief resize adaptor buffer to given size
 *
 * makes sure that the internal adaptorbuffer of ambix can hold at least
 *  frames*(ambichannels+otherchannels) samples of size typesize
 *
 * @param ambix valid ambix handle
 * @param frames number of frames target buffer must be able to hold
 * @param typesize sizeof(sampletype)
 * @return error code indicating success
 */
ambix_err_t _ambix_adaptorbuffer_resize(ambix_t*ambix, uint64_t frames, uint16_t typesize);
/** @brief free an adaptor buffer
 * @param ambix valid ambix handle
 * @return error code indicating success
 */
ambix_err_t _ambix_adaptorbuffer_destroy(ambix_t*ambix);

/** @brief extract ambisonics and non-ambisonics channels from interleaved (32bit floating point) data
 *
 * extract the first ambichannels channels from the source into dest_ambi
 * the remaining source channels are written into dest_other
 * it is an error if ambichannels>sourcechannels (resulting in a negative number of non-ambisonics channels)
 *
 * @param source the interleaved samplebuffer to read from
 * @param sourcechannels the number of channels in the source
 * @param dest_ambi the ambisonics channels (interleaved)
 * @param ambichannels the number of ambisonics channels to extract
 * @param dest_other the non-ambisonics channels (interleaved)
 * @param frames number of frames to extract
 * @return error code indicating success
 */
ambix_err_t _ambix_splitAdaptor_float32(float32_t*source, uint32_t sourcechannels, uint32_t ambichannels, float32_t*dest_ambi, float32_t*dest_other, int64_t frames);
/** @brief extract ambisonics and non-ambisonics channels from interleaved (32bit signed integer) data
 * @see _ambix_splitAdapator_float32
 */
ambix_err_t _ambix_splitAdaptor_int32(int32_t*source, uint32_t sourcechannels, uint32_t ambichannels, int32_t*dest_ambi, int32_t*dest_other, int64_t frames);
/** @brief extract ambisonics and non-ambisonics channels from interleaved (16bit signed integer) data
 * @see _ambix_splitAdapator_float32
 */
ambix_err_t _ambix_splitAdaptor_int16(int16_t*source, uint32_t sourcechannels, uint32_t ambichannels, int16_t*dest_ambi, int16_t*dest_other, int64_t frames);

/** @brief extract ambisonics and non-ambisonics channels from interleaved data using matrix operations
 *
 * extract matrix.rows ambisonics channels into dest_ambi by multiplying the matrix with matrix.cols source channels
 * the remaining source channels are written into dest_other
 * it is an error if matrix.cols>sourcechannels (resulting in a negative number of non-ambisonics channels)
 *
 * @param source the interleaved samplebuffer to read from
 * @param sourcechannels the number of channels in the source
 * @param dest_ambi the ambisonics channels (interleaved)
 * @param dest_other the non-ambisonics channels (interleaved)
 * @param frames number of frames to extract
 * @return error code indicating success
 */
ambix_err_t _ambix_splitAdaptormatrix_float32(float32_t*source, uint32_t sourcechannels, ambixmatrix_t*matrix, float32_t*dest_ambi, float32_t*dest_other, int64_t frames);
/* @see _ambix_splitAdaptormatrix_float32 */
ambix_err_t _ambix_splitAdaptormatrix_int32(int32_t*source, uint32_t sourcechannels, ambixmatrix_t*matrix, int32_t*dest_ambi, int32_t*dest_other, int64_t frames);
/* @see _ambix_splitAdaptormatrix_float32 */
ambix_err_t _ambix_splitAdaptormatrix_int16(int16_t*source, uint32_t sourcechannels, ambixmatrix_t*matrix, int16_t*dest_ambi, int16_t*dest_other, int64_t frames);


/** @brief merge two separate interleaved (32bit floating point) audio data blocks into one
 *
 * append ambisonics and non-ambisonics channels into one big interleaved chunk
 *
 * @param source1 the first interleaved samplebuffer to read from
 * @param source1channels the number of channels in source1
 * @param source2 the second interleaved samplebuffer to read from
 * @param source2channels the number of channels in source2
 * @param destination the samplebuffer to merge the data info; must be big enough to hold frames*(source1channels+source2channels) samples
 * @param frames number of frames to extract
 * @return error code indicating success
 */
ambix_err_t _ambix_mergeAdaptor_float32(float32_t*source1, uint32_t source1channels, float32_t*source2, uint32_t source2channels, float32_t*destination, int64_t frames);
/** @brief merge two separate interleaved (32bit signed integer) audio data blocks into one
 * @see _ambix_mergeAdapator_float32
 */
ambix_err_t _ambix_mergeAdaptor_int32(int32_t*source1, uint32_t source1channels, int32_t*source2, uint32_t source2channels, int32_t*destination, int64_t frames);
/** @brief merge two separate interleaved (16bit signed integer) audio data blocks into one
 * @see _ambix_mergeAdapator_float32
 */
ambix_err_t _ambix_mergeAdaptor_int16(int16_t*source1, uint32_t source1channels, int16_t*source2, uint32_t source2channels, int16_t*destination, int64_t frames);



#define MARK() printf("%s:%d[%s]\n", __FILE__, __LINE__, __FUNCTION__)
