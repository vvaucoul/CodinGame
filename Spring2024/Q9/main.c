#include <ctype.h>
#include <json-c/json.h>
#include <limits.h>
#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * La liste des codes binaires dans la table.
 */
typedef struct stringArray {
    int length;
    char **items;
} stringArray;

bool isPrefix(const char *prefix, const char *word) {
    while (*prefix && *word) {
        if (*prefix++ != *word++) {
            return false;
        }
    }
    return *prefix == '\0'; // Si tout le préfixe a été parcouru, alors c'est un préfixe
}

int compare(const void *a, const void *b) {
    return strcmp(*(const char **) a, *(const char **) b);
}

char *crashDecode(stringArray codes) {

    int codesSize = codes.length;

    qsort(codes.items, codesSize, sizeof(char *), compare);
    for (int i = 0; i < codesSize; i++) {
        for (int j = i + 1; j < codesSize; j++) {
            if (isPrefix(codes.items[i], codes.items[j])) {
                char *ambiguous = malloc(strlen(codes.items[i]) + 2);
                strcpy(ambiguous, codes.items[i]);
                ambiguous[strlen(codes.items[i])] = codes.items[j][strlen(codes.items[i])];
                ambiguous[strlen(codes.items[i]) + 1] = '\0';
                return ambiguous;
            }
        }
    }
    return "X";
}


/* Ignore and do not change the code below */

void trySolution(char *ambiguous_sequence) {
    struct json_object *output_json;
    output_json = json_object_new_string(ambiguous_sequence);
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
    stringArray codes;

    stringArray array = {json_object_array_length(parsed_json0), malloc(sizeof(char *) * array.length)};

    for (int i = 0; i < array.length; i++) {
        struct json_object *json_object_tmp = json_object_array_get_idx(parsed_json0, i);
        char *json_string_tmp2 = malloc(sizeof(char) * (json_object_get_string_len(json_object_tmp) + 1));
        strcpy(json_string_tmp2, json_object_get_string(json_object_tmp));
        array.items[i] = json_string_tmp2;
    }
    codes = array;

    json_object_put(parsed_json0);
    char *output = crashDecode(codes);

    trySolution(output);
}
/* Ignore and do not change the code above */
