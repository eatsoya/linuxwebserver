#include "csapp.h"
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

int open_listen_sock(int port) 
{
  int listen_sock,optval=1;
  struct sockaddr_in serveraddr;
if((listen_sock = socket(AF_INET,SOCK_STREAM,0))<0)
return -1;

if(setsockopt(listen_sock,SOL_SOCKET,SO_REUSEADDR,(const void*)&optval,sizeof(int))<0)
return -1;

bzero((char *)&serveraddr,sizeof(serveraddr));
serveraddr.sin_family = AF_INET;
serveraddr.sin_port = htons((unsigned short)port);
if(bind(listen_sock,(SA*)&serveraddr,sizeof(serveraddr))<0)
return -1;

if(listen(listen_sock,LISTENQ)<0)
return -1;
return listen_sock;
}


void sigchld_handler(int sig){
    while(waitpid(-1,0,WNOHANG)>0);
    return;
}

int main(int argc,char **argv)
{
    int listen_sock,conn_sock,port,clientlen;
    struct sockaddr_in clientaddr;
    
/*检查输入的命令数*/
if(argc!=2){
fprintf(stderr,"usage:%s<port>\n",argv[0]);
exit(1);
}

     signal(SIGCHLD,sigchld_handler);   


port = atoi(argv[1]);

listen_sock=open_listen_sock(port);
while(1){
    clientlen = sizeof(clientaddr);
    conn_sock = accept(listen_sock,(SA*)&clientaddr,&clientlen);
    if(fork()==0)/*子进程运行*/
      {
         close(listen_sock);  /*child process closes its listening socket*/
         process_trans(conn_sock); /*Child process services client*/
         close(conn_sock);     /*Child process closes connection with client*/
        /* sleep(5);*/
         exit(0);              /*Child process exits*/
      }
    close(conn_sock);   /*Parent closes connected socket(important!)*/
         }
}


void process_trans(int fd);
int is_static(char *uri);
void parse_static_uri(char *uri,char *filename);
void parse_dynamic_uri(char *uri,char *filename,char *cgiargs);
void feed_static(int fd,char *filename,int filesize);
void get_filetype(char *filename,char *filetype);
void feed_dynamic(int fd,char *filename,char *cgiargs,char *method,int contentlength);
void error_request(int fd,char *cause,char *errnum,char *shortmsg,char *description);

void process_trans(int fd)
{
int static_flag;
struct stat sbuf;
char buf[MAXLINE],method[MAXLINE],uri[MAXLINE],version[MAXLINE];
char filename[MAXLINE],cgiargs[MAXLINE];
int contentlength=0;
FILE *f = fdopen(fd, "r");
if (!f) {
perror("Unable to open input fd");
close(fd);
return;
}
setbuf(f, 0);
if (!fgets(buf, MAXLINE, f)) {
fclose(f);
return;
}
sscanf(buf,"%s %s %s",method,uri,version);
if(strcasecmp(method,"GET")&&strcasecmp(method,"POST")){
error_request(fd,method,"501","NOT Implemented","Group28 does not implement this method");
return;
}