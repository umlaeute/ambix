/** @file
 *  @brief General documentation using doxygen
 *  @private
 */

/* libambix.h -  Documentation for Ambisonics Xchange Library              -*- c -*-

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
 * @mainpage AMBIX - the AMBIsonics eXchange library
 *
 * @section intro_sec Introduction
 *
 * libambix is a library that allows you to read and write AMBIX-files.
 * It is strongly modelled after libsndfile, which it uses as a backend
 *
 *
 * @section usage_sec Usage
 *
 * libambix provides a (hopefully easy to use) C-interface.
 * Basically you open a file using ambix_open(), read audio data using @ref ambix_readf
 * or write audio data using @ref ambix_writef and call ambix_close() once you are done.
 *
 * For a more detailed documentation, see the @link usage Usage page @endlink.
 *
 * @section format_sec Format
 *
 * For a shortish specification of the ambix format see the @link format Format page @endlink.
 */

/**
 * @page usage Usage
 *
 * @section readsimple_usage Reading SIMPLE ambix files
 *
 * You can read any ambix file as "SIMPLE" by setting the 'fileformat' member of the ambix_info_t struct
 * to AMBIX_SIMPLE prior to opening the file.
 * This will automatically do any conversion needed, by pre-multiplying the raw ambisonics data with an embedded
 * adaptor matrix.
 *
 * @code
   ambix_t*ambix = NULL;
   ambix_info_t*info  = calloc(1, sizeof(ambix_info_t));

   ambix->fileformat = AMBIX_SIMPLE; 

   ambix = ambix_open("ambixfile.caf", AMBIX_READ, info);
   if(ambix) {
     uint64_t frames = info->frames;
     uint64_t blocksize = 1024;

     float32_t*ambidata  = calloc(info->ambichannels  * blocksize, sizeof(float32_t));
     float32_t*extradata = calloc(info->extrachannels * blocksize, sizeof(float32_t));

     while(frames>blocksize) {
       uint64_t blocks = ambix_readf_float32(ambix, ambidata, extradata, blocksize);

       // process blocks frames of interleaved ambidata and interleaved extradata
       // ...

       frames-=blocks;
     }

     ambix_readf_float32(ambix, ambidata, extradata, frames);

     // process last block of interleaved ambidata and interleaved extradata
     // ...

     ambix_close(ambix);
     free(ambidata);
     free(extradata);
   }
 * @endcode
 *
 *
 *
 * @section readextended_usage Reading EXTENDED ambix files
 *
 * You can read an ambix file as "EXTENDED" by setting the 'fileformat' member of the ambix_info_t struct
 * to AMBIX_EXTENDED prior to opening the file.
 * You will then have to retrieve the adaptor matrix from the file, in order to be able to reconstruct the 
 * full ambisonics set.
 * You can also analyse the matrix to make educated guesses about the original channel layout.
 *
 * @code
   ambix_t*ambix = NULL;
   ambix_info_t*info  = calloc(1, sizeof(ambix_info_t));

   // setting the fileformat to AMBIX_EXTENDED forces the ambi data to be delivered as stored in the file
   ambix->fileformat = AMBIX_EXTENDED; 

   ambix = ambix_open("ambixfile.caf", AMBIX_READ, info);
   if(ambix) {
     uint64_t frames = info->frames;
     uint64_t blocksize = 1024;

     float32_t*ambidata  = calloc(info->ambichannels  * blocksize, sizeof(float32_t));
     float32_t*extradata = calloc(info->extrachannels * blocksize, sizeof(float32_t));

     const ambix_matrix_t*adaptormatrix=ambix_get_adaptormatrix(ambix);

     while(frames>blocksize) {
       uint64_t blocks = ambix_readf_float32(ambix, ambidata, extradata, blocksize);

       // process blocks frames of interleaved ambidata and interleaved extradata,
       // using the adaptormatrix
       // ...

       frames-=blocks;
     }

     ambix_readf_float32(ambix, ambidata, extradata, frames);

     // process last block of interleaved ambidata and interleaved extradata
     // using the adaptormatrix
     // ...

     ambix_close(ambix);
     free(ambidata);
     free(extradata);
   }
 * @endcode
 *
 *
 * @section readunknown_usage Reading any ambix files
 *
 * If you don't specify the format prior to opening, you can query the format of the file
 * from the ambix_info_t struct.
 *
 * @code
   ambix_t*ambix = NULL;
   ambix_info_t*info  = calloc(1, sizeof(ambix_info_t)); // initialize the format field (among others) to 0

   ambix = ambix_open("ambixfile.caf", AMBIX_READ, info);

   if(ambix) {
     switch(ambix->fileformat) {
     case(AMBIX_SIMPLE):
       printf("this file is ambix simple\n");
       break;
     case(AMBIX_EXTENDED):
       printf("this file is ambix extended\n");
       break;
     case(AMBIX_NONE):
       printf("this file is not an ambix file\n");
       break;
     default:
       printf("this file is of unknown format...\n");
     }

     // using the adaptormatrix
     // ...

     ambix_close(ambix);
   }
 * @endcode
 *
 *
 */

/**
 * @page format The ambix format
 *
 * @section todo_format TODO
 *
 * document the ambix format
 */
