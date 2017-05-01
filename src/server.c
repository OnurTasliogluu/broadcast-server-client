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
int32_t chiled_process(int32_t *p_shm_socket_fd)
{
    DEBUG_INFO("Process Olusturuldu - Chiled Process");
    int32_t main_socket, new_socket;
    main_socket = bind_connection();
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
        /*
            Socket yapılmasının yapılacağı fonksiyona git.
            Dönen değer 1025. 
        */
        if (p_shm_socket_fd[MAX_CONNECTION] == MAX_CONNECTION)
        {
        	for(;;)
            {
                /*
                    Socket bağlantısı düşene kadar 5 sn bekle.
                */
                sleep(5);
                DEBUG_INFO("Connection_size : %d",p_shm_socket_fd[MAX_CONNECTION]);

                /*
                    Bağlantı sayısı düşmüşse yeniden bağlantı kabulü için beklemeye geç.
                */
                if(p_shm_socket_fd[MAX_CONNECTION] != MAX_CONNECTION)
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
            p_shm_socket_fd[p_shm_socket_fd[MAX_CONNECTION]] = new_socket;
            p_shm_socket_fd[MAX_CONNECTION]++;
            DEBUG_INFO("Connection_size : %d",p_shm_socket_fd[MAX_CONNECTION]);
        }
        DEBUG_INFO("Bağlantı Sayısı Maximum Degerine Ulaştı Connection_size : %d",p_shm_socket_fd[MAX_CONNECTION]);
    }
}

/**
 * @brief Gelen datayı işlemek için kullanılır.
 *
 * @param Shared olarak paylaşılan socket dizisidir.
 *
*/
void receive_data(int32_t *p_shm_socket_fd)
{
    DEBUG_INFO("Parent Process");
	int32_t temp_sock;
	int32_t i = 0;
    int32_t read_length;
    int32_t sock_fd;
    char buffer[1024];
	fd_set active_fd_set, temp_fd_set;

    FD_ZERO(&active_fd_set);

    /*
        Max bağlantı değeri kadar for yap ve FD_SET değerini set et.
    */
    for ( i = 0 ; i < p_shm_socket_fd[1024] ; i++) 
    {
        temp_sock = p_shm_socket_fd[i];
         
        if(temp_sock > 0)
            FD_SET( temp_sock , &temp_fd_set);
    }

    active_fd_set = temp_fd_set;

    select( p_shm_socket_fd[1024] + 1 , &active_fd_set , NULL , NULL , NULL);

    /*
        Her bir fd kontrol edilir FD_ISSET ile, eğer veri varsa diğer socketlere gönderilmesi gerekmektedir.
        Eğer veri okunamazsa socket düşmüş demektir.
    */
    for (i = 0; i < p_shm_socket_fd[1024]; ++i)
    {
       
        if (FD_ISSET(p_shm_socket_fd[i], &active_fd_set)) 
        {
          
            sock_fd = p_shm_socket_fd[i];
            memset(buffer,'\0',sizeof(buffer));
            if ((read_length = read( sock_fd , buffer, 1024)) == 0)
            {
                /*
                    Bağlantı sonlandırılır.
                    Bağlantı sayısı değeri 1 azaltılır.
                */
                close(sock_fd);
                p_shm_socket_fd[1024]--;
            }
            else
            {
                buffer[read_length] = '\0';
                send(sock_fd , buffer , strlen(buffer) , 0 );
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
    int32_t	*p_shm_socket_fd;
    /*
        p_shm_socket_fd[1024] <-> 1024. byte connection size ı belirler.
    */
    int32_t result;
    pid_t   accept_child_pid;
    int32_t    ShmID;

    /*
        1024 Byte degerinde "shared memory" elde edilir.
    */
    ShmID = shmget(IPC_PRIVATE, 1025*sizeof(int32_t), IPC_CREAT | 0666);
    if (ShmID < 0)
    {
        DEBUG_ERROR("SMGGET hatalı dönüş yaptı.");
        exit(1);
    }

    p_shm_socket_fd = (int32_t *) shmat(ShmID, NULL, 0);
    if ((int32_t) p_shm_socket_fd == -1)
    {
        DEBUG_ERROR("SHMAT hatalı dönüş yaptı.");
        exit(1);
    }
    DEBUG_INFO("1025 byte kadar shared memory olusturuldu.");

    memset(p_shm_socket_fd,'\0',1025*sizeof(int32_t));
    /*
        Cocuk process olusturulur.
    */
    if((accept_child_pid = fork()) == -1)
    {
        DEBUG_ERROR("Process olusturulamadi.");
        return -1;
    }

    if(accept_child_pid == 0)
    {
        /*
            Cocuk Process
            Bu process sadece client'dan gelen baglantı isteklerini kabul edecektir.
        */
        chiled_process(p_shm_socket_fd);
        
    }
    else
    {
        /*
            Parent Process
            Bu process sadece client'dan gelen verileri alacaktır.
        */
        DEBUG_INFO("Process Olusturuldu - Parent Process");
        receive_data(p_shm_socket_fd);
    }
}