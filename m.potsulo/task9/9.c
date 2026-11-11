
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    pid_t pid;
    int status;

    // Check arguments
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
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
        printf("[CHILD] My PID: %ld\n", (long)getpid());
        fflush(stdout);

        execlp("cat", "cat", argv[1], (char *)NULL);

        perror("execlp");
        _exit(1);
    }
    else
    {
        // Parent process
        printf("[PARENT] My PID: %ld\n", (long)getpid());
        printf("[PARENT] Child PID: %ld\n", (long)pid);
        printf("[PARENT] Printing some text...\n");
        fflush(stdout);

        // Wait for child process
        if (waitpid(pid, &status, 0) == -1)
        {
            perror("waitpid");
            return 1;
        }

        if (WIFEXITED(status))
        {
            printf("[PARENT] Child exited with status: %d\n",
                   WEXITSTATUS(status));
        }

        // Last line after child finishes
        printf("[PARENT] Child has finished. This is the last line.\n");
    }

    return 0;
}
