#include "common.h"
#include <stdlib.h>
#include <stdio.h>


void *xmalloc(size_t size)
{
  void *p = malloc(size);
  if(p == NULL) {
    fprintf(stderr, "malloc() failed: %ld\n", (long)size);  
    FAILURE;
  }
  return p;
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
