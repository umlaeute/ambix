/* winhacks.h -  AMBIsonics eXchange hacks for w32 compilation     -*- c -*-

   Copyright © 2012 IOhannes m zmölnig <zmoelnig@iem.at>.
         Institute of Electronic Music and Acoustics (IEM),
         University of Music and Dramatic Arts, Graz

   Copyright © 2005 Free Software Foundation, Inc.
         Written by Kaveh R. Ghazi <ghazi@caip.rutgers.edu>.

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

#ifdef __MINGW32__
int _get_output_format( void ) {
	return 1;
}
#endif /* __MINGW32__ */

