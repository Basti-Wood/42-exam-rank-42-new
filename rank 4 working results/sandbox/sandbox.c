#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
static void do_nothing(int sig)
{
    (void)sig;
}

int sandbox(void (*f)(void), unsigned int timeout, bool verbose)
{
    /* Prüfe Eingabe: Funktionspointer darf nicht NULL sein */
    if (f == NULL)
    {
        errno = EINVAL;
        return -1;
    }
    pid_t pid = fork();
    if (pid < 0)
        return -1;

    if (pid == 0)
    {
        /* Kindprozess: eigenen Alarm setzen (Timeout für f) */
        alarm(timeout);
        /* Funktion f ausführen; im Kind mit _exit beenden, damit keine stdio-flushes passieren */
        f();
        _exit(0);
    }

    struct sigaction sa = {0};
    sa.sa_handler = do_nothing;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGALRM, &sa, NULL) < 0)
        return -1;

    /* Elternprozess: SIGALRM-Handler setzen und eigenen Alarm starten.
       Der Elternalarm dient dazu, waitpid durch ein Signal zu unterbrechen (EINTR) */
    alarm(timeout);

    int status;
    pid_t r = waitpid(pid, &status, 0);
    if (r < 0)
    {
        /* Wenn waitpid durch ein Signal (wahrscheinlich der Eltern-Alarm) unterbrochen wurde,
           behandeln wir das als Timeout: Kind mit SIGKILL beenden und als "bad" werten */
        if (errno == EINTR)
        {
            kill(pid, SIGKILL);
            /* Auf Kind warten (erzwungener Abbruch) */
            waitpid(pid, NULL, 0);
            if (verbose)
                printf("Bad function: timed out after %u seconds\n", timeout);
            return 0;
        }
        /* Sonst ein echter Fehler beim waitpid */
        return -1;
    }

    /* Timer im Eltern deaktivieren: kein weiterer Alarm nötig */
    alarm(0);

    if (WIFEXITED(status))
    {
        int code = WEXITSTATUS(status);
        if (code == 0)
        {
            if (verbose)
                printf("Nice function!\n");
            return 1;
        }
        if (verbose)
            printf("Bad function: exited with code %d\n", code);
        return 0;
    }

    if (WIFSIGNALED(status))
    {
        int sig = WTERMSIG(status);
        if (sig == SIGALRM)
        {
            if (verbose)
                printf("Bad function: timed out after %u seconds\n", timeout);
        }
        else
        {
            if (verbose)
                printf("Bad function: %s\n", strsignal(sig));
        }
        return 0;
    }
    return -1;
}
