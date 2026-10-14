#define _XOPEN_SOURCE 600
#define __EXTENSIONS__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>

/* Глобальные переменные для доступа из обработчика сигнала */
static char *g_file_map = NULL;
static size_t g_file_size = 0;

/* Структура для хранения информации о строках */
typedef struct
{
    size_t offset;
    size_t length;
} LineInfo;

/* 1. Обработчик сигнала SIGALRM (вызывается при истечении 5 секунд) */
void alarm_handler(int sig)
{
    (void)sig;

    const char msg[] = "\n\n[TIMEOUT] Time is up (5 sec)! Printing full file content:\n--------------------------------------------------\n";
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);

    if (g_file_map != NULL && g_file_size > 0)
    {
        /* Печать всего содержимого отображенного файла за один вызов */
        write(STDOUT_FILENO, g_file_map, g_file_size);
    }

    const char end_msg[] = "\n--------------------------------------------------\nExiting program.\n";
    write(STDOUT_FILENO, end_msg, sizeof(end_msg) - 1);

    /* Завершение процесса */
    _exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* 1. Открытие файла */
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1)
    {
        perror("open error");
        return EXIT_FAILURE;
    }

    /* 2. Получение размера файла через fstat() */
    struct stat st;
    if (fstat(fd, &st) == -1)
    {
        perror("fstat error");
        close(fd);
        return EXIT_FAILURE;
    }

    if (st.st_size == 0)
    {
        printf("File is empty.\n");
        close(fd);
        return EXIT_SUCCESS;
    }

    g_file_size = (size_t)st.st_size;

    /* 3. Отображение файла в память через mmap() */
    g_file_map = (char *)mmap(NULL, g_file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (g_file_map == MAP_FAILED)
    {
        perror("mmap error");
        close(fd);
        return EXIT_FAILURE;
    }

    /* Дескриптор файла больше не нужен — отображение сохраняется в памяти */
    close(fd);

    /* 4. Регистрация обработчика сигнала */
    signal(SIGALRM, alarm_handler);

    /* 5. Построение таблицы смещений и длин строк по массиву в памяти */
    size_t capacity = 10;
    size_t num_lines = 0;
    LineInfo *table = (LineInfo *)malloc(capacity * sizeof(LineInfo));
    if (table == NULL)
    {
        perror("malloc table error");
        munmap(g_file_map, g_file_size);
        return EXIT_FAILURE;
    }

    size_t line_start_offset = 0;
    size_t current_line_len = 0;

    for (size_t i = 0; i < g_file_size; i++)
    {
        current_line_len++;

        if (g_file_map[i] == '\n')
        {
            if (num_lines >= capacity)
            {
                capacity *= 2;
                LineInfo *temp = (LineInfo *)realloc(table, capacity * sizeof(LineInfo));
                if (temp == NULL)
                {
                    perror("realloc table error");
                    free(table);
                    munmap(g_file_map, g_file_size);
                    return EXIT_FAILURE;
                }
                table = temp;
            }

            table[num_lines].offset = line_start_offset;
            table[num_lines].length = current_line_len;
            num_lines++;

            line_start_offset = i + 1;
            current_line_len = 0;
        }
    }

    /* Обработка последней строки без перевода '\n' на конце */
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
    printf("--- Debug: Line Table (%zu lines) ---\n", num_lines);
    for (size_t i = 0; i < num_lines; i++)
    {
        printf("Line %2zu: Offset = %6zu, Length = %3zu\n",
               i + 1, table[i].offset, table[i].length);
    }
    printf("-----------------------------------------\n\n");

    /* 6. Интерактивный цикл с таймером */
    int line_num;
    while (1)
    {
        printf("Enter line number (1..%zu, 0 to quit) [5 sec timeout]: ", num_lines);
        fflush(stdout);

        alarm(5);
        int scanf_res = scanf("%d", &line_num);
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

        /* Вывод строки прямо из памяти с помощью fwrite */
        size_t offset = table[line_num - 1].offset;
        size_t length = table[line_num - 1].length;

        printf("Line %d: ", line_num);
        fwrite(g_file_map + offset, 1, length, stdout);
        if (g_file_map[offset + length - 1] != '\n')
        {
            printf("\n");
        }
    }

    /* 7. Очистка ресурсов */
    free(table);
    munmap(g_file_map, g_file_size);

    return EXIT_SUCCESS;
}