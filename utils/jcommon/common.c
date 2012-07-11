#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include "common.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>

void *xmalloc(size_t size)
{
  void *p = malloc(size);
  if(p == NULL) {
    fprintf(stderr, "malloc() failed: %ld\n", (long)size);
    FAILURE;
  }
  return p;
}

int xpipe(int filedes[2])
{
  int err = pipe(filedes);
  if(err) {
    perror("pipe() failed");
    FAILURE;
  }
  return err;
}

ssize_t xwrite(int filedes, const void *buffer, size_t size)
{
  ssize_t err = write(filedes, buffer, size);
  if(err == -1) {
    perror("write() failed");
    FAILURE;
  }
  return err;
}

ssize_t xread(int filedes, void *buffer, size_t size)
{
  ssize_t err = read(filedes, buffer, size);
  if(err == -1) {
    perror("read() failed");
    FAILURE;
  }
  return err;
}


void jack_client_minimal_error_handler(const char *desc)
{
  eprintf("jack error: %s\n", desc);
}

void jack_client_minimal_shutdown_handler(void *arg)
{
  eprintf("jack shutdown\n");
  FAILURE;
}

int jack_transport_is_rolling(jack_client_t *client)
{
  jack_transport_state_t s = jack_transport_query(client , NULL);
  return s & JackTransportRolling;
}

jack_client_t *jack_client_unique(char *name)
{
  int n = (int)getpid();
  char uniq[64];
  snprintf(uniq, 64, "%s-%d", name, n);
  strncpy(name,uniq,64);
  jack_client_t *client = jack_client_open(uniq,JackNullOption,NULL);
  if(! client) {
    eprintf("jack_client_open() failed: %s\n", uniq);
    FAILURE;
  }
  return client;
}
jack_client_t *jack_client_unique_(const char *name)
{
  char uniq[64];
  snprintf(uniq, 64, "%s", name);
  return jack_client_unique(uniq);
}


jack_port_t*_jack_port_register(jack_client_t *client, int direction, const char*format, int n) {
  char name[64];
  jack_port_t*port;
  snprintf(name, 64, format, n);
  port = jack_port_register(client, name, JACK_DEFAULT_AUDIO_TYPE, direction, 0);
  if(!port) {
    eprintf("jack_port_register() failed at %s\n", name);
    FAILURE;
  }
  return port;
}
