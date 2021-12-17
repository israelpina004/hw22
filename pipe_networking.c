#include "pipe_networking.h"

/*=========================
  server_setup
  args:
  creates the WKP (upstream) and opens it, waiting for a
  connection.
  removes the WKP once a connection has been made
  returns the file descriptor for the upstream pipe.
  =========================*/
int server_setup() {
  int from_client = 0;
  mkfifo(WKP, 0644);
  printf("Connecting ...\n");

  from_client = open(WKP, O_RDONLY);
  if(from_client < 0) {
    printf("Opening WKP unsuccessful, terminating connection.\n");
    return 1;
  }
  printf("Removing WKP ...\n");

  remove(WKP);

  return from_client;
}

/*=========================
  server_connect
  args: int from_client
  handles the subserver portion of the 3 way handshake
  returns the file descriptor for the downstream pipe.
  =========================*/
int server_connect(int from_client) {
  int to_client  = 0;
  char id[50];
  int r = read(from_client, id, sizeof(id));
  if(r < 1) {
    printf("Error reading from WKP, termination connection.\n");
    return 1;
  }

  to_client = open(id, O_WRONLY, 0);
  if(to_client < 1) {
    printf("Connection to client FIFO unsuccesful, terminating connection.\n");
    return 1;
  }
  write(to_client, ACK, strlen(ACK)+1);
  printf("Wrote ACK to client.\n");

  char check[10];
  read(from_client, check, sizeof(check));
  if(strcmp(ACK, check)) {
    printf("ACK not returned from client. Connection unsuccesful.\n");
    return 1;
  }
  printf("Connection successful.\n");

  return to_client;
}


/*=========================
  server_handshake
  args: int * to_client
  Performs the client side pipe 3 way handshake.
  Sets *to_client to the file descriptor to the downstream pipe.
  returns the file descriptor for the upstream pipe.
  =========================*/
int server_handshake(int *to_client) {
  int from_client = 0;
  mkfifo(WKP, 0644);
  printf("Waiting for a connection ...");

  from_client = open(WKP, O_RDONLY);
  if(from_client < 0) {
    printf("Opening WKP unsuccessful, terminating connection.\n");
    return 1;
  }

  printf("Reading from WKP ...");
  char id[50];
  int r = read(from_client, id, sizeof(id));
  if(r < 1) {
    printf("Error reading from WKP, terminating connection.\n");
    return 1;
  }
  printf("Reading successful, removing WKP ...");
  remove(WKP);

  printf("Writing ACK into client ...");
  *to_client = open(id, O_WRONLY);
  if(*to_client < 1) {
    printf("Connection to client FIFO unsuccesful, terminating connection.\n");
    return 1;
  }
  write(*to_client, ACK, strlen(ACK)+1);
  printf("Wrote ACK to client.\n");

  char check[10];
  printf("Checking for ACK from client ...");
  read(from_client, check, sizeof(check));
  if(strcmp(check, ACK)) {
    printf("ACK not returned from client. Connection unsuccesful.");
    return 1;
  }
  printf("Connection successful.\n");

  return from_client;
}


/*=========================
  client_handshake
  args: int * to_server
  Performs the client side pipe 3 way handshake.
  Sets *to_server to the file descriptor for the upstream pipe.
  returns the file descriptor for the downstream pipe.
  =========================*/
int client_handshake(int *to_server) {
  int from_server = 0;
  char pid[50];
  sprintf(pid, "%d", getpid());
  mkfifo(pid, 0644);

  *to_server = open(WKP, O_WRONLY);
  if(*to_server < 1) {
    printf("Connection to server unsuccesful, terminating connection.\n");
    return 1;
  }
  printf("Connected to server, writing client information to server ...\n");
  write(*to_server, pid, sizeof(pid));

  printf("Receiving ACK from server ...\n");
  from_server = open(pid, O_RDONLY);
  if(from_server < 1) {
    printf("Connection to server through client FIFO unsuccesful, terminating connection.\n");
    return 1;
  }
  char ack[10];
  read(from_server, ack, sizeof(ack));
  printf("Received, removing client FIFO ...\n");
  remove(pid);

  printf("Writing ACK back to server ...\n");
  write(*to_server, ack, sizeof(ack));
  printf("Connection successful.\n");

  return from_server;
}
