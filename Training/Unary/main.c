#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

static char *str_to_binary(char *str)
{
    if (str == NULL)
        return (NULL);
    size_t len = strlen(str);
    char *binary = calloc(sizeof(char), (len + 1));

    for (int i = 0; i < len; i++)
    {
        char c = str[i];
        for (int j = 6; j >= 0; j--)
        {
            if (c & (1 << j))
                strcat(binary, "1");
            else
                strcat(binary, "0");
        }
    }
    return (binary);
}

static size_t nb_char(char *str, char c)
{
    size_t nb_c = 0;
    for (size_t i = 0; str[i]; i++)
    {
        if (str[i] == c)
            nb_c++;
        else if (str[i] != c)
            break;
    }
    return (nb_c);
}

static void print_nb_char(size_t nb, char c)
{
    for (size_t i = 0; i < nb; i++)
        printf("%c", c);
}

int main()
{
    char MESSAGE[101];
    scanf("%[^\n]", MESSAGE);

    char *binary = str_to_binary(MESSAGE);

    for (size_t i = 0; binary[i]; i++)
    {
        size_t nb_c = 0;

        if (binary[i] == '1')
        {
            nb_c = nb_char(binary + i, '1');
            printf("%s ", "0");
            print_nb_char(nb_c, '0');
            i += nb_c - 1;
        }
        else if (binary[i] == '0')
        {
            nb_c = nb_char(binary + i, '0');
            printf("%s ", "00");
            print_nb_char(nb_c, '0');
            i += nb_c - 1;
        }
        if (binary[i + 1])
            printf(" ");
    }
    return (0);
}