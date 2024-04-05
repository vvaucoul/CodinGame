#include <ctype.h>
#include <json-c/json.h>
#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * La hauteur de chaque bâtiment
 */
typedef struct intArray {
    int length;
    int *items;
} intArray;

/**
 * La représentation des n bâtiments
 */
typedef struct stringArray {
    int length;
    char **items;
} stringArray;

/**
 * @param n Le nombre de bâtiments
 * @param building_map La représentation des n bâtiments
 * @return La hauteur de chaque bâtiment
 */
intArray buildingHeights(int n, stringArray building_map) {
    intArray int_array = {n, malloc(sizeof(int) * n)};

    for (int i = 0; i < n; i++) {
        int_array.items[i] = 0;
        for (int j = 0; j < strlen(building_map.items[i]); j++) {
            if (building_map.items[i][j] == '*') {
                int_array.items[i]++;
            }
        }
    }
    return int_array;
}

/* Ignore and do not change the code below */

void trySolution(intArray output) {
    struct json_object *output_json;

    output_json = json_object_new_array();
    struct json_object *current_value;
    for (int i = 0; i < output.length; i++) {
        current_value = json_object_new_int(output.items[i]);
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
    stringArray building_map;

    stringArray array = {json_object_array_length(parsed_json1), malloc(sizeof(char *) * array.length)};

    for (int i = 0; i < array.length; i++) {
        struct json_object *json_object_tmp = json_object_array_get_idx(parsed_json1, i);
        char *json_string_tmp2 = malloc(sizeof(char) * (json_object_get_string_len(json_object_tmp) + 1));
        strcpy(json_string_tmp2, json_object_get_string(json_object_tmp));
        array.items[i] = json_string_tmp2;
    }
    building_map = array;

    json_object_put(parsed_json0);
    json_object_put(parsed_json1);
    intArray output = buildingHeights(n, building_map);

    trySolution(output);
}
/* Ignore and do not change the code above */
