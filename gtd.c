#define DEBUG 1

#include "gtd.h"
#include "serial.h"
#include "datalist.h"
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
  dumpTable();
  unlink(SOCKET);
  close(socklisten);
  exit(0);
}

int init(){
  int ret=0;
  struct sockaddr_un server;
	struct sigaction act = { .sa_handler=end, .sa_flags = 0, };

  init_serial("r");
  read_next_entry();
  end_serial();
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

  chmod(SOCKET, S_IRWXU | S_IRWXG | S_IRWXO);
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


