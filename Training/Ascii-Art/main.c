#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

char _AROWS[4096][1025] = {0};
int L;
int H;

static char *add_letter(char *line, size_t letter, size_t y)
{
    size_t len = strlen(line);

    for (size_t i = 0; i < L; i++)
    {
        size_t j = i - (L * letter);
        line[len + i] = _AROWS[y][letter + i];
        line[len + i + 1] = '\0';
    }
    return (line);
}

int main()
{
    scanf("%d", &L);
    scanf("%d", &H);
    fgetc(stdin);

    char T[257];
    scanf("%[^\n]", T);
    fgetc(stdin);
    for (int i = 0; i < H; i++)
    {
        char ROW[1025];
        scanf("%[^\n]", ROW);
        fgetc(stdin);
        strcpy(_AROWS[i], ROW);
    }
    for (size_t y = 0; y < H; y++)
    {
        char *line = malloc(sizeof(char) * 1025);
        memset(line, 0, 1025);
        for (size_t x = 0; x < strlen(T); x++)
        {
            T[x] = toupper(T[x]);
            size_t nLetter = T[x] - 'A';
            if (isalpha(T[x]) == false)
                nLetter = 26;
            size_t start_letter_x = L * nLetter;
            size_t end_letter_x = start_letter_x + L;
            line = add_letter(line, start_letter_x, y);
        }
        printf("%s\n", line);
        free(line);
    }
    return (0);
}