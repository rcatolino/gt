#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "serial.h"
#include "datalist.h"
#define DEBUG 1 //Must be defined before #include "gtd.h"
#include "gtd.h"

static FILE * hist = NULL;

int init_serial(char * mode) {
  if(hist) {
    return end_serial();
  }
  hist = fopen(HISTFILE, mode);
  if (!hist) {
    perrord("init_serial: fopen");
    return -1;
  }

  return 0;
}

int end_serial() {

  if(!hist) {
    printd("end_serial: no histfile\n");
    return -1;
  }
  if(fclose(hist) != 0) {
    perrord("end_serial: fclose:");
    return -1;
  }
  hist = NULL;

  return 0;
}

int read_next_entry() {
  char * buff = NULL;
  char * path;
  char * dirname;
  char * remains;
  ssize_t length;
  size_t len = 0;
  if (!hist){
    printd("read_next_entry: serial has not been initialized!\n");
    return -1;
  }
  while (1) {
    length = getline(&buff, &len, hist);
    if (length == -1) {
      break;
    }
    printd("length : %lu, value : %s",length, buff);
    dirname = buff;
    path = strchr(buff,' ');
    if (!path) {
      break;
    }
    path[0] = '\0';
    path++;
    remains = strchr(path,' ');
    remains[0] = '\0';
    // TODO: treat remainings candidates
    printd("path : %s, dirname : %s \n", path, dirname);
    update_entry(path, dirname);
  }
  free(buff);
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
  if (ret <= 0){
    perrord("fprintf:");
  }
  for (c_dump = c_dump->next; c_dump != to_dump->first_candidate; c_dump = c_dump->next) {
    fprintf(hist, "%s %d ", c_dump->path, c_dump->score);
  }
  fputc('\n', hist);
}
