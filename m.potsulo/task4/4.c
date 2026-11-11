#define _XOPEN_SOURCE 600
#define __EXTENSIONS__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LEN 1024

/* 1. Определение структуры узла односвязного списка */
struct Node
{
    char *data;         /* Указатель на динамически выделенную строку */
    struct Node *next;  /* Указатель на следующий элемент */
};

/* 2. Добавление строки в конец списка (append) */
void append(struct Node **head, const char *str)
{
    /* Выделение памяти под новый узел */
    struct Node *new_node = (struct Node *)malloc(sizeof(struct Node));
    if (new_node == NULL)
    {
        perror("malloc for Node failed");
        return;
    }

    /* Выделение памяти под копию строки (+1 байт под '\0') */
    new_node->data = (char *)malloc(strlen(str) + 1);
    if (new_node->data == NULL)
    {
        perror("malloc for data failed");
        free(new_node);
        return;
    }

    /* Копирование строки и зануление next */
    strcpy(new_node->data, str);
    new_node->next = NULL;

    /* Если список пуст, новый узел становится головой */
    if (*head == NULL)
    {
        *head = new_node;
    }
    else
    {
        /* Поиск последнего узла */
        struct Node *current = *head;
        while (current->next != NULL)
        {
            current = current->next;
        }
        current->next = new_node;
    }
}

/* 3. Печать всех элементов списка */
void print_list(const struct Node *head)
{
    const struct Node *current = head;
    printf("\n=== Entered Lines ===\n");
    while (current != NULL)
    {
        printf("%s\n", current->data);
        current = current->next;
    }
}


void print_reverse(const struct Node *head)
{
    const struct Node *current = head;
    int count = 0;
    int i = 0;

    while (current != NULL)
    {
        count++;
        current = current->next;
    }

    char **lines = malloc(count * sizeof(char *));
    if (lines == NULL && count > 0)
    {
        perror("malloc failed");
        return;
    }

    current = head;

    while (current != NULL)
    {
        lines[i] = current->data;
        i++;
        current = current->next;
    }

    printf("\n=== Entered Lines ===\n");

    for (i = count - 1; i >= 0; i--)
    {
        printf("%s\n", lines[i]);
    }

    free(lines);
}


/* 4. Освобождение всей динамической памяти списка */
void free_list(struct Node *head)
{
    struct Node *current = head;
    while (current != NULL)
    {
        struct Node *next_node = current->next;
        free(current->data); /* Сначала освобождаем строку */
        free(current);       /* Затем освобождаем сам узел */
        current = next_node;
    }
}

int main(void)
{
    struct Node *head = NULL;
    char buffer[MAX_LINE_LEN];

    printf("Enter lines (start line with '.' to stop):\n");

    while (fgets(buffer, sizeof(buffer), stdin) != NULL)
    {
        /* Удаляем символ перевода строки '\n', если он присутствует */
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n')
        {
            buffer[len - 1] = '\0';
        }

        /* Проверяем условие остановки: первая точка в строке */
        if (buffer[0] == '.')
        {
            break;
        }

        /* Добавляем введенную строку в список */
        append(&head, buffer);
    }

    /* Вывод и очистка */
    print_reverse(head);
    free_list(head);

    return 0;
}