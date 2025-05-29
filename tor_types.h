#ifndef TOR_TYPES_H
#define TOR_TYPES_H

#include <pthread.h>

#define MAX_NODES 10
#define MAX_CONNECTIONS 100
#define BUFFER_SIZE 1024
#define MAX_ROUTE_LENGTH 5

#define MSG_DATA 1
#define MSG_RESPONSE 2

#define NODE_INTERMEDIATE 1
#define NODE_EXIT 2

typedef struct {
    int msg_type;
    int connection_id;
    int route_length;
    int route[MAX_ROUTE_LENGTH];
    int final_destination_port;
    char data[BUFFER_SIZE];
    int data_length;
} Message;

typedef struct {
    int connection_id;
    int prev_node_port;
    int is_active;
} ConnectionInfo;

typedef struct {
    int port;
    int node_type;
    int socket_fd;
    ConnectionInfo connections[MAX_CONNECTIONS];
    pthread_t thread;
} Node;

#endif