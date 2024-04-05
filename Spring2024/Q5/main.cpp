#include <cstring>
#include <iostream>
#include <jsoncpp/json/json.h>
#include <locale>
#include <set>
#include <unordered_map>
#include <vector>
using namespace std;

/**
 * @param n La taille de l'image
 * @param target_image Les lignes de l'image souhaitée, de haut en bas
 */
vector<string> solve(int n, vector<string> target_image) {
    deque<pair<int, int>> operationsQueue;              // Paire (type d'opération, index) : 0 pour R, 1 pour C
    vector<vector<int>> fillMetrics(2, vector<int>(n)); // Compteurs pour les lignes et les colonnes
    vector<int> validated(2, 0);                        // Nombre de lignes/colonnes déjà complétées

    // Analyse initiale des lignes
    for (int row = 0; row < n; row++) {
        for (int col = 0; col < n; col++) {
            fillMetrics[0][row] += target_image[row][col] == '.';
        }
        // Si une ligne est complètement blanche, l'ajouter à la file d'opérations
        if (fillMetrics[0][row] == n) {
            operationsQueue.push_back({0, row});
        }
    }

    // Analyse initiale des colonnes
    for (int col = 0; col < n; col++) {
        for (int row = 0; row < n; row++) {
            fillMetrics[1][col] += target_image[row][col] == '#';
        }
        // Si une colonne est complètement noire, l'ajouter à la file d'opérations
        if (fillMetrics[1][col] == n) {
            operationsQueue.push_back({1, col});
        }
    }

    vector<string> results; // Résultat final des opérations
    // Traitement des opérations
    while (!operationsQueue.empty()) {
        auto [operationType, index] = operationsQueue.front();
        operationsQueue.pop_front();

        // Construction de la chaîne de l'opération
        results.push_back((operationType == 1 ? "C " : "R ") + to_string(index));

        // Mise à jour du compteur des opérations validées
        ++validated[operationType];
        // Vérifier si l'opération inverse est nécessaire pour d'autres lignes/colonnes
        for (int i = 0; i < n; i++) {
            if (fillMetrics[1 - operationType][i] + validated[operationType] == n) {
                operationsQueue.push_back({1 - operationType, i});
            }
        }
    }

    // Inversion de l'ordre des résultats pour correspondre à l'ordre d'exécution
    reverse(results.begin(), results.end());

    return results;
}

/* Ignore and do not change the code below */

void trySolution(vector<string> commands) {
    Json::Value output_json;

    output_json = Json::Value(Json::arrayValue);
    Json::Value current_value;
    for (int i = 0; i < commands.size(); i++) {
        current_value = Json::Value(commands[i]);
        output_json.append(current_value);
    }

    Json::FastWriter fastWriter;
    std::string output_str = fastWriter.write(output_json);
    cout << "" << output_str.c_str();
}

int main() {
    setlocale(LC_ALL, "en_US.UTF-8");
    Json::Reader reader;
    string line;
    Json::Value parsed_json;
    std::getline(std::cin, line, '\n');
    reader.parse(line, parsed_json);
    int n = parsed_json.asInt();
    std::getline(std::cin, line, '\n');
    reader.parse(line, parsed_json);

    vector<string> array;
    for (int i = 0; i < parsed_json.size(); i++) {
        array.push_back(parsed_json[i].asString());
    }
    vector<string> target_image = array;

    vector<string> output = solve(n, target_image);
    trySolution(output);
}
/* Ignore and do not change the code above */
