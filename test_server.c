#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFER_SIZE 1024

void* test_server_thread(void* arg) {
    int port = *(int*)arg;
    int sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Test server socket creation failed");
        return NULL;
    }
    
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    if (bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Test server bind failed");
        close(sock);
        return NULL;
    }
    
    printf("Test server listening on port %d\n", port);
    
    while (1) {
        int bytes_received = recvfrom(sock, buffer, BUFFER_SIZE - 1, 0,
                                    (struct sockaddr*)&client_addr, &client_len);
        
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            printf("Test server %d: Received: %s\n", port, buffer);
            
            char response[BUFFER_SIZE];
            snprintf(response, BUFFER_SIZE, "Server %d response to: %s", port, buffer);
            
            sendto(sock, response, strlen(response), 0,
                   (struct sockaddr*)&client_addr, client_len);
            
            printf("Test server %d: Sent response\n", port);
        }
    }
    
    close(sock);
    return NULL;
}