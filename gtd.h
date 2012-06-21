#ifndef GTD_H_
#define GTD_H_

#define SOCKET "/tmp/gtd.socket"
#define MAX_SIZE 100
#define DETACHED 1
#define ATTACHED 0
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
