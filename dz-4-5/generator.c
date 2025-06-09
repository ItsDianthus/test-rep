#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc != 4) {
    fprintf(stderr, "Usage: %s <server_ip> <server_port> <delay_ms>\n",
            argv[0]);
    exit(EXIT_FAILURE);
  }
  char *sip = argv[1];
  int sport = atoi(argv[2]);
  int delay = atoi(argv[3]);
  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  struct sockaddr_in serv_addr;
  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  inet_pton(AF_INET, sip, &serv_addr.sin_addr);
  serv_addr.sin_port = htons(sport);
  printf("[Generator] Sending new customers every %d ms to %s:%d\n", delay, sip,
         sport);
  while (1) {
    sendto(sock, "NEW", 3, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    printf("[Generator] New customer sent.\n");
    usleep(delay * 1000);
  }
  close(sock);
  return 0;
}
