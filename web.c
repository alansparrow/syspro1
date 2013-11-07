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
      respond(slot);
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
	      char *html1 = "<html><body>Hellow World";
	      char *html2 = "</body></html>";
	      printf("file: %s\n", path);
	      sprintf(html,"%s path is %s port is %s %s\0", html1,ROOT,PORT,html2);
	      write(clients[n], html,strlen(html));
	    }
	}
    }
  //Closing SOCKET
  shutdown (clients[n], SHUT_RDWR);      //All further send and recieve operations are DISABLED...
  close(clients[n]);
  clients[n]=-1;
  exit(0);
}

