#include "pipe_networking.h"

int main() {

  int to_client;
  int from_client;

  while(1) {
    from_client = server_setup();
    int f = fork();

    if(f) {
      close(from_client);
      remove(WKP);
    }
    else {
      to_client = server_connect(from_client);

      while(1) {
        char temp[100];
        read(from_client, temp, sizeof(temp));
        printf("Processing string ...\n");
        int i = 0;
        while(temp[i]) {
          if(temp[i] >= 'a' && temp[i] <= 'z') {
            temp[i] = temp[i] - 32;
          }
          i++;
        }
        write(to_client, temp, sizeof(temp));
      }
      return 0;
    }
  }
}
