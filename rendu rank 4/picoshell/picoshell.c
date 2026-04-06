#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int    picoshell(char **cmds[])
{
	int i = 0;
	int fd[2];
	int in_fd = 0;

	if (!cmds || !cmds[0])
		return (1);
	
	while (cmds[i])
	{
		if (cmds[i + 1] && pipe(fd) == -1)
			return (-1);
		
		if (fork() == 0)
		{
			if (in_fd != 0)
			{
				dup2(in_fd, 0);
				close(in_fd);
			}
			if (cmds[i + 1])
			{
				close(fd[0]);
				dup2(fd[1], 1);
				close(fd[1]);
			}
			execvp(cmds[i][0], cmds[i]);
			exit(1);
		}
		if (in_fd != 0)
			close(in_fd);
		if (cmds[i + 1])
		{
			close(fd[1]);
			in_fd = fd[0];
		}
		i++;
	}
	while (wait(NULL) > 0)
		;
	return (0);
}