// close, fork, wait, exit, execvp, dup2, pipe

#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

/*
 * picoshell:
 * - Erwartet ein Array von Befehls-Argument-Arrays: char **cmds[]
 *   z.B. cmds = { cmd0_argv, cmd1_argv, ..., NULL }
 *   Jedes cmdX_argv ist ein NULL-terminiertes Array (wie execvp erwartet).
 *
 * Verhalten:
 * - Baut eine Pipeline aus mehreren Kommandos auf, wenn mehr als ein
 *   Kommando vorhanden ist. Beispiel: cmds = { {"ls","-l",NULL}, {"grep","foo",NULL}, NULL }
 * - Für jede Übergabe (außer dem letzten) wird eine Pipe erzeugt.
 * - Für jedes Kommando wird ein Kindprozess geforkt. Das Kind:
 *     - leitet bei Bedarf stdin auf das vorherige Pipe-Leseende (in_fd) um
 *     - leitet bei Bedarf stdout auf das aktuelle Pipe-Schreibeende (fd[1]) um
 *     - führt execvp(cmds[i][0], cmds[i]) aus
 * - Der Elternprozess schließt nicht mehr benötigte FDs und behält das
 *   Leseende der aktuellen Pipe in `in_fd`, das als stdin für das nächste
 *   Kommando dient.
 * - Am Ende werden verbleibende FDs geschlossen und auf alle Kinder gewartet.
 *
 * Rückgabe: 0 bei Erfolg, 1 bei Fehlern (einfacher Fehlercode, kein errno-Forwarding)
 */
int    picoshell(char **cmds[])
{
	int i = 0;
	int fd[2];        /* Temporäre Pipe für die aktuelle Stufe */
	int in_fd = 0;    /* Leseende der vorherigen Pipe; 0 == stdin (anfangs) */
	pid_t pid;

	/* Basiskontrolle: cmds und erstes Kommando müssen existieren */
	if (!cmds || !cmds[0])
		return 1;

	while (cmds[i])
	{
		/* Wenn das aktuelle Kommando leer ist -> Fehler */
		if (!cmds[i][0])
		{
			if (in_fd != 0)
				close(in_fd);
			return 1;
		}

		/* Wenn es ein folgendes Kommando gibt, brauchen wir eine Pipe */
		if (cmds[i+1])
		{
			if (pipe(fd) == -1)
			{
				/* Fehler beim Anlegen der Pipe: vorheriges in_fd schließen */
				if (in_fd != 0)
					close(in_fd);
				return 1;
			}
		}
		else
		{
			/* Kein weiteres Kommando: markiere fd als ungültig */
			fd[0] = -1;
			fd[1] = -1;
		}

		pid = fork();
		if (pid == -1)
		{
			/* Fork fehlgeschlagen: Aufräumen und Fehler zurückgeben */
			if (in_fd != 0)
				close(in_fd);
			if (fd[1] != -1)
			{
				close(fd[0]);
				close(fd[1]);
			}
			return 1;
		}

		if (pid == 0)
		{
			/* Kindprozess */
			if (in_fd != 0)
			{
				/* Wenn in_fd != 0, bedeutet das: stdin soll aus in_fd gelesen werden */
				if (dup2(in_fd, STDIN_FILENO) == -1)
					exit(1);
				/* Ursprungliches in_fd kann jetzt geschlossen werden */
				close(in_fd);
			}
			if (fd[1] != -1)
			{
				/* Wenn fd[1] gültig ist, bedeutet das: stdout soll in die neue Pipe schreiben */
				if (dup2(fd[1], STDOUT_FILENO) == -1)
					exit(1);
				/* Beide Pipe-Ends können im Kind geschlossen werden (dup2 hat Kopien erstellt) */
				close(fd[1]);
				close(fd[0]);
			}
			/* Ersetze das Kind-Image durch das gewünschte Kommando */
			execvp(cmds[i][0], cmds[i]);
			/* Exec ist fehlgeschlagen */
			exit(1);
		}
		else
		{
			/* Elternprozess: Aufräumen für diese Stufe */
			if (in_fd != 0)
				close(in_fd); /* schließe das zuvor verwendete Leseende */
			if (fd[1] != -1)
				close(fd[1]); /* Eltern braucht das Schreibende nicht */
			/* Das Leseende der gerade erstellten Pipe wird zum in_fd für die nächste Stufe */
			in_fd = fd[0];
			i++;
		}
	}

	/* Nach der Schleife: wenn noch ein offenes in_fd existiert, schließen */
	if (in_fd != 0)
		close(in_fd);

	/* Auf alle Kindprozesse warten (einfaches wait-Loop) */
	while (wait(NULL) > 0)
		;

	return 0;
}
/*
int main(int argc, char **argv)
{
	(void)argc;
	(void)argv;
	char *cmd1[] = {"ls", "-l", NULL};
	char *cmd2[] = {"echo", "picoshell", NULL};
	char **cmds[] = {};
	for (int i = 0; i < 3; i++)
	{
		picoshell(cmds);
	}


	return 0;
}
*/