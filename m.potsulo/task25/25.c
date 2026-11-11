
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define BUFFER_SIZE 1024

int main()
{
    int pipefd[2];
    pid_t pid;
    char buffer[BUFFER_SIZE];
    ssize_t n;
    ssize_t i;

    /* Create pipe */
    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        return 1;
    }

    /* Create child process */
    pid = fork();

    if (pid < 0)
    {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return 1;
    }

    if (pid == 0)
    {
        /* Child process */

        /* Close unused write end */
        close(pipefd[1]);

        printf("[CHILD] Waiting for data...\n");
        fflush(stdout);

        /* Read data from pipe */
        while ((n = read(pipefd[0], buffer, sizeof(buffer))) > 0)
        {
            /* Convert characters to uppercase */
            for (i = 0; i < n; i++)
            {
                buffer[i] = toupper((unsigned char)buffer[i]);
            }

            /* Print converted text */
            printf("[CHILD] Received: ");
            fwrite(buffer, 1, (size_t)n, stdout);
            fflush(stdout);
        }

        if (n < 0)
            perror("read");

        /* Close read end */
        close(pipefd[0]);

        exit(n < 0 ? EXIT_FAILURE : EXIT_SUCCESS);
    }
    else
    {
        /* Parent process */

        /* Close unused read end */
        close(pipefd[0]);

        printf("[PARENT] Pipe connection established!\n");
        printf("[PARENT] Enter text (empty line or Ctrl+D to exit):\n");
        fflush(stdout);

        /* Read text from keyboard */
        while (1)
        {
            size_t len;
            size_t sent = 0;

            printf("> ");
            fflush(stdout);

            if (fgets(buffer, sizeof(buffer), stdin) == NULL)
                break;

            /* Stop on empty line */
            if (strcmp(buffer, "\n") == 0)
            {
                printf("[PARENT] Empty line. Stopping input.\n");
                break;
            }

            len = strlen(buffer);

            /* Write text to pipe */
            while (sent < len)
            {
                n = write(pipefd[1], buffer + sent, len - sent);

                if (n < 0 && errno == EINTR)
                    continue;

                if (n <= 0)
                {
                    perror("write");
                    close(pipefd[1]);
                    waitpid(pid, NULL, 0);
                    return 1;
                }

                sent += (size_t)n;
            }
        }

        /* Close write end to send EOF */
        close(pipefd[1]);

        /* Wait for child process */
        if (waitpid(pid, NULL, 0) == -1)
        {
            perror("waitpid");
            return 1;
        }

        printf("[PARENT] Child finished. Parent exiting.\n");
    }

    return 0;
}
