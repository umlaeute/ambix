/*  jack-ringbuffer.c - (c) rohan drape, 2005-2006 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <errno.h>
#include <sys/types.h>

#include "jack-ringbuffer.h"
#include "common.h"

void jack_ringbuffer_print_debug(const jack_ringbuffer_t *r, const char *s)
{
  eprintf("%s: read_ptr=%d,write_ptr=%d,size=%d,size_mask=%d\n",
	  s, (int)r->read_ptr, (int)r->write_ptr,
	  (int)r->size, (int)r->size_mask);
}

int jack_ringbuffer_wait_for_read(const jack_ringbuffer_t *r,
				  int nbytes, int fd)
{
  int space = (int) jack_ringbuffer_read_space(r);
  while(space < nbytes) {
    char b;
    if(read(fd, &b, 1)== -1) {
      eprintf("%s: error reading communication pipe\n", __func__);
      FAILURE;
    }
    space = (int) jack_ringbuffer_read_space(r);
  }
  return space;
}

int jack_ringbuffer_wait_for_write(jack_ringbuffer_t *r, int nbytes, int fd)
{
  int space = (int)jack_ringbuffer_write_space(r);
  while(space < nbytes) {
    char b;
    if(read(fd, &b, 1)== -1) {
      fprintf (stderr, "%s: error reading communication pipe\n", __func__);
      FAILURE;
    }
    space = (int) jack_ringbuffer_write_space(r);
  }
  return space;
}

void jack_ringbuffer_read_exactly(jack_ringbuffer_t *r, char *buf, int n)
{
  int err = jack_ringbuffer_read(r, buf, n);
  if(err != n) {
    eprintf("%s: error reading ring buffer (%d != %d)\n",
	    __func__, err, n);
    FAILURE;
  }
}

void jack_ringbuffer_write_exactly(jack_ringbuffer_t *r, const char *buf, int n)
{
  int err = jack_ringbuffer_write(r, buf, n);
  if(err != n) {
    eprintf("%s: error writing ring buffer (%d != %d)\n",  __func__, err, n);
    FAILURE;
  }
}
