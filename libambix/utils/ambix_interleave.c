/* ambix_interleave -  create an ambix file              -*- c -*-

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
 * @brief ambix_interleave - merge several (multi-channel) audio files into a single ambix file
 *
 * ambix_interleave -o <outfile> [-O <order>] [-X <matrixfile>] <infile1> [<infile2> ...]
 * merge several (multi-channel) audio files into a single ambix file;
 * infile1 becomes W-channel, infile2 becomes X-channel,...
 * by default this will write an 'ambix simple' file (only full sets are accepted) 
 * eventually files are written as 'ambix extended' file with adaptor matrix set to unity
 * if 'order' is specified, all inchannels not needed for the full set are written as 'extrachannels'
 * 'matrixfile' is a soundfile/octavefile that is interpreted as matrix: each channel is a row, sampleframes are columns
 * if 'matrix' is specified it must construct a full-set (it must satisfy rows=(O+1)^2)
 * if 'matrix' is specified, all inchannels not needed to reconstruct to a full set are 'extrachannels'
 * if both 'order' and 'matrix' are specified they must match
 */
#include "ambix/ambix.h"

#include <stdio.h>
#include <string.h>

void help(const char*path) {

}

int main(int argc, char**argv) {
  printf("%s not implemented yet\n", argv[0]);

  return 0;
}
