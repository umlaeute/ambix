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
 * @section read_usage Reading ambix files
 *
 * When opening a file for read, the ambix_info_t struct should be set to 0.
 * On successfull open, all fields are filled by the library.
 * If the open fails, the state of the ambix_info_t fields is undefined.
 *
 * The only exception to this is, if you want to force the ambix file to be read as either
 * "SIMPLE" or "EXTENDED", e.g. because you don't want to care about adaptor matrices or
 * because you do. In this case you must set the fileformat field the requested format.
 *
 *
 *
 * @subsection readsimple_usage Reading SIMPLE ambix files
 *
 * You can read any ambix file as "SIMPLE" by setting the 'fileformat' member of the ambix_info_t struct
 * to AMBIX_SIMPLE prior to opening the file.
 * This will automatically do any conversion needed, by pre-multiplying the raw ambisonics data with an embedded
 * adaptor matrix.
 *
 * real "SIMPLE" files lack extra audio channels.
 * However, when opening a file that is not a "SIMPLE" ambix file (e.g. an "EXTENDED" ambix file) as a "SIMPLE" one,
 * extra channels might be readable.
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
 * @subsection readextended_usage Reading EXTENDED ambix files
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
 * @subsection readunknown_usage Reading any ambix files
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
 * @section write_usage Writing ambix files
 *
 * To write data to an ambix file, you have to open it with the AMBIX_WRITE flag.
 * You also need to specify some global properties of the output data, namely the samplerate and the sampleformat,
 * as well as the number of ambisonics channels and the number of extra channels that are phyiscally stored on the disk.
 *
 * @subsection writesimple_usage Writing SIMPLE ambix files
 *
 * You can write "SIMPLE" ambix files by setting the 'fileformat' member of the ambix_info_t struct
 * to AMBIX_SIMPLE prior to opening the file.
 *
 * You will need to provide a full set of ambisonics channels when writing data to the file,
 * and must not set an adaptor matrix.
 * A full set of ambisonics must always satisfy the formula @f$channels=(order_{ambi}+1)^2@f$.
 *
 * You cannot write extra audio channels into a "SIMPLE" ambix file.
 *
 * @code
   ambix_t*ambix = NULL;
   ambix_info_t*info  = calloc(1, sizeof(ambix_info_t));

   // need to specify samplerate and sampleformat
   ambix->samplerate = 44100;
   ambix->sampleformat = AMBIX_SAMPLEFORMAT_PCM16;
   ambix->fileformat = AMBIX_SIMPLE;
   ambix->ambichannels = 16; // 16 channels means 3rd order ambisonics, according to L=(2N+1)^2

   ambix = ambix_open("ambixfile.caf", AMBIX_WRITE, info);
   if(ambix) {
     uint64_t frames = info->frames;
     uint64_t blocksize = 1024;
     uint64_t block;

     float32_t*ambidata  = calloc(info->ambichannels  * blocksize, sizeof(float32_t));

     while(haveData) {
       // acquire blocksize samples of a full set of 3rd order ambisonics data (16 channels)
       // into ambidata (interleaved)
       // ...

       block = ambix_writef_float32(ambix, ambidata, NULL, blocksize);
     }

     ambix_close(ambix);
     free(ambidata);
   }
 * @endcode
 *
 * @subsection writeextended_usage Writing EXTENDED ambix files
 *
 * You can write "EXTENDED" ambix files by setting the 'fileformat' member of the ambix_info_t struct
 * to AMBIX_EXTENDED prior to opening the file.
 *
 * You MUST set an adaptormatrix (to convert the reduced set to a full ambisonics set) using ambix_set_adaptormatrix() and
 * ensure that it gets written to disk by calling ambix_write_header() prior to writing any samples to the file.
 *
 * @code
   ambix_t*ambix = NULL;
   ambix_info_t*info  = calloc(1, sizeof(ambix_info_t));

   // need to specify samplerate and sampleformat
   ambix->samplerate = 44100;
   ambix->sampleformat = AMBIX_SAMPLEFORMAT_PCM16;
   ambix->fileformat = AMBIX_EXTENDED;
   ambix->ambichannels = 8;  // a reduced ambisonics set
   ambix->extrachannels = 1; // an extrachannel, e.g. click-track

   ambix = ambix_open("ambixfile.caf", AMBIX_WRITE, info);
   if(ambix) {
     uint64_t frames = info->frames;
     uint64_t blocksize = 1024;
     uint64_t block;

     float32_t*ambidata  = calloc(info->ambichannels  * blocksize, sizeof(float32_t));
     float32_t*extradata  = calloc(info->extrachannels  * blocksize, sizeof(float32_t));

     // create an adaptormatrix:
     ambix_matrix_t adaptormatrix = {0, 0, NULL};
     ambix_matrix_init(16, 8, &adaptormatrix);
     // fill the adaptormatrix, that expands our 8 channels to a full 3D 3rd-order set (16 channels)
     // ...

     ambix_set_adapatormatrix(ambix, &adaptormatrix);
     ambix_write_header(ambix);

     while(haveData) {
       // acquire blocksize samples of a full set of reduced ambisonics data (8 channels)
       // into ambidata (interleaved), and a some (1) extra channels
       // ...

       block = ambix_writef_float32(ambix, ambidata, extradata, blocksize);
     }

     ambix_close(ambix);
     ambix_matrix_deinit(&adaptormatrix);
     free(ambidata);
     free(extradata);
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
