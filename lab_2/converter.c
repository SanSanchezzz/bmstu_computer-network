#include "converter.h"

bool is_number(const char buf[BUF_SIZE])
{
    for (int i = 0; buf[i] != '\0'; i++)
    {
        if ((buf[i] < '0' || buf[i] > '9') && buf[i] != '-')
        {
            return false;
        }
    }

    return true;
}

char symbol(int number)
{
    if (number <= 9)
    {
        return '0' + number;
    }
    else
    {
        return 'a' - 10 + number;
    }
}

void convert_number_system(const char number[BUF_SIZE], int basis, char result[BUF_SIZE])
{
    int num = atoi(number);
    int idx = 0;
    bool negative = false;

    if (num == 0)
    {
        result[0] = '0';
        result[1] = '\0';

        return;
    }
    if (num < 0)
    {
        negative = true;
        num *= -1;
    }

    while (num)
    {
        result[idx++] = symbol(num % basis);
        num /= basis;
    }

    if (negative)
    {
        result[idx++] = '-';
    }

    result[idx] = '\0';

    for (int i = 0; i < idx / 2; ++i)
    {
        char tmp = result[i];
        result[i] = result[idx - i - 1];
        result[idx - i - 1] = tmp;
    }
}

int converter(char buf[BUF_SIZE])
{
    char result[BUF_SIZE];

    if (!is_number(buf))
    {
        return ERROR;
    }

    printf("Getted number - %s\n", buf);

    convert_number_system(buf, 2, result);
    printf("2: %s\n", result);

    convert_number_system(buf, 8, result);
    printf("8: %s\n", result);

    convert_number_system(buf, 12, result);
    printf("12: %s\n", result);

    convert_number_system(buf, 16, result);
    printf("16: %s\n", result);

    return 0;
}

