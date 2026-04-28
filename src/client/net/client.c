#include <protocol/messages.h>
#include "./client.h"
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "./util.h"
#include <protocol/messages.h>
#include <net.h>

static struct {
    enum ConnectionState state;
    char address[256];
    pthread_t network_thread;
} CLIENT_STATE = {
    .state = DISCONNECTED,
    .address = "",
    .network_thread = 0,
};

static char status_message[1024];

const char* get_status_message() {
    return status_message;
}

enum ConnectionState get_connection_state() {
    return CLIENT_STATE.state;
}

const char* get_server_address() {
    return CLIENT_STATE.address;
}

static void network_thread_wrapper();

void spawn_network_thread(const char* address) {
    if (CLIENT_STATE.network_thread != 0) {
        fprintf(stderr, "Attempted to spawn network thread while one is already active\n");
        return;
    }

    strcpy(status_message, "Starting network thread...");
    CLIENT_STATE.state = CONNECTING; printf("CLIENT_STATE.state = CONNECTING;\n");
    strncpy(CLIENT_STATE.address, address, 255);
    CLIENT_STATE.address[255] = '\0';

    pthread_t thread_id;

    int result = pthread_create(&thread_id, NULL, (void* (*)(void*)) network_thread_wrapper, NULL);
    if (result != 0) {
        perror("pthread_create");
        CLIENT_STATE.state = DISCONNECTED; printf("CLIENT_STATE.state = DISCONNECTED;\n");
        strcpy(status_message, "Failed to create network thread");
        return;
    }

    CLIENT_STATE.network_thread = thread_id;
}

void shutdown_network_thread() {
    if (CLIENT_STATE.network_thread == 0) {
        return;
    }
    
    pthread_kill(CLIENT_STATE.network_thread, SIGUSR2);
}

static void handle_sigusr2(int signum) {
    if (signum != SIGUSR2) return;

    strcpy(status_message, "Shutting down network thread...");
    CLIENT_STATE.state = DISCONNECTED; printf("CLIENT_STATE.state = DISCONNECTED;\n");
    CLIENT_STATE.network_thread = 0;

    // TODO: will need more for this to work properly

    pthread_exit(NULL);
}

static int create_connection(int *out_sock) {
    strcpy(status_message, "Parsing server address...");

    struct ParsedAddress parsed;
    if (parse_address(CLIENT_STATE.address, &parsed) != 0) {
        fprintf(stderr, "Failed to parse server address: %s\n", CLIENT_STATE.address);
        strcpy(status_message, "Failed to parse server address");
        return -1;
    }

    if (parsed.port == 0) {
        fprintf(stderr, "No port specified in server address: %s\n", CLIENT_STATE.address);
        strcpy(status_message, "No port specified in server address");
        return -1;
    }

    strcpy(status_message, "Resolving server address...");

    // If it's a hostname, resolve it to an IP address
    if (parsed.type == HOSTNAME) {
        char resolved_ip[40];
        if (resolve_hostname(parsed.host, resolved_ip) != 0) {
            fprintf(stderr, "Failed to resolve hostname: %s\n", parsed.host);
            strcpy(status_message, "Failed to resolve hostname");
            return -1;
        }
        strncpy(parsed.host, resolved_ip, 39);
        parsed.host[39] = '\0';

        if (strchr(parsed.host, ':')) {
            parsed.type = IPv6;
        } else {
            parsed.type = IPv4;
        }
    }

    sprintf(status_message, "Connecting to server at %s:%d...", parsed.host, parsed.port);

    int socket_domain = (parsed.type == IPv6) ? AF_INET6 : AF_INET;
    
    int sock = socket(socket_domain, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        strcpy(status_message, "Failed to create socket");
        return -1;
    }

    struct sockaddr_in server_addr_v4 = {0};
    struct sockaddr_in6 server_addr_v6 = {0};
    struct sockaddr *server_addr_ptr = NULL;
    socklen_t server_addr_len = 0;

    if (parsed.type == IPv6) {
        server_addr_v6.sin6_family = AF_INET6;
        server_addr_v6.sin6_port = htons(parsed.port);
        if (inet_pton(AF_INET6, parsed.host, &server_addr_v6.sin6_addr) <= 0) {
            fprintf(stderr, "Invalid IPv6 address: %s\n", parsed.host);
            strcpy(status_message, "Invalid server address");
            close(sock);
            return -1;
        }
        server_addr_ptr = (struct sockaddr *)&server_addr_v6;
        server_addr_len = sizeof(server_addr_v6);
    } else {
        server_addr_v4.sin_family = AF_INET;
        server_addr_v4.sin_port = htons(parsed.port);
        if (inet_pton(AF_INET, parsed.host, &server_addr_v4.sin_addr) <= 0) {
            fprintf(stderr, "Invalid IPv4 address: %s\n", parsed.host);
            strcpy(status_message, "Invalid server address");
            close(sock);
            return -1;
        }
        server_addr_ptr = (struct sockaddr *)&server_addr_v4;
        server_addr_len = sizeof(server_addr_v4);
    }
 
    if (connect(sock, server_addr_ptr, server_addr_len) < 0) {
        perror("connect");
        strcpy(status_message, "Failed to connect to server");
        close(sock);
        return -1;
    }

    printf("Connected to %s:%d\n", parsed.host, parsed.port);

    *out_sock = sock;
    return 0;
}

static void network_thread_main() {
    // SIGUSR2 is used to signal the thread to shut down
    signal(SIGUSR2, handle_sigusr2);

    int sock;

    if (create_connection(&sock) < 0) {
        return;
    }

    CLIENT_STATE.state = HANDSHAKE; printf("CLIENT_STATE.state = HANDSHAKE;\n");

    strcpy(status_message, "Connection established! Performing handshake...");

    const Message handshake_msg = {
        .type = MSG_HELLO,
        .sender_id = 0, // Will be assigned by server
        .target_id = 255, // Server
        .data.hello = {
            .client_id = "leg-dis-bom-client",
            .client_name = "Player" // TODO: allow user to specify name
        },
    };

    if (send_message(sock, &handshake_msg) < 0) {
        fprintf(stderr, "Failed to send handshake message\n");
        close(sock);
        strcpy(status_message, "Failed to perform handshake");
        return;
    }


}

static void network_thread_wrapper() {
    network_thread_main();
    CLIENT_STATE.network_thread = 0;
    CLIENT_STATE.state = DISCONNECTED; printf("CLIENT_STATE.state = DISCONNECTED;\n");
    CLIENT_STATE.address[0] = '\0';
}
