#include <protocol/messages.h>
#include "./client.h"
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

static struct {
    enum ConnectionState state;
    char address[256];
    pthread_t network_thread;
} CLIENT_STATE = {
    .state = DISCONNECTED,
    .address = "",
    .network_thread = 0,
};

enum ConnectionState get_connection_state() {
    return CLIENT_STATE.state;
}

const char* get_server_address() {
    return CLIENT_STATE.address;
}

static void network_thread_main();

void spawn_network_thread(const char* address) {
    if (CLIENT_STATE.network_thread != 0) {
        fprintf(stderr, "Attempted to spawn network thread while one is already active");
        return;
    }

    // TODO
    CLIENT_STATE.state = CONNECTING;
    strncpy(CLIENT_STATE.address, address, 255);
    CLIENT_STATE.address[255] = '\0';

    pthread_t thread_id;

    int result = pthread_create(&thread_id, NULL, (void* (*)(void*)) network_thread_main, NULL);
    if (result != 0) {
        perror("pthread_create");
        CLIENT_STATE.state = DISCONNECTED;
        return;
    }

    CLIENT_STATE.network_thread = thread_id;
}

void shutdown_network_thread() {
    if (CLIENT_STATE.network_thread == 0) {
        fprintf(stderr, "Attempted to shutdown network thread when none is active");
        return;
    }

    pthread_kill(CLIENT_STATE.network_thread, SIGUSR2);
}

static void handle_sigusr2(int signum) {
    if (signum != SIGUSR2) return;
    
    CLIENT_STATE.state = DISCONNECTED;
    CLIENT_STATE.network_thread = 0;
    pthread_exit(NULL);
}

static void network_thread_main() {
    // SIGUSR2 is used to signal the thread to shut down
    signal(SIGUSR2, handle_sigusr2);

    sleep(1);
    CLIENT_STATE.state = HANDSHAKE;
    sleep(1);
    CLIENT_STATE.state = ESTABLISHED;

    for (;;) sleep(1);
}
