#ifndef GTD_H_
#define GTD_H_

#include <stdio.h>

#define SOCKET "/tmp/gtd.socket"
#define HISTFILE "/usr/local/share/gtd/gtd.hist"

#ifdef DEBUG
  #define printd(...) fprintf(stderr,__VA_ARGS__)
  #define perrord(a) perror(a)
#else
  #define printd(...)
  #define perrord(a)
#endif

#endif
