#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#define TRUE 1

void sigchld_handler(int s) {
  while(waitpid(-1, NULL, WNOHANG) > 0);
}


int main(void) {
	int new_socket=0;
  //AF_INET is a IPv4 Internet protocols
  //SOCK_STREAM is a Provides sequenced, reliable, two-way, connection-based byte streams. An out-of-band data transmission mechanism may be supported.
  // create a TCP Stream socket.On success, a file descriptor for the new socket is returned. On error, -1 is returned
  int s = socket(AF_INET, SOCK_STREAM, 0);
  if (s < 0) return -1;

  // set socket option: reusead  On success, zero is returned.
  int on = 1;
  if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
    return -1;
  }

  // create an address and bind the socket to it
  struct sockaddr_in sa;
  memset(&sa, 0, sizeof(sa));
  sa.sin_family = AF_INET;
  sa.sin_port = htons(2300);
  sa.sin_addr.s_addr = htonl(INADDR_ANY);
  char *bind_address = "127.0.0.1" ;
  if (inet_aton(bind_address, &sa.sin_addr) == 0) {
    printf("invalid bind address\n");
    close(s);
    return -1;
  }
  pid_t pid = fork();
  if (pid == -1) {
      perror("fork");
      exit(0);
   } 
   if (pid > 0) {
	   printf("I'm server\n");
  // bind socket to sa
  if (bind(s, (struct sockaddr*)&sa, sizeof(sa)) == -1) {
    printf("can't bind on this address.\n");
    close(s);
    return -1;
  }

  // ok, socket has been created and binded to an address, now listen on this socket now...
  printf("listening\n");
  if (listen(s, 511) == -1) { // this magic 511 backlog value is fron nginx
    printf("ERROR: listen: %s\n", strerror(errno));
    close(s);
    return -1;
  }

  // register signal handler
  struct sigaction sig_a;
  sig_a.sa_handler = sigchld_handler; // reap all dead processes
  sigemptyset(&sig_a.sa_mask);
  sig_a.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &sig_a, NULL) == -1) {
    printf("register signal handler failed.\n");
    return -1;
  }

  // all right, now we listen for new connections on this socket and handle them in a infinite loop
  struct sockaddr_in peer_addr;
  socklen_t addr_len = sizeof(peer_addr);
  while(TRUE) {
    printf("trying to accept connections, pid: %d...\n", getpid());
    new_socket = accept(s, (struct sockaddr*)&peer_addr, &addr_len);
    if (new_socket == -1) {
      perror("can't accept connection on socket");
      exit(-1);
    }
    printf("server: accepted connection from %s\n", inet_ntoa(peer_addr.sin_addr));
    //sleep(1);

   
      // in child process
      close(s);
      //printf("child process pid: %d\n", getpid());
      char *response = "[message from server] hello, world!\n";
      int len = strlen(response);
      int bytes_sent = send(new_socket, response, len, 0);
      if (bytes_sent == -1) {
        perror("write error on socket!\n");
        close(new_socket);
        exit(-1);
      } else if (bytes_sent == 0) {
        
      } else if (bytes_sent > 0) {
        printf("Great! message was sent to client!\n");
        printf("bytes to send: %d\n", len);
        printf("actually sent: %d\n", bytes_sent);
      }
      printf("ok, nothing interesting to to, we're going to close the socket...\n");
      close(new_socket);
      printf("socket closed, exit...\n");
      exit(0);
    } 
    }
    else if (pid ==0) {
		char buffer[500 + 1]; /* +1 so we can add null terminator */
		int len, mysocket;
		struct sockaddr_in dest; 
		 
		mysocket = socket(AF_INET, SOCK_STREAM, 0);
		  
		memset(&dest, 0, sizeof(dest));                /* zero the struct */
		dest.sin_family = AF_INET;
		dest.sin_addr.s_addr = htonl(INADDR_LOOPBACK); /* set destination IP number - localhost, 127.0.0.1*/ 
		dest.sin_port = htons(2300);                /* set destination port number */
		connect(mysocket, (struct sockaddr *)&dest, sizeof(struct sockaddr));
		len = recv(mysocket, buffer, 500, 0);
		 
		   /* We have to null terminate the received data ourselves */
		buffer[len] = '\0';
		printf("Received %s (%d bytes).\n", buffer, len);
		close(mysocket);
    }
  

  return 0;
}

