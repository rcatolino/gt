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

#define SOCKET "/tmp/gtd.socket"
#define MAX_SIZE 60

int move(const char *path){
  int chret=0;
  int sysret=0;
  const char * err;
  if (!path){
    //fprintf(stderr,"gt: no path specified\n");
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
    err = strerror(errno);
    fprintf(stderr,"gt: %s\n",err);
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
    printf("isdir \"%s\": file open, not a directory\n",path);
    return 0;
  } else {
    if (errno==EISDIR){
      printf("isdir \"%s\": error EISDIR, file is a directory\n",path);
      return 1;
    }
    perror("isdir");
    return 0;
  }
  return 0;
}

void push_event(char event, const char * path, int relative){
  int sockfd;
  int ret;
  int size;
  char buff[MAX_SIZE+1];
  struct sockaddr_un server={ .sun_family=AF_UNIX, };
  strcpy(server.sun_path,SOCKET);

  sockfd=socket(AF_UNIX, SOCK_STREAM,0);
  ret=connect(sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr_un));
  if(ret==-10){
    perror("connect");
    close(sockfd);
    return;
  }

  buff[0]=event;
  size=strlen(path);
  size=size>MAX_SIZE-1 ? MAX_SIZE-1:size;
  if (relative) {
    /*
    char * pwd = getenv("PWD");
    int pwd_size;
    if (!pwd) {
      printf("No curent working directory\n");
      return;
    }
    pwd_size = strlen(pwd);
    if (pwd_size > (MAX_SIZE-size-1)){
      printf("absolute path too long\n");
      return;
    }
    memcpy(buff+1,pwd,pwd_size);
    buff[1+pwd_size]='/';
    memcpy(buff+2+pwd_size,path,size);
    size+=1+pwd_size;
    */
    realpath(path,buff+1);
    size = strlen(buff)+1;
  } else {
    size++;
    memcpy(buff+1,path,size);
  }
  printf("push_event: sending %s\n", buff);
  printf("buffer len : %lu, sending %d bytes\n",strlen(buff),size+1);
  ret=send(sockfd, buff, size+1, 0);
  if(ret==-1){
    perror("send");
  }
}

int handle(const char * path){
  if (!path) return 0;
  if (!isrel(path)) {
    printf("pushing store event : %s\n",path);
    return 0;
  }
  if (isdir(path)==1){
    printf("pushing store event : %s\n",path);
    push_event('s',path,1);
    return 0;
  }else{
    printf("pushing find event : %s\n",path);
  }
  printf("%s",path);
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
        printf("%s ",argv[i]);
      }
  }
  return 0;
}
