#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <ctype.h>

#include "debug.h"
#include "server.h"

#define port 9999
#define ip_address "127.0.0.1"
#define client_thread_size 50
#define client_listen_size 50

int32_t socket_size[client_thread_size];
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief For keep socket number
 *
 * @param For set sock_id number or clean sock_id
 *
 * @param If it is 0 : clean sock_id number
 *        Or set sock_id number
 *
 * @return Error number
*/
int32_t search_empty_and_set_socket_number(int32_t sock_id, int32_t condition)
{
    int32_t i;
    pthread_mutex_lock(&lock);
    if(condition)
    {
        for(i = 0; i<client_thread_size ; i++)
        {
            if(socket_size[i] == 0)
            {
                socket_size[i] = sock_id;
                break;
            }
        }
    }
    else
    {
        for(i = 0; i<client_thread_size ; i++)
        {
            if(socket_size[i] == sock_id)
            {
                socket_size[i] = 0;
                break;
            }
        }
    }
    pthread_mutex_unlock(&lock);
    return 0;
}

/**
 * @brief For send all client message
 *
 * @param Sender sock_id
 *
 * @param Incoming message from sender
 *
 * @return Error number
*/
int32_t send_broadcast(int32_t base_socket, int8_t *client_message)
{
    int32_t i;

    for (i = 0; i < client_thread_size; i++)
    {
        if(socket_size[i] != 0 & (socket_size[i] != base_socket))
        {
            if(send(socket_size[i],client_message,1024,MSG_NOSIGNAL) < 0)
            {
                #ifdef DEBUG
                    DEBUG_INFO("Didn't send data\n");
                #endif
                return -1;
            }
        }
    }
    return 0;
}

/**
 * @brief It creates for each socket.
 *
 * @param Sock_id pointer
*/
void *socket_thread(void *arg)
{
    int32_t new_socket = *((int32_t *)arg);
    int32_t keepalive = 1;
    int8_t client_message[1024];
    while (1)
    {
        memset(client_message, '\0', 1024);
        if(recv(new_socket , client_message , 1024, MSG_NOSIGNAL) <= 0)
        {
            #ifdef DEBUG
                DEBUG_INFO("Exit socket_thread \n");
            #endif
            close(new_socket);
            search_empty_and_set_socket_number(new_socket, 0);
            pthread_exit(NULL);
            break;
        }

        if(send_broadcast(new_socket, client_message))
        {
            #ifdef DEBUG
                DEBUG_INFO("Exit socket_thread \n");
            #endif
            close(new_socket);
            search_empty_and_set_socket_number(new_socket, 0);
            pthread_exit(NULL);
            break;
        }
    }
}

/**
 * @brief Main loop
*/
void start_server()
{
    int32_t server_socket, new_socket;
    struct sockaddr_in server_addr;
    struct sockaddr_storage server_storage;
    socklen_t addr_size;
    server_socket = socket(PF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip_address);
    memset(server_addr.sin_zero, '\0', sizeof server_addr.sin_zero);
    bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if(listen(server_socket,client_listen_size) == 0)
    {
        #ifdef DEBUG
            DEBUG_INFO("Listening\n");
        #endif
    }
    else
    {
        #ifdef DEBUG
            DEBUG_INFO("Error\n");
        #endif
    }

    pthread_t tid[client_thread_size];
    int32_t i = 0;

    while(1)
    {
        addr_size = sizeof server_storage;
        new_socket = accept(server_socket, (struct sockaddr *) &server_storage, &addr_size);
        if (new_socket < 0)
        {
            #ifdef DEBUG
                DEBUG_INFO("Didn't create socket\n");
            #endif
        }
        else
        {
            if(i <= client_thread_size)
            {
                if(pthread_create(&tid[i], NULL, socket_thread, &new_socket) != 0)
                {
                    #ifdef DEBUG
                        DEBUG_INFO("Failed to create thread\n");
                    #endif
                }
                else
                {
                    search_empty_and_set_socket_number(new_socket,1);
                    #ifdef DEBUG
                        DEBUG_INFO("Thread olusturuldu. Socket number = %d", new_socket);
                        DEBUG_INFO("i=%d",i);
                    #endif
                    i++;
                }
            }
        }

        if(i >= client_thread_size)
        {
            #ifdef DEBUG
                DEBUG_INFO("i >= client_thread_size");
            #endif
            i = 0;
            while(i < client_thread_size)
            {
                pthread_join(tid[i++],NULL);
            }
            i = 0;
        }
    }
}
