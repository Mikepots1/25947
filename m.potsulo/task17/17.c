
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>
#include <signal.h>

#define MAX_LEN 4096
#define WIDTH 40

struct termios original_term;
char buffer[MAX_LEN];
int pos = 0;
int lines = 1;
int erase_char;
int kill_char;

/* Restore terminal settings */
void disable_raw_mode()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_term);
}

/* Enable raw mode */
void enable_raw_mode()
{
    struct termios raw;

    if (tcgetattr(STDIN_FILENO, &original_term) == -1)
    {
        perror("tcgetattr");
        exit(1);
    }

    atexit(disable_raw_mode);

    erase_char = original_term.c_cc[VERASE];
    kill_char = original_term.c_cc[VKILL];

    raw = original_term;
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    {
        perror("tcsetattr");
        exit(1);
    }
}

/* Redraw current line with word wrapping */
void redraw()
{
    int i = 0;
    int col = 0;
    int new_lines = 1;
    int word_len;
    int j;

    /* Move to the beginning of the edited area */
    printf("\r");

    for (j = 1; j < lines; j++)
        printf("\033[A\r");

    /* Clear all previously displayed lines */
    for (j = 0; j < lines; j++)
    {
        printf("\033[2K");

        if (j < lines - 1)
            printf("\033[B\r");
    }

    for (j = 1; j < lines; j++)
        printf("\033[A\r");

    /* Print buffer with word wrapping */
    while (i < pos)
    {
        if (buffer[i] == ' ')
        {
            if (col >= WIDTH)
            {
                printf("\n");
                col = 0;
                new_lines++;
            }

            putchar(' ');
            col++;
            i++;
        }
        else
        {
            word_len = 0;

            while (i + word_len < pos &&
                   buffer[i + word_len] != ' ')
                word_len++;

            if (col > 0 && col + word_len > WIDTH)
            {
                printf("\n");
                col = 0;
                new_lines++;
            }

            for (j = 0; j < word_len; j++)
            {
                if (col >= WIDTH)
                {
                    printf("\n");
                    col = 0;
                    new_lines++;
                }

                putchar(buffer[i + j]);
                col++;
            }

            i += word_len;
        }
    }

    lines = new_lines;
    fflush(stdout);
}

/* Main program */
int main()
{
    unsigned char c;
    ssize_t n;

    enable_raw_mode();

    printf("> ");
    fflush(stdout);

    while (1)
    {
        n = read(STDIN_FILENO, &c, 1);

        if (n == 0)
            break;

        if (n < 0)
        {
            perror("read");
            break;
        }

        /* Ctrl+D: exit only at beginning of line */
        if (c == 4)
        {
            if (pos == 0)
                break;

            putchar('\a');
            fflush(stdout);
        }

        /* ERASE: delete last character */
        else if (c == erase_char || c == 127 || c == 8)
        {
            if (pos > 0)
            {
                pos--;
                buffer[pos] = '\0';
                redraw();
            }
            else
            {
                putchar('\a');
                fflush(stdout);
            }
        }

        /* KILL: delete entire current line */
        else if (c == kill_char)
        {
            pos = 0;
            buffer[0] = '\0';
            redraw();
        }

        /* Ctrl+W: delete last word and spaces */
        else if (c == 23)
        {
            while (pos > 0 && buffer[pos - 1] == ' ')
                pos--;

            while (pos > 0 && buffer[pos - 1] != ' ')
                pos--;

            buffer[pos] = '\0';
            redraw();
        }

        /* Enter: start a new line */
        else if (c == '\n' || c == '\r')
        {
            printf("\n> ");
            fflush(stdout);

            pos = 0;
            buffer[0] = '\0';
            lines = 1;
        }

        /* Printable characters */
        else if (isprint(c))
        {
            if (pos < MAX_LEN - 1)
            {
                buffer[pos++] = c;
                buffer[pos] = '\0';
                redraw();
            }
            else
            {
                putchar('\a');
                fflush(stdout);
            }
        }

        /* Other control characters: bell */
        else
        {
            putchar('\a');
            fflush(stdout);
        }
    }

    printf("\nExiting...\n");

    return 0;
}
