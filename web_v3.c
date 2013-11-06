#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<signal.h>
#include<fcntl.h>

#define CONNMAX 1
#define BYTES 1024

char *ROOT;
char *PORT;
int listenfd, clients[CONNMAX];
void error(char *);
void startServer(char *);
void respond(int);

int main(int argc, char* argv[])
{
  struct sockaddr_in clientaddr;
  socklen_t addrlen;
  char c;

  //Default Values PATH = ~/ and PORT=10000
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
	PORT = malloc(strlen(optarg));
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
  addrlen = sizeof(clientaddr);
  clients[slot] = accept (listenfd, (struct sockaddr *) &clientaddr, &addrlen);
  if (clients[slot]<0)
    error ("accept() error");
  else
    {
      // add if pid == 0 then do, else don't do (do only if it is a child process!
      respond(slot);//1 request but many responds of the same socket
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
  char *result = (char *) malloc(sizeof(char) * 10000);// result return to client
  char *pure_path = (char *) malloc(sizeof(char) * 10000);// get path 
  char *buf = (char *) malloc(sizeof(char) * 10000);
  char *cmd1 = (char *) malloc(sizeof(char) * 1000);// command for 'ls -l'
  char *cmd2 = (char *) malloc(sizeof(char) * 1000);// command for 'cat '
  char *param = "/?param=";
  char *pos;
  char c;
  int index;
  FILE *file;// use pipeline
  struct stat s;// use to know if the path is a file or a directory

  char mesg[99999], *reqline[3], data_to_send[BYTES], path[99999], html[999];
  int rcvd, fd, bytes_read;

  memset( (void*)mesg, (int)'\0', 99999 );

  //rcvd=recv(clients[n], mesg, 99999, 0);
  rcvd=read(clients[n], mesg, 99999);

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
	      pos = strstr(reqline[1], param) + 8; //get 1st position when meet '/?param=', +8 means pass through until the real path. Ex: /home/ubuntu/syspro
	      
	      index = 0;
	      // get path(not include '/?param='), terminate when meet ' '
	      while ((c = *pos) != NULL) {
		pure_path[index++] = c;
		pos++;
	      }
	      pure_path[index] = '\n';
	     	      if (stat(pure_path, &s) == 0) {
		if (s.st_mode & S_IFDIR) {
		  printf("path is a directory\n");
		  strcat(cmd1, "ls -l ");//initialize comd1 as 'ls -l'
		  strcat(cmd1, pure_path);//now, it becomes, for example, "ls -l /home/ubuntu/"
		  file = popen(cmd1, "r");//execute command and open a pipeline
		  while (fgets(buf, 10000, file))
		    strcat(result, buf);//read and add content of the pipeline into result
		  pclose(file);
		  printf("%s\n", result);
		} else if (s.st_mode & S_IFREG) {
		  printf("path is a file\n");
		  strcat(cmd2, "cat ");//initialize comd2 as 'cat '
		  strcat(cmd2, pure_path);//now, it becomes, for example, "cat /home/ubuntu/syspro/f13.c"
		  file = popen(cmd2, "r");//excute command and open a pipeline
		  while (fgets(buf, 10000, file))
		    strcat(result, buf);//read and add content of the pipeline into result
		  pclose(file);
		  printf("%s\n", result);
		} else {
		  printf("something else!\n");
	        }
	      } else {
		strcat(result, "Error!\n");
		printf("wtf! Error!!\n");
	      }
	      write(clients[n], result,strlen(result));
	    }
	}
    }
  //Closing SOCKET
  shutdown (clients[n], SHUT_RDWR);      //All further send and recieve operations are DISABLED...
  close(clients[n]);
  clients[n]=-1;
  exit(0);
}

