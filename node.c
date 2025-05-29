#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include "node.h"
#include "network_utils.h"
#include <sys/time.h>

extern Node nodes[MAX_NODES];
extern int node_count;

void handle_intermediate_node(Node* node, Message* msg, int sender_port) {
    printf("Intermediate node %d: Received message from port %d\n", 
           node->port, sender_port);

    store_connection_info(node, msg->connection_id, sender_port);
    
    int current_pos = -1;
    for (int i = 0; i < msg->route_length; i++) {
        if (msg->route[i] == node->port) {
            current_pos = i;
            break;
        }
    }
    
    if (current_pos == -1 || current_pos >= msg->route_length - 1) {
        printf("Error: Invalid routing information\n");
        return;
    }
    

    int next_port = msg->route[current_pos + 1];
    printf("Intermediate node %d: Forwarding to port %d\n", node->port, next_port);
    
    if (send_udp_message(next_port, msg) < 0) {
        printf("Failed to forward message\n");
    }
}

void handle_exit_node(Node* node, Message* msg, int sender_port) {
    printf("Exit node %d: Received message from port %d\n", node->port, sender_port);
    
    store_connection_info(node, msg->connection_id, sender_port);
    
    printf("Exit node %d: Sending to final destination port %d\n", 
           node->port, msg->final_destination_port);
    printf("Data: %s\n", msg->data);
    
    int sock;
    struct sockaddr_in dest_addr;
    char response_buffer[BUFFER_SIZE];
    
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        printf("Exit node %d: Failed to create socket for external communication\n", node->port);
        return;
    }
    

    struct timeval timeout;
    timeout.tv_sec = 5; 
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(msg->final_destination_port);
    dest_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  // Localhost for testing
    
    int bytes_sent = sendto(sock, msg->data, msg->data_length, 0,
                           (struct sockaddr*)&dest_addr, sizeof(dest_addr));
    
    if (bytes_sent < 0) {
        printf("Exit node %d: Failed to send to final destination\n", node->port);
        snprintf(response_buffer, BUFFER_SIZE, "Error: Failed to reach destination port %d", 
                msg->final_destination_port);
    } else {
        printf("Exit node %d: Successfully sent %d bytes to destination\n", node->port, bytes_sent);
        
        socklen_t addr_len = sizeof(dest_addr);
        int bytes_received = recvfrom(sock, response_buffer, BUFFER_SIZE - 1, 0,
                                    (struct sockaddr*)&dest_addr, &addr_len);
        
        if (bytes_received > 0) {
            response_buffer[bytes_received] = '\0';
            printf("Exit node %d: Received response from destination: %s\n", 
                   node->port, response_buffer);
        } else {
            printf("Exit node %d: No response from destination (timeout or error)\n", node->port);
            snprintf(response_buffer, BUFFER_SIZE, "No response from destination port %d", 
                    msg->final_destination_port);
        }
    }
    
    close(sock);
    
    Message response;
    response.msg_type = MSG_RESPONSE;
    response.connection_id = msg->connection_id;
    response.route_length = 0;
    response.final_destination_port = 0;
    strncpy(response.data, response_buffer, BUFFER_SIZE - 1);
    response.data[BUFFER_SIZE - 1] = '\0';
    response.data_length = strlen(response.data);
    

    printf("Exit node %d: Sending response back to port %d\n", 
           node->port, sender_port);
    
    if (send_udp_message(sender_port, &response) < 0) {
        printf("Failed to send response\n");
    }
}

void handle_response(Node* node, Message* msg, int sender_port) {
    printf("Node %d: Routing response back for connection %d\n", 
           node->port, msg->connection_id);
    
    int prev_port = find_prev_node_port(node, msg->connection_id);
    if (prev_port == -1) {
        printf("Error: No connection info found for connection %d\n", 
               msg->connection_id);
        return;
    }
    
    printf("Node %d: Sending response back to port %d\n", node->port, prev_port);
    
    if (send_udp_message(prev_port, msg) < 0) {
        printf("Failed to route response back\n");
    }
}

void* node_thread(void* arg) {
    Node* node = (Node*)arg;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    Message msg;

    node->socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (node->socket_fd < 0) {
        perror("Socket creation failed");
        return NULL;
    }
    
    int opt = 1;
    setsockopt(node->socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(node->port);
    
    if (bind(node->socket_fd, (struct sockaddr*)&server_addr, 
             sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(node->socket_fd);
        return NULL;
    }
    
    printf("Node %d (%s) listening on port %d\n", 
           node->port, 
           (node->node_type == NODE_INTERMEDIATE) ? "Intermediate" : "Exit",
           node->port);
    
    while (1) {
        int bytes_received = recvfrom(node->socket_fd, &msg, sizeof(Message), 0,
                                    (struct sockaddr*)&client_addr, &client_len);
        
        if (bytes_received < 0) {
            perror("Receive failed");
            continue;
        }
        
        int sender_port = ntohs(client_addr.sin_port);
        
        if (msg.msg_type == MSG_DATA) {
            if (node->node_type == NODE_INTERMEDIATE) {
                handle_intermediate_node(node, &msg, sender_port);
            } else if (node->node_type == NODE_EXIT) {
                handle_exit_node(node, &msg, sender_port);
            }
        } else if (msg.msg_type == MSG_RESPONSE) {
            handle_response(node, &msg, sender_port);
        }
    }
    
    close(node->socket_fd);
    return NULL;
}

void create_node(int port, int node_type) {
    if (node_count >= MAX_NODES) {
        printf("Maximum number of nodes reached\n");
        return;
    }
    
    nodes[node_count].port = port;
    nodes[node_count].node_type = node_type;
    
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        nodes[node_count].connections[i].is_active = 0;
    }
    
    if (pthread_create(&nodes[node_count].thread, NULL, 
                      node_thread, &nodes[node_count]) != 0) {
        perror("Thread creation failed");
        return;
    }
    
    node_count++;
}