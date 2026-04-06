//fork, waitpid, exit, alarm, sigaction, kill, printf, strsignal,
//errno, sigaddset, sigemptyset, sigfillset, sigdelset, sigismember
#include <stdbool.h>
#include <stddef.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

void no(int sig)
{
	(void) sig;
}

int sandbox(void (*f)(void), unsigned int timeout, bool verbose)
{
	if (f == NULL || timeout == 0)
		return -1;
	pid_t pid = fork();
	if (pid < 0)
		return -1;
	if (pid == 0) 
	{
		alarm(timeout);
		f();
		exit(0);
	}
	else
	{
		struct sigaction sa = {0};
		sa.sa_handler = no;
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = 0;
		if(sigaction(SIGALRM, &sa, 0) < 0)
			return -1;

		alarm(timeout);
		int status;
		pid_t r = waitpid(pid, &status, 0);
		if (r < 0)
		{
			if (errno == EINTR)
			{
				kill(pid, SIGKILL);
				waitpid(pid, &status, 0);
				if (verbose)
					printf("Bad function: timed out after %u seconds\n", timeout);
				return 0;
			}
        	return -1;
		}

		alarm(0);

		if (WIFEXITED(status))
		{
			int exit_code = WEXITSTATUS(status);
			if(exit_code == 0)
			{
				if (verbose)
					printf("Nice function!\n");
				return 1;
			}
			else
			{
				if (verbose)
					printf("Bad function: exited with code %d\n", exit_code);
				return 0;
			}
		}
		if (WIFSIGNALED(status))
		{
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
}

void blocks_sigalrm(void) {
    sigset_t set;
    sigfillset(&set);
    sigprocmask(SIG_BLOCK, &set, NULL);  // Block all signals including SIGALRM
    while(1) { sleep(1); }
}

int main()
{
	sandbox(blocks_sigalrm, 5, true);
	return 0;
}
