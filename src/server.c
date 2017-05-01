#include "server.h"

#include <unistd.h>
#include <stdio.h>

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

/**
 * @brief Programın ana başlangıç noktasıdır.
 *        Çocuk process ve Parent process'i ayrıştırır.
 *
 * @return Hata değeridir.
 *			-1: Hata oluştu.
*/
int32_t server_init()
{
	int32_t	socket_fd[MAX_CONNECTION];
	int32_t connection_size = 0;
	pid_t   accept_child_pid;
	
	/*
		Paylaşım yapılacak şekilde pipe ile ilgili değerlerin ortak kullanılacağı belirtilir.
	*/
	pipe(socket_fd);
	pipe(connection_size);

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
		for(;;)
		{
			/*
				Socket yapılmasının yapılacağı fonksiyona git.
			*/
			sock = accept_connection();
			/*
				Bağlantı başarıylı mı değil mi?
			*/
			if (sock < 0)
			{
				DEBUG_ERROR("Socket Olusturulamadi");
			}
			/*
				Bağlantı başarılıysa:
				sock değeri array'e taşınır.
				connection_size 1 arttırılır.
			*/
			else
				{
					DEBUG_INFO("Socket başarıyla oluturuldu sock_no : %d",sock);
					socket_fd[connection_size] = sock;
					connection_size++;
					DEBUG_INFO("Connection_size : %d",connection_size);
				}
				if (connection_size == 1024)
				{
					DEBUG_INFO("Bağlantı Sayısı Maximum Degerine Ulaştı Connection_size : %d",connection_size);
					for(;;)
					{
						/*
							Socket bağlantısı düşene kadar 5 sn bekle.
						*/
						sleep(5);
						DEBUG_INFO("Connection_size : %d",connection_size);
						/*
							Bağlantı sayısı düşmüşse yeniden bağlantı kabulü için beklemeye geç.
						*/
						if(connection_size != 1024)
						{
							break;
						}
					}
					
				}
			}
			
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