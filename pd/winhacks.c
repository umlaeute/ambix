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

#ifdef _WIN32
#include <string.h>
#include <stdlib.h>
char *
strndup (const char *s, size_t n)
{
  char *result = NULL;
  size_t len = strlen (s);

  if (n < len)
    len = n;

  result = (char *) malloc (len + 1);
  if (!result)
    return 0;

  result[len] = '\0';
  return (char *) memcpy (result, s, len);
}
#endif /* _WIN32 */
