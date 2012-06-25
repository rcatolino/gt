#include <stdio.h>
#include "serial.h"

static FILE * hist = NULL;

int init_serial() {

  hist = fopen(HISTFILE, "a+");
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
  int ret = 0;
  if (!hist){
    printd("dump_entry: serial has not been initialized!\n");
    return;
  }
  printd("dumping entry\n");
  
  c_dump = to_dump->first_candidate;
  ret = fprintf(hist, "%s %s %d ", to_dump->dirname, c_dump->path, c_dump->score);
  //printd("%s %s %d %d", to_dump->dirname, c_dump->path, c_dump->score, ret);
  if (ret <= 0){
    perror("fprintf:");
  }
  for (c_dump = c_dump->next; c_dump != to_dump->first_candidate; c_dump = c_dump->next) {
    printf("dumping candidate\n");
    fprintf(hist, "%s %d ", c_dump->path, c_dump->score);
  }
  fprintf(hist, "\n");
}
