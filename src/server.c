#include "server.h"

#define MAX_CONNECTION 1024


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
*/
void accept_connection()
{
	int32_t *fd = socket_fd;

}
void receive_data()
{

}

/**
 * @brief Programın ana başlangıç noktasıdır.
 *        Çocuk process ve Parent process'i ayrıştırır.
 *
*/
void server_init()
{
	int32_t	socket_fd[MAX_CONNECTION];
	pid_t   accept_child_pid;
	/*
		Cocuk Process
	*/
    pipe(socket_fd);

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
    		DEBUG_INFO("Process Olusturuldu - Chiled Process");
    		void accept_connection((void*)socket_fd);
    }
    else
    {
            /*
            	Parent Process
            	Bu process sadece client'dan gelen verileri alacaktır.
            */
    		DEBUG_INFO("Process Olusturuldu - Parent Process");
    		void receive_data((void*)socket_fd);
    }
}