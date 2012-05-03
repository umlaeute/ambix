#ifndef _COMMON_JACK_RINGBUFFER_H
#define _COMMON_JACK_RINGBUFFER_H

#include <jack/jack.h>
#include <jack/ringbuffer.h>

void jack_ringbuffer_print_debug(const jack_ringbuffer_t *r, const char *s);
int jack_ringbuffer_wait_for_read(const jack_ringbuffer_t *r, int nbytes, int fd);
int jack_ringbuffer_wait_for_write(jack_ringbuffer_t *r, int nbytes, int fd);
void jack_ringbuffer_read_exactly(jack_ringbuffer_t *r, char *buf, int n);
void jack_ringbuffer_write_exactly(jack_ringbuffer_t *r, const char *buf, int n);

#endif
