#define _XOPEN_SOURCE 600
#define __EXTENSIONS__

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void)
{
    time_t now;
    struct tm *sp;

    /* 1. Установка часового пояса Калифорнии (Лос-Анджелес) */
    if (putenv("TZ=America/Los_Angeles") != 0)
    {
        perror("putenv");
        return 1;
    }

    /* 2. Инициализация системной информации о часовом поясе */
    tzset();

    /* 3. Получение текущего времени в формате UTC (секунды с 1970 года) */
    if (time(&now) == (time_t)-1)
    {
        perror("time");
        return 1;
    }

    /* 4. Преобразование UTC-времени в локальную структуру времени */
    sp = localtime(&now);
    if (sp == NULL)
    {
        perror("localtime");
        return 1;
    }

    /* 
     * 5. Форматированный вывод
     * tm_mon начинается с 0 (0 - январь, 11 - декабрь), поэтому +1
     * tm_year отсчитывает годы с 1900, поэтому +1900
     * tzname[sp->tm_isdst] выводит "PST" или "PDT" в зависимости от летнего времени
     */
    printf("%02d/%02d/%04d %02d:%02d %s\n",
           sp->tm_mon + 1,
           sp->tm_mday,
           sp->tm_year + 1900,
           sp->tm_hour,
           sp->tm_min,
           tzname[sp->tm_isdst]);

    return 0;
}