#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

int main()
{

    // game loop
    while (1)
    {
        for (int i = 0; i < 8; i++)
        {
            // represents the height of one mountain.
            int mountain_h;
            scanf("%d", &mountain_h);

            printf("%d\n", mountain_h);
        }

        // Write an action using printf(). DON'T FORGET THE TRAILING \n
        // To debug: fprintf(stderr, "Debug messages...\n");

        printf("1\n");
    }

    return 0;
}