#include <ctype.h>
#include <json-c/json.h>
#include <locale.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Example Array
 *
 * bbCCCCC3cC3c
 * 1CAABBBC3c3c
 * aCAAbbbC312A
 * aCCBbabCB111
 * 3CBBbbbC2C12
 * CCBAca3C2a1C
 * 2ACCCCCaaaaa
 * b33ccccccccc
 */

/**
 * Les caractéristiques du plus grand cercle [ligneCentre, colonneCentre, rayon].
 */
typedef struct intArray {
    int length;
    int *items;
} intArray;

/**
 * Les pixels de l'image, donnés ligne par ligne de haut en bas.
 */
typedef struct stringArray {
    int length;
    char **items;
} stringArray;

static int min(int a, int b) {
    return a < b ? a : b;
}

static int max(int a, int b) {
    return a > b ? a : b;
}

static bool isWithinBounds(int x, int y, int width, int height) {
    return x >= 0 && x < width && y >= 0 && y < height;
}

static bool isValidCircle(int centerRow, int centerCol, int radius, stringArray image, int n_cols, int n_rows) {
    char color = '\0';
    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            int x = centerCol + dx;
            int y = centerRow + dy;
            double distance = sqrt(dx * dx + dy * dy);
            // Check only the perimeter of the circle
            if (distance >= radius && distance < radius + 1) {
                if (!isWithinBounds(x, y, n_cols, n_rows)) {
                    return false; // Perimeter point is out of bounds
                }
                if (color == '\0') {
                    color = image.items[y][x]; // Set the color to the first perimeter pixel color
                } else if (image.items[y][x] != color) {
                    return false; // Different color found at the perimeter
                }
            }
        }
    }
    return color != '\0'; // If color is still '\0', no circle was found, otherwise a circle was found
}

intArray findLargestCircle(int n_rows, int n_cols, stringArray image) {
    intArray largestCircle = {3, (int *)malloc(sizeof(int) * 3)};
    largestCircle.items[0] = largestCircle.items[1] = largestCircle.items[2] = 0;

    for (int row = 0; row < n_rows; row++) {
        for (int col = 0; col < n_cols; col++) {
            for (int radius = min(n_rows, n_cols) / 2; radius > 0; radius--) {
                if (isValidCircle(row, col, radius, image, n_cols, n_rows)) {
                    if (radius > largestCircle.items[2]) { // Found a larger circle
                        largestCircle.items[0] = row;
                        largestCircle.items[1] = col;
                        largestCircle.items[2] = radius;
                        break; // Stop searching for smaller circles at this center
                    }
                }
            }
        }
    }
    return largestCircle;
}

/* Ignore and do not change the code below */

void trySolution(intArray largest_circle) {
    struct json_object *output_json;

    output_json = json_object_new_array();
    struct json_object *current_value;
    for (int i = 0; i < largest_circle.length; i++) {
        current_value = json_object_new_int(largest_circle.items[i]);
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
    stringArray image;

    stringArray array = {json_object_array_length(parsed_json2), malloc(sizeof(char *) * array.length)};

    for (int i = 0; i < array.length; i++) {
        struct json_object *json_object_tmp = json_object_array_get_idx(parsed_json2, i);
        char *json_string_tmp2 = malloc(sizeof(char) * (json_object_get_string_len(json_object_tmp) + 1));
        strcpy(json_string_tmp2, json_object_get_string(json_object_tmp));
        array.items[i] = json_string_tmp2;
    }
    image = array;

    json_object_put(parsed_json0);
    json_object_put(parsed_json1);
    json_object_put(parsed_json2);
    intArray output = findLargestCircle(n_rows, n_cols, image);

    trySolution(output);
}
/* Ignore and do not change the code above */
