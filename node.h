#ifndef NODE_H
#define NODE_H

#include "tor_types.h"

void handle_intermediate_node(Node* node, Message* msg, int sender_port);
void handle_exit_node(Node* node, Message* msg, int sender_port);
void handle_response(Node* node, Message* msg, int sender_port);
void* node_thread(void* arg);
void create_node(int port, int node_type);

#endif