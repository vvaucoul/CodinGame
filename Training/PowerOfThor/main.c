#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

int main()
{
    int light_x;
    int light_y;
    int initial_tx;
    int initial_ty;
    scanf("%d%d%d%d", &light_x, &light_y, &initial_tx, &initial_ty);

    while (1)
    {
        int remaining_turns;
        scanf("%d", &remaining_turns);

        if (initial_tx == light_x && initial_ty == light_y)
            printf("WAIT\n");

        if (initial_tx == light_x)
        {
            if (initial_ty > light_y)
            {
                printf("N\n");
                initial_ty--;
            }
            else
            {
                printf("S\n");
                initial_ty++;
            }
        }
        else if (initial_ty == light_y)
        {
            if (initial_tx > light_x)
            {
                printf("W\n");
                initial_tx--;
            }
            else
            {
                printf("E\n");
                initial_tx++;
            }
        }
        else
        {
            if (initial_tx > light_x && initial_ty > light_y)
            {
                printf("NW\n");
                initial_tx--;
                initial_ty--;
            }
            else if (initial_tx > light_x && initial_ty < light_y)
            {
                printf("SW\n");
                initial_tx--;
                initial_ty++;
            }
            else if (initial_tx < light_x && initial_ty > light_y)
            {
                printf("NE\n");
                initial_tx++;
                initial_ty--;
            }
            else if (initial_tx < light_x && initial_ty < light_y)
            {
                printf("SE\n");
                initial_tx++;
                initial_ty++;
            }
        }
    }
    return (0);
}