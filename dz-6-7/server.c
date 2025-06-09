#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUF_SIZE 16

typedef struct {
  int queue_1_size;
  int queue_2_size;
  int customers_served_1;
  int customers_served_2;
} QueueStatus;

int main(int argc, char *argv[]) {
  if (argc != 7) {
    fprintf(stderr,
            "Usage: %s <listen_port> <cashier1_ip> <cashier1_port> "
            "<cashier2_ip> <cashier2_port> <queue_size>\n",
            argv[0]);
    exit(EXIT_FAILURE);
  }
  int listen_port = atoi(argv[1]);
  char *c1_ip = argv[2];
  int c1_port = atoi(argv[3]);
  char *c2_ip = argv[4];
  int c2_port = atoi(argv[5]);
  int Qmax = atoi(argv[6]);

  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in serv_addr;
  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(listen_port);

  if (bind(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    perror("bind");
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in cashier[2];
  memset(&cashier, 0, sizeof(cashier));
  cashier[0].sin_family = AF_INET;
  inet_pton(AF_INET, c1_ip, &cashier[0].sin_addr);
  cashier[0].sin_port = htons(c1_port);
  cashier[1].sin_family = AF_INET;
  inet_pton(AF_INET, c2_ip, &cashier[1].sin_addr);
  cashier[1].sin_port = htons(c2_port);

  int qlen[2] = {0, 0};
  int customers_served_1 = 0, customers_served_2 = 0;
  char buf[BUF_SIZE];
  struct sockaddr_in cli_addr;
  socklen_t addrlen = sizeof(cli_addr);
  printf("[Server] Listening on port %d, queuesize=%d\n", listen_port, Qmax);

  while (1) {
    ssize_t n = recvfrom(sock, buf, BUF_SIZE, 0, (struct sockaddr *)&cli_addr,
                         &addrlen);
    if (n < 0)
      continue;
    buf[n] = '\0';

    if (strcmp(buf, "NEW") == 0) {
      int idx = -1;
      if (qlen[0] < Qmax && (qlen[0] <= qlen[1] || qlen[1] >= Qmax))
        idx = 0;
      else if (qlen[1] < Qmax)
        idx = 1;
      if (idx < 0) {
        printf("[Server] Both queues full. Customer leaves.\n");
      } else {
        qlen[idx]++;
        printf("[Server] Customer assigned to cashier %d (queue=%d).\n",
               idx + 1, qlen[idx]);
        sendto(sock, "SERVE", 5, 0, (struct sockaddr *)&cashier[idx],
               sizeof(cashier[idx]));
      }
    } else if (strcmp(buf, "DONE") == 0) {
      int idx = -1;
      if (cli_addr.sin_port == cashier[0].sin_port &&
          cli_addr.sin_addr.s_addr == cashier[0].sin_addr.s_addr)
        idx = 0;
      else if (cli_addr.sin_port == cashier[1].sin_port &&
               cli_addr.sin_addr.s_addr == cashier[1].sin_addr.s_addr)
        idx = 1;
      if (idx >= 0 && qlen[idx] > 0) {
        qlen[idx]--;
        if (idx == 0)
          customers_served_1++;
        else
          customers_served_2++;
        printf("[Server] Cashier %d done. Remaining queue=%d.\n", idx + 1,
               qlen[idx]);
      }
    } else if (strcmp(buf, "STATUS") == 0) {
      QueueStatus status = {qlen[0], qlen[1], customers_served_1,
                            customers_served_2};
      sendto(sock, &status, sizeof(status), 0, (struct sockaddr *)&cli_addr,
             addrlen);
    }
  }
  close(sock);
  return 0;
}
