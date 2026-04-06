#include <unistd.h>      // fork, alarm, _exit
#include <sys/wait.h>    // waitpid
#include <stdlib.h>      // exit
#include <signal.h>      // sigaction, kill, sigaddset, sigemptyset, sigfillset, sigdelset, sigismember
#include <stdio.h>       // printf
#include <string.h>      // strsignal
#include <errno.h>       // errno
#include <stdbool.h>     // bool

int sandbox(void (*f)(void), unsigned int timeout, bool verbose)
{
	if (timeout == 0 || f == NULL)
		return -1;
	pid_t pid = fork();
	if (pid < 0)
		return -1;
	if (pid == 0) {
		alarm(timeout);
		f();
		exit(0);
	}
	int status;
	if (waitpid(pid, &status, 0) == -1)
		return -1;
	
	if (WIFEXITED(status)) {
		int exit_code = WEXITSTATUS(status);
		if (exit_code == 0) {
			if (verbose)
				printf("Nice function!\n");
			return 1;
		}
		if (verbose)
			printf("Bad function: exited with code %d\n", exit_code);
		return 0;
	}
	if (WIFSIGNALED(status)) {
		if (verbose) {
			int sig = WTERMSIG(status);
			if (sig == SIGALRM)
				printf("Bad function: timed out after %u seconds\n", timeout);
			else
				printf("Bad function: %s\n", strsignal(sig));
		}
		return 0;
	}
	return 0;
}