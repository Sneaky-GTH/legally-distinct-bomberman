#include "./util.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int parse_address(const char* address, struct ParsedAddress* out) {
    if (!address || !out) return -1;
    
    out->port = 0;
    
    if (address[0] == '[') {
        const char* end_bracket = strchr(address, ']');
        if (!end_bracket) return -1;
        
        size_t len = end_bracket - address - 1;
        if (len >= sizeof(out->host)) return -1;
        
        strncpy(out->host, address + 1, len);
        out->host[len] = '\0';
        out->type = IPv6;
        
        if (end_bracket[1] == ':') {
            out->port = atoi(end_bracket + 2);
        } else if (end_bracket[1] != '\0') {
            return -1;
        }
    } else {
        const char* colon = strrchr(address, ':');
        if (colon) {
            size_t len = colon - address;
            if (len >= sizeof(out->host)) return -1;
            
            strncpy(out->host, address, len);
            out->host[len] = '\0';
            out->port = atoi(colon + 1);
        } else {
            size_t len = strlen(address);
            if (len >= sizeof(out->host)) return -1;
            
            strcpy(out->host, address);
        }
        
        struct in_addr ipv4_addr;
        if (inet_pton(AF_INET, out->host, &ipv4_addr) == 1) {
            out->type = IPv4;
        } else {
            out->type = HOSTNAME;
        }
    }
    
    return 0;
}

int resolve_hostname(const char* hostname, char* out_ip) {
    if (!hostname || !out_ip) return -1;
    
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    if (getaddrinfo(hostname, NULL, &hints, &res) != 0) {
        return -1;
    }
    
    void *addr;
    if (res->ai_family == AF_INET) {
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
        addr = &(ipv4->sin_addr);
    } else {
        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)res->ai_addr;
        addr = &(ipv6->sin6_addr);
    }
    
    inet_ntop(res->ai_family, addr, out_ip, 40); // Need at least INET6_ADDRSTRLEN
    freeaddrinfo(res);
    
    return 0;
}
