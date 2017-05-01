#include "server.h"

int main()
{
	int     accept_connection[1000];
    pid_t   childpid;

    pipe(fd);

    if((childpid = fork()) == -1)
    {
            perror("fork");
            exit(1);
    }

    if(childpid == 0)
    {
            /* Child process closes up input side of pipe */
            close(fd[0]);
    }
    else
    {
            /* Parent process closes up output side of pipe */
            close(fd[1]);
    }
	return 0;
}