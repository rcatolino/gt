#include "gtd.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>

static int socklisten;
static struct entry *htable[53]={NULL}; //other + Upper case + Lower case

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
    new_entry->prev=new_entry;
    new_entry->next=new_entry;
    htable[key]=new_entry;
  }else{
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

struct entry * find_entry(const char *dirname, int key){
  struct entry * current;
  printd("find_entry %s, key: %d\n",dirname, key);
  if (htable[key]!=NULL){
    current=htable[key];
    if (strcmp(dirname,current->dirname)==0){
      return current;
    }
    for (; current->next==htable[key]; current=current->next){
      if(strcmp(dirname,current->dirname)==0){
        return current;
      }
    }
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

struct candidate * find_candidate(const char *path, struct entry* current_e){
  struct candidate *current_c=current_e->first_candidate;
  if (strcmp(path,current_c->path)==0){
    return current_c;
  }
  for (; current_c->next==current_e->first_candidate; current_c=current_c->next){
    if(strcmp(path,current_c->path)==0){
      return current_c;
    }
  }
  return NULL;
}

struct entry * update_entry(const char *path, const char *dirname){
//find entry corresponding to dirname, create new entry if it doesn't exist yet.
  struct entry * current;
  struct candidate * current_c;
  int key=hash(dirname);
  current=find_entry(dirname,key);
  if (!current){
    //No such entry yet, create it:
    current=create_entry(path,dirname);
    attach(current,key);
    return current;
  }

  current_c=find_candidate(path,current);
  if(!current_c){
    push_candidate(current,path);
  }else{
    current_c->score++;
    if(current_c!=current->first_candidate && current_c->score > current_c->prev->score){
      swap_left(current,current_c);
    }
  }
  return current;
}

int store_event(const char * buff){
  char * dirname;
  const char * path;
  path=buff;
  dirname=strrchr(path,'/')+1;
  if(dirname==NULL){
    printd("store_event: invalid path\n");
    return -1;
  }
  //ignore all trailing '/' characters:
  while(strlen(dirname)==1){
    dirname[0]='\0';
    dirname=strrchr(path,'/');
    if(dirname==NULL){
      printd("store_event: invalid path\n");
      return -1;
    }
  }
  printd("store_event: dirname=%s, path=%s\n",dirname,path);
  update_entry(path,dirname);
  return 0; 
}

const char * find_event(const char * buff){
  struct entry * src;
  int key=hash(buff);
  printd("find_event: %s\n",buff);
  src=find_entry(buff, key);
  if (src==NULL){
    return NULL;
  } else {
    printd("find_event: found path associated with dirname, %s\n",src->first_candidate->path);
    return src->first_candidate->path;
  }
}

int read_event(int sockfd){
  char buffer[MAX_SIZE+1];
  int data_read=0;
  int ret=0;

  data_read=recv(sockfd,buffer,MAX_SIZE,0);
  while(data_read<MAX_SIZE){
    ret=recv(sockfd,buffer+data_read,MAX_SIZE-data_read,MSG_DONTWAIT);
    if (ret==-1){
      if (errno==EAGAIN || errno==EWOULDBLOCK){
        break;
      } else {
        perror("recv");
        return -1;
      }
    }
    data_read+=ret;
  }
  buffer[MAX_SIZE]='\0';
  printd("buffer len : %lu\n",strlen(buffer));
  printd("event %c\n",buffer[0]);

  if(buffer[0]=='s'){
    //add a dirname-path pair to storage;
    store_event(buffer+1);
  }else if(buffer[0]=='f'){ 
    const char * path;
    //find a match for a dirname:
    path=find_event(buffer+1);
    if(path==NULL){
      send(sockfd,"\n",2,0);
      printd("read_event: no path corresponding to dirname\n");
    } else {
      if(send(sockfd,path,strlen(path)+1,0)==-1){
        perror("send");
        return -1;
      }
      ret = recv(sockfd,buffer,2,0); //Wait for connection closed
      printd("read_event: connection closed, ret = %d\n", ret);
    }
  }else{
    printd("read_event: received invalid message : %s\n",buffer);
    return -1;
  }
  return 0;
}

int pop_event(){
  int sockcon;

  sockcon=accept(socklisten, NULL, NULL);
  if(sockcon==-1){
    perror("accept");
    return -1;
  }

  read_event(sockcon);
  close(sockcon);
  return 0;
}

void end(int signb){
  printd("end: removing resources\n");
  unlink(SOCKET);
  close(socklisten);
  exit(0);
}

int init(){
  int ret=0;
  struct sockaddr_un server;
	struct sigaction act = { .sa_handler=end, .sa_flags = 0, };

  socklisten=socket(AF_UNIX, SOCK_STREAM,0);
  if (socklisten==-1){
    perror("socket");
    goto out;
  }

  server.sun_family=AF_UNIX;
  strcpy(server.sun_path,SOCKET);
  ret=bind(socklisten,(struct sockaddr*)&server,sizeof(struct sockaddr_un));
  if (ret==-1){
    perror("bind");
    goto outbind;
  }

  ret=listen(socklisten,1);
  if (ret==-1){
    perror("listen");
    goto outlisten;
  }
//The socket has been succesfully initialized

//set up signals handlers :
	if(sigaction(SIGINT, &act, NULL)==-1) goto outsig;
	if(sigaction(SIGTERM, &act, NULL)==-1) goto outsig;
  return 0;
//Error :
outsig:
outlisten:
  unlink(SOCKET);
outbind:
  close(socklisten);
out:
  return -1;
}

int main(int argc, char * argv[]){
  if (init()==-1){
    return 1;
  }

  while(1){
    if(pop_event()==-1) end(0);
  } 

  return 0;
}


