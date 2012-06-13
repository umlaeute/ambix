/* jcommon/common.h -  common helper functions for jack-utils            -*- c -*-

   Copyright © 2003-2001 Rohan Drape <rd@slavepianos.org>
   Copyright © 2012 IOhannes m zmölnig <zmoelnig@iem.at>.
         Institute of Electronic Music and Acoustics (IEM),
         University of Music and Dramatic Arts, Graz

   This file is based on Rohan Drape's "jack-tools" collection

   you can redistribute it and/or modify
   it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   ambix-jplay is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU General Public
   License along with this program; if not, see <http://www.gnu.org/licenses/>.
*/

#ifndef JCOMMON_COMMON_H
#define JCOMMON_COMMON_H

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */

#include <jack/jack.h>


#ifndef FAILURE
#define FAILURE exit(1)
#endif

#define eprintf(...) fprintf(stderr,__VA_ARGS__)

void *xmalloc(size_t size);
int xpipe(int filedes[2]);
ssize_t xwrite(int filedes, const void *buffer, size_t size);
ssize_t xread(int filedes, void *buffer, size_t size);





void jack_client_minimal_error_handler(const char *desc);
void jack_client_minimal_shutdown_handler(void *arg);
int jack_transport_is_rolling(jack_client_t *client);
jack_client_t *jack_client_unique_(const char*name); /* this simply create a unique client-name, withbout telling us... */
jack_client_t *jack_client_unique(char*name); /* this will change 'name' to the actual result */

jack_port_t*_jack_port_register(jack_client_t *client, int direction, const char*format, int n);

#endif /* JCOMMON_COMMON_H */
