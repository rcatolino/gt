#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <limits.h>
#include <errno.h>
#include "gtd.h"

int move(const char *path){
  int chret=0;
  int sysret=0;
  if (!path){
    printd("gt: no path specified\n");
    return 0;
  }
  switch(path[0]){
    case '/':
    case '~':
    default:
      chret=chdir(path);
      sysret=system("cd /home/");
      break;
  }
  if(chret==-1 || sysret==-1){
#ifdef DEBUG
    const char * err;
    err = strerror(errno);
#endif
    printd("gt: %s\n",err);
    return 1;
  }
  return 0;
}

int check(const char * opt, const char * path){
  const char * target=path;
  if (opt[0]=='-'){
    switch (opt[1]){
      case '\0':
      case ' ':
        target=getenv("OLDPWD");
        if (!target) return 0;
        break;
      case 'L':
      case 'P':
        break;
      default:
        target=opt;
        break;
    }
  }else{
    target=opt;
  }
  return move(target);
}

int isrel(const char * path){    
  return (path[0]=='/' || path[0]=='~') ? 0 : 1;
}

int isdir(const char * path){
  int ret=0;
  ret=open(path,O_RDWR);
  if (ret!=-1){
    close(ret);
    printd("isdir \"%s\": file open, not a directory\n",path);
    return 0;
  } else {
    if (errno==EISDIR){
      printd("isdir \"%s\": error EISDIR, file is a directory\n",path);
      return 1;
    }
    perrord("isdir");
    return 0;
  }
  return 0;
}

int push_event(char event, const char * path, int relative) {
  int sockfd;
  int ret;
  int size;
  char buff[MAX_SIZE+1];
  struct sockaddr_un server={ .sun_family=AF_UNIX, };
  strcpy(server.sun_path,SOCKET);

  sockfd=socket(AF_UNIX, SOCK_STREAM,0);
  ret=connect(sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr_un));
  if(ret==-10){
    perrord("connect");
    close(sockfd);
    return -1;
  }

  buff[0]=event;
  size=strlen(path);
  size=size>MAX_SIZE-1 ? MAX_SIZE-1:size;
  if (relative) {
    /*
    char * pwd = getenv("PWD");
    int pwd_size;
    if (!pwd) {
      printd("No curent working directory\n");
      return;
    }
    pwd_size = strlen(pwd);
    if (pwd_size > (MAX_SIZE-size-1)){
      printd("absolute path too long\n");
      return;
    }
    memcpy(buff+1,pwd,pwd_size);
    buff[1+pwd_size]='/';
    memcpy(buff+2+pwd_size,path,size);
    size+=1+pwd_size;
    */
    realpath(path,buff+1);
    size = strlen(buff);
  } else {
    size++;
    memcpy(buff+1,path,size);
  }
  printd("push_event: sending %s\n", buff);
  printd("buffer len : %lu, sending %d bytes\n",strlen(buff),size+1);
  ret=send(sockfd, buff, size+1, 0);
  if(ret==-1){
    perrord("send");
    return -1;
  }
  if (event == 'f') {
    int data_read=recv(sockfd,buff,MAX_SIZE,0);
    while(data_read<MAX_SIZE){
      ret=recv(sockfd,buff+data_read,MAX_SIZE-data_read,MSG_DONTWAIT);
      if (ret==-1){
        if (errno==EAGAIN || errno==EWOULDBLOCK){
          break;
        } else {
          perrord("recv");
          return -1;
        }
      } else if (ret == 0){
        printd("recv: connection shut down by server\n");
        return -1;
      }
      data_read+=ret;
    }
    if (data_read < 0) {
      perrord("recv:");
      return -1;
    }
    close(sockfd);
    buff[data_read]='\0';
    printd("path received: %s, data_read: %d\n", buff, data_read);
    if (buff[0] == '\n') {
      printd("no path found in historic\n");
      return -1;
    }
    printf("%s\n", buff);
  } else if (event == 's') {
    printf("%s\n", path);
  }
  return 0;
}

int handle(const char * path){
  if (!path) return 0;
  if (!isrel(path)) {
    printd("pushing store event : %s\n",path);
    printf("%s",path);
    return 0;
  }
  if (isdir(path)==1) {
    printd("pushing store event : %s\n",path);
    if (push_event('s',path,1) == -1) {
      printf("%s",path);
    }
    return 0;
  } else {
    printd("pushing find event : %s\n",path);
    if (push_event('f',path,0) == -1) {
      printf("%s",path);
    }
  }
  return 0;
}

int main(int argc, char *argv[]){
  int i;
  switch(argc){
    case 1:
      return 0;
      break;
    case 2:
      return handle(argv[1]);
      break;
    default:
      for (i=1; i<argc; i++){
        printd("%s ",argv[i]);
      }
  }
  return 0;
}
