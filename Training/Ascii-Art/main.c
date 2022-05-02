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
    scanf("%d", &L);
    int H;
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
        fprintf(stderr, "ROW[%d] = %s\n", i, ROW);
    }

    // Write an answer using printf(). DON'T FORGET THE TRAILING \n
    // To debug: fprintf(stderr, "Debug messages...\n");

    printf("answer\n");

    return 0;
}