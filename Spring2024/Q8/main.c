#include <ctype.h>
#include <json-c/json.h>
#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct intArray {
    int length;
    int *items;
} intArray;

/**
 * La suite de coordonnées de pixels à activer pour reproduire l'image cible.
 */
typedef struct intArrayArray {
    int length;
    intArray *items;
} intArrayArray;

/**
 * L'image cible, donnée ligne par ligne.
 */
typedef struct stringArray {
    int length;
    char **items;
} stringArray;

void toggle(char **grid, int r, int c, int n_rows, int n_cols) {
    // Toggle the chosen cell
    grid[r][c] = grid[r][c] == '.' ? '#' : '.';

    // Toggle the neighbors
    if (r > 0) {
        grid[r - 1][c] = grid[r - 1][c] == '.' ? '#' : '.'; // Above
    }
    if (r < n_rows - 1) {
        grid[r + 1][c] = grid[r + 1][c] == '.' ? '#' : '.'; // Below
    }
    if (c > 0) {
        grid[r][c - 1] = grid[r][c - 1] == '.' ? '#' : '.'; // Left
    }
    if (c < n_cols - 1) {
        grid[r][c + 1] = grid[r][c + 1] == '.' ? '#' : '.'; // Right
    }
}
void togglePixelAndNeighbors(char **grid, int x, int y, int n_rows, int n_cols) {
    int dx[] = {0, 1, -1, 0, 0}; // Inclut le pixel lui-même et ses voisins
    int dy[] = {0, 0, 0, 1, -1};

    for (int i = 0; i < 5; i++) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        if (nx >= 0 && nx < n_rows && ny >= 0 && ny < n_cols) {
            grid[nx][ny] = (grid[nx][ny] == '.') ? '#' : '.';
        }
    }
}

// Fonction pour vérifier si la grille actuelle correspond au motif cible
int isMatch(char **grid, stringArray target_pattern, int n_rows, int n_cols) {
    for (int i = 0; i < n_rows; i++) {
        for (int j = 0; j < n_cols; j++) {
            if (grid[i][j] != target_pattern.items[i][j]) {
                return 0; // Ne correspond pas
            }
        }
    }
    return 1; // Correspond
}

// Fonction récursive de backtracking pour trouver la séquence d'activation optimale
void findPattern(int n_rows, int n_cols, stringArray target_pattern, char **grid, intArrayArray *best_solution, intArray current_solution, int step, int x, int y) {
    // Si la grille correspond au motif cible et que la solution actuelle est meilleure
    if (isMatch(grid, target_pattern, n_rows, n_cols)) {
        if (best_solution->length == 0 || current_solution.length < best_solution->length) {
            // Libérer l'ancienne meilleure solution
            for (int i = 0; i < best_solution->length; i++) {
                free(best_solution->items[i].items);
            }
            free(best_solution->items);

            // Copier la solution actuelle comme la meilleure solution
            best_solution->length = current_solution.length;
            best_solution->items = malloc(sizeof(intArray) * current_solution.length);
            for (int i = 0; i < current_solution.length; i++) {
                best_solution->items[i].length = 2;
                best_solution->items[i].items = malloc(sizeof(int) * 2);
                best_solution->items[i].items[0] = current_solution.items[i * 2];
                best_solution->items[i].items[1] = current_solution.items[i * 2 + 1];
            }
        }
        return;
    }

    // Si nous avons atteint la fin de la grille sans correspondance
    if (step == n_rows * n_cols) {
        return;
    }

    // Calculer la prochaine position
    int next_x = (x + 1) % n_cols;
    int next_y = y + (x + 1) / n_cols;

    // Essayer d'activer le pixel (x, y) et chercher plus loin
    togglePixelAndNeighbors(grid, x, y, n_rows, n_cols);

    // current_solution.items[current_solution.length * 2] = x;
    // current_solution.items[current_solution.length * 2 + 1] = y;
    current_solution.items[current_solution.length * 2] = y;
    current_solution.items[current_solution.length * 2 + 1] = x;

    current_solution.length++;
    findPattern(n_rows, n_cols, target_pattern, grid, best_solution, current_solution, step + 1, next_x, next_y);

    // Annuler l'activation et chercher plus loin sans activer le pixel (x, y)
    togglePixelAndNeighbors(grid, x, y, n_rows, n_cols);
    current_solution.length--;
    findPattern(n_rows, n_cols, target_pattern, grid, best_solution, current_solution, step + 1, next_x, next_y);
}

intArrayArray createPattern(int n_rows, int n_cols, stringArray target_pattern) {
    // Créer une grille vide
    char **grid = malloc(sizeof(char *) * n_rows);
    for (int i = 0; i < n_rows; i++) {
        grid[i] = malloc(sizeof(char) * n_cols);
        for (int j = 0; j < n_cols; j++) {
            grid[i][j] = '.';
        }
    }

    // Initialiser la meilleure solution
    intArrayArray best_solution = {0, NULL};

    // Initialiser la solution actuelle
    intArray current_solution = {0, NULL};
    current_solution.items = malloc(sizeof(int) * n_rows * n_cols * 2);

    // Trouver la séquence d'activation optimale
    findPattern(n_rows, n_cols, target_pattern, grid, &best_solution, current_solution, 0, 0, 0);

    // Libérer la mémoire
    for (int i = 0; i < n_rows; i++) {
        free(grid[i]);
    }
    free(grid);
    free(current_solution.items);

    for (int i = 0; i < best_solution.length; i++) {
        fprintf(stderr, "[%d] -> [%d %d]\n", i, best_solution.items[i].items[0], best_solution.items[i].items[1]);
    }

    return best_solution;
}

/* Ignore and do not change the code below */

void trySolution(intArrayArray output) {
    struct json_object *output_json;

    output_json = json_object_new_array();
    struct json_object *current_value;
    for (int i = 0; i < output.length; i++) {

        current_value = json_object_new_array();
        struct json_object *current_value2;
        for (int j = 0; j < output.items[i].length; j++) {
            current_value2 = json_object_new_int(output.items[i].items[j]);
            json_object_array_add(current_value, current_value2);
        }
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
    int n_rows;
    n_rows = json_object_get_int(parsed_json0);

    line = NULL;
    getline(&line, &len, stdin);
    struct json_object *parsed_json1 = json_tokener_parse(line);
    free(line);
    int n_cols;
    n_cols = json_object_get_int(parsed_json1);

    line = NULL;
    getline(&line, &len, stdin);
    struct json_object *parsed_json2 = json_tokener_parse(line);
    free(line);
    stringArray target_pattern;

    stringArray array = {json_object_array_length(parsed_json2), malloc(sizeof(char *) * array.length)};

    for (int i = 0; i < array.length; i++) {
        struct json_object *json_object_tmp = json_object_array_get_idx(parsed_json2, i);
        char *json_string_tmp2 = malloc(sizeof(char) * (json_object_get_string_len(json_object_tmp) + 1));
        strcpy(json_string_tmp2, json_object_get_string(json_object_tmp));
        array.items[i] = json_string_tmp2;
    }
    target_pattern = array;

    json_object_put(parsed_json0);
    json_object_put(parsed_json1);
    json_object_put(parsed_json2);
    intArrayArray output = createPattern(n_rows, n_cols, target_pattern);

    trySolution(output);
}
/* Ignore and do not change the code above */
