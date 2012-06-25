#include <stdio.h>
#include "serial.h"

static FILE * hist = NULL;

int init_serial() {

 // hist = fopen(HISTFILE, "a+");
  hist = fopen(HISTFILE, "w+"); // TODO: change w+ to a+ once it's possible to read serialized files
  if (!hist) {
    perrord("init_serial: fopen:");
    return -1;
  }

  return 0;
}

int end_serial() {

  if(fclose(hist) != 0) {
    perrord("end_serial: fclose:");
    return -1;
  }

  return 0;
}

void dump_entry(const struct entry * to_dump) {
  struct candidate * c_dump;
  if (!hist){
    printd("dump_entry: serial has not been initialized!\n");
    return;
  }
  
  c_dump = to_dump->first_candidate;
  fprintf(hist, "%s %s %d ", to_dump->dirname, c_dump->path, c_dump->score);
  for (c_dump = c_dump->next; c_dump != to_dump->first_candidate; c_dump = c_dump->next) {
    fprintf(hist, "%s %d ", c_dump->path, c_dump->score);
  }
  fprintf(hist, "\n");
}
