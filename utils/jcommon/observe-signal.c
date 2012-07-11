/*  observe-signal.c - (c) rohan drape, 2005-2006 */

#define _XOPEN_SOURCE 600 /* To use SA_RESTART and SA_RESETHAND */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

#include <pthread.h>

#include "observe-signal.h"

volatile unsigned int signal_received = 0;

static void signal_management_handler(int s)
{
  signal_received |= 1 << s;
}

static void *signal_management_thread(void *PTR)
{
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, 0);
  sigset_t blocked;
  sigprocmask(SIG_SETMASK, 0, &blocked);
  int s;
  sigwait(&blocked, &s);
  if(s != SIGSEGV) {
    sigprocmask(SIG_UNBLOCK, &blocked, 0);
  }
  signal_management_handler(s);
  return NULL;
}

int observe_signals(void)
{
  sigset_t signals;
  signal_received = 0;
  sigemptyset(&signals);
  sigaddset(&signals, SIGHUP);
  sigaddset(&signals, SIGINT);
  sigaddset(&signals, SIGQUIT);
  sigaddset(&signals, SIGPIPE);
  sigaddset(&signals, SIGTERM);
  struct sigaction action;
  action.sa_handler = signal_management_handler;
  action.sa_mask = signals;
  action.sa_flags = SA_RESTART | SA_RESETHAND;
  int i;
  for(i = 1; i < 32; i++) {
    if(sigismember(&signals, i)) {
      if(sigaction(i, &action, 0)) {
	fprintf(stderr, "sigaction() failed: %d\n", i);
	return -1;
      }
    }
  }
  if(pthread_sigmask(SIG_SETMASK, &signals, 0)) {
    fprintf(stderr, "pthread_sigmask() failed: %s\n", strerror(errno));
    return -1;
  }
  pthread_t signal_thread_id;
  if(pthread_create(&signal_thread_id, 0, signal_management_thread, 0)) {
    fprintf(stderr, "pthread_create() failed\n");
    return -1;
  }
  pthread_detach(signal_thread_id);
  return 0;
}

bool observe_end_of_process(void)
{
  return(signal_received & 1<<SIGHUP ||
	 signal_received & 1<<SIGINT ||
	 signal_received & 1<<SIGQUIT ||
	 signal_received & 1<<SIGPIPE ||
	 signal_received & 1<<SIGTERM);
}
