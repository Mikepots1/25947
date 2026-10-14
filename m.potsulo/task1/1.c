#define _XOPEN_SOURCE 600
#define __EXTENSIONS__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <limits.h>
#include <string.h>
#include <errno.h>

extern char **environ;

struct Option
{
    int option;
    char *arg;
    int bad_option;
};

int main(int argc, char *argv[])
{
    char *options = "ispuU:cC:dvV:";
    int c;
    int count = 0;
    int i;
    struct Option *saved_options = NULL;
    struct Option *temp;

    opterr = 0;

    /* Parse options and save them */
    while ((c = getopt(argc, argv, options)) != -1)
    {
        temp = realloc(saved_options, (count + 1) * sizeof(struct Option));

        if (temp == NULL)
        {
            perror("realloc");
            free(saved_options);
            return 1;
        }

        saved_options = temp;
        saved_options[count].option = c;
        saved_options[count].arg = optarg;
        saved_options[count].bad_option = optopt;
        count++;
    }

    if (optind < argc)
    {
        for (i = optind; i < argc; i++)
        {
            fprintf(stderr, "Non-option argument: %s\n", argv[i]);
        }
    }

    /* Execute options from right to left */
    for (i = count - 1; i >= 0; i--)
    {
        c = saved_options[i].option;

        switch (c)
        {
            case 'i':
                printf("UID: %ld, EUID: %ld, GID: %ld, EGID: %ld\n",
                       (long)getuid(), (long)geteuid(),
                       (long)getgid(), (long)getegid());
                break;

            case 's':
                if (setpgid(0, 0) == -1)
                    perror("setpgid");
                else
                    printf("Process became group leader\n");
                break;

            case 'p':
                printf("PID: %ld, PPID: %ld, PGID: %ld\n",
                       (long)getpid(), (long)getppid(), (long)getpgrp());
                break;

            case 'u':
            {
                struct rlimit limit;

                if (getrlimit(RLIMIT_NOFILE, &limit) == -1)
                    perror("getrlimit");
                else if (limit.rlim_cur == RLIM_INFINITY)
                    printf("ulimit: unlimited\n");
                else
                    printf("ulimit: %ld\n", (long)limit.rlim_cur);
                break;
            }

            case 'U':
            {
                struct rlimit limit;
                long new_val;

                if (saved_options[i].arg == NULL)
                {
                    fprintf(stderr, "Missing argument for -U\n");
                    break;
                }

                new_val = atol(saved_options[i].arg);

                if (getrlimit(RLIMIT_NOFILE, &limit) == -1)
                {
                    perror("getrlimit");
                    break;
                }

                limit.rlim_cur = (rlim_t)new_val;

                if (setrlimit(RLIMIT_NOFILE, &limit) == -1)
                    perror("setrlimit (invalid U limit)");
                else
                    printf("ulimit changed to %ld\n", new_val);

                break;
            }

            case 'c':
            {
                struct rlimit limit;

                if (getrlimit(RLIMIT_CORE, &limit) == -1)
                    perror("getrlimit");
                else if (limit.rlim_cur == RLIM_INFINITY)
                    printf("Core size: unlimited\n");
                else
                    printf("Core size: %ld bytes\n", (long)limit.rlim_cur);
                break;
            }

            case 'C':
            {
                struct rlimit limit;
                long new_val;

                if (saved_options[i].arg == NULL)
                {
                    fprintf(stderr, "Missing argument for -C\n");
                    break;
                }

                new_val = atol(saved_options[i].arg);

                if (getrlimit(RLIMIT_CORE, &limit) == -1)
                {
                    perror("getrlimit");
                    break;
                }

                limit.rlim_cur = (rlim_t)new_val;

                if (setrlimit(RLIMIT_CORE, &limit) == -1)
                    perror("setrlimit (invalid C limit)");
                else
                    printf("Core size changed to %ld bytes\n", new_val);

                break;
            }

            case 'd':
            {
                char cwd[PATH_MAX];

                if (getcwd(cwd, sizeof(cwd)) == NULL)
                    perror("getcwd");
                else
                    printf("Current directory: %s\n", cwd);
                break;
            }

            case 'v':
            {
                char **envp;

                for (envp = environ; *envp != NULL; envp++)
                    printf("%s\n", *envp);

                break;
            }

            case 'V':
            {
                if (saved_options[i].arg == NULL || strchr(saved_options[i].arg, '=') == NULL)
                {
                    fprintf(stderr, "Use -Vname=value\n");
                    break;
                }

                /* putenv требует сохранения выделенной памяти в течение всей жизни процесса */
                char *env_str = strdup(saved_options[i].arg);
                if (env_str == NULL)
                {
                    perror("strdup");
                    break;
                }

                if (putenv(env_str) != 0)
                {
                    perror("putenv");
                    free(env_str);
                }
                else
                {
                    printf("Set env: %s\n", env_str);
                }

                break;
            }

            case '?':
                if (saved_options[i].bad_option != 0)
                    fprintf(stderr, "Invalid option or missing argument: -%c\n",
                            saved_options[i].bad_option);
                else
                    fprintf(stderr, "Invalid option\n");
                break;
        }
    }

    free(saved_options);

    return 0;
}