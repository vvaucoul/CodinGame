#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

static int nearest_temperature_to_zero(int *temperatures, int n)
{
    int nearest_temperature = 0;

    for (int i = 0; i < n; i++)
    {
        fprintf(stderr, "temperatures[%d] = %d\n", i, temperatures[i]);
        if (nearest_temperature == 0)
            nearest_temperature = temperatures[i];
        else if (temperatures[i] > 0 && temperatures[i] <= abs(nearest_temperature))
            nearest_temperature = temperatures[i];
        else if (temperatures[i] < 0 && - temperatures[i] < abs(nearest_temperature))
            nearest_temperature = temperatures[i];
    }
    return (nearest_temperature);
}

int main()
{
    int n;
    int *temperatures;
    scanf("%d", &n);
    if (!(temperatures = malloc(sizeof(int) * (n + 1))))
        return (1);
    memset(temperatures, 0, sizeof(int) * (n + 1));
    for (int i = 0; i < n; i++)
    {
        int t;
        scanf("%d", &t);
        temperatures[i] = t;
    }
    if (n == 0)
        printf("0\n");
    else
        printf("%d\n", nearest_temperature_to_zero(temperatures, n));
    return (0);
}