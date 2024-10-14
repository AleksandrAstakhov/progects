#include <arpa/inet.h>
#include <linux/limits.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

typedef struct StorageItem {
  char key[PATH_MAX];
  char value[PATH_MAX];
  struct StorageItem* next;
} StorageItem;

typedef struct Storage {
  struct StorageItem* head;
  pthread_mutex_t mutex;
} Storage;

StorageItem* find(Storage* storage, char* key) {
  StorageItem* current = storage->head;
  while (current != NULL) {
    if (strncmp(current->key, key, PATH_MAX) == 0) {
      return current;
    }
    current = current->next;
  }
  return NULL;
}

void set(Storage* storage, char* key, char* value) {
  StorageItem* element = find(storage, key);
  if (element == NULL) {
    element = malloc(sizeof(StorageItem));
    strncpy(element->key, key, PATH_MAX - 1);
    element->next = storage->head;
    storage->head = element;
  }
  strncpy(element->value, value, PATH_MAX - 1);
}

char* get(Storage* storage, char* key) {
  StorageItem* element = find(storage, key);
  if (element == NULL) {
    return "";
  } else {
    return element->value;
  }
}

int Socket(int domain, int type, int protocol) {
  int res = socket(domain, type, protocol);
  if (res == -1) {
    perror("brocken socket");
    exit(EXIT_FAILURE);
  }
  return res;
}

void Bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen) {
  int res = bind(sockfd, addr, addrlen);
  if (res == -1) {
    perror("error bind");
    exit(EXIT_FAILURE);
  }
}

void Listen(int sockfd, int backlog) {
  ssize_t res = listen(sockfd, backlog);
  if (res == -1) {
    perror("listen failed");
    exit(EXIT_FAILURE);
  }
}

int Accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen) {
  int res = accept(sockfd, addr, addrlen);
  if (res == -1) {
    perror("accept failed");
    exit(EXIT_FAILURE);
  }
  return res;
}

struct thread_args {
  Storage* storage;
  int subServerSocket;
  pthread_t thread_id;
};

void* client_handler(void* thread_args_packet) {
  struct thread_args* args = (struct thread_args*)thread_args_packet;
  char buffer[1024];
  ssize_t bytes_read = 0;

  while (1) {
    if ((bytes_read = read(args->subServerSocket, buffer, 1024)) == -1) {
      return NULL;
    }

    char* command = strtok(buffer, " ");
    pthread_mutex_lock(&args->storage->mutex);
    if (strcmp(command, "get") == 0) {
      char* key = strtok(NULL, " \n");
      char value[PATH_MAX] = "";
      char* tmp = get(args->storage, key);
      size_t tmp_len = strlen(tmp);
      memcpy(value, tmp, tmp_len);
      strcat(value, "\n");
      send(args->subServerSocket, value, strlen(value), 0);
    } else if (strcmp(command, "set") == 0) {
      char* key = strtok(NULL, " ");
      char* value = strtok(NULL, "\n");
      set(args->storage, key, value);
    }
    pthread_mutex_unlock(&args->storage->mutex);
  }
  close(args->subServerSocket);
  return NULL;
}

int main(int argc, char* argv[]) {
  Storage* storage = malloc(sizeof(Storage));
  storage->head = NULL;
  pthread_mutex_init(&storage->mutex, NULL);

  int server_socket, client_socket, addr_size;
  struct sockaddr_in server_addr, client_addr;

  server_socket = Socket(AF_INET, SOCK_STREAM, 0);

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  server_addr.sin_port = htons(atoi(argv[1]));

  Bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
  Listen(server_socket, 5);

  struct thread_args args[10];
  int arg_ind = 0;
  addr_size = sizeof(struct sockaddr_in);

  while (1) {
    client_socket = accept(server_socket, (struct sockaddr*)&client_addr,
                           (socklen_t*)&addr_size);
    if (client_socket == -1) break;

    args[arg_ind].storage = storage;
    args[arg_ind].subServerSocket = client_socket;

    pthread_create(&args[arg_ind].thread_id, NULL, client_handler,
                   (void*)&args[arg_ind]);
    ++arg_ind;
  }
  for (size_t ind = 0; ind < arg_ind; ++ind) {
    pthread_join(args[arg_ind].thread_id, NULL);
  }
  pthread_mutex_destroy(&storage->mutex);
  close(server_socket);
}
