#include "server.h"

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

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
int32_t accept_connection()
{
    int32_t sock;
    struct sockaddr_in s_name;

    /*
        Socket oluşturuluyor.
    */
	sock = socket (PF_INET, SOCK_STREAM, 0);
    if (sock < 0)
	{
        DEBUG_ERROR("Socket Olusturulamadi.");
        return -1;
    }

    s_name.sin_family = AF_INET;
    s_name.sin_port = htons (PORT);
    s_name.sin_addr.s_addr = htonl (INADDR_ANY);
    if (bind (sock, (struct sockaddr *) &s_name, sizeof (s_name)) < 0)
	{
        DEBUG_ERROR("Bind Yapilamadi.");
        return -1;
    }

    if (listen(s_name, 3) < 0)
	{
        DEBUG_ERROR("Listen Yapilamadi");
        return -1;
    }
    return sock;
}

void receive_data()
{

}

int32_t chiled_process(int32_t *p_shm_socket_fd)
{
    DEBUG_INFO("Process Olusturuldu - Chiled Process");

    int32_t result;
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
        result = accept_connection();
        /*
            Bağlantı başarıylı mı değil mi?
        */
        if (result < 0)
        {
        	DEBUG_ERROR("Socket olusturulamadi.");
        	return -1;
        }
        /*
            Bağlantı başarılıysa:
            sock değeri array'e taşınır.
            connection_size 1 arttırılır.
        */
        else
        {
            DEBUG_INFO("Socket başarıyla oluturuldu sock_no : %d",result);
            p_shm_socket_fd[p_shm_socket_fd[MAX_CONNECTION]] = result;
            p_shm_socket_fd[MAX_CONNECTION]++;
            DEBUG_INFO("Connection_size : %d",p_shm_socket_fd[MAX_CONNECTION]);
        }
        /*
            MAX Socket baglantısı kontrol edilir.
        */
        if (p_shm_socket_fd[1024] == MAX_CONNECTION)
        {
            DEBUG_INFO("Bağlantı Sayısı Maximum Degerine Ulaştı Connection_size : %d",connection_size);
            
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
    fd_set active_fd_set, temp_fd_set;
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
        receive_data((void*)socket_fd);
    }
}