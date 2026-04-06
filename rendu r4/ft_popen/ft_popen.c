//pipe, fork, dup2, execvp, close, exit
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>


int ft_popen(const char *file, char *const argv[], char type)
{
	if((type != 'r' && type != 'w') || file == NULL || argv == NULL)
		return (-1);

	int pipefd[2];
	pid_t pid;

	if (pipe(pipefd) == -1)
		return (-1);
	if ((pid = fork()) < 0)
		return (-1);
	if (pid == 0)
	{
		if (type =='r')
		{
			close(pipefd[0]);
			dup2(pipefd[1], 1);
		}
		else if (type == 'w')
		{
			close(pipefd[1]);
			dup2(pipefd[0], 0);
		}
		close(pipefd[0]);
		close(pipefd[1]);
		execvp(file, argv);
		exit(1);
	}
	else
	{
		if (type == 'r')
		{
			close(pipefd[1]);
			return (pipefd[0]);
		}
		else if (type == 'w')
		{
			close(pipefd[0]);
			return (pipefd[1]);
		}
	}
	return (-1);
}