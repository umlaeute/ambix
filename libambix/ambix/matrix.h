/* ambix/matrix.h -  Matrix utilities              -*- c -*-

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
 * @file	ambix/matrix.h
 * @brief	Matrix utilities
 * @details This file is part of libambix
 * @author IOhannes m zmölnig <zmoelnig@iem.at>
 * @date 2012
 * @copyright LGPL-2.1
 */

#ifndef AMBIX_AMBIX_H
# error please dont include <ambix/matrix.h>...use <ambix/ambix.h> instead!
#endif

#ifndef AMBIX_MATRIX_H
#define AMBIX_MATRIX_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/** @brief Create a matrix
 *
 * Allocates a new (empty) matrix object.
 * It's equivalent to calling ambix_matrix_init(0, 0, NULL);
 *
 * @return a new matrix object or NULL
 */
AMBIX_API
ambix_matrix_t* ambix_matrix_create (void);

/** @brief Destroy a matrix
 *
 * Frees all ressources allocated for the matrix object.
 * It's a shortcut for ambix_matrix_deinit(mtx), free(mtx)
 *
 * @param mtx matrix object to destroy
 */
AMBIX_API
void ambix_matrix_destroy (ambix_matrix_t*mtx);

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
 */
AMBIX_API
ambix_matrix_t* ambix_matrix_init (uint32_t rows, uint32_t cols, ambix_matrix_t*mtx);


/** @brief De-initialize a matrix
 *
 * Frees associated ressources and sets rows/columns to 0
 *
 * @param mtx matrix object to deinitialize
 */
AMBIX_API
void ambix_matrix_deinit (ambix_matrix_t*mtx);


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
 * @return pointer to the matrix object, if the type was not valid (for the
 * input matrix)
 */
AMBIX_API
ambix_matrix_t* ambix_matrix_fill (ambix_matrix_t*matrix, ambix_matrixtype_t type);

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
 */
AMBIX_API
ambix_err_t ambix_matrix_fill_data (ambix_matrix_t*mtx, const float32_t*data);

/** @brief Copy a matrix to another matrix
 *
 * Copy a matrix, possibly resizing or creating the destination
 *
 * @param src the source matrix to copy the data from
 *
 * @param dest the destination matrix (if NULL a new matrix will be created)
 *
 * @return pointer to the destination matrix
 */
AMBIX_API
ambix_matrix_t* ambix_matrix_copy (const ambix_matrix_t*src, ambix_matrix_t*dest);
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
AMBIX_API
ambix_matrix_t* ambix_matrix_multiply (const ambix_matrix_t*A, const ambix_matrix_t*B, ambix_matrix_t*result);


/** @brief Multiply a matrix with data
 * @defgroup ambix_matrix_multiply_data ambix_matrix_multiply_data()
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
ambix_err_t ambix_matrix_multiply_float32(float32_t*dest, const ambix_matrix_t*mtx, const float32_t*source, int64_t frames);
/** @brief Multiply a matrix with (32bit signed integer) data
 *
 * @ingroup ambix_matrix_multiply_data
 */
AMBIX_API
ambix_err_t ambix_matrix_multiply_int32(int32_t*dest, const ambix_matrix_t*mtx, const int32_t*source, int64_t frames);
/** @brief Multiply a matrix with (16 bit signed integer) data
 *
 * @ingroup ambix_matrix_multiply_data
 */
AMBIX_API
ambix_err_t ambix_matrix_multiply_int16(int16_t*dest, const ambix_matrix_t*mtx, const int16_t*source, int64_t frames);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */
#endif /* AMBIX_MATRIX_H */
