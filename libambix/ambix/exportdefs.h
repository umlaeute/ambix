/* ambix/exportdefs.h -  defines for dll import/export  -*- c -*-

   Copyright © 2012 IOhannes m zmölnig <zmoelnig@iem.at>
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
 * @file        ambix/exportdefs.h
 * @brief       export definitions for various compilers
 * @details This file is part of libambix
 * @author IOhannes m zmölnig <zmoelnig@iem.at>
 * @date 2012
 * @copyright LGPL-2.1
 */

#ifndef AMBIX_EXPORTDEFS_H
#define AMBIX_EXPORTDEFS_H

#if defined _MSC_VER
/* MSVC always uses dllimport/dllexport */
# define DLL_EXPORT
#endif /* _MSC_VER */

#ifdef DLL_EXPORT
/* Windows requires explicit import and exporting of functions and classes.
 * While this is a pain to do sometimes, in large software development
 *      projects, it is very useful.
 */
# define AMBIX_EXPORT __declspec(dllexport)
# define AMBIX_IMPORT __declspec(dllimport)

# define AMBIX_DEPRECATED __declspec(deprecated)

#elif defined __GNUC__
# define AMBIX_EXPORT  __attribute__ ((visibility("default")))
# define AMBIX_IMPORT

# define AMBIX_DEPRECATED __attribute__ ((deprecated))

#else
/* unknown compiler */
# warning set up compiler specific defines

/** mark symbols to be exported from libambix */
# define AMBIX_EXPORT
/** mark symbols to be imported from libambix */
# define AMBIX_IMPORT
/** mark symbols to be deprecated */
# define AMBIX_DEPRECATED
#endif

#ifdef AMBIX_INTERNAL
/** mark symbols to be useable outside the library */
# define AMBIX_API AMBIX_EXPORT
#else
/** mark symbols to be useable outside the library */
# define AMBIX_API AMBIX_IMPORT
#endif

#endif /* AMBIX_EXPORTDEFS_H */
