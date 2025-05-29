#ifndef NETWORK_UTILS_H
#define NETWORK_UTILS_H

#include "tor_types.h"

Node* find_node_by_port(int port);
int send_udp_message(int port, Message* msg);
void store_connection_info(Node* node, int connection_id, int prev_port);
int find_prev_node_port(Node* node, int connection_id);

#endif 