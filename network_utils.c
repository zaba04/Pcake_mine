#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "network_utils.h"

extern Node nodes[MAX_NODES];
extern int node_count;

Node* find_node_by_port(int port) {
    for (int i = 0; i < node_count; i++) {
        if (nodes[i].port == port) {
            return &nodes[i];
        }
    }
    return NULL;
}

int send_udp_message(int port, Message* msg) {
    int sock;
    struct sockaddr_in server_addr;
    
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return -1;
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    int result = sendto(sock, msg, sizeof(Message), 0, 
                      (struct sockaddr*)&server_addr, sizeof(server_addr));
    
    close(sock);
    return result;
}

void store_connection_info(Node* node, int connection_id, int prev_port) {
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (!node->connections[i].is_active) {
            node->connections[i].connection_id = connection_id;
            node->connections[i].prev_node_port = prev_port;
            node->connections[i].is_active = 1;
            break;
        }
    }
}

int find_prev_node_port(Node* node, int connection_id) {
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (node->connections[i].is_active && 
            node->connections[i].connection_id == connection_id) {
            return node->connections[i].prev_node_port;
        }
    }
    return -1;
}