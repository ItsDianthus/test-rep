#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUF_SIZE 256

typedef struct {
  int queue_1_size;
  int queue_2_size;
  int customers_served_1;
  int customers_served_2;
} QueueStatus;

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <server_ip> <server_port>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  char *server_ip = argv[1];
  int server_port = atoi(argv[2]);

  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  inet_pton(AF_INET, server_ip, &server_addr.sin_addr);
  server_addr.sin_port = htons(server_port);

  char buf[BUF_SIZE];
  QueueStatus status;

  while (1) {
    sendto(sock, "STATUS", 6, 0, (struct sockaddr *)&server_addr,
           sizeof(server_addr));

    socklen_t addr_len = sizeof(server_addr);
    ssize_t n = recvfrom(sock, &status, sizeof(status), 0,
                         (struct sockaddr *)&server_addr, &addr_len);
    if (n < 0) {
      perror("recvfrom");
      continue;
    }

    printf("\n--- Supermarket Status ---\n");
    printf("Queue 1: %d customers (served: %d)\n", status.queue_1_size,
           status.customers_served_1);
    printf("Queue 2: %d customers (served: %d)\n", status.queue_2_size,
           status.customers_served_2);
    printf("--------------------------------\n");

    sleep(2);
  }

  close(sock);
  return 0;
}
