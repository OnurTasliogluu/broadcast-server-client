#ifndef SERVER_H
#define SERVER_H

#include <stdint.h>

int32_t bind_connection();
int32_t accept_connection(int32_t main_socket);
int32_t create_socket_thread(int32_t *p_shm_socket_fd);
int32_t server_init();
void receive_data(void *v_socket_fd);

#endif