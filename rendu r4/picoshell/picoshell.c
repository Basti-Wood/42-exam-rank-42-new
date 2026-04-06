#include <unistd.h>     // close, fork, execvp, dup2, pipe
#include <sys/types.h>  // fork, wait (for pid_t type)
#include <sys/wait.h>   // wait, waitpid
#include <stdlib.h>     // exit


int    picoshell(char **cmds[])
{
	if(!cmds) {
		return 1;
	}
	if(!cmds[0]) {
		return 1;
	}
	int i = 0;
	int in_fd = 0; // Initial input file descriptor (stdin)
	pid_t pid;
	while(cmds[i])
	{
		int pipe_fd[2];
		if(cmds[i + 1]) {
			if(pipe(pipe_fd) == -1) {
				return -1;
			}
		}
		pid = fork();
		if(pid == -1) {
			return -1;
		}
		if (pid == 0)
		{
			if (in_fd != 0) {
				dup2(in_fd, 0); // Redirect stdin to in_fd
				close(in_fd);
			}
			if (cmds[i + 1]) {
				close(pipe_fd[0]); // Close unused read end
				dup2(pipe_fd[1], 1); // Redirect stdout to pipe write end
				close(pipe_fd[1]);
			}
			if (cmds[i] && cmds[i][0]) {
				execvp(cmds[i][0], cmds[i]);
			}
			exit(EXIT_FAILURE);
		}
		else
		{
			if (in_fd != 0) {
				close(in_fd); // Close previous input fd
			}
			if (cmds[i + 1]) {
				close(pipe_fd[1]); // Close unused write end
				in_fd = pipe_fd[0]; // Next command reads from here
			}
			i++;
		}
	}
	
	// Wait for all children after all processes are started
	while (wait(NULL) > 0)
		;
	
	return 0;
}