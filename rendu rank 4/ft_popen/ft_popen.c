// pipe, fork, dup2, execvp, close, exit
#include <unistd.h>     // pipe, fork, dup2, close, execvp, exit
#include <stdlib.h>     // exit


int ft_popen(const char *file, char *const argv[], char type)
{
	if ((type != 'r' && type != 'w') || file == NULL || argv == NULL)
		return -1; // Invalid type
	int pipefd[2];
	pid_t pid;
	if (pipe(pipefd) == -1)
		return -1;
	pid = fork();
	if (pid == -1)
	{
		close(pipefd[0]);
		close(pipefd[1]);
		return -1;
	}
	if (pid == 0) // Child process
	{
		if (type == 'r')
		{
			close(pipefd[0]); // Close unused read end
			dup2(pipefd[1], STDOUT_FILENO); // Redirect stdout to pipe write end
		}
		else if (type == 'w')
		{
			close(pipefd[1]); // Close unused write end
			dup2(pipefd[0], STDIN_FILENO); // Redirect stdin to pipe read end
		}
		close(pipefd[0]);
		close(pipefd[1]);
		execvp(file, argv);
		_exit(EXIT_FAILURE); // If execvp fails
	}
	else // Parent process
	{
		if (type == 'r')
		{
			close(pipefd[1]); // Close unused write end
			return pipefd[0]; // Return read end
		}
		else if (type == 'w')
		{
			close(pipefd[0]); // Close unused read end
			return pipefd[1]; // Return write end
		}
	}
	return -1; // In case of invalid type
}