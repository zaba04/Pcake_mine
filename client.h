#ifndef CLIENT_H
#define CLIENT_H

#include "tor_types.h"

void send_tor_message(int client_port, int route[], int route_length, 
                     int final_dest, const char* data);
void* client_thread(void* arg);

#endif