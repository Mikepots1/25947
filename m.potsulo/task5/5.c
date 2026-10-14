#define _XOPEN_SOURCE 600
#define __EXTENSIONS__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

/* Структура для хранения смещения и длины строки */
typedef struct
{
    long offset; /* Смещение от начала файла в байтах */
    int length;  /* Длина строки в байтах (включая '\n') */
} LineInfo;

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* 1. Открытие файла через низкоуровневый системный вызов open() */
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1)
    {
        perror("open error");
        return EXIT_FAILURE;
    }

    /* Инициализация динамического массива таблиц */
    size_t capacity = 10;
    size_t num_lines = 0;
    LineInfo *table = (LineInfo *)malloc(capacity * sizeof(LineInfo));
    if (table == NULL)
    {
        perror("malloc table error");
        close(fd);
        return EXIT_FAILURE;
    }

    char ch;
    long line_start_offset = 0;
    int current_line_len = 0;

    /* 2. Построение таблицы смещений и длин строк (первый проход) */
    while (read(fd, &ch, 1) > 0)
    {
        current_line_len++;

        if (ch == '\n')
        {
            /* Расширяем динамический массив при необходимости */
            if (num_lines >= capacity)
            {
                capacity *= 2;
                LineInfo *temp = (LineInfo *)realloc(table, capacity * sizeof(LineInfo));
                if (temp == NULL)
                {
                    perror("realloc table error");
                    free(table);
                    close(fd);
                    return EXIT_FAILURE;
                }
                table = temp;
            }

            table[num_lines].offset = line_start_offset;
            table[num_lines].length = current_line_len;
            num_lines++;

            /* Подготовка к следующей строке */
            line_start_offset = lseek(fd, 0L, SEEK_CUR);
            current_line_len = 0;
        }
    }

    /* Если файл не заканчивается символом '\n', сохраняем последнюю строку */
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

    /* 3. Отладочная печать построенной таблицы */
    printf("--- Debug: Line Table (%size_t lines) ---\n", num_lines);
    for (size_t i = 0; i < num_lines; i++)
    {
        printf("Line %2zu: Offset = %6ld, Length = %3d\n",
               i + 1, table[i].offset, table[i].length);
    }
    printf("-----------------------------------------\n\n");

    /* 4. Интерактивный цикл чтения по номеру строки */
    int line_num;
    while (1)
    {
        printf("Enter line number (1..%zu, 0 to quit): ", num_lines);
        if (scanf("%d", &line_num) != 1)
        {
            break;
        }

        if (line_num == 0)
        {
            break;
        }

        if (line_num < 1 || (size_t)line_num > num_lines)
        {
            printf("Invalid line number. Please enter a value between 1 and %zu.\n", num_lines);
            continue;
        }

        /* Получаем метаданные строки (переводим нумерацию 1..N в индекс 0..N-1) */
        long offset = table[line_num - 1].offset;
        int length = table[line_num - 1].length;

        /* Позиционируемся на начало строки */
        if (lseek(fd, offset, SEEK_SET) == -1)
        {
            perror("lseek error");
            continue;
        }

        /* Выделяем буфер под строку +1 для завершающего '\0' */
        char *buffer = (char *)malloc(length + 1);
        if (buffer == NULL)
        {
            perror("malloc buffer error");
            continue;
        }

        /* Читаем точную длину строки */
        ssize_t bytes_read = read(fd, buffer, length);
        if (bytes_read > 0)
        {
            buffer[bytes_read] = '\0'; /* Обязательно делаем C-строку */
            printf("Line %d: %s", line_num, buffer);
            /* Если строка не оканчивалась \n, добавим перенос вручную */
            if (buffer[bytes_read - 1] != '\n')
            {
                printf("\n");
            }
        }
        else
        {
            perror("read line error");
        }

        free(buffer);
    }

    /* 5. Освобождение ресурсов */
    free(table);
    close(fd);

    return EXIT_SUCCESS;
}