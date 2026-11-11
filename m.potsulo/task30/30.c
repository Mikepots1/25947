
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "./socket"

/* Send all bytes */
int write_all(int fd, const char *buffer, size_t len)
{
    size_t sent = 0;
    ssize_t n;

    while (sent < len)
    {
        n = write(fd, buffer + sent, len - sent);

        if (n < 0 && errno == EINTR)
            continue;

        if (n <= 0)
            return -1;

        sent += (size_t)n;
    }

    return 0;
}

int main()
{
    int sock_fd;
    struct sockaddr_un addr;
    int i;

    const char *messages[] = {
        "Hello World!\n",
        "This is a Test Message!!!\n",
        "Unix domain sockets are awesome.\n"
    };

    /* Ignore SIGPIPE */
    signal(SIGPIPE, SIG_IGN);

    /* Create socket */
    sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (sock_fd == -1)
    {
        perror("socket");
        return 1;
    }

    /* Set server address */
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCKET_PATH);

    /* Connect to server */
    if (connect(sock_fd, (struct sockaddr *)&addr,
                sizeof(addr)) == -1)
    {
        perror("connect");
        close(sock_fd);
        return 1;
    }

    printf("[CLIENT] Connected to server!\n");

    /* Send messages */
    for (i = 0; i < 3; i++)
    {
        printf("[CLIENT] Sending: %s", messages[i]);

        if (write_all(sock_fd, messages[i],
                      strlen(messages[i])) == -1)
        {
            perror("write");
            close(sock_fd);
            return 1;
        }
    }

    /* Close connection */
    close(sock_fd);

    printf("[CLIENT] All messages sent. Connection closed.\n");

    return 0;
}
