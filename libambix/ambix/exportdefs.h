/* exportdefs.h -  defines for dll import/export              -*- c -*-

   Copyright (C) 2012 IOhannes m zmölnig <zmoelnig@iem.at>
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

#ifndef AMBIX_EXPORTDEFS_H
#define AMBIX_EXPORTDEFS_H

#if defined _MSC_VER
/* MSVC always uses dllimport/dllexport */
# define DLL_EXPORT
#endif /* _MSC_VER */


#ifdef DLL_EXPORT
// Windows requires explicit import and exporting of functions and classes.
// While this is a pain to do sometimes, in large software development
//      projects, it is very usefull.
# define AMBIX_EXPORT __declspec(dllexport)
# define AMBIX_IMPORT __declspec(dllimport)

# define AMBIX_DEPRECATED __declspec(deprecated)

#elif defined __GNUC__
# define AMBIX_EXPORT
# define AMBIX_IMPORT

# define AMBIX_DEPRECATED __attribute__ ((deprecated))

#else
/* unknown compiler */
# warning set up compiler specific defines
#endif


#ifdef AMBIX_INTERNAL
# define AMBIX_EXTERN AMBIX_EXPORT
#else
# define AMBIX_EXTERN AMBIX_IMPORT
#endif




#endif /* AMBIX_EXPORTDEFS_H */
