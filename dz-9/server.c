#include <arpa/inet.h>
#include <pthread.h>
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

typedef struct {
  int sock;
  struct sockaddr_in client_addr;
  socklen_t addrlen;
  pthread_t thread_id;
} ClientData;

pthread_mutex_t client_lock = PTHREAD_MUTEX_INITIALIZER;
ClientData *clients[10];
int client_count = 0;

void *handle_client(void *arg) {
  ClientData *data = (ClientData *)arg;
  int sock = data->sock;
  struct sockaddr_in client_addr = data->client_addr;
  socklen_t addrlen = data->addrlen;

  char buf[BUF_SIZE];
  QueueStatus status = {0, 0, 0, 0};

  while (1) {
    ssize_t n = recvfrom(sock, buf, BUF_SIZE, 0,
                         (struct sockaddr *)&client_addr, &addrlen);
    if (n < 0) {
      perror("recvfrom");
      continue;
    }
    buf[n] = '\0';

    if (strcmp(buf, "STATUS") == 0) {
      sendto(sock, &status, sizeof(status), 0, (struct sockaddr *)&client_addr,
             addrlen);
    } else if (strcmp(buf, "NEW") == 0) {
      status.queue_1_size++;
      printf("New customer added. Queue 1 size: %d\n", status.queue_1_size);
    } else if (strcmp(buf, "DISCONNECT") == 0) {
      break;
    }
  }

  pthread_mutex_lock(&client_lock);
  for (int i = 0; i < client_count; i++) {
    if (clients[i] == data) {
      for (int j = i; j < client_count - 1; j++) {
        clients[j] = clients[j + 1];
      }
      client_count--;
      break;
    }
  }
  pthread_mutex_unlock(&client_lock);

  close(sock);
  free(data);
  printf("Client disconnected.\n");
  return NULL;
}

void stop_all_clients() {
  pthread_mutex_lock(&client_lock);
  for (int i = 0; i < client_count; i++) {
    char disconnect_msg[] = "DISCONNECT";
    sendto(clients[i]->sock, disconnect_msg, strlen(disconnect_msg), 0,
           (struct sockaddr *)&clients[i]->client_addr, clients[i]->addrlen);
    pthread_cancel(clients[i]->thread_id);
    close(clients[i]->sock);
    free(clients[i]);
  }
  client_count = 0;
  pthread_mutex_unlock(&client_lock);
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <listen_port> <queue_size>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  int listen_port = atoi(argv[1]);
  int Qmax = atoi(argv[2]);

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

  printf("[Server] Listening on port %d\n", listen_port);

  while (1) {
    ClientData *data = (ClientData *)malloc(sizeof(ClientData));
    data->sock = sock;
    data->addrlen = sizeof(data->client_addr);
    ssize_t n = recvfrom(sock, NULL, 0, 0,
                         (struct sockaddr *)&data->client_addr, &data->addrlen);
    if (n < 0) {
      perror("recvfrom");
      continue;
    }

    pthread_mutex_lock(&client_lock);
    clients[client_count++] = data;
    pthread_mutex_unlock(&client_lock);

    pthread_create(&data->thread_id, NULL, handle_client, data);
    pthread_detach(
        data->thread_id);
  }

  stop_all_clients();

  close(sock);
  return 0;
}
