#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "serial.h"
#include "datalist.h"
#define DEBUG 1 //Must be defined before #include "gtd.h"
#include "gtd.h"

static FILE * hist = NULL;

int init_serial(char * mode) {
  char * filename;
  char * homepath;
  if(hist) {
    return end_serial();
  }

  homepath = getenv("HOME");
  if (homepath == NULL) {
    int namesize = strlen(DEFAULT_PATH) + 1 + sizeof(HISTFILE); // /etc + / + gtd.hist
    filename = malloc(namesize);
    snprintf(filename, namesize, "%s/%s", DEFAULT_PATH, HISTFILE);
  } else {
    int namesize = strlen(homepath) + 2 + sizeof(HISTFILE); // /home/name + /. + gtd.hist
    filename = malloc(namesize);
    snprintf(filename, namesize, "%s/.%s", homepath, HISTFILE);
  }

  printd("using histfile : %s\n", filename);
  hist = fopen(filename, mode);
  free(filename);
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
  char *buff = NULL;
  char *path;
  char *dirname;
  char *saveptr;
  ssize_t length;
  size_t len = 0;
  int linenb = 0;
  if (!hist){
    printd("read_next_entry: serial has not been initialized!\n");
    return -1;
  }

  while (1) {
    // Parse a line
    length = getline(&buff, &len, hist);
    if (length == -1) {
      break;
    }

    linenb++;
    printd("length : %lu, value : %s", length, buff);
    dirname = strtok_r(buff, " ", &saveptr);
    if (!dirname) {
      printd("Invalid line %d, skipping\n", linenb);
      continue;
    }

    while ((path = strtok_r(NULL, " \n", &saveptr)) != NULL) {
      unsigned long scorel = 0;
      char *score = strtok_r(NULL, " ", &saveptr);
      char *remains;
      if (!score) {
        printd("Error line %d, no score for path '%s'\n", linenb, path);
        break;
      }

      errno = 0;
      scorel = strtoul(score, &remains, 10);
      if ((remains && remains[0] != '\0') || errno != 0) {
        printd("Error line %d, invalid score for path '%s' : %s, %s\n",
               linenb, path, remains, strerror(errno));
        break;
      }

      printd("path : %s, dirname : %s, score : %lu\n", path, dirname, scorel);
      update_entry_score(path, dirname, scorel);
    }
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
