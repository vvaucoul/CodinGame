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
    // Number of elements which make up the association table.
    int N;
    scanf("%d", &N);
    // Number Q of file names to be analyzed.
    int Q;
    scanf("%d", &Q);

    char A_FNAME[4096][257];
    char A_EXT[4096][11];
    char A_MT[4096][51];

    for (int i = 0; i < N; i++)
    {
        // file extension
        char EXT[11];
        // MIME type.
        char MT[51];
        scanf("%s%s", EXT, MT);
        fgetc(stdin);
        fprintf(stderr, "EXT : %s MT : %s\n", EXT, MT);
        strcpy(A_EXT[i], EXT);
        A_EXT[i + 1][0] = 0;
        strcpy(A_MT[i], MT);
        A_MT[i + 1][0] = 0;
    }
    for (int i = 0; i < Q; i++)
    {
        // One file name per line.
        char FNAME[257];
        scanf("%[^\n]", FNAME);
        fgetc(stdin);
        fprintf(stderr, "FNAME : %s\n", FNAME);
        strcpy(A_FNAME[i], FNAME);
        A_FNAME[i + 1][0] = 0;
    }

    for (size_t i = 0; A_FNAME[i]; i++)
    {
    }

    printf("UNKNOWN\n");

    return 0;
}