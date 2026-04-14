#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "Router.h"
#include "managers/GameManager.h"
#include "packets_handlers/ConnectionHandler.h"
#include "packets_handlers/StateManager.h"

struct client_data {
    int sock;
    struct sockaddr_in client_addr;
    char message[2000];
    int read_size;

    GameState* game_state;
};

void *connection_handler(void *arg) {
    struct client_data *data = (struct client_data *)arg;

    route_message(data->message, data->read_size, data->sock, &data->client_addr, data->game_state);

    free(data);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    int listenfd = 0;
    struct sockaddr_in serv_addr;
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

    GameState global_game;
    game_manager_init(&global_game, listenfd);

    pthread_t timeout_thread;
    pthread_create(&timeout_thread, NULL, timeout_checker, (void*)&global_game);
    pthread_detach(timeout_thread);

    pthread_t state_broadcaster_thread;
    pthread_create(&state_broadcaster_thread, NULL, state_broadcaster, (void*)&global_game);
    pthread_detach(state_broadcaster_thread);

    for (;;) {
        struct client_data *data = malloc(sizeof(struct client_data));
        socklen_t client_len = sizeof(data->client_addr);
        data->sock = listenfd;

        data->game_state = &global_game;

        data->read_size = recvfrom(listenfd, data->message, 2000, 0,
                                   (struct sockaddr*)&data->client_addr, &client_len);

        pthread_t thread_id;

        if (data->read_size > 0) {
            pthread_create(&thread_id, NULL, connection_handler, (void*)data);

            pthread_detach(thread_id);
        } else {
            free(data);
        }
    }

    return 0;
}