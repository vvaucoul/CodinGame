#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

static int *find_highest_height(int *heights)
{
    int highest_index = 0;
    int highest_height = 0;
    for (int i = 0; i < 8; i++)
    {
        if (heights[i] > highest_height)
        {
            highest_height = heights[i];
            highest_index = i;
        }
    }
    return ((int [2]){highest_index, highest_height});
}

int main()
{
    while (1)
    {
        int heights[8] = {0};
        for (int i = 0; i < 8; i++)
        {
            int mountain_h;
            scanf("%d", &mountain_h);
            heights[i] = mountain_h;
        }
        printf("%d\n", find_highest_height(heights)[0]);
    }
    return (0);
}