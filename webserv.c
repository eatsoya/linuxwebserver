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

void parse_static_uri(char *uri,char *filename)
{
	char *ptr;
	strcpy(filename,".");
	strcat(filename,uri);
	if(uri[strlen(uri)-1]=='/')
		strcat(filename,"home.html");
}

void feed_static(int fd,char *filename,int filesize)
{
	int srcfd;
	char *srcp,filetype[MAXLINE],buf[MAXBUF];
	get_filetype(filename,filetype);
	sprintf(buf,"HTTP/1.0 200 OK\r\n");
	sprintf(buf,"%sServer:Group28 Web Server\r\n",buf);
	sprintf(buf,"%sContent-length:%d\r\n",buf,filesize);
	sprintf(buf,"%sContent-type:%s\r\n\r\n",buf,filetype);
	write(fd,buf,strlen(buf));

	srcfd = open(filename,O_RDONLY,0);
	srcp = mmap(0,filesize,PROT_READ,MAP_PRIVATE,srcfd,0);
	close(srcfd);
	write(fd,srcp,filesize);
	munmap(srcp,filesize);
}

void get_filetype(char *filename,char *filetype)
{
	if(strstr(filename,".html"))
		strcpy(filetype,"text/html");
	else if(strstr(filename,".jpg"))
		strcpy(filetype,"image/jpeg");
	else if(strstr(filename,".mpeg"))
		strcpy(filetype,"video/mpeg");
	else
		strcpy(filetype,"text/html");
}

void sigchld_handler(int sig){
    while(waitpid(-1,0,WNOHANG)>0);
    return;
}


/* read headers and parse "Content-length"*/
char buf_header[MAXLINE];

while (fgets(buf_header, 150, f) && (strlen(buf_header) > 2)) {
if (strncasecmp(buf_header, "Content-length:", 15) == 0) {
contentlength = atoi(buf_header + 15);
}
}

static_flag=is_static(uri);
if(static_flag){
parse_static_uri(uri,filename);
strcpy(cgiargs,"");}
else
parse_dynamic_uri(uri,filename,cgiargs);

if(stat(filename,&sbuf)<0){
error_request(fd,filename,"404","NOT found","Group28 could not find this file");
return;
}

if(static_flag){
  if(!(S_ISREG(sbuf.st_mode))||!(S_IRUSR&sbuf.st_mode)){
     error_request(fd,filename,"403","Forboden","Group28 is not permtted to read this file");
     return;
  }
  feed_static(fd,filename,sbuf.st_size);
 }
  else{
     if(!(S_ISREG(sbuf.st_mode))||!(S_IXUSR&sbuf.st_mode)){
        error_request(fd,filename,"403","Forbiden","Group28 could not run the CGI program");
        return;
     }
     feed_dynamic(fd,filename,cgiargs,method,contentlength);
   }
}


int is_static(char *uri)
{
  if(!strstr(uri,"cgi-bin"))
  return 1;
  else
  return 0;
}


void error_request(int fd,char *cause,char *errnum,char *shortmsg,char *description)
{
char buf[MAXLINE],body[MAXBUF];

/*Build the HTTP response body*/
sprintf(body,"<html><title>error request</title>");
sprintf(body,"%s<body bgcolor=""ffffff"">\r\n",body);
sprintf(body,"%s%s:%s\r\n",body,errnum,shortmsg);
sprintf(body,"%s<p>%s:%s\r\n",body,description,cause);
sprintf(body,"%s<hr><em>Group28 Web server</em>\r\n",body);

/*Send the HTTP response*/
sprintf(buf,"HTTP/1.0%s%s\r\n",errnum,shortmsg);
write(fd,buf,strlen(buf));
sprintf(buf,"Content-type:text/html\r\n");
write(fd,buf,strlen(buf));
sprintf(buf,"Content-length:%d\r\n\r\n",(int)strlen(body));
write(fd,buf,strlen(buf));
write(fd,body,strlen(body));
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