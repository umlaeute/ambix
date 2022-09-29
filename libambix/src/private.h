/* private.h -  AMBIsonics eXchange Library Private Interface              -*- c -*-

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
 * @file	private.h
 * @brief	AMBIsonics eXchange Library Private Interface
 * @details This file is part of libambix
 * @author IOhannes m zmölnig <zmoelnig@iem.at>
 * @date 2012
 * @copyright LGPL-2.1
 * @internal
 */
#ifndef AMBIX_PRIVATE_H
#define AMBIX_PRIVATE_H

#ifndef AMBIX_INTERNAL
# error private.h must only be used from within libambix
#endif

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#ifdef _MSC_VER
# define inline __inline
#endif

#include <ambix/ambix.h>

/** this is for passing data about the opened ambix file between the host application and the library */
struct ambix_t_struct {
  /** private data by the actual backend */
  void*private_data;

  /** whether the file is a valid AMBIX file (CAF) */
  int is_AMBIX;

  /** whether the file has an UUID-chunk, and which AMBIX version it describs */
  ambix_fileformat_t format;

  /** read/write mode */
  ambix_filemode_t filemode;

  /** whether the file is byteswapped in relation to host */
  int byteswap;

  /** full number of channels in the file */
  int32_t channels;

  /** ambisonics info chunk as presented to the outside world */
  ambix_info_t info;
  /** ambisonics info chunk as read from disk */
  ambix_info_t realinfo;

  /** reconstruction matrix */
  ambix_matrix_t matrix;
  /** final reconstruction matrix (potentially includes another adaptor matrix) */
  ambix_matrix_t matrix2;
  /** whether to use the matrix(1), the finalmatrix(2), or no matrix when decoding */
  int use_matrix;

  /** buffer for adaptor signals */
  void*adaptorbuffer;
  /** size of the adaptor buffer (in bytes) */
  uint64_t adaptorbuffersize;
  /** default adaptorbuffer size in frames */
#define DEFAULT_ADAPTORBUFFER_SIZE 64

  /** ambisonics order of the full set */
  uint32_t ambisonics_order;

  /** the number of stored markers */
  uint32_t num_markers;
  /** storage for markers */
  ambix_marker_t *markers;
  /** the number of stored regions */
  uint32_t num_regions;
  /** storage for regions */
  ambix_region_t *regions;

  /** whether we already started reading samples */
  int startedReading;
  /** whether we already started writing samples */
  int startedWriting;

  /** whether we have pending headers to write */
  int pendingHeaders;
};


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
ambix_err_t	_ambix_open	(ambix_t*ambix, const char *path, const ambix_filemode_t mode, const ambix_info_t*ambixinfo);
/** @brief Do close an ambix file
 *
 * this is implemented by the various backends (currently only libsndfile)
 *
 * @param ambix a pointer to a valid ambix structure (has to be freed outside)
 * @return errorcode indicating success
 */
ambix_err_t	_ambix_close	(ambix_t*ambix);

/** @brief seek in ambix-file
 * @param ambix The handle to an ambix file
 * @param frames frame offset from the position given in whence
 * @param whence location from where to seek; (see lseek)
 * @return the offset in (multichannel) frames from the start of the audio data or -1 if an error occurred
 */
int64_t _ambix_seek (ambix_t* ambix, int64_t frames, int whence);

/** @brief Do get an libsndfile handle
 *
 * this is implemented by the various backends (currently only libsndfile)
 *
 * @param ambix a pointer to a valid ambix structure
 * @return an SNDFILE handle or NULL if not possible
 */
void*_ambix_get_sndfile	(ambix_t*ambix);

/** @brief read 32bit float data from file
 * @param ambix a pointer to a valid ambix structure
 * @param data pointer to an float32_t array that can hold at least frames*channels values
 * @param frames number of sample frames to read
 * @return number of sample frames successfully read
 */
int64_t _ambix_readf_float32   (ambix_t*ambix, float32_t*data, int64_t frames);
/** @see _ambix_readf_float64
 * @remark this operates on 64bit float data (double)
 */
int64_t _ambix_readf_float64   (ambix_t*ambix, float64_t*data, int64_t frames);
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
int64_t _ambix_writef_float32   (ambix_t*ambix, const float32_t*data, int64_t frames);
/** @see _ambix_writef_float32
 * @remark this operates on 64bit float data
 */
int64_t _ambix_writef_float64   (ambix_t*ambix, const float64_t*data, int64_t frames);
/** @see _ambix_writef_float32
 * @remark this operates on 32bit integer data
 */
int64_t _ambix_writef_int32   (ambix_t*ambix, const int32_t*data, int64_t frames);
/** @see _ambix_writef_float32
 * @remark this operates on 16bit integer data
 */
int64_t _ambix_writef_int16   (ambix_t*ambix, const int16_t*data, int64_t frames);

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
ambix_matrix_t*_ambix_uuid1_to_matrix(const void*data, uint64_t datasize, ambix_matrix_t*mtx, int byteswap);
/** @brief generate UUID-chunk (v1) from matrix
 * @param matrix data to store in chunk
 * @param data pointer to memory to store the UUID-chunk in (or NULL)
 * @param byteswap TRUE if data has to be byteswapped (e.g. when reading BIG_ENDIAN data on LITTLE_ENDIAN machines)
 * @return datasize needed for the UUID-chunk
 * @remark you should call this two times: first with data=NULL, which will return the datasize you need to allocate;
 *         then you allocate enough data (datasize bytes) and call the function again
 */
uint64_t _ambix_matrix_to_uuid1(const ambix_matrix_t*matrix, void*data, int byteswap);
/** @brief write UUID chunk to file
 * @param ambix valid ambix handle
 * @param data pointer to memory holding the UUID-chunk
 * @param datasize size of data
 * @return error code indicating success
 * @remark you should only call this once, after opening the file and before writing sample frames to it
 */
ambix_err_t _ambix_write_uuidchunk(ambix_t*ax, const void*data, int64_t datasize);

/** @brief read marker, region and corresponding strings chunk from file
 * @param ambix valid ambix handle
 * @return error code indicating success
 */
ambix_err_t _ambix_read_markersregions(ambix_t*ax);
/** @brief write marker, region and corresponding strings chunk to file
 * @param ambix valid ambix handle
 * @return error code indicating success
 */
ambix_err_t _ambix_write_markersregions(ambix_t*ax);
/** @brief write general chunk to file
 * @param ambix valid ambix handle
 * @param id four-character code identifying the chunk
 * @param data pointer to memory holding the chunk
 * @param datasize size of data
 * @return error code indicating success
 */
ambix_err_t _ambix_write_chunk(ambix_t*ax, uint32_t id, const void*data, int64_t datasize);
/** @brief read general chunk to file
 * @param ambix valid ambix handle
 * @param id four-character code identifying the chunk
 * @param chunk_it try to get the chunk_it-th chunk with the specified id
 * @param datasize returns size of data
 * @return data pointer to memory holding the chunk
 *
 * @remark several chunks with the same id may exist in the file
 * use chunk_it from 0...n for retrieving all existing chunks
 *
 * @remark in case of success the caller has to free the returned data! 
 */
void* _ambix_read_chunk(ambix_t*ax, uint32_t id, uint32_t chunk_it, int64_t *datasize);

/** @brief Fill a matrix with byteswapped values
 *
 * Fill data into a properly initialized matrix
 *
 * @param mtx initialized matrix object to copy data into
 * @param data pointer to at least (mtx->rows*mtx->cols) values; data is ordered row-by-row with no padding (A[0,0], A[0,1], .., A[0,cols-1],  A[1, 0], .. A[rows-1, cols-1])
 * @return an error code indicating success
 */
ambix_err_t
_ambix_matrix_fill_data_byteswapped(ambix_matrix_t*mtx, const number32_t*data);

/** @brief Transpose a matrix
 *
 * swap rows/columns: a[i][j] -> a[j][i]
 *
 * @param matrix the matrix to transpose
 * @param xirtam the result matrix (if NULL one will be allocated for you)
 * @return a pointer to the result matrix (or NULL on failure)
 */
ambix_matrix_t*
_ambix_matrix_transpose(const ambix_matrix_t*matrix, ambix_matrix_t*xirtam);
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
 */
ambix_matrix_t*
_ambix_matrix_multiply (const ambix_matrix_t *A, const ambix_matrix_t *B, ambix_matrix_t *result) ;
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
 */
ambix_matrix_t*
_ambix_matrix_pinv(const ambix_matrix_t*matrix, ambix_matrix_t*pinv) ;

/** @brief Invert a matrix using Gauss-Jordan
 *
 * Invert a square matrix using the Gauss-Jordan algorithm
 *
 * @param matrix the matrix to invert
 * @param result the result matrix (if NULL one will be allocated for you)
 * @param eps threshold to detect singularities
 * @return a pointer to the result matrix (or NULL on failure)
 *
 * @note the input matrix will be modified!
 */
ambix_matrix_t*
_ambix_matrix_invert_gaussjordan(ambix_matrix_t*matrix, ambix_matrix_t*result, float32_t eps);

/** @brief Invert a matrix using Cholesky
 *
 * Invert a matrix using the Cholesky decomposition.
 * If the matrix is non-square, this computes the pseuo-inverse.
 *
 * @param matrix the matrix to invert
 * @param result the result matrix (if NULL one will be allocated for you)
 * @param tolerance threshold for pivoting (scaled by the largest matrix values)
 * @return a pointer to the result matrix (or NULL on failure)
 *
 */
ambix_matrix_t*
_ambix_matrix_pinvert_cholesky(const ambix_matrix_t*matrix, ambix_matrix_t*result, float32_t tolerance);

/** @brief byte-swap 32bit data
 * @param n a 32bit chunk in the wrong byte order
 * @return byte-swapped data
 */
static inline uint32_t swap4(uint32_t n)
{
  return (((n & 0xff) << 24) | ((n & 0xff00) << 8) |
          ((n & 0xff0000) >> 8) | ((n & 0xff000000) >> 24));
}
/** @brief byte-swap 64bit data
 * @param n a 64bit chunk in the wrong byte order
 * @return byte-swapped data
 */
static inline uint64_t swap8(uint64_t n)
{
  return ((((n) & 0xff00000000000000ull) >> 56) |
          (((n) & 0x00ff000000000000ull) >> 40) |
          (((n) & 0x0000ff0000000000ull) >> 24) |
          (((n) & 0x000000ff00000000ull) >> 8 ) |
          (((n) & 0x00000000ff000000ull) << 8 ) |
          (((n) & 0x0000000000ff0000ull) << 24) |
          (((n) & 0x000000000000ff00ull) << 40) |
          (((n) & 0x00000000000000ffull) << 56));
}
/** @brief byte-swap arrays of 32bit data
 * @param data a pointer to an array of 32bit data to be byteswapped
 * @param datasize the size of the array
 */
void _ambix_swap4array(uint32_t*data, uint64_t datasize);
/** @brief byte-swap arrays of 64bit data
 * @param data a pointer to an array of 64bit data to be byteswapped
 * @param datasize the size of the array
 */
void _ambix_swap8array(uint64_t*data, uint64_t datasize);

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
ambix_err_t _ambix_splitAdaptor_float32(const float32_t*source, uint32_t sourcechannels, uint32_t ambichannels, float32_t*dest_ambi, float32_t*dest_other, int64_t frames);
/** @brief extract ambisonics and non-ambisonics channels from interleaved (64bit float) data
 * @see _ambix_splitAdaptor_float64
 */
ambix_err_t _ambix_splitAdaptor_float64(const float64_t*source, uint32_t sourcechannels, uint32_t ambichannels, float64_t*dest_ambi, float64_t*dest_other, int64_t frames);
/** @brief extract ambisonics and non-ambisonics channels from interleaved (32bit signed integer) data
 * @see _ambix_splitAdapator_float32
 */
ambix_err_t _ambix_splitAdaptor_int32(const int32_t*source, uint32_t sourcechannels, uint32_t ambichannels, int32_t*dest_ambi, int32_t*dest_other, int64_t frames);
/** @brief extract ambisonics and non-ambisonics channels from interleaved (16bit signed integer) data
 * @see _ambix_splitAdapator_float32
 */
ambix_err_t _ambix_splitAdaptor_int16(const int16_t*source, uint32_t sourcechannels, uint32_t ambichannels, int16_t*dest_ambi, int16_t*dest_other, int64_t frames);

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
ambix_err_t _ambix_splitAdaptormatrix_float32(const float32_t*source, uint32_t sourcechannels, const ambix_matrix_t*matrix, float32_t*dest_ambi, float32_t*dest_other, int64_t frames);
/* @see _ambix_splitAdaptormatrix_float32 */
ambix_err_t _ambix_splitAdaptormatrix_float64(const float64_t*source, uint32_t sourcechannels, const ambix_matrix_t*matrix, float64_t*dest_ambi, float64_t*dest_other, int64_t frames);
/* @see _ambix_splitAdaptormatrix_float32 */
ambix_err_t _ambix_splitAdaptormatrix_int32(const int32_t*source, uint32_t sourcechannels, const ambix_matrix_t*matrix, int32_t*dest_ambi, int32_t*dest_other, int64_t frames);
/* @see _ambix_splitAdaptormatrix_float32 */
ambix_err_t _ambix_splitAdaptormatrix_int16(const int16_t*source, uint32_t sourcechannels, const ambix_matrix_t*matrix, int16_t*dest_ambi, int16_t*dest_other, int64_t frames);


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
ambix_err_t _ambix_mergeAdaptor_float32(const float32_t*source1, uint32_t source1channels, const float32_t*source2, uint32_t source2channels, float32_t*destination, int64_t frames);
/** @brief merge two separate interleaved (64bit float) audio data blocks into one
 * @see _ambix_mergeAdapator_float32
 */
ambix_err_t _ambix_mergeAdaptor_float64(const float64_t*source1, uint32_t source1channels, const float64_t*source2, uint32_t source2channels, float64_t*destination, int64_t frames);
/** @brief merge two separate interleaved (32bit signed integer) audio data blocks into one
 * @see _ambix_mergeAdapator_float32
 */
ambix_err_t _ambix_mergeAdaptor_int32(const int32_t*source1, uint32_t source1channels, const int32_t*source2, uint32_t source2channels, int32_t*destination, int64_t frames);
/** @brief merge two separate interleaved (16bit signed integer) audio data blocks into one
 * @see _ambix_mergeAdapator_float32
 */
ambix_err_t _ambix_mergeAdaptor_int16(const int16_t*source1, uint32_t source1channels, const int16_t*source2, uint32_t source2channels, int16_t*destination, int64_t frames);


/** @brief merge interleaved ambisonics and interleaved non-ambisonics channels into a single interleaved audio data block using matrix operations
 *
 * multiply matrix.rows ambisonics channels with the matrix to get a (reduced) set of
 * matrix.cols ambix-extended channels.
 * append ambix-extended and non-ambisonics channels into one big interleaved chunk
 *
 * @param source1 the first interleaved samplebuffer (full ambisonics set) to read from
 * @param matrix the encoder-matrix
 * @param source2 the second interleaved samplebuffer to read from
 * @param source2channels the number of channels in source2
 * @param destination the samplebuffer to merge the data info; must be big enough to hold frames*(matrix.cols+source2channels) samples
 * @param frames number of frames to extract
 * @return error code indicating success
 */
ambix_err_t _ambix_mergeAdaptormatrix_float32(const float32_t*source1, const ambix_matrix_t*matrix, const float32_t*source2, uint32_t source2channels, float32_t*destination, int64_t frames);
/* @see _ambix_mergeAdaptormatrix_float32 */
ambix_err_t _ambix_mergeAdaptormatrix_float64(const float64_t*source1, const ambix_matrix_t*matrix, const float64_t*source2, uint32_t source2channels, float64_t*destination, int64_t frames);
/* @see _ambix_mergeAdaptormatrix_float32 */
ambix_err_t _ambix_mergeAdaptormatrix_int32(const int32_t*source1, const ambix_matrix_t*matrix, const int32_t*source2, uint32_t source2channels, int32_t*destination, int64_t frames);
/* @see _ambix_mergeAdaptormatrix_float32 */
ambix_err_t _ambix_mergeAdaptormatrix_int16(const int16_t*source1, const ambix_matrix_t*matrix, const int16_t*source2, uint32_t source2channels, int16_t*destination, int64_t frames);


/** @brief debugging printout for ambix_info_t
 * @param info an ambixinfo struct
 */
void _ambix_print_info(const ambix_info_t*info);

#define MARK() printf("%s:%d[%s]\n", __FILE__, __LINE__, __FUNCTION__)



/** @brief create a diagonal matrix from a vector
 * @param orgmatrix pointer to the matrix object that will hold the result or NULL
 * @param diag array of count floats that will form the diagonal vector
 * @param count number of elements in diag
 * @return pointer to the resulting diagonal matrix, or NULL in case something went wrong
 */
ambix_matrix_t*_matrix_diag(ambix_matrix_t*orgmatrix, const float32_t*diag, uint32_t count);

/** @brief create a permutation matrix
 * @param orgmatrix pointer to the matrix object that will hold the result or NULL
 * @param route permutation vector (if(route[1]==4)then{row#1 of output matrix will be [0 0 0 0 1 0 ...]}
 * @param count number of elements in route
 * @param transpose whether the result should be transposed (swapped rows and columns)
 * @return pointer to the resulting permutation matrix, or NULL in case something went wrong
 */
ambix_matrix_t*_matrix_router(ambix_matrix_t*orgmatrix, const float32_t*route, uint32_t count, int transpose);
/** @brief create a permutation matrix
 * @param matrix pointer to a properly initialized matrix object that will hold the result
 * @param permutate permutation vector (if(permutate[1]==4)then{row#1 of output matrix will be [0 0 0 0 1 0 ...]}; 
 *        the permutate vector must have matrix->rows elements (matrix->cols if swap is TRUE);
 *        negative permutation indices will skip the given row (if(permutate[3]=-1)then{row#3 will be [0 0 0 ...]};
 *        permutation indices exceeding the valid range will restult in an error
 * @param transpose whether the result should be transposed (swapped rows and columns)
 * @return pointer to the resulting permutation matrix, or NULL in case something went wrong
 */
ambix_matrix_t*_matrix_permutate(ambix_matrix_t*matrix, const float32_t*permutate, int swap);

/** @brief calculate the permutation vector to convert from SID to ACN
 * @param data caller-allocated array of count elements that will hold the result
 * @param count number of elements in data
 * @result 1 on success, 0 if something went wrong (in which case the content of data is undefined
 */
int _matrix_sid2acn(float32_t*data, uint32_t count);

/** @brief calculate the adaptor matrix to convert from FuMa to standard matrices
 * @param channels number of Furse-Malham channels that need to be converted to standard set (3, 4, 5, 6, 7, 8, 9, 11, 16)
 * @result an adaptor matrix (or NULL in case of failure); it's the responsibility of the caller to free the matrix
 * @see http://members.tripod.com/martin_leese/Ambisonic/B-Format_file_format.html
 */
ambix_matrix_t*_matrix_fuma2ambix(uint32_t channels);
ambix_matrix_t*_matrix_ambix2fuma(uint32_t channels);


#endif /* AMBIX_PRIVATE_H */
