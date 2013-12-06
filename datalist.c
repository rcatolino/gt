#include "datalist.h"
#define DEBUG 1
#include "gtd.h"
#include "serial.h"
#include <stdlib.h>
#include <string.h>

struct entry* htable[HTABLE_SIZE]={NULL}; //other + Upper case + Lower case

void dumpTable() {
  int i = 0;
  printd("dumping histfile\n");
  init_serial("w");
  for(; i < HTABLE_SIZE; i++) {
    if(htable[i] != NULL) {
      struct entry *j = htable[i];
      do {
        dump_entry(j);
        j = j->next;
      } while (j != htable[i]);
    }
  }
  printd("end: removing resources\n");
  end_serial();
}

struct entry * create_entry(const char * path, const char *dirname){
  struct entry *new_entry=malloc(sizeof(struct entry));

  strcpy(new_entry->dirname,dirname);
  new_entry->prev=NULL;
  new_entry->next=NULL;
  new_entry->state=DETACHED;
  new_entry->first_candidate=malloc(sizeof(struct candidate));
  new_entry->first_candidate->prev=new_entry->first_candidate;
  new_entry->first_candidate->next=new_entry->first_candidate;
  new_entry->first_candidate->score=1;
  strcpy(new_entry->first_candidate->path,path);
  return new_entry;
}

int hash(const char * dirname){
  int letter = dirname[0];
  if(letter>64 && letter<91){
    return letter-64;
  }else if(letter>96 && letter<123){
    return letter-70;
  }else{
    return 0;
  }
  return 0;//TODO find an addapted hash function
}

void attach(struct entry * new_entry, int key){
  if(htable[key]==NULL){
    printd("attach entry %s, key %d. No previous entry for this key\n", new_entry->dirname, key);
    new_entry->prev=new_entry;
    new_entry->next=new_entry;
    htable[key]=new_entry;
  }else{
    printd("attach entry %s, key %d. Existing entry for this key : %s\n", new_entry->dirname, key, htable[key]->dirname);
//Update new_entry's links:
    new_entry->prev=htable[key]->prev;
    new_entry->next=htable[key];
//Update last element's links:
    htable[key]->prev->next=new_entry;
//Update first element's links:
    htable[key]->prev=new_entry;
//Update pointer on first element
//This is not strictly necessary, if we don't, new entry could be considered
//to be added at last position, if we do it will be added at first position.
//Next time we look for this key's first entry we'll find the last element added.
//This is a good thing if we consider that the last element added is most likely to
//be searched again in the near future.
    htable[key]=new_entry;
  }
  new_entry->state=ATTACHED;
}

struct entry *find_entry(const char *dirname, int key){
  printd("find_entry %s, key: %d\n",dirname, key);
  if (htable[key] != NULL){
    struct entry *current = htable[key];
    do {
      if (strcmp(dirname, current->dirname) == 0) {
        return current;
      }

      current = current->next;
    } while (current != htable[key]);
  }

  printd("find_entry: no such path\n");
  return NULL;
}

void swap_left(struct entry* current_e, struct candidate *current_c){
  struct candidate *prev_c=current_c->prev;
  //update current_e link to first candidate:
  if(current_e->first_candidate==current_c){
    current_e->first_candidate=prev_c;
  }else if(current_e->first_candidate==prev_c){
    current_e->first_candidate=current_c;
  }
  //optimisations:
  if(current_c==prev_c || current_c==prev_c->prev){
    //there is only two or less candidates, nothing to do
    return;
  }
  //remove current_c:
  prev_c->next=current_c->next;
  current_c->next->prev=prev_c;
  //insert it before it's predecessor:
  current_c->prev=prev_c->prev;
  current_c->next=prev_c;
  prev_c->prev->next=current_c;
  prev_c->prev=current_c;
}

void push_candidate(struct entry *current, const char *path){
  struct candidate *first=current->first_candidate;
  struct candidate *last=first->prev;
  struct candidate *new_candidate=malloc(sizeof(struct candidate));
  new_candidate->score=1;
  strcpy(new_candidate->path,path);
  new_candidate->next=first;
  new_candidate->prev=last;
  last->next=new_candidate;
  first->prev=new_candidate;
}

struct candidate *find_candidate(const char *path, struct entry *current_e) {
  struct candidate *current_c = current_e->first_candidate;
  if (strcmp(path, current_c->path) == 0){
    return current_c;
  }

  for (current_c = current_c->next; current_c != current_e->first_candidate; current_c = current_c->next){
    if(strcmp(path, current_c->path) == 0){
      return current_c;
    }
  }

  return NULL;
}

struct entry *update_entry(const char *path, const char *dirname) {
  return update_entry_score(path, dirname, 1);
}

struct entry *update_entry_score(const char *path, const char *dirname, unsigned long score) {
//find entry corresponding to dirname, create new entry if it doesn't exist yet.
  struct entry *current;
  struct candidate *current_c;
  int key=hash(dirname);
  current=find_entry(dirname, key);
  if (!current){
    //No such entry yet, create it:
    current=create_entry(path, dirname);
    current->first_candidate->score = score;
    attach(current, key);
    return current;
  }

  current_c=find_candidate(path, current);
  if(!current_c){
    push_candidate(current, path);
    current->first_candidate->prev->score = score;
  } else {
    current_c->score += score;
    if(current_c != current->first_candidate && current_c->score > current_c->prev->score){
      swap_left(current, current_c);
    }
  }

  return current;
}

