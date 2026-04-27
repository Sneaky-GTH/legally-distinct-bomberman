#pragma once
#include <stdint.h>

enum ConnectionState {
    DISCONNECTED,
    CONNECTING,
    HANDSHAKE,
    ESTABLISHED,
    TEARDOWN,
};

enum ConnectionState get_connection_state();
const char* get_server_address();

void spawn_network_thread(const char* address);
void shutdown_network_thread();
