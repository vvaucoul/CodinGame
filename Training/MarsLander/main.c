#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

int main()
{
    int surface_n;
    scanf("%d", &surface_n);
    for (int i = 0; i < surface_n; i++)
    {
        int land_x;
        int land_y;
        scanf("%d%d", &land_x, &land_y);
    }

    // game loop
    while (1)
    {
        int X;
        int Y;
        int h_speed;
        int v_speed;
        int fuel;
        int rotate;
        int power;

        scanf("%d%d%d%d%d%d%d", &X, &Y, &h_speed, &v_speed, &fuel, &rotate, &power);
        if (Y <= 1800)
            printf("0 4\n");
        else
            printf("0 3\n");
    }
    return (0);
}