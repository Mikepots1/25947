
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

volatile sig_atomic_t beep_count = 0;
volatile sig_atomic_t quit_flag = 0;

/* SIGINT handler */
void sigint_handler(int sig)
{
    (void)sig;

    write(STDOUT_FILENO, "\a", 1);
    beep_count++;
}

/* SIGQUIT handler */
void sigquit_handler(int sig)
{
    (void)sig;

    quit_flag = 1;
}

int main()
{
    struct sigaction sa_int;
    struct sigaction sa_quit;
    sigset_t block_mask;
    sigset_t old_mask;

    /* Configure SIGINT handler */
    memset(&sa_int, 0, sizeof(sa_int));
    sa_int.sa_handler = sigint_handler;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = SA_RESTART;

    if (sigaction(SIGINT, &sa_int, NULL) == -1)
    {
        perror("sigaction SIGINT");
        return 1;
    }

    /* Configure SIGQUIT handler */
    memset(&sa_quit, 0, sizeof(sa_quit));
    sa_quit.sa_handler = sigquit_handler;
    sigemptyset(&sa_quit.sa_mask);
    sa_quit.sa_flags = 0;

    if (sigaction(SIGQUIT, &sa_quit, NULL) == -1)
    {
        perror("sigaction SIGQUIT");
        return 1;
    }

    /* Block SIGQUIT before waiting */
    sigemptyset(&block_mask);
    sigaddset(&block_mask, SIGQUIT);

    if (sigprocmask(SIG_BLOCK, &block_mask, &old_mask) == -1)
    {
        perror("sigprocmask");
        return 1;
    }

    printf("Beeper started!\n");
    printf("Press Ctrl+C to beep\n");
    printf("Press Ctrl+\\ to quit\n");
    printf("Waiting for signals...\n");
    fflush(stdout);

    /* Main loop */
    while (!quit_flag)
    {
        sigsuspend(&old_mask);
    }

    /* Restore signal mask */
    sigprocmask(SIG_SETMASK, &old_mask, NULL);

    printf("\nTotal beeps: %d\n", (int)beep_count);
    printf("Program terminated by SIGQUIT\n");

    return 0;
}
