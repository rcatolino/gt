#ifndef GTD_H_
#define GTD_H_

#define SOCKET "/tmp/gtd.socket"
#define HISTFILE "/usr/local/share/gtd/gtd.hist"
#define MAX_SIZE 100
#define DETACHED 1
#define ATTACHED 0

#ifdef DEBUG
  #define printd(...) fprintf(stderr,__VA_ARGS__)
  #define perrord(a) perror(a)
#else
  #define printd(...)
  #define perrord(a)
#endif

struct entry{
  struct entry* prev;
  struct entry* next;
  char dirname[MAX_SIZE]; 
  struct candidate *first_candidate;
  int state;
};

struct candidate{
  struct candidate* prev;
  struct candidate* next;
  char path[MAX_SIZE];
  int score;
};

#endif
