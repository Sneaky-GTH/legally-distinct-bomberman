#include <protocol/messages.h>
#include "./client.h"
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "./util.h"
#include <protocol/messages.h>
#include <protocol/serial.h>
#include <net.h>
#include "../game/handler.h"

#ifdef DEBUG
#define LOG(fmt, ...) fprintf(stderr, fmt "\n", ##__VA_ARGS__)
#else
#define LOG(fmt, ...)
#endif

static struct {
    enum ConnectionState state;
    char address[256];
    pthread_t network_thread;
    uint8_t client_id;
} CLIENT_STATE = {
    .state = DISCONNECTED,
    .address = "",
    .network_thread = 0,
    .client_id = 0,
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
    CLIENT_STATE.state = CONNECTING; LOG("CLIENT_STATE.state = CONNECTING;");
    strncpy(CLIENT_STATE.address, address, 255);
    CLIENT_STATE.address[255] = '\0';

    pthread_t thread_id;

    int result = pthread_create(&thread_id, NULL, (void* (*)(void*)) network_thread_wrapper, NULL);
    if (result != 0) {
        perror("pthread_create");
        CLIENT_STATE.state = DISCONNECTED; LOG("CLIENT_STATE.state = DISCONNECTED;");
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
    CLIENT_STATE.state = DISCONNECTED; LOG("CLIENT_STATE.state = DISCONNECTED;");
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

    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &(struct timeval){.tv_sec = 5, .tv_usec = 0}, sizeof(struct timeval)); // 5 second receive timeout
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &(struct timeval){.tv_sec = 5, .tv_usec = 0}, sizeof(struct timeval)); // 5 second send timeout   
    setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &(int){1}, sizeof(int)); // Enable TCP keepalive

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

static void handle_message(const Message *msg, int sock) {
    switch (msg->type) {
        case MSG_WELCOME:
        case MSG_DISCONNECT:
            // Unreachable, handled in main loop before calling handle_message
            break;

        case MSG_HELLO:
            struct GameEvent new_player_event = {
                .type = EVENT_NEW_PLAYER,
                .new_player = {
                    .player_id = msg->sender_id,
                },
            };
            strncpy(new_player_event.new_player.name, msg->data.hello.client_name, 29);
            new_player_event.new_player.name[29] = '\0';
            enqueue_event(&new_player_event);
            break;

        case MSG_PING:
            send_message(sock, &(Message) {
                .type = MSG_PONG,
                .sender_id = CLIENT_STATE.client_id,
                .target_id = 255, // Server
            });
            break;

        case MSG_PONG:
            break; // No response needed

        case MSG_LEAVE:
            if (msg->sender_id == CLIENT_STATE.client_id) {
                fprintf(stderr, "LEAVE from us? what?\n");
                shutdown_network_thread();
                return; // unreachable
            }
            enqueue_event(&(struct GameEvent) {
                .type = EVENT_PLAYER_REMOVED,
                .player_removed = {
                    .player_id = msg->sender_id,
                },
            });
            break;

        case MSG_ERROR:
            fprintf(stderr, "Error from server: %s\n", msg->data.error.error_message);
            break;

        case MSG_SET_READY:
            if (msg->sender_id == CLIENT_STATE.client_id) {
                // This is just an echo of our own SET_READY message, ignore
                break;
            }
            enqueue_event(&(struct GameEvent) {
                .type = EVENT_PLAYER_STATUS,
                .player_status = {
                    .player_id = msg->sender_id,
                    .ready = true,
                },
            });
            break;

        case MSG_SET_STATUS:
            enqueue_event(&(struct GameEvent) {
                .type = EVENT_STATUS_UPDATE,
                .status_update = {
                    .status = msg->data.set_status.status,
                },
            });
            break;

        case MSG_WINNER:
            enqueue_event(&(struct GameEvent) {
                .type = EVENT_WINNER,
                .winner = {
                    .winner_id = msg->data.winner.winner_id,
                },
            });
            break;

        case MSG_MOVE_ATTEMPT:
            // This should never be received from the server, ignore
            fprintf(stderr, "Received unexpected MOVE_ATTEMPT message from server\n");
            break;

        case MSG_BOMB_ATTEMPT:
            // This should never be received from the server, ignore
            fprintf(stderr, "Received unexpected BOMB_ATTEMPT message from server\n");
            break;

        case MSG_MOVED:
            enqueue_event(&(struct GameEvent) {
                .type = EVENT_MOVE,
                .move = {
                    .player_id = msg->data.moved.player_id,
                    .new_position = msg->data.moved.new_position,
                },
            });
            break;

        case MSG_BOMB:
            enqueue_event(&(struct GameEvent) {
                .type = EVENT_PLACE_BOMB,
                .place_bomb = {
                    .position = msg->data.bomb.position,
                    // We ignore player_id here since the game thread will treat all PLACE_BOMB events the same regardless of
                    // who placed it, but we could include it if we wanted to do fancy stuff like showing different colored
                    // bombs for different players
                },
            });
            break;

        case MSG_EXPLOSION_START:
            enqueue_event(&(struct GameEvent) {
                .type = EVENT_EXPLOSION_START, // TODO: separate event type for explosion?
                .explosion_start = {
                    .position = msg->data.bomb.position,
                    .radius = msg->data.explosion.radius,
                },
            });
            break;

        case MSG_EXPLOSION_END:
            enqueue_event(&(struct GameEvent) {
                .type = EVENT_EXPLOSION_END,
                .explosion_end = {
                    .position = msg->data.explosion.position,
                    .radius = msg->data.explosion.radius,
                },
            });
            break;

        case MSG_DEATH:
            enqueue_event(&(struct GameEvent) {
                .type = EVENT_DEATH,
                .death = {
                    .player_id = msg->data.death.player_id,
                },
            });
            break;

        case MSG_BONUS_AVAILABLE:
            enqueue_event(&(struct GameEvent) {
                .type = EVENT_BONUS_AVAILABLE,
                .bonus_available = {
                    .bonus_type = msg->data.bonus_available.bonus_type,
                    .position = msg->data.bonus_available.position,
                },
            });
            break;

        case MSG_BONUS_RETRIEVED:
            enqueue_event(&(struct GameEvent) {
                .type = EVENT_BONUS_RETRIEVED,
                .bonus_retrieved = {
                    .player_id = msg->data.bonus_retrieved.player_id,
                    .position = msg->data.bonus_retrieved.position,
                },
            });
            break;

        case MSG_BLOCK_DESTROYED:
            enqueue_event(&(struct GameEvent) {
                .type = EVENT_BLOCK_DESTROYED,
                .block_destroyed = {
                    .position = msg->data.block_destroyed.position,
                },
            });
            break;

        default:
            fprintf(stderr, "Received message with unknown type: %d\n", msg->type);
            break;
    }
}

static void network_thread_main() {
    // SIGUSR2 is used to signal the thread to shut down
    signal(SIGUSR2, handle_sigusr2);

    int sock;

    if (create_connection(&sock) < 0) {
        return;
    }

    CLIENT_STATE.state = HANDSHAKE; LOG("CLIENT_STATE.state = HANDSHAKE;");

    CLIENT_STATE.client_id =  (uint8_t) (time(NULL) % 253) + 1; // Generate a random client ID (1-253) for now

    strcpy(status_message, "Connection established! Performing handshake...");

    if (send_message(sock, &(Message) {
        .type = MSG_HELLO,
        .sender_id = CLIENT_STATE.client_id,
        .target_id = 255, // Server
        .data.hello = {
            .client_id = "leg-dis-bom-client",
            .client_name = "Player" // TODO: allow user to specify name
        },
    }) < 0) {
        fprintf(stderr, "Failed to send handshake message\n");
        close(sock);
        strcpy(status_message, "Failed to perform handshake");
        return;
    }

    uint8_t buf[4096]; // Pre-allocated buffer
    int valid_len = 0; // Tracks unparsed data length across calls
    Message recv_msg;
    
    int epfd = epoll_create1(0);
    if (epfd < 0) {
        perror("epoll_create1");
        goto shutdown;
    }

    struct epoll_event ev, events[2];
    ev.events = EPOLLIN;
    ev.data.fd = sock;
    epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &ev);
    
    int net_fd = get_net_event_fd();
    ev.events = EPOLLIN;
    ev.data.fd = net_fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, net_fd, &ev);

    for (;;) {
        // Drain pending parsed messages first without waiting
        int parse_res = read_message(buf, valid_len, &recv_msg);
        if (parse_res > 0) {
            int remaining = valid_len - parse_res;
            memmove(buf, buf + parse_res, remaining);
            valid_len = remaining;
            
            // Successfully received and parsed message!
            printf("Received message type: %d\n", recv_msg.type);

            switch (recv_msg.type) {
                case MSG_WELCOME:
                    if (CLIENT_STATE.state != HANDSHAKE) {
                        fprintf(stderr, "Received WelcomeMessage while not in handshake state\n");
                        strcpy(status_message, "Unexpected message during handshake");
                        goto shutdown;
                    }

                    CLIENT_STATE.state = ESTABLISHED; LOG("CLIENT_STATE.state = ESTABLISHED;");
                    strcpy(status_message, "Handshake successful! Entering lobby...");

                    enqueue_event(&(struct GameEvent) { .type = EVENT_RESET });
                    enqueue_event(&(struct GameEvent) {
                        .type = EVENT_NEW_PLAYER,
                        .new_player = {
                            // This is the first player, so the game thread should treat this as "us"
                            .player_id = CLIENT_STATE.client_id,
                            .name = "Player", 
                        }
                    });

                    break;
                case MSG_DISCONNECT:
                    fprintf(stderr, "Server disconnected during handshake\n");
                    if (CLIENT_STATE.state == HANDSHAKE) {
                        strcpy(status_message, "Server cannot accept our connection yet");
                    } else {
                        strcpy(status_message, "Server disconnected");
                    }
                    goto shutdown;
                default:
                    handle_message(&recv_msg, sock);
                    break;
            }

            free_message(&recv_msg);
            continue;
        } else if (parse_res < 0 && parse_res != EREADNODATA) {
            fprintf(stderr, "Message parse error\n");
            goto shutdown;
        }

        // Wait for I/O
        int nfds = epoll_wait(epfd, events, 2, 20000); // 20 second timeout to send ping occasionally
        if (nfds < 0 && errno != EINTR) {
            perror("epoll_wait");
            goto shutdown;
        }

        if (nfds == 0) {
            // Timeout, send ping to keep connection alive
            if (send_message(sock, &(Message) {
                .type = MSG_PING,
                .sender_id = CLIENT_STATE.client_id,
                .target_id = 255, // Server
            }) < 0) {
                fprintf(stderr, "Failed to send ping message\n");
                goto shutdown;
            }
            continue;
        }

        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == net_fd) {
                uint64_t val;
                if (read(net_fd, &val, sizeof(val)) > 0) {
                    struct GameEvent net_evt;
                    while (dequeue_net_event(&net_evt)) {
                        switch (net_evt.type) {
                            case EVENT_MOVE_ATTEMPT:
                                if (send_message(sock, &(Message) {
                                    .type = MSG_MOVE_ATTEMPT,
                                    .sender_id = CLIENT_STATE.client_id,
                                    .target_id = 255, // Server
                                    .data.move_attempt.direction = net_evt.move_attempt.dir,
                                }) < 0) {
                                    fprintf(stderr, "Failed to send move attempt message\n");
                                    goto shutdown;
                                }
                                break;
                            case EVENT_PLACE_BOMB_ATTEMPT:
                                if (send_message(sock, &(Message) {
                                    .type = MSG_BOMB_ATTEMPT,
                                    .sender_id = CLIENT_STATE.client_id,
                                    .target_id = 255, // Server
                                }) < 0) {
                                    fprintf(stderr, "Failed to send bomb attempt message\n");
                                    goto shutdown;
                                }
                                break;
                            default:
                                fprintf(stderr, "Unknown event type from game thread: %d\n", net_evt.type);
                                break;
                        }
                    }
                }
            } else if (events[i].data.fd == sock) {
                int n = recv(sock, buf + valid_len, sizeof(buf) - valid_len, 0);
                if (n == 0) {
                    fprintf(stderr, "Connection closed by server during handshake\n");
                    strcpy(status_message, "Connection closed by server");
                    goto shutdown;
                } else if (n < 0) {
                    perror("recv");
                    goto shutdown;
                }
                valid_len += n;
            }
        }
    }

    shutdown:

    send_message(sock, &(Message) {
        .type = MSG_LEAVE,
        .sender_id = 0,
        .target_id = 255,
    }); // Best effort to notify server of disconnect, but we don't really care about the result since we're shutting down anyway

    close(sock);
}

static void network_thread_wrapper() {
    network_thread_main();
    CLIENT_STATE.network_thread = 0;
    CLIENT_STATE.state = DISCONNECTED; LOG("CLIENT_STATE.state = DISCONNECTED;");
    CLIENT_STATE.address[0] = '\0';
}
