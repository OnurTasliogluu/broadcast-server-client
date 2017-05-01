#include "server.h"

int main()
{
	int     accept_connection[1000];
    pid_t   accept_child_pid;

    pipe(accept_connection);

    /*
    	Cocuk process olusturulur.
    */
    if((accept_child_pid = fork()) == -1)
    {
            DEBUG_PRINT("DEBUG_ERROR : Process olusturulamadi.");
            return -1;
    }

    if(accept_child_pid == 0)
    {
            // 
    }
    else
    {
            
    }
	return 0;
}