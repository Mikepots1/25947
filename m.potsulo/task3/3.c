#define _XOPEN_SOURCE 600
#define __EXTENSIONS__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

void print_uids(const char *phase_msg)
{
    printf("=== %s ===\n", phase_msg);
    printf("Real UID      (getuid) : %ld\n", (long)getuid());
    printf("Effective UID (geteuid): %ld\n\n", (long)geteuid());
}

void try_open_file(const char *filename)
{
    FILE *f = fopen(filename, "r");
    if (f == NULL)
    {
        perror("fopen error");
    }
    else
    {
        printf("Successfully opened '%s'\n", filename);
        fclose(f);
    }
}

int main(void)
{
    const char *filename = "data.txt";

    /* 1. Вывод начальных UID и первая попытка открыть файл */
    print_uids("1. Initial state (Before setuid)");
    try_open_file(filename);

    printf("\n----------------------------------------\n\n");

    /* 2. Сброс эффективного UID до реального */
    if (setuid(getuid()) == -1)
    {
        perror("setuid error");
        return EXIT_FAILURE;
    }

    /* 3. Вывод UID после сброса и вторая попытка открыть файл */
    print_uids("2. After setuid(getuid())");
    try_open_file(filename);

    return EXIT_SUCCESS;
}