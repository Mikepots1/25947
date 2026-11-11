
#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char *argv[])
{
    pid_t pid;
    int status;

    // Check arguments
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <command> [arguments]\n", argv[0]);
        return 1;
    }

    // Create child process
    pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return 1;
    }

    if (pid == 0)
    {
        // Child process
        execvp(argv[1], &argv[1]);

        perror("Execvp failed");
        _exit(127);
    }
    else
    {
        // Parent process
        printf("[PARENT] Starting command: %s\n", argv[1]);
        fflush(stdout);

        // Wait for child process
        if (waitpid(pid, &status, 0) == -1)
        {
            perror("waitpid");
            return 1;
        }

        // Check exit status
        if (WIFEXITED(status))
        {
            printf("[PARENT] Command exited normally with code: %d\n",
                   WEXITSTATUS(status));
        }
        else if (WIFSIGNALED(status))
        {
            printf("[PARENT] Command terminated by signal: %d (%s)\n",
                   WTERMSIG(status), strsignal(WTERMSIG(status)));
        }
    }

    return 0;
}
