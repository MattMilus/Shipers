#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "Router.h"

struct client_data {
    int sock;
    struct sockaddr_in client_addr;
    char message[2000];
    int read_size;
};

void *connection_handler(void *arg) {
    struct client_data *data = (struct client_data *)arg;

    route_message(data->message, data->read_size, data->sock, &data->client_addr);

    free(data);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    int listenfd = 0;
    struct sockaddr_in serv_addr;

    pthread_t thread_id;
    listenfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (listenfd < 0) {
        perror("Error while creating socket");
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5000);

    if (bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Bind function error (eg. port id already occupied)");
        exit(EXIT_FAILURE);
    }

    fprintf(stderr, "Server UDP is ready on port 5000\n");


    for (;;) {
        struct client_data *data = malloc(sizeof(struct client_data));
        socklen_t client_len = sizeof(data->client_addr);
        data->sock = listenfd;

        data->read_size = recvfrom(listenfd, data->message, 2000, 0,
                                   (struct sockaddr*)&data->client_addr, &client_len);

        if (data->read_size > 0) {
            pthread_create(&thread_id, NULL, connection_handler, (void*)data);

            pthread_detach(thread_id);
        } else {
            free(data);
        }
    }

    return 0;
}