#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "tor_types.h"
#include "node.h"
#include "client.h"

Node nodes[MAX_NODES];
int node_count = 0;
int next_connection_id = 1;

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
    
    // Listening lool
    while (1) {
        int bytes_received = recvfrom(sock, buffer, BUFFER_SIZE - 1, 0,
                                    (struct sockaddr*)&client_addr, &client_len);
        
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            printf("Test server %d: Received: %s\n", port, buffer);
            
            char response[BUFFER_SIZE];
            if (port == 8080) {
                snprintf(response, BUFFER_SIZE, "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello from %d", port);
            } else {
                snprintf(response, BUFFER_SIZE, "Server %d ACK: Message received successfully", port);
            }
            
            sendto(sock, response, strlen(response), 0,
                   (struct sockaddr*)&client_addr, client_len);
            
            printf("Test server %d: Sent response (%zu bytes)\n", port, strlen(response));
        }
    }
    
    close(sock);
    return NULL;
}

int main() {
    printf("Starting simplified TOR network...\n");
    

    pthread_t server1_thread, server2_thread;
    int server1_port = 8080;  
    int server2_port = 8443; // non-privilged on 80 and 443 doesnt work :<
    
    pthread_create(&server1_thread, NULL, test_server_thread, &server1_port);
    pthread_create(&server2_thread, NULL, test_server_thread, &server2_port);
    

    sleep(1);
    

    create_node(8001, NODE_INTERMEDIATE);  // P1
    create_node(8002, NODE_INTERMEDIATE);  // P2
    
    create_node(8003, NODE_EXIT); // Exit node
    
    sleep(1);
    
    pthread_t client1_thread, client2_thread;
    int client1_port = 9001, client2_port = 9002;
    
    pthread_create(&client1_thread, NULL, client_thread, &client1_port);
    pthread_create(&client2_thread, NULL, client_thread, &client2_port);
    
    sleep(1);
    
    printf("\n=== Testing with Client 1 (to local HTTP test server port 8080) ===\n");
    int route1[] = {8001, 8002, 8003};  // P1 -> P2 -> Exit
    send_tor_message(client1_port, route1, 3, 8080, "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n");
    
    sleep(3);
    
    printf("\n=== Testing with Client 2 (to local HTTPS test server port 8443) ===\n");
    int route2[] = {8002, 8001, 8003};  // P2 -> P1 -> Exit (different route)
    send_tor_message(client2_port, route2, 3, 8443, "Hello HTTPS test server!");
    

    printf("\n=== Testing with Client 1 (to public UDP echo server) ===\n");
    send_tor_message(client1_port, route1, 3, 7, "Echo test message");  // Port 7 is echo service


    printf("\nNetwork is running with LOCAL test servers that respond to requests.\n");
    printf("Test servers: 8080 (HTTP-like), 8443 (HTTPS-like)\n");
    printf("Press Ctrl+C to stop.\n");
    while (1) {
        sleep(1);
    }
    
    return 0;
}