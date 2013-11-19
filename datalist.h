#ifndef DATALIST_H_
#define DATALIST_H_

#define HTABLE_SIZE 53
#define MAX_SIZE 254
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

void dumpTable();

struct entry* create_entry(const char* path, const char* dirname);

int hash(const char* dirname);

void attach(struct entry* new_entry, int key);

struct entry* find_entry(const char* dirname, int key);

void swap_left(struct entry* current_e, struct candidate* current_c);

void push_candidate(struct entry* current, const char* path);

struct candidate* find_candidate(const char* path, struct entry* current_e);

struct entry* update_entry(const char* path, const char* dirname);

struct entry* update_entry_score(const char* path, const char* dirname, unsigned long score);

#endif
