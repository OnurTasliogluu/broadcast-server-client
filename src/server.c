#include "server.h"
#include "debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <arpa/inet.h>
#include <netinet/in.h>

#define MAX_CONNECTION 1024
#define PORT 5000


/*
    typedef signed char 	int8_t
    typedef unsigned char 	uint8_t
    typedef signed int 	int16_t
    typedef unsigned int 	uint16_t
    typedef signed long int 	int32_t
    typedef unsigned long int 	uint32_t
    typedef signed long long int 	int64_t
    typedef unsigned long long int 	uint64_t
*/

/**
 * @brief Client'dan gelen istekleri kabul eder.
 *
 * @return Socket numarasıdır.
*/
int32_t bind_connection()
{
    int32_t sock;
    struct sockaddr_in server_name;

    /*
        Socket oluşturuluyor.
    */
	sock = socket (PF_INET, SOCK_STREAM, 0);
    if (sock < 0)
	{
        DEBUG_ERROR("Socket Olusturulamadi.");
        return -1;
    }

    server_name.sin_family = AF_INET;
    server_name.sin_port = htons (PORT);
    server_name.sin_addr.s_addr = htonl (INADDR_ANY);
    if (bind (sock, (struct sockaddr *) &server_name, sizeof (server_name)) < 0)
	{
        DEBUG_ERROR("Bind Yapilamadi.");
        return -1;
    }

    if (listen(sock, 3) < 0)
	{
        DEBUG_ERROR("Listen Yapilamadi");
        return -1;
    }
    return sock;
}

/**
 * @brief Client'dan gelen istekleri kabul eder.
 *
 * @return Socket numarasıdır.
*/
int32_t accept_connection(int32_t main_socket)
{
    int32_t sock, addr_len;
    struct sockaddr_in server_name;

    server_name.sin_family = AF_INET;
    server_name.sin_port = htons (PORT);
    server_name.sin_addr.s_addr = htonl (INADDR_ANY);

    sock = accept(main_socket, (struct sockaddr *)&server_name, (socklen_t*)&addr_len);

    if (sock < 0)
    {
        DEBUG_ERROR("Accept Yapilamadi.");
        return -1;
    }
    return sock;
}
/**
 * @brief Socket baglantısı oluştuktan sonra p_shm_socket_fd değişken dizisine "file descripter" değeri alınır.
 * Bu değer Parent process de kullanılmak üzere atanmış olur.
 *
*/
int32_t create_socket_thread(int32_t *socket_fd)
{
    DEBUG_INFO("Process Olusturuldu - Chiled Process");
    int32_t main_socket, new_socket;
    main_socket = bind_connection();
    connection_size;
    /*
        Bağlantı başarıylı mı değil mi?
    */
    if (main_socket < 0)
    {
        DEBUG_ERROR("Socket olusturulamadi.");
        return -1;
    }

    DEBUG_ERROR("Ana socket oluşturuldu.");
    for(;;)
    {

        if (socket_fd[MAX_CONNECTION] == MAX_CONNECTION)
        {
        	for(;;)
            {
                /*
                    Socket bağlantısı düşene kadar 5 sn bekle.
                */
                sleep(5);
                DEBUG_INFO("Bağlantı Sayısı Maximum Degerine Ulaştı Connection_size : %d",socket_fd[MAX_CONNECTION]);

                /*
                    Bağlantı sayısı düşmüşse yeniden bağlantı kabulü için beklemeye geç.
                */
                if(socket_fd[MAX_CONNECTION] != MAX_CONNECTION)
                {
                    break;
                }
            }
        }
        /*
            Bağlantı başarılıysa:
            sock değeri array'e taşınır.
            connection_size 1 arttırılır.
        */
        else
        {
            new_socket = accept_connection(main_socket);
            DEBUG_INFO("Socket başarıyla oluturuldu sock_no : %d",new_socket);
            /*
                1024. integer değer connection sayısını belirtir.
            */
            socket_fd[socket_fd[MAX_CONNECTION]] = new_socket;
            socket_fd[MAX_CONNECTION]++;
            DEBUG_INFO("Connection_size : %d",socket_fd[MAX_CONNECTION]);
        }
        
    }
}

/**
 * @brief Gelen datayı işlemek için kullanılır.
 *
 * @param Shared olarak paylaşılan socket dizisidir.
 *
*/
void receive_data(int32_t *socket_fd)
{
    DEBUG_INFO("Parent Process");
	int32_t temp_sock;
	int32_t i = 0;
    int32_t read_length;
    int32_t sock_fd;
    char buffer[1024];
	fd_set active_fd_set, temp_fd_set;

    FD_ZERO(&active_fd_set);

    for (;;)
    {
        /*
            Max bağlantı değeri kadar for yap ve FD_SET değerini set et.
        */
        for ( i = 0 ; i < socket_fd[MAX_CONNECTION] ; i++) 
        {
            temp_sock = socket_fd[i];
             
            if(temp_sock > 0)
                FD_SET( temp_sock , &temp_fd_set);
        }

        active_fd_set = temp_fd_set;

        select( socket_fd[MAX_CONNECTION] + 1 , &active_fd_set , NULL , NULL , NULL);

        /*
            Her bir fd kontrol edilir FD_ISSET ile, eğer veri varsa diğer socketlere gönderilmesi gerekmektedir.
            Eğer veri okunamazsa socket düşmüş demektir.
        */
        for (i = 0; i < socket_fd[MAX_CONNECTION]; ++i)
        {
           
            if (FD_ISSET(socket_fd[i], &active_fd_set)) 
            {
              
                sock_fd = socket_fd[i];
                memset(buffer,'\0',sizeof(buffer));
                if ((read_length = read( sock_fd , buffer, 1024)) == 0)
                {
                    /*
                        Bağlantı sonlandırılır.
                        Bağlantı sayısı değeri 1 azaltılır.
                    */
                    close(sock_fd);
                    socket_fd[1024]--;
                }
                else
                {
                    send(sock_fd , buffer , strlen(buffer) , 0 );
                }
            }
        }
    }
}

/**
 * @brief Programın ana başlangıç noktasıdır.
 *        Çocuk process ve Parent process'i ayrıştırır.
 *
 * @return Hata değeridir.
 *			-1: Hata oluştu.
*/
int32_t server_init()
{
    int32_t	socket_fd[MAX_CONNECTION+1];
    int32_t result;

    pthread_t thread_create_socket;

    int result = pthread_create( &thread_create_socket, NULL, create_socket_thread, (void*)socket_fd);

    /*
        Thread
        Bu thread sadece client'dan gelen baglantı isteklerini kabul edecektir.
    */
    if(result < 0)
    {
        DEBUG_ERROR("Thread olusturulamadi.");
        return -1;
    }
    else
    {
        pthread_join(thread_create_socket, NULL);
        /*
            Parent Process
            Bu process sadece client'dan gelen verileri alacaktır.
        */
        receive_data(socket_fd);
    }
}