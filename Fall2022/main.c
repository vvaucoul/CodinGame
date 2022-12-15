#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <limits.h>

#define MAP_MAX_X 24
#define MAP_MAX_Y 12

#define MAP_RATIO (game.width / game.height)

#define SCRAPS_TO_SPAWN 10
#define SCRAPS_TO_BUILD 10

#define OWN_CELL 1
#define OPP_CELL 0
#define EMPTY_CELL -1

#define FOR_EACH_CELLS(x, y)              \
    for (int y = 0; y < game.height; y++) \
        for (int x = 0; x < game.width; x++)

#define FOR_IN_RANGE(x, y, range)         \
    for (int y = -range; y <= range; y++) \
        for (int x = -range; x <= range; x++)

#define LOOP_CHECKER(x, y) (x == 0 && y == 0 || x == 1 && y == 1 || x == -1 && y == -1 || x == 1 && y == -1 || x == -1 && y == 1)

#define IS_OPP_RECYCLER(x, y) \
    (game.map[y][x].owner == OPP_CELL && game.map[y][x].recycler == 1)

#define MAP_MIDDLE_CENTER               \
    (vec_t)                             \
    {                                   \
        game.width / 2, game.height / 2 \
    }
#define MAP_TOP_CENTER    \
    (vec_t)               \
    {                     \
        game.width / 2, 0 \
    }
#define MAP_BOTTOM_CENTER               \
    (vec_t)                             \
    {                                   \
        game.width / 2, game.height - 1 \
    }

typedef struct s_vec
{
    int x;
    int y;
} vec_t;

typedef struct s_unit
{
    vec_t pos;
} unit_t;

typedef struct s_cell
{
    int scrap_amount;
    int owner;
    int units;
    int recycler;
    int can_build;
    int can_spawn;
    int in_range_of_recycler;
    vec_t pos;
} cell_t;

typedef struct s_game
{
    int width;
    int height;

    int my_matter;
    int opp_matter;

    int game_turn;
    int recycler_count;
    int units_count;

    vec_t opp_base_location;
    vec_t base_location;

    cell_t map[MAP_MAX_Y][MAP_MAX_X];
} game_t;

game_t game;
unit_t *units = NULL;

/*******************************************************************************
 *                               UTILS FUNCTIONS                               *
 ******************************************************************************/

bool isValidPos(vec_t pos)
{
    return (pos.x >= 0 && pos.x < game.width && pos.y >= 0 && pos.y < game.height);
}

bool isValidCell(cell_t *cell)
{
    return (game.map[cell->pos.y][cell->pos.x].scrap_amount > 0 && isValidPos(cell->pos));
}

float getDistanceTo(vec_t pos1, vec_t pos2)
{
    return (sqrt(pow(pos1.x - pos2.x, 2) + pow(pos1.y - pos2.y, 2)));
}

vec_t findNearestValidCell(vec_t cell)
{
    int range = 1;
    int x = 0, y = 0;
    while (range < game.width)
    {
        FOR_IN_RANGE(x, y, range)
        {
            if (isValidPos((vec_t){cell.x + x, cell.y + y}) == false)
                continue;
            if (LOOP_CHECKER(x, y))
                continue;
            cell_t *cell = &game.map[y][x];
            if (isValidCell(cell))
                return (cell->pos);
        }
        range++;
    }
    return (cell);
}

void setBasesLocations(void)
{
    int x = 0, y = 0;
    FOR_EACH_CELLS(x, y)
    {
        if (game.map[y][x].owner == OWN_CELL &&
            game.map[y + 1][x].owner == OWN_CELL &&
            game.map[y][x + 1].owner == OWN_CELL &&
            game.map[y - 1][x].owner == OWN_CELL &&
            game.map[y][x - 1].owner == OWN_CELL)
            game.base_location = (vec_t){x, y};
        else if (game.map[y][x].owner == OPP_CELL &&
                 game.map[y + 1][x].owner == OPP_CELL &&
                 game.map[y][x + 1].owner == OPP_CELL &&
                 game.map[y - 1][x].owner == OPP_CELL &&
                 game.map[y][x - 1].owner == OPP_CELL)
            game.opp_base_location = (vec_t){x, y};

        if (game.base_location.x != 0 && game.base_location.y != 0 &&
            game.opp_base_location.x != 0 && game.opp_base_location.y != 0)
            break;
    }
}

void updateUnits(void)
{
    if (units = NULL)
        units = malloc(sizeof(unit_t) * game.units_count + 1);
    else
    {
        free(units);
        units = malloc(sizeof(unit_t) * game.units_count + 1);
    }
    bzero(units, sizeof(unit_t) * game.units_count + 1);

    int i = 0;
    int x = 0, y = 0;

    FOR_EACH_CELLS(x, y)
    {
        cell_t *cell = &game.map[y][x];
        if (cell->owner == OWN_CELL)
        {
            for (int j = 0; j < cell->units; j++)
            {
                units[i].pos.x = x;
                units[i].pos.y = y;
                i++;
            }
        }
    }
}

/*******************************************************************************
 *                               PATH FUNCTIONS                                *
 ******************************************************************************/

typedef struct s_path
{
    vec_t path[MAP_MAX_X * MAP_MAX_Y];
    int path_len;
} path_t;

static char **pathCreateGraph(void)
{
    char **map = malloc(sizeof(char) * game.height * game.width);

    bzero(map, sizeof(char) * game.height * game.width);

    for (int y = 0; y < game.height; y++)
    {
        map[y] = malloc(sizeof(char) * game.width);
        bzero(map[y], sizeof(char) * game.width);
        for (int x = 0; x < game.width; x++)
        {
            if (game.map[y][x].scrap_amount > 0)
                map[y][x] = 1;
            else
                map[y][x] = 0;
        }
    }
    return (map);
}

bool pathIsValid(path_t *path)
{
    if (path == NULL)
        return (false);
    if (path->path_len == 1)
        return (false);
    return (true);
}

path_t *pathSearch(vec_t start, vec_t end)
{
    if (!isValidPos(start) || !isValidPos(end))
    {
        fprintf(stderr, "Path: Invalid position\n");
        return (NULL);
    }

    if (start.x == end.x && start.y == end.y)
    {
        fprintf(stderr, "Path: Same position\n");
        return (NULL);
    }

    path_t *path = malloc(sizeof(path_t));
    path->path_len = 0;

    char **graph = pathCreateGraph();

    for (int y = 0; y < game.height; y++)
    {
        for (int x = 0; x < game.width; x++)
            fprintf(stderr, "%d", graph[y][x]);
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");

    vec_t current = start;
    vec_t next = start;

    int depth = 0;

    while (current.x != end.x || current.y != end.y || depth >= MAP_MAX_X + MAP_MAX_Y)
    {
        float distance = INT_MAX;

        for (int i = -1; i <= 1; i++)
        {
            for (int j = -1; j <= 1; j++)
            {
                if (i == 0 && j == 0 || i == 1 && j == 1 || i == -1 && j == -1 || i == 1 && j == -1 || i == -1 && j == 1)
                    continue;

                if (isValidPos((vec_t){current.x + j, current.y + i}) && graph[current.y + i][current.x + j] == 1 &&
                    getDistanceTo((vec_t){current.x + j, current.y + i}, end) < distance &&
                    graph[current.y + i][current.x + j] != 0)
                {
                    distance = getDistanceTo((vec_t){current.x + j, current.y + i}, end);
                    next.x = current.x + j;
                    next.y = current.y + i;
                }
            }
        }

        if (distance == INT_MAX)
            break;
        else
        {
            path->path[path->path_len] = next;
            path->path_len++;
            current = next;
        }
        depth++;
    }

    return (path);
}

/*******************************************************************************
 *                                AI FUNCTIONS                                 *
 ******************************************************************************/

void buildRecyclers(void)
{
    fprintf(stderr, "\n### Build recyclers ###\n\n");

    int x = 0, y = 0;
    FOR_EACH_CELLS(x, y)
    {
        cell_t *ccell = &game.map[y][x];
        if (game.my_matter < SCRAPS_TO_BUILD)
            break;
        else if (ccell->can_build == false || isValidCell(ccell) == false)
            continue;

        int nbOppCells = 0;
        int nbValidCells = 0;
        int nbOppUnits = 0;

        int score = 0;

        int x2 = 0, y2 = 0;
        FOR_IN_RANGE(x2, y2, 1)
        {
            if (isValidPos((vec_t){x + x2, y + y2}) == false)
                continue;
            if ((LOOP_CHECKER(x + x2, y + y2)) == true)
                continue;
            else
            {
                cell_t *cell = &game.map[y + y2][x + x2];

                if (cell->owner == OPP_CELL)
                {
                    nbOppUnits += cell->units;
                    nbOppCells++;
                }
                else if (cell->owner == OWN_CELL)
                    nbValidCells++;
            }
        }

        score = nbOppUnits * 10 + nbOppCells * 5 + nbValidCells * 4;
        fprintf(stderr, "\t- Score for (%d, %d) is %d\n", x, y, score);

        if (score > 50)
        {
            fprintf(stderr, "\t\t- Build recycler at (%d, %d)\n", x, y);
            printf("BUILD %d %d;", x, y);
            game.my_matter -= SCRAPS_TO_BUILD;
        }
    }
}

void spawnUnits(void)
{
    int units_spawnable = game.my_matter / SCRAPS_TO_SPAWN;

    fprintf(stderr, "\n### Find best spawn location ###\n\n");

    for (int ccell = 0; ccell < units_spawnable; ccell++)
    {
        cell_t *best_cell = NULL;
        int best_score = 0;

        int x = 0, y = 0;
        FOR_EACH_CELLS(x, y)
        {
            cell_t *cell = &game.map[y][x];
            if (cell->can_spawn)
            {
                int nbEmptyCellsAround = 0;
                int nbOppCellsAround = 0;
                int nbCellsAround = 0;
                int nbOppUnitsAround = 0;
                int nbUnitsAround = 0;

                int nbUnits = 0;

                int current_score = 0;

                int x2 = 0, y2 = 0;
                FOR_IN_RANGE(x2, y2, 1)
                {
                    if (isValidPos((vec_t){x + x2, y + y2}) == false)
                        continue;
                    if (LOOP_CHECKER(x + x2, y + y2) == true)
                    {
                        nbUnits += game.map[y + y2][x + x2].units;
                        continue;
                    }

                    cell_t *newCell = &game.map[y + y2][x + x2];

                    if (isValidCell(newCell) == false)
                        continue;

                    if (newCell->owner == OWN_CELL)
                    {
                        nbUnitsAround += newCell->units;
                        nbCellsAround++;
                    }
                    else if (newCell->owner == OPP_CELL)
                    {
                        nbOppUnitsAround += newCell->units;
                        nbOppCellsAround++;
                    }
                    else if (newCell->owner == EMPTY_CELL)
                    {
                        nbEmptyCellsAround++;
                    }
                }
                current_score = nbEmptyCellsAround * 40 + nbOppCellsAround * 100 + nbCellsAround * 10 + nbOppUnitsAround * 40 + nbUnitsAround * 10 + nbUnits * 10;

                if (current_score > best_score)
                {
                    best_score = current_score;
                    best_cell = cell;
                }
            }
        }

        if (best_cell != NULL)
        {
            fprintf(stderr, "\t- SPAWN 1 %d %d\n", best_cell->pos.x, best_cell->pos.y);
            printf("SPAWN 1 %d %d;", best_cell->pos.x, best_cell->pos.y);
            game.my_matter -= SCRAPS_TO_SPAWN;
        }
    }
}

/*
** Move units
*/

int ai_rush_top = 0, ai_rush_middle = 0, ai_rush_bottom = 0;

void moveUnits(void)
{
    fprintf(stderr, "\n### Move units [%d Max] ###\n\n", game.units_count);

    for (int i = 0; i < game.units_count; i++)
    {
        unit_t cUnit = units[i];

        /*
        ** IA Type:
        ** Explorer: 0 -> get all empty scraps
        ** Attacker: 1 -> get all enemy scraps
        ** Rusher: 2 -> Rush to ennemy base -> try to split top middle and bottom side to place recyclers [DEFAULT]
        */
        int ia_type = 2;

        // Assign AI Type
        {
            int nbOppCellAround = 0;

            int x = 0, y = 0;
            FOR_IN_RANGE(x, y, 1)
            {
                if (isValidPos((vec_t){cUnit.pos.x + x, cUnit.pos.y + y}) == false)
                    continue;
                if (LOOP_CHECKER(cUnit.pos.x + x, cUnit.pos.y + y) == true)
                    continue;
                cell_t *cell = &game.map[cUnit.pos.y + y][cUnit.pos.x + x];

                if (isValidCell(cell) == false)
                    continue;
                if (cell->owner == OPP_CELL)
                {
                    nbOppCellAround++;
                }
            }

            if (game.game_turn < 15)
                ia_type = 2;
            else if (nbOppCellAround > 0)
                ia_type = 1;
            else
                ia_type = 0;
            fprintf(stderr, "\t- IA [%d, %d] Type: %d\n", cUnit.pos.x, cUnit.pos.y, ia_type);
        }

        vec_t next_pos = (vec_t){-1, -1};

    switch_ai:

        switch (ia_type)
        {
        /*
        ** Find Nearest Empty Cell
        */
        case 0:
        {
            vec_t nearest = (vec_t){-1, -1};
            int distance = INT_MAX;

            int x = 0, y = 0;
            FOR_EACH_CELLS(x, y)
            {
                cell_t *cell = &game.map[y][x];

                if (isValidCell(cell) == false)
                    continue;
                if (cell->owner == EMPTY_CELL)
                {
                    int new_distance = getDistanceTo(cUnit.pos, cell->pos);
                    if (new_distance < distance)
                    {
                        distance = new_distance;
                        nearest = cell->pos;
                    }
                }
            }
            next_pos = nearest;
            if (next_pos.x == -1 && next_pos.y == -1)
            {
                ia_type = 1;
                goto switch_ai;
            }
            break;
        }
        /*
        ** Find Nearest Opponent Cell
        ** - Check if unit can go to the cell without dying
        ** - Check if cell is reachable
        */
        case 1:
        {
            vec_t nearest = (vec_t){-1, -1};
            int distance = INT_MAX;

            int x = 0, y = 0;
            FOR_EACH_CELLS(x, y)
            {
                cell_t *cell = &game.map[y][x];
                if (cell->owner == OPP_CELL)
                {
                    if (cell->units > game.map[cUnit.pos.y][cUnit.pos.x].units)
                        continue;

                    int new_distance = getDistanceTo(cUnit.pos, cell->pos);
                    if (new_distance < distance)
                    {
                        distance = new_distance;
                        nearest = cell->pos;
                    }
                }
            }
            next_pos = nearest;
            break;
        }
        /*
        ** Rush to enemy base
        ** Spread units on 3 lines to place recyclers (top, middle, bottom)
        */
        case 2:
        {
            int split_attribute = 0;

            split_attribute = rand() % 3;

            if (split_attribute == 0)
                ai_rush_top++;
            else if (split_attribute == 1)
                ai_rush_middle++;
            else if (split_attribute == 2)
                ai_rush_bottom++;

            switch (split_attribute)
            {
            case 0:
                next_pos = (MAP_TOP_CENTER);
                break;
            case 1:
                next_pos = (MAP_MIDDLE_CENTER);
                break;
            case 2:
                next_pos = (MAP_BOTTOM_CENTER);
                break;
            }
        }
        }

        fprintf(stderr, "\t- MOVE 1 %d %d %d %d\n", cUnit.pos.x, cUnit.pos.y, next_pos.x, next_pos.y);

        if (isValidPos(next_pos) == true)
        {
            printf("MOVE 1 %d %d %d %d;", cUnit.pos.x, cUnit.pos.y, next_pos.x, next_pos.y);
        }
    }
}

/*******************************************************************************
 *                                MAIN FUNCTION                                *
 ******************************************************************************/

int main()
{
    /* Get Map Size */
    scanf("%d%d", &game.width, &game.height);

    static bool __init = false;

    srand(time(NULL));

    game.game_turn = 0;
    // game loop
    while (1)
    {
        scanf("%d%d", &game.my_matter, &game.opp_matter);

        bzero(game.map, sizeof(game.map));
        for (int i = 0; i < game.height; i++)
        {
            bzero(game.map[i], sizeof(game.map[i]));
        }

        game.units_count = 0;
        game.recycler_count = 0;

        ai_rush_top = 0, ai_rush_middle = 0, ai_rush_bottom = 0;

        for (int y = 0; y < game.height; y++)
        {
            for (int x = 0; x < game.width; x++)
            {
                cell_t *cell = &game.map[y][x];
                scanf("%d%d%d%d%d%d%d", &cell->scrap_amount, &cell->owner, &cell->units, &cell->recycler, &cell->can_build, &cell->can_spawn, &cell->in_range_of_recycler);

                cell->pos.x = x;
                cell->pos.y = y;

                if (cell->owner == OWN_CELL)
                {
                    if (cell->recycler == 1)
                        game.recycler_count += 1;
                    else if (cell->units > 0)
                        game.units_count += cell->units;
                }
            }
        }

        if (__init == false)
        {
            __init = true;
            setBasesLocations();
        }

        updateUnits();

        // path_t *path = pathSearch((vec_t){0, 2}, (vec_t){14, 2});

        // if (!path)
        //     fprintf(stderr, "No path found\n");
        // for (int i = 0; i < path->path_len; i++)
        //     fprintf(stderr, "%d %d\n", path->path[i].x, path->path[i].y);

        /* 1. Build Recyclers */
        {
            buildRecyclers();
        }

        /* 2. Spawn Units */
        {
            spawnUnits();
        }

        /* 3. Move Units */
        {
            moveUnits();
        }

        // printf("WAIT\n");
        printf("\n");
        game.game_turn++;
    }
    return (0);
}

// todo:
// - ne pas faire spawn les units si il y a deja toute les cases dans un ilot
// - stop a un moment le spawn des recyclers
// - ne pas faire spawn les recyclers si il y a deja toute les cases dans un ilot
// - essayer de ne pas s auto block avec les recyclers

// Seeds:
// island 1 : seed=6172162294877792000
// Little: seed=-768181820591969300

/*
Strat:
- avancer jusqu'au milieur de la map avec la majorité des units
    - En laisser 0 vers notre base (tout avancer !)
    - Spread a fond pour que quand on arrive au milieu une grosse ligne d'unité soit présente

- Si un ennemie est proche, vérifier si les unités peuvent ensemble le tuer / capturer le point
- Si oui, les unités continuent d'avancer ver la base ennemie.
- Si non, placer des recyclers pour bloquer l'ennemie et continuer d'avancer vers la base ennemie.

- S'il y a trop d'ennemie a coter, placer full recycler, terran strat pour faire un mur vertical

- grandes map, placer des recyclers au début pour agrandir l'armée et placer plus de recyclers.

- les unités esquivent les zones qui vont être détruites par le recycler le prochain tour.

*/