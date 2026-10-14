#define _XOPEN_SOURCE 600
#define __EXTENSIONS__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>

/* Глобальные переменные для доступа из обработчика сигнала */
static int g_fd = -1;

/* Структура для хранения информации о строках */
typedef struct
{
    long offset;
    int length;
} LineInfo;

/* 1. Обработчик сигнала SIGALRM (вызывается при истечении 5 секунд) */
void alarm_handler(int sig)
{
    (void)sig; /* Подавляем предупреждение о неупотребленном параметре */

    const char msg[] = "\n\n[TIMEOUT] Time is up (5 sec)! Printing full file content:\n--------------------------------------------------\n";
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);

    if (g_fd != -1)
    {
        /* Перемещаемся в начало файла */
        lseek(g_fd, 0L, SEEK_SET);

        char buffer[1024];
        ssize_t bytes_read;

        /* Читаем и выводим весь файл целиком на stdout */
        while ((bytes_read = read(g_fd, buffer, sizeof(buffer))) > 0)
        {
            write(STDOUT_FILENO, buffer, bytes_read);
        }
    }

    const char end_msg[] = "\n--------------------------------------------------\nExiting program.\n";
    write(STDOUT_FILENO, end_msg, sizeof(end_msg) - 1);

    /* Асинхронно-безопасное завершение процесса */
    _exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* Открытие файла */
    g_fd = open(argv[1], O_RDONLY);
    if (g_fd == -1)
    {
        perror("open error");
        return EXIT_FAILURE;
    }

    /* 2. Регистрируем обработчик сигнала SIGALRM */
    signal(SIGALRM, alarm_handler);

    /* Построение таблицы смещений и длин строк */
    size_t capacity = 10;
    size_t num_lines = 0;
    LineInfo *table = (LineInfo *)malloc(capacity * sizeof(LineInfo));
    if (table == NULL)
    {
        perror("malloc table error");
        close(g_fd);
        return EXIT_FAILURE;
    }

    char ch;
    long line_start_offset = 0;
    int current_line_len = 0;

    while (read(g_fd, &ch, 1) > 0)
    {
        current_line_len++;

        if (ch == '\n')
        {
            if (num_lines >= capacity)
            {
                capacity *= 2;
                LineInfo *temp = (LineInfo *)realloc(table, capacity * sizeof(LineInfo));
                if (temp == NULL)
                {
                    perror("realloc table error");
                    free(table);
                    close(g_fd);
                    return EXIT_FAILURE;
                }
                table = temp;
            }

            table[num_lines].offset = line_start_offset;
            table[num_lines].length = current_line_len;
            num_lines++;

            line_start_offset = lseek(g_fd, 0L, SEEK_CUR);
            current_line_len = 0;
        }
    }

    if (current_line_len > 0)
    {
        if (num_lines >= capacity)
        {
            capacity++;
            LineInfo *temp = (LineInfo *)realloc(table, capacity * sizeof(LineInfo));
            if (temp != NULL)
            {
                table = temp;
            }
        }
        if (num_lines < capacity)
        {
            table[num_lines].offset = line_start_offset;
            table[num_lines].length = current_line_len;
            num_lines++;
        }
    }

    /* Вывод отладочной таблицы */
    printf("--- Debug: Line Table (%size_t lines) ---\n", num_lines);
    for (size_t i = 0; i < num_lines; i++)
    {
        printf("Line %2zu: Offset = %6ld, Length = %3d\n",
               i + 1, table[i].offset, table[i].length);
    }
    printf("-----------------------------------------\n\n");

    /* 3. Интерактивный цикл с таймером */
    int line_num;
    while (1)
    {
        printf("Enter line number (1..%zu, 0 to quit) [5 sec timeout]: ", num_lines);
        fflush(stdout); /* Обязательно скидываем буфер вывода */

        /* Запускаем таймер на 5 секунд перед вводом */
        alarm(5);

        int scanf_res = scanf("%d", &line_num);

        /* Успели ввести значение — немедленно сбрасываем таймер! */
        alarm(0);

        if (scanf_res != 1 || line_num == 0)
        {
            break;
        }

        if (line_num < 1 || (size_t)line_num > num_lines)
        {
            printf("Invalid line number. Range: 1..%zu\n", num_lines);
            continue;
        }

        /* Чтение и вывод выбранной строки */
        long offset = table[line_num - 1].offset;
        int length = table[line_num - 1].length;

        if (lseek(g_fd, offset, SEEK_SET) == -1)
        {
            perror("lseek error");
            continue;
        }

        char *buffer = (char *)malloc(length + 1);
        if (buffer == NULL)
        {
            perror("malloc buffer error");
            continue;
        }

        ssize_t bytes_read = read(g_fd, buffer, length);
        if (bytes_read > 0)
        {
            buffer[bytes_read] = '\0';
            printf("Line %d: %s", line_num, buffer);
            if (buffer[bytes_read - 1] != '\n')
            {
                printf("\n");
            }
        }

        free(buffer);
    }

    /* Освобождаем ресурсы при нормальном завершении (по вводу 0) */
    free(table);
    close(g_fd);

    return EXIT_SUCCESS;
}