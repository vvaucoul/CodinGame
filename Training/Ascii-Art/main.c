#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/

int main()
{
    int L;
    int H;
    scanf("%d", &L);
    scanf("%d", &H);
    fgetc(stdin);

    char T[257];
    scanf("%[^\n]", T);

    fprintf(stderr, "T = %s\n", T);
    fgetc(stdin);

    char AROWS[5][1025];
    for (int i = 0; i < H; i++)
    {
        char ROW[1025];
        scanf("%[^\n]", ROW);
        fgetc(stdin);
        strcpy(AROWS[i], ROW);
        AROWS[i][L * 26] = '\0';
    }
    for (size_t i = 0; i < 5; i++)
        fprintf(stderr, "AROWS[%d] = %s\n", i, AROWS[i]);
    for (size_t i = 0; T[i]; i++)
    {
        size_t nLetter = T[i] - 'A';
        fprintf(stderr, "NLetter = %zu\n", nLetter);
        size_t start_letter_x = L * nLetter;
        size_t end_letter_x = start_letter_x + L;

        for (size_t j = 0; j < H; j++)
        {
            char line[1025];
            size_t start_letter_y = j;
            size_t end_letter_y = H;

            fprintf(stderr, " end - start = %ld\n", end_letter_x - start_letter_x);
            strncpy(line, AROWS[start_letter_y] + start_letter_x, end_letter_x - start_letter_x);
            line[end_letter_x - start_letter_x] = '\0';
            fprintf(stderr, "line = %s\n", line);
            printf("%s\n", line);
        }
    }

    return (0);
}