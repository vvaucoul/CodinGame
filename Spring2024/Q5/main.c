#include <ctype.h>
#include <json-c/json.h>
#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct stringArray {
    int length;
    char **items;
} stringArray;

/**
 * @brief Create a Command object
 *
 * @param type
 * @param index
 * @return char*
 */

char *createCommand(char type, int index) {
    char *command = malloc(10);             // Assurez-vous d'allouer suffisamment de mémoire
    sprintf(command, "%c %d", type, index); // 0-indexed
    return command;
}


bool isSolved(int n, char currentImage[n][n], stringArray target_image) {
    for (int i = 0; i < n; i++) {
        if (strncmp(currentImage[i], target_image.items[i], n) != 0) {
            return false;
        }
    }
    return true;
}

// Trouvez la première colonne qui ne correspond pas à l'image cible et qui n'a pas été encore résolue
int findMismatchedColumn(int n, char currentImage[n][n], stringArray target_image) {
    for (int col = 0; col < n; col++) {
        bool mismatched = false;
        for (int row = 0; row < n; row++) {
            if (currentImage[row][col] != target_image.items[row][col]) {
                mismatched = true;
                break; // Dès qu'un pixel ne correspond pas, arrêtez de vérifier cette colonne
            }
        }
        if (mismatched) {
            return col;
        }
    }
    return -1;
}

// Trouvez la première rangée qui ne correspond pas à l'image cible et qui n'a pas été encore résolue
int findMismatchedRow(int n, char currentImage[n][n], stringArray target_image) {
    for (int row = 0; row < n; row++) {
        if (strncmp(currentImage[row], target_image.items[row], n) != 0) {
            return row;
        }
    }
    return -1;
}

// Applique une commande sur une colonne ou une rangée spécifique, en tenant compte de l'état requis
void applyCommand(int n, char currentImage[n][n], char type, int index, stringArray target_image) {
    if (type == 'C') {
        for (int row = 0; row < n; row++) {
            currentImage[row][index] = target_image.items[row][index]; // Applique l'état de la colonne de l'image cible
        }
    } else if (type == 'R') {
        for (int col = 0; col < n; col++) {
            currentImage[index][col] = target_image.items[index][col]; // Applique l'état de la rangée de l'image cible
        }
    }
}

void displayCurrentImage(int n, char currentImage[n][n]) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            fprintf(stderr, "%c", currentImage[i][j]);
        }
        fprintf(stderr, "\n");
    }
}

// Fonction principale pour résoudre le problème
stringArray solve(int n, stringArray target_image) {
    stringArray commands = {0, NULL};
    char currentImage[n][n];
    memset(currentImage, '.', sizeof(currentImage)); // Initialisez l'image actuelle à blanc

    while (!isSolved(n, currentImage, target_image)) {
        int col = findMismatchedColumn(n, currentImage, target_image);
        int row = findMismatchedRow(n, currentImage, target_image);

        fprintf(stderr, "Mismatched column: %d, row: %d\n", col, row);

        // Ajoutez une commande pour la première colonne ou rangée qui ne correspond pas
        if (col != -1) {
            commands.items = realloc(commands.items, sizeof(char *) * (commands.length + 1));
            commands.items[commands.length] = createCommand('C', col);
            commands.length++;
            applyCommand(n, currentImage, 'C', col, target_image);
            displayCurrentImage(n, currentImage);
        } else if (row != -1) {
            commands.items = realloc(commands.items, sizeof(char *) * (commands.length + 1));
            commands.items[commands.length] = createCommand('R', row);
            commands.length++;
            applyCommand(n, currentImage, 'R', row, target_image);
            displayCurrentImage(n, currentImage);
        }
        if (commands.length > n * 2) {
            break;
        }
    }

    return commands;
}

/* Ignore and do not change the code below */

void trySolution(stringArray commands) {
    struct json_object *output_json;

    output_json = json_object_new_array();
    struct json_object *current_value;
    for (int i = 0; i < commands.length; i++) {
        current_value = json_object_new_string(commands.items[i]);
        json_object_array_add(output_json, current_value);
    }
    printf("%s\n", json_object_to_json_string_ext(output_json, JSON_C_TO_STRING_NOSLASHESCAPE));
    json_object_put(output_json);
}

int main() {
    setlocale(LC_ALL, "en_US.UTF-8");
    char *line;
    size_t len = 0;

    line = NULL;
    getline(&line, &len, stdin);
    struct json_object *parsed_json0 = json_tokener_parse(line);
    free(line);
    int n;
    n = json_object_get_int(parsed_json0);

    line = NULL;
    getline(&line, &len, stdin);
    struct json_object *parsed_json1 = json_tokener_parse(line);
    free(line);
    stringArray target_image;

    stringArray array = {json_object_array_length(parsed_json1), malloc(sizeof(char *) * array.length)};

    for (int i = 0; i < array.length; i++) {
        struct json_object *json_object_tmp = json_object_array_get_idx(parsed_json1, i);
        char *json_string_tmp2 = malloc(sizeof(char) * (json_object_get_string_len(json_object_tmp) + 1));
        strcpy(json_string_tmp2, json_object_get_string(json_object_tmp));
        array.items[i] = json_string_tmp2;
    }
    target_image = array;

    json_object_put(parsed_json0);
    json_object_put(parsed_json1);
    stringArray output = solve(n, target_image);

    trySolution(output);
}
/* Ignore and do not change the code above */
