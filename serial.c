#include <stdio.h>
#include "serial.h"

static FILE * hist = NULL;

int init_serial(char * mode) {
  if(hist) {
    return end_serial();
  }
  hist = fopen(HISTFILE, mode);
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

int read_next_entry(struct entry * to_fill) {
  if (!to_fill) {
    printd("Unallocated buffer\n");
    return -1;
  }

}
void dump_entry(const struct entry * to_dump) {
  struct candidate * c_dump;
  int ret = 0;
  if (!hist){
    printd("dump_entry: serial has not been initialized!\n");
    return;
  }
  printd("dumping entry\n");
  
  c_dump = to_dump->first_candidate;
  ret = fprintf(hist, "%s %s %d ", to_dump->dirname, c_dump->path, c_dump->score);
  if (ret <= 0){
    perrord("fprintf:");
  }
  for (c_dump = c_dump->next; c_dump != to_dump->first_candidate; c_dump = c_dump->next) {
    fprintf(hist, "%s %d ", c_dump->path, c_dump->score);
  }
  fprintf(hist, "\n");
}
