#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>

#define CONNMAX 1000
#define BYTES 1024

char *ROOT;
int listenfd, clients[CONNMAX];
void error(char *);
void startServer(char *);
void respond(int);
void cmd_exec(char *[], int);
void utf_to_ascii(char *);
void urldecode(char *, char *, char *);
inline int ishex(int);
int decode(const char *, char *);

int main(int argc, char* argv[])
{
  struct sockaddr_in clientaddr;
  socklen_t addrlen;
  char c;    
    
  //Default Values PATH = ~/ and PORT=10000
  char PORT[6];
  ROOT = getenv("PWD");
  strcpy(PORT,"10000");

  int slot=0;

  //Parsing the command line arguments
  while ((c = getopt (argc, argv, "p:r:")) != -1)
    switch (c)
      {
      case 'r':
	ROOT = malloc(strlen(optarg));
	strcpy(ROOT,optarg);
	break;
      case 'p':
	strcpy(PORT,optarg);
	break;
      case '?':
	fprintf(stderr,"Wrong arguments given!!!\n");
	exit(1);
      default:
	exit(1);
      }
    
  printf("Server started at port no. %s%s%s with root directory as %s%s%s\n","\033[92m",PORT,"\033[0m","\033[92m",ROOT,"\033[0m");
  // Setting all elements to -1: signifies there is no client connected
  int i;
  for (i=0; i<CONNMAX; i++)
    clients[i]=-1;
  startServer(PORT);

  // ACCEPT connections
  while (1)
    {
      addrlen = sizeof(clientaddr);
      clients[slot] = accept (listenfd, (struct sockaddr *) &clientaddr, &addrlen);

      if (clients[slot]<0)
	error ("accept() error");
      else
        {
	  if ( fork()==0 )
            {
	      respond(slot);
	      exit(0);
            }
        }

      while (clients[slot]!=-1) slot = (slot+1)%CONNMAX;
    }

  return 0;
}

//start server
void startServer(char *port)
{
  struct addrinfo hints, *res, *p;

  // getaddrinfo for host
  memset (&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  if (getaddrinfo( NULL, port, &hints, &res) != 0)
    {
      perror ("getaddrinfo() error");
      exit(1);
    }
  // socket and bind
  for (p = res; p!=NULL; p=p->ai_next)
    {
      listenfd = socket (p->ai_family, p->ai_socktype, 0);
      if (listenfd == -1) continue;
      if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) break;
    }
  if (p==NULL)
    {
      perror ("socket() or bind()");
      exit(1);
    }

  freeaddrinfo(res);

  // listen for incoming connections
  if ( listen (listenfd, 1000000) != 0 )
    {
      perror("listen() error");
      exit(1);
    }
}

//client connection
void respond(int n)
{
  char mesg[99999], *reqline[3], data_to_send[BYTES], path[99999],html[99999],*param[3], *target,*result;
  int rcvd, fd, bytes_read, tmp_fd;

  // my code
  char *reply_content, *cmd_to_execute, *err_msg;
  int buf_size = 10000;
  FILE *fpin, *tmp_file1;

  memset( (void*)mesg, (int)'\0', 99999 );

  rcvd=recv(clients[n], mesg, 99999, 0);

  if (rcvd<0)    // receive error
    fprintf(stderr,("recv() error\n"));
  else if (rcvd==0)    // receive socket closed
    fprintf(stderr,"Client disconnected upexpectedly.\n");
  else    // message received
    {
      printf("%s", mesg);
      reqline[0] = strtok (mesg, " \t\n");
      if ( strncmp(reqline[0], "GET\0", 4)==0 )
        {
	  reqline[1] = strtok (NULL, " \t");
	  reqline[2] = strtok (NULL, " \t\n");
	  if ( strncmp( reqline[2], "HTTP/1.0", 8)!=0 && strncmp( reqline[2], "HTTP/1.1", 8)!=0 )
            {
	      write(clients[n], "HTTP/1.0 400 Bad Request\n", 25);
            }
	  else
            {            
	      if (strncmp(reqline[1], "/\0", 2) == 0)           
                reqline[1] = "/index.html";
              strcpy(path, ROOT);
              strcpy(&path[strlen(ROOT)], reqline[1]);
              printf("file: %s\n", path);
              
              if ((fd=open(path, O_RDONLY)) != -1) {
		printf("open \n");
		write(clients[n], "HTTP/1.0 200 OK\n\n", 17);
                while ((bytes_read=read(fd, data_to_send, BYTES))>0 )                
                  write (clients[n], data_to_send, bytes_read);
              } else {
                param[0] = strdup(reqline[1]);
                if (strncmp(param[0], "/server_cmd", 4) == 0) {  
		  target = (char *)malloc(strlen(param[0]));
		  result = (char *)malloc(strlen(param[0]));
		  decode(param[0],result);

		  
		  printf("result: %s \n",result);
		  cmd_to_execute = result+16; // get the command line string
		  printf("cmd: %s \n", cmd_to_execute);
		  reply_content = (char *) malloc(sizeof(char) * buf_size);  // prepare reply content string
		  
		  
		

		  // open pipe for reading content of executed command
		  if ((fpin = popen(cmd_to_execute, "r")) == NULL) 
		      printf("command error!\n");

		  param[1] = strtok(result, "?");
		  param[2] = strtok(NULL, "?");
		  write(clients[n], "HTTP/1.0 200 OK\n\n", 17);
		  write(clients[n], "<html><head>", 12);

		  // check if there is error when execute command
		  
		  
		  // write form again for convenience
		  if ((fd=open("./index.html", O_RDONLY)) != -1) { // read index.html from present directory
		    while ((bytes_read=read(fd, data_to_send, BYTES))>0 )                
		      write (clients[n], data_to_send, bytes_read);
		  }
		  
		  while (fgets(reply_content, buf_size, fpin)) {  // read the content and write to clients
		    printf("Reply content: %s\n", reply_content);
		    write(clients[n], reply_content, strlen(reply_content));
		    write(clients[n], "<br>", 4);
		  }
		  
		  write(clients[n], "</head><body>", 13);
		  write(clients[n], "</body></html>", 14);
	      } else {
		printf("Not found\n");
		write(clients[n], "HTTP/1.0 404 Not Found\n", 23);
                }
              }
	    }
	}
    }
  //Closing SOCKET
  shutdown (clients[n], SHUT_RDWR);
  close(clients[n]);
  clients[n]=-1;
}
void urldecode(char *src, char *last, char *dest) {
  for(;src<=last;src++,dest++){
    if(*src=='%'){
      int code;
      if(sscanf(src+1, "%2x", &code)!=1) code='?';
      *dest = code;
      src+=2;
    }else if(*src=='+') {
      *dest = ' ';
    }else {
      *dest = *src;
    }
  }
  *(++dest) = '\0';
}


inline int ishex(int x)
{
  return(x >= '0' && x <= '9')||
    (x >= 'a' && x <= 'f')||
    (x >= 'A' && x <= 'F');
}
 
int decode(const char *s, char *dec)
{
  char *o;
  const char *end = s + strlen(s);
  int c;
 
  for (o = dec; s <= end; o++) {
    c = *s++;
    if (c == '+') c = ' ';
    else if (c == '%' && (!ishex(*s++)||
			  !ishex(*s++)||
			  !sscanf(s - 2, "%2x", &c)))
      return -1;
 
    if (dec) *o = c;
  }
 
  return o - dec;
}
 
