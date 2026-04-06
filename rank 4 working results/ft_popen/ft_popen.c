#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

/*
 * ft_popen: vereinfachte popen-ähnliche Funktion
 * - file: Programm/Datei, die ausgeführt werden soll (argv[0] für execvp)
 * - av: Argumentliste (NULL-terminiertes Array), wie bei execvp
 * - type: 'r'  -> Elternprozess liest die stdout des Kindes
 *         'w'  -> Elternprozess schreibt in die stdin des Kindes
 *
 * Rückgabe: Dateideskriptor für das elterliche Ende der Pipe (>=0)
 *           oder -1 bei Fehlern.
 *
 * Wichtige Hinweise:
 * - Diese Funktion erstellt eine Pipe und forkt. Im Kind wird
 *   das passende Ende der Pipe auf STDIN/STDOUT geduppt und
 *   anschließend execvp aufgerufen (Kind ersetzt sich selbst).
 * - Im Elternprozess wird das für ihn relevante Pipe-Ende
 *   zurückgegeben; das andere Ende wird geschlossen.
 */
int ft_popen(const char *file, char *const av[], int type)
{
    /* Eingabevalidierung: file und av müssen vorhanden sein;
       type muss entweder 'r' (lesen) oder 'w' (schreiben) sein */
    if (!file || !av || (type != 'r' && type != 'w'))
        return -1;

    int fd[2];
    /* Erzeuge eine Pipe: fd[0] = Leseende, fd[1] = Schreibeende */
    if (pipe(fd) < 0)
        return -1;

    pid_t pid = fork();
    if (pid < 0) {
        /* Fork schlug fehl: beide Enden schließen und Fehler zurückgeben */
        close(fd[1]);
        close(fd[0]);
        return -1;
    }

    if (pid == 0) {
        /* Kindprozess */
        if (type == 'r') {
            /* Eltern will lesen: Kind muss seine stdout -> Pipe schreiben */
            close(fd[0]); /* Kind braucht das Leseende nicht */
            /* Dupliziere das Schreibende der Pipe auf STDOUT
               (so werden alle Ausgaben des Kindes in die Pipe geschrieben) */
            if (dup2(fd[1], STDOUT_FILENO) < 0)
                exit(-1);
        } else {
            /* type == 'w': Eltern will schreiben -> Kind soll stdin aus Pipe lesen */
            close(fd[1]); /* Kind braucht das Schreibeende nicht */
            /* Dupliziere das Leseende der Pipe auf STDIN
               (so liest das Kind von der Pipe als wäre es stdin) */
            if (dup2(fd[0], STDIN_FILENO) < 0)
                exit(-1);
        }

        /* Nach dup2 sind die Standard-FDs gesetzt; die originalen Pipe-FDs
           können geschlossen werden (dup2 hat Kopien erstellt). */
        close(fd[0]);
        close(fd[1]);

        /* Ersetze das Kind-Image durch das neue Programm.
           Bei Erfolg kehrt execvp nicht zurück; bei Fehler -> exit(-1). */
        execvp(file, av);
        exit(-1);
    }

    /* Elternprozess: gib das für ihn relevante Ende der Pipe zurück */
    if (type == 'r') {
        /* Eltern liest: schließe das Schreibende und gib das Leseende zurück */
        close(fd[1]);
        return fd[0];
    } else {
        /* Eltern schreibt: schließe das Leseende und gib das Schreibeende zurück */
        close(fd[0]);
        return fd[1];
    }
}