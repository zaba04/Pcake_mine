#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "client.h"
#include "network_utils.h"

extern int next_connection_id;

void send_tor_message(int client_port, int route[], int route_length, 
                     int final_dest, const char* data) {
    Message msg;
    msg.msg_type = MSG_DATA;
    msg.connection_id = next_connection_id++;
    msg.route_length = route_length;
    msg.final_destination_port = final_dest;

    for (int i = 0; i < route_length; i++) {
        msg.route[i] = route[i];
    }

    strncpy(msg.data, data, BUFFER_SIZE - 1);
    msg.data[BUFFER_SIZE - 1] = '\0';
    msg.data_length = strlen(msg.data);
    
    printf("Client %d: Sending message through TOR network\n", client_port);
    printf("Route: ");
    for (int i = 0; i < route_length; i++) {
        printf("%d ", route[i]);
    }
    printf("-> %d\n", final_dest);
    printf("Data: %s\n", data);
    
    if (send_udp_message(route[0], &msg) < 0) {
        printf("Failed to send message to first node\n");
    }
}

void* client_thread(void* arg) {
    int client_port = *(int*)arg;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    Message response;
    int sock;
    
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Client socket creation failed");
        return NULL;
    }
    
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(client_port);
    
    if (bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Client bind failed");
        close(sock);
        return NULL;
    }
    
    printf("Client listening on port %d for responses\n", client_port);
    
    while (1) {
        int bytes_received = recvfrom(sock, &response, sizeof(Message), 0,
                                    (struct sockaddr*)&client_addr, &client_len);
        
        if (bytes_received > 0 && response.msg_type == MSG_RESPONSE) {
            printf("Client %d: Received response: %s\n", client_port, response.data);
        }
    }
    
    close(sock);
    return NULL;
}