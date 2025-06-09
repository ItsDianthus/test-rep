#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUF_SIZE 16

int main(int argc, char *argv[]) {
  if (argc != 5) {
    fprintf(stderr,
            "Usage: %s <listen_port> <server_ip> <server_port> <service_ms>\n",
            argv[0]);
    exit(EXIT_FAILURE);
  }
  int listen_port = atoi(argv[1]);
  char *sip = argv[2];
  int sport = atoi(argv[3]);
  int service = atoi(argv[4]);
  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  struct sockaddr_in my_addr, serv_addr;
  memset(&my_addr, 0, sizeof(my_addr));
  my_addr.sin_family = AF_INET;
  my_addr.sin_addr.s_addr = INADDR_ANY;
  my_addr.sin_port = htons(listen_port);
  if (bind(sock, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0) {
    perror("bind");
    exit(EXIT_FAILURE);
  }
  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  inet_pton(AF_INET, sip, &serv_addr.sin_addr);
  serv_addr.sin_port = htons(sport);
  char buf[BUF_SIZE];
  struct sockaddr_in src;
  socklen_t addrlen = sizeof(src);
  printf("[Cashier:%d] Waiting on port %d, service time=%d ms\n", listen_port,
         listen_port, service);
  while (1) {
    ssize_t n =
        recvfrom(sock, buf, BUF_SIZE, 0, (struct sockaddr *)&src, &addrlen);
    if (n < 0)
      continue;
    buf[n] = '\0';
    if (strcmp(buf, "SERVE") == 0) {
      printf("[Cashier:%d] Serving customer...\n", listen_port);
      usleep(service * 1000);
      sendto(sock, "DONE", 4, 0, (struct sockaddr *)&serv_addr,
             sizeof(serv_addr));
      printf("[Cashier:%d] Done.\n", listen_port);
    }
  }
  close(sock);
  return 0;
}
