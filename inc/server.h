#ifndef SERVER_H
#define SERVER_H

#include <stdint.h>

int32_t server_init();
int32_t accept_connection();
void receive_data();
void chiled_process(int32_t *p_shm_socket_fd)

#endif