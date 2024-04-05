#include <ctype.h>
#include <json-c/json.h>
#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MOD 1000000007
#define TOTAL_CHARS 62 // 26 lettres minuscules + 26 lettres majuscules + 10 chiffres

typedef struct stringArray {
    int length;
    char **items;
} stringArray;

typedef struct {
    int x, y;
} Point;

int charToIndex(char c) {
    if (isdigit(c))
        return c - '0';
    if (islower(c))
        return c - 'a' + 10;
    if (isupper(c))
        return c - 'A' + 36;
    return -1;
}

void findAllOccurrences(int n_rows, int n_cols, stringArray image, Point ***occurrences, int *typeCounts) {
    for (int i = 0; i < n_rows; i++) {
        for (int j = 0; j < n_cols; j++) {
            int type = (int)image.items[i][j];
            occurrences[type][typeCounts[type]]->x = i;
            occurrences[type][typeCounts[type]]->y = j;
            typeCounts[type]++;
        }
    }
}

int compare(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
}

long long calculateTotalDistance(int *coords, int count) {
    qsort(coords, count, sizeof(int), compare);
    long long totalDistance = 0;
    long long preSum = 0;

    for (int i = 0; i < count; i++) {
        totalDistance = (totalDistance + (long long)coords[i] * i - preSum) % MOD;
        preSum = (preSum + coords[i]) % MOD;
    }

    return (totalDistance < 0 ? totalDistance + MOD : totalDistance);
}

int getCableLength(int n_rows, int n_cols, stringArray image) {
    long long totalLength = 0;
    Point **occurrences[TOTAL_CHARS];
    int typeCounts[TOTAL_CHARS] = {0};

    for (int i = 0; i < TOTAL_CHARS; i++) {
        occurrences[i] = malloc(sizeof(Point *) * (n_rows * n_cols));
        for (int j = 0; j < n_rows * n_cols; j++) {
            occurrences[i][j] = malloc(sizeof(Point));
        }
    }

    for (int i = 0; i < n_rows; i++) {
        for (int j = 0; j < n_cols; j++) {
            int type = charToIndex(image.items[i][j]);
            if (type != -1) {
                occurrences[type][typeCounts[type]]->x = i;
                occurrences[type][typeCounts[type]]->y = j;
                typeCounts[type]++;
            }
        }
    }

    for (int i = 0; i < TOTAL_CHARS; i++) {
        if (typeCounts[i] > 0) {
            int *xCoords = malloc(typeCounts[i] * sizeof(int));
            int *yCoords = malloc(typeCounts[i] * sizeof(int));
            for (int j = 0; j < typeCounts[i]; j++) {
                xCoords[j] = occurrences[i][j]->x;
                yCoords[j] = occurrences[i][j]->y;
            }
            totalLength = (totalLength + calculateTotalDistance(xCoords, typeCounts[i])) % MOD;
            totalLength = (totalLength + calculateTotalDistance(yCoords, typeCounts[i])) % MOD;
            free(xCoords);
            free(yCoords);
        }
    }

    for (int i = 0; i < TOTAL_CHARS; i++) {
        for (int j = 0; j < n_rows * n_cols; j++) {
            free(occurrences[i][j]);
        }
        free(occurrences[i]);
    }

    return (int)(totalLength * 2 % MOD);
}

/* Ignore and do not change the code below */

void trySolution(int cable_length) {
    struct json_object *output_json;
    output_json = json_object_new_int(cable_length);
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
    int output = getCableLength(n_rows, n_cols, image);

    trySolution(output);
}
/* Ignore and do not change the code above */
