#ifndef GTD_H_
#define GTD_H_

#include <stdio.h>

#define SOCKET "/tmp/gtd.socket"
#define HISTFILE "gtd.hist"
#define DEFAULT_PATH "/etc"

#ifdef DEBUG
  #define printd(...) fprintf(stderr,__VA_ARGS__)
  #define perrord(a) perror(a)
#else
  #define printd(...)
  #define perrord(a)
#endif

#endif
