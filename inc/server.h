#ifndef SERVER_H
#define SERVER_H

#include <stdint.h>

int32_t search_empty_and_set_socket_number(int32_t sock_id, int32_t condition);
int32_t send_broadcast(int32_t base_socket, int8_t *client_message);
void *socket_thread(void *arg);
void start_server();

#endif