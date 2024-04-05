#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/

int main() {
    int n; // taille de la map
    scanf("%d", &n);
    fgetc(stdin);

    char buffer[n + 1][n + 1];
    for (int i = 0; i < n; i++) {
        memset(buffer[i], '.', n);
        buffer[i][n] = '\0';
    }

    // game loop
    while (1) {
        char command[201];
        scanf("%[^\n]", command);
        fgetc(stdin);

        char type;
        int coord;
        sscanf(command, "%c %d", &type, &coord);

        // C: Column (paint vertical line)
        // R: Line (paint horizontal line)

        if (type == 'C') {
            for (int i = 0; i < n; i++) {
                buffer[i][coord] = '#';
            }
        } else if (type == 'R') {
            for (int i = 0; i < n; i++) {
                buffer[coord][i] = '.';
            }
        }

        for (int i = 0; i < n; i++) {

            printf("%s\n", buffer[i]);            
        }
    }

    return 0;
}