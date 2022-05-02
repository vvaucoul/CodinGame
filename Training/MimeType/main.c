#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/

static char *getExtension(char *filename)
{
    char *dot = strrchr(filename, '.');
    if (!dot || dot == filename)
        return (NULL);
    return (dot + 1);
}

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
        strcpy(A_EXT[i], EXT);
        strcpy(A_MT[i], MT);
    }
    for (int i = 0; i < Q; i++)
    {
        // One file name per line.
        char FNAME[257];
        scanf("%[^\n]", FNAME);
        fgetc(stdin);
        strcpy(A_FNAME[i], FNAME);
    }

    fprintf(stderr, "N : %d Q : %d\n\n\n", N, Q);
    bool found = false;
    for (size_t i = 0; i < Q; i++)
    {
        found = false;
        fprintf(stderr, "FNAME : %s\n", A_FNAME[i]);
        char *extension = getExtension(A_FNAME[i]);
        fprintf(stderr, "extension : %s\n", extension);
        if (extension)
        {
            for (size_t j = 0; j < N; j++)
            {
                fprintf(stderr, "EXT : %s\n", A_EXT[j]);
                if (strcmp(extension, A_EXT[j]) == 0)
                {
                    printf("%s\n", A_MT[j]);
                    found = true;
                    fprintf(stderr, "FOUND : %s\n", A_MT[j]);
                }
            }
        }
        if (found == false)
            printf("UNKNOWN\n");
    }

    return (0);
}