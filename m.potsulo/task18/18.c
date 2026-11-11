
#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>
#include <libgen.h>

/* Get file type and permissions */
void get_permissions(mode_t mode, char *permissions)
{
    permissions[0] = S_ISDIR(mode) ? 'd' :
                     S_ISREG(mode) ? '-' : '?';

    permissions[1] = (mode & S_IRUSR) ? 'r' : '-';
    permissions[2] = (mode & S_IWUSR) ? 'w' : '-';
    permissions[3] = (mode & S_IXUSR) ? 'x' : '-';

    permissions[4] = (mode & S_IRGRP) ? 'r' : '-';
    permissions[5] = (mode & S_IWGRP) ? 'w' : '-';
    permissions[6] = (mode & S_IXGRP) ? 'x' : '-';

    permissions[7] = (mode & S_IROTH) ? 'r' : '-';
    permissions[8] = (mode & S_IWOTH) ? 'w' : '-';
    permissions[9] = (mode & S_IXOTH) ? 'x' : '-';

    permissions[10] = '\0';
}

int main(int argc, char *argv[])
{
    struct stat st;
    struct passwd *pw;
    struct group *gr;

    char permissions[11];
    char date[32];
    char size[32];

    char *filename;
    char *path_copy;

    time_t now;
    int i;

    /* Check arguments */
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <file> [files...]\n", argv[0]);
        return 1;
    }

    now = time(NULL);

    /* Process each argument */
    for (i = 1; i < argc; i++)
    {
        /* Get file information */
        if (lstat(argv[i], &st) == -1)
        {
            perror(argv[i]);
            continue;
        }

        /* Get file permissions */
        get_permissions(st.st_mode, permissions);

        /* Get owner and group */
        pw = getpwuid(st.st_uid);
        gr = getgrgid(st.st_gid);

        /* Get file size */
        if (S_ISREG(st.st_mode))
            snprintf(size, sizeof(size), "%lld",
                     (long long)st.st_size);
        else
            size[0] = '\0';

        /* Format modification date */
        if (localtime(&st.st_mtime) == NULL)
        {
            strcpy(date, "?");
        }
        else if (st.st_mtime > now ||
                 difftime(now, st.st_mtime) > 15552000)
        {
            strftime(date, sizeof(date),
                     "%b %e %Y", localtime(&st.st_mtime));
        }
        else
        {
            strftime(date, sizeof(date),
                     "%b %e %H:%M", localtime(&st.st_mtime));
        }

        /* Get filename without path */
        path_copy = strdup(argv[i]);

        if (path_copy == NULL)
        {
            perror("strdup");
            continue;
        }

        filename = basename(path_copy);

        /* Print file information */
        printf("%-10s %4lu %-10s %-10s %10s %-12s %s\n",
               permissions,
               (unsigned long)st.st_nlink,
               pw ? pw->pw_name : "?",
               gr ? gr->gr_name : "?",
               size,
               date,
               filename);

        free(path_copy);
    }

    return 0;
}
