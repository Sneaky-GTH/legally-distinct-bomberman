#include <stdint.h>

struct ParsedAddress {
    char host[256];
    uint16_t port;
    enum {
        IPv6,
        IPv4,
        HOSTNAME,
    } type;
};

int parse_address(const char* address, struct ParsedAddress* out);
int resolve_hostname(const char* hostname, char* out_ip); // Assumes at least 40 bytes (longest ipv6 address + null byte)

