#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

struct client_data {
    int sock;
    struct sockaddr_in client_addr;
    char message[2000];
    int read_size;
};

void *connection_handler(void *arg) {
    struct client_data *data = (struct client_data *)arg;
    data->message[data->read_size] = '\0';

    fprintf(stderr, "Otrzymano od %s:%d -> %s\n",
            inet_ntoa(data->client_addr.sin_addr),
            ntohs(data->client_addr.sin_port),
            data->message);

    if (strncmp(data->message, "CONNECT", 7) == 0) {
        char response[50];
        int new_player_id = 1;

        snprintf(response, sizeof(response), "ACCEPTED %d", new_player_id);

        sendto(data->sock, response, strlen(response), 0,
               (struct sockaddr*)&data->client_addr, sizeof(data->client_addr));

        fprintf(stderr, "Wyslano do klienta: %s\n", response);
    } else {
        // Tutaj w przyszłości dodasz obsługę innych wiadomości od klienta (np. ruchy gracza)
        // sendto(data->sock, data->message, data->read_size, 0, ...);
    }

    free(data);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    int listenfd = 0;
    struct sockaddr_in serv_addr;

    pthread_t thread_id;
    listenfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (listenfd < 0) {
        perror("Blad przy tworzeniu gniazda (socket)");
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5000);

    if (bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Blad funkcji bind (np. port jest juz zajety)");
        exit(EXIT_FAILURE);
    }

    fprintf(stderr, "Server UDP is ready on port 5000\n");


    for (;;) {
        fprintf(stderr, "Waiting for a packet\n");

        struct client_data *data = malloc(sizeof(struct client_data));
        socklen_t client_len = sizeof(data->client_addr);
        data->sock = listenfd;

        data->read_size = recvfrom(listenfd, data->message, 2000, 0,
                                   (struct sockaddr*)&data->client_addr, &client_len);
        fprintf(stderr, "Received\n");

        if (data->read_size > 0) {
            pthread_create(&thread_id, NULL, connection_handler, (void*)data);

            //pthread_detach(thread_id);
        } else {
            free(data);
        }
    }

    return 0;
}