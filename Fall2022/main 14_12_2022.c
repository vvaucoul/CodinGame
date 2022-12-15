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

#define IS_OPP_RECYCLER(x, y) \
    (game.map[y][x].owner == OPP_CELL && game.map[y][x].recycler == 1)

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

    vec_t base_location;
    vec_t opp_base_location;

    int game_turn;
    int recycler_count;
    int units_count;

    cell_t map[MAP_MAX_Y][MAP_MAX_X];
} game_t;

game_t game;
unit_t *units = NULL;

/*******************************************************************************
 *                                    UTILS                                    *
 ******************************************************************************/

float getDistanceTo(vec_t pos1, vec_t pos2)
{
    return (sqrt(pow(pos1.x - pos2.x, 2) + pow(pos1.y - pos2.y, 2)));
}

bool isValidPos(vec_t pos)
{
    if (pos.x < 0 || pos.x >= game.width || pos.y < 0 || pos.y >= game.height)
        return (false);
    return (true);
}

bool isValidCell(cell_t *cell)
{
    if (cell->scrap_amount <= 0)
        return (false);
    return (true);
}

vec_t getBaseLocation()
{
    vec_t base_location = {-1, -1};

    /* Find our base location */
    {
        for (int y = 0; y < game.height; y++)
        {
            for (int x = 0; x < game.width; x++)
            {
                cell_t *cell = &game.map[y][x];
                if (cell->owner == OWN_CELL)
                {
                    if (game.map[y + 1][x].owner == OWN_CELL && game.map[y - 1][x].owner == OWN_CELL &&
                        game.map[y][x + 1].owner == OWN_CELL && game.map[y][x - 1].owner == OWN_CELL)
                    {
                        base_location.x = x;
                        base_location.y = y;
                        break;
                    }
                }
            }
        }
    }
    return (base_location);
}

vec_t getOppBaseLocation()
{
    vec_t opp_base_location = {-1, -1};

    /* Find Opponent base location */
    {
        for (int y = 0; y < game.height; y++)
        {
            for (int x = 0; x < game.width; x++)
            {
                cell_t *cell = &game.map[y][x];
                if (cell->owner == OPP_CELL)
                {
                    if (game.map[y + 1][x].owner == OPP_CELL && game.map[y - 1][x].owner == OPP_CELL &&
                        game.map[y][x + 1].owner == OPP_CELL && game.map[y][x - 1].owner == OPP_CELL)
                    {
                        opp_base_location.x = x;
                        opp_base_location.y = y;
                        break;
                    }
                }
            }
        }
    }
    return (opp_base_location);
}

cell_t *getTopCenterCell()
{
    vec_t center = {-1, -1};

    center.x = game.width / 2;
    center.y = 0;

    return (&game.map[center.y][center.x]);
}

cell_t *getBottomCenterCell()
{
    vec_t center = {-1, -1};

    center.x = game.width / 2;
    center.y = game.height - 1;

    return (&game.map[center.y][center.x]);
}

cell_t *getCenterCell()
{
    vec_t center = {-1, -1};

    center.x = game.width / 2;
    center.y = game.height / 2;

    return (&game.map[center.y][center.x]);
}

/* If cell is invalid, return the nearest valid cell */
cell_t *getNearestValidCell(cell_t *cell)
{
    if (isValidCell(cell))
        return (cell);

    vec_t pos = cell->pos;
    int dist = 1;

    while (dist < game.width)
    {
        for (int y = pos.y - dist; y <= pos.y + dist; y++)
        {
            for (int x = pos.x - dist; x <= pos.x + dist; x++)
            {
                if (isValidPos((vec_t){x, y}))
                {
                    cell_t *cell = &game.map[y][x];
                    if (isValidCell(cell))
                        return (cell);
                }
            }
        }
        dist++;
    }
    return (NULL);
}

/*******************************************************************************
 *                                 GAME UTILS                                  *
 ******************************************************************************/

/*
** BFS to find the distance between two positions in char **map
*/

int distToPos(vec_t start, vec_t end)
{
    int dist = 0;
    int visited[MAP_MAX_Y][MAP_MAX_X] = {0};
    int queue[MAP_MAX_Y * MAP_MAX_X][2] = {0};
    int queue_size = 0;
    int queue_start = 0;

    queue[queue_size][0] = start.x;
    queue[queue_size][1] = start.y;
    queue_size++;
    visited[start.y][start.x] = 1;

    while (queue_size > queue_start)
    {
        int x = queue[queue_start][0];
        int y = queue[queue_start][1];
        queue_start++;

        if (x == end.x && y == end.y)
            return (dist);

        if (game.map[y][x].scrap_amount <= 0)
            continue;

        if (x > 0 && !visited[y][x - 1])
        {
            queue[queue_size][0] = x - 1;
            queue[queue_size][1] = y;
            queue_size++;
            visited[y][x - 1] = 1;
        }
        if (x < game.width - 1 && !visited[y][x + 1])
        {
            queue[queue_size][0] = x + 1;
            queue[queue_size][1] = y;
            queue_size++;
            visited[y][x + 1] = 1;
        }
        if (y > 0 && !visited[y - 1][x])
        {
            queue[queue_size][0] = x;
            queue[queue_size][1] = y - 1;
            queue_size++;
            visited[y - 1][x] = 1;
        }
        if (y < game.height - 1 && !visited[y + 1][x])
        {
            queue[queue_size][0] = x;
            queue[queue_size][1] = y + 1;
            queue_size++;
            visited[y + 1][x] = 1;
        }
        dist++;
    }
    return (-1);
}

bool cellWillBeDestroyed(cell_t *cell)
{
    int __count = 0;

    vec_t pos = cell->pos;
    const int __current_scraps = cell->scrap_amount;

    if (cell->scrap_amount > 1)
        return (false);
    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            if (i == 0 && j == 0 || i == 1 && j == 1 || i == -1 && j == -1 || i == 1 && j == -1 || i == -1 && j == 1)
                continue;
            else
            {
                if (isValidPos((vec_t){pos.x + i, pos.y + j}))
                {
                    cell_t *cell = &game.map[pos.y + j][pos.x + i];

                    if (isValidCell(cell))
                    {
                        if (cell->recycler == 1)
                            return (true);
                    }
                }
            }
        }
    }
    return (false);
}

bool entityWillBeDestroyedNextTurn()
{
}

static int getUnitsCount(void)
{
    int count = 0;

    for (int y = 0; y < game.height; y++)
    {
        for (int x = 0; x < game.width; x++)
        {
            cell_t *cell = &game.map[y][x];
            if (cell->owner == OWN_CELL)
                count += cell->units;
        }
    }
    return (count);
}

void updateUnits(void)
{
    if (units = NULL)
        units = malloc(sizeof(unit_t) * getUnitsCount() + 1);
    else
    {
        free(units);
        units = malloc(sizeof(unit_t) * getUnitsCount() + 1);
    }

    int i = 0;
    for (int y = 0; y < game.height; y++)
    {
        for (int x = 0; x < game.width; x++)
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
}

/*
** Check if the cell is in an island
** Improve it with bfs or A*
*/
bool cellIsInIsland(vec_t pos)
{
    if (isValidPos(pos) == true && game.map[pos.y][pos.x].scrap_amount > 0)
    {
        for (int i = -1; i <= 1; i++)
        {
            for (int j = -1; j <= 1; j++)
            {
                if (i == 0 && j == 0 || i == 1 && j == 1 || i == -1 && j == -1 || i == 1 && j == -1 || i == -1 && j == 1)
                    continue;
                else
                {
                    if (isValidPos((vec_t){pos.x + i, pos.y + j}) == false)
                        return (false);
                    else
                    {
                        // fprintf(stderr, "Checking %d %d\n", pos.y + i, pos.x + j);
                        cell_t *cell = &game.map[pos.y + j][pos.x + i];
                        if (cell->scrap_amount > 0)
                            return (cellIsInIsland((vec_t){pos.x + i, pos.y + j}));
                        else
                            return (true);
                    }
                }
            }
        }
    }
    return (true);
}

int nbEntitiesInOppBase(void)
{
    int __count = 0;
    /* Check if units is in opp side */
    {
        int x = 0, y = 0;
        FOR_EACH_CELLS(x, y)
        {
            cell_t *cell = &game.map[y][x];
            if (cell->owner == OWN_CELL && cell->units > 0)
            {
                float dist_to_opp = getDistanceTo((vec_t){x, y}, game.opp_base_location);
                float dist_to = getDistanceTo((vec_t){x, y}, game.base_location);

                if (dist_to_opp - 2 < dist_to)
                    __count += cell->units;
            }
        }
    }
    return (__count);
}

/*******************************************************************************
 *                                   AI CODE                                   *
 ******************************************************************************/

static bool recyclerInRange(vec_t pos, int range)
{
    for (int i = -range; i <= range; i++)
    {
        for (int j = -range; j <= range; j++)
        {
            if (i == 0 && j == 0)
                continue;
            if (isValidPos((vec_t){pos.x + i, pos.y + j}))
            {
                cell_t *cell = &game.map[pos.y + j][pos.x + i];
                if (cell->recycler)
                    return (true);
            }
        }
    }
    return (false);
}

static int checkValidCellsAround(vec_t pos, int range)
{
    int count = 0;

    for (int i = -range; i <= range; i++)
    {
        for (int j = -range; j <= range; j++)
        {
            if (i == 0 && j == 0)
                continue;
            if (isValidPos((vec_t){pos.x + i, pos.y + j}))
            {
                cell_t *cell = &game.map[pos.y + j][pos.x + i];
                if (isValidCell(cell) && cell->scrap_amount > 0)
                    count++;
            }
        }
    }
    return (count);
}

static int getNbEnnemyCellsInDist(vec_t pos, int range)
{
    int count = 0;

    for (int i = -range; i <= range; i++)
    {
        for (int j = -range; j <= range; j++)
        {
            if (i == 0 && j == 0)
                continue;
            if (isValidPos((vec_t){pos.x + i, pos.y + j}))
            {
                cell_t *cell = &game.map[pos.y + j][pos.x + i];
                if (cell->owner == OPP_CELL)
                    count++;
            }
        }
    }
    return (count);
}

static float getDistanceToNearestEnnemyCell(vec_t pos)
{
    float distance = INT_MAX;

    for (int y = 0; y < game.height; y++)
    {
        for (int x = 0; x < game.width; x++)
        {
            cell_t *cell = &game.map[y][x];
            if (cell->owner == OPP_CELL)
            {
                float dist = getDistanceTo(pos, (vec_t){x, y});
                if (dist < distance)
                    distance = dist;
            }
        }
    }
    return (distance);
}

/*
** AI Build:
*/

vec_t findBestBuildLocation()
{
    /*
    Try Strat to build recycler center of map to block ennemy
    */

    fprintf(stderr, "\n### Finding best build location ###\n");

    vec_t best_pos = {-1, -1};

    int best_score = 0;

    int x = 0, y = 0;
    FOR_EACH_CELLS(x, y)
    {
        vec_t pos = {x, y};
        int score = 0;
        cell_t *cell = &game.map[y][x];
        if (cell->can_build)
        {
            int nbEnnemyCells = 0;
            int nbValidCells = 0;
            int nbEnnemyAround = 0;
            int nnEntitesAround = 0;
            for (int i = -1; i <= 1; i++)
            {
                for (int j = -1; j <= 1; j++)
                {
                    if (i == 0 && j == 0 || i == 1 && j == 1 || i == -1 && j == -1 || i == 1 && j == -1 || i == -1 && j == 1)
                        continue;
                    else
                    {
                        if (isValidPos((vec_t){x + i, y + j}))
                        {
                            cell_t *cell = &game.map[y + j][x + i];

                            if (isValidCell(cell) == false)
                                continue;
                            if (cell->scrap_amount > 0)
                                nbValidCells++;
                            if (cell->owner == OPP_CELL)
                                nbEnnemyCells++;
                            if (cell->owner == OPP_CELL)
                                nbEnnemyAround += cell->units;
                            if (cell->owner == OWN_CELL)
                                nnEntitesAround += cell->units;
                        }
                    }
                }
            }
            score = nbValidCells * 5 + nbEnnemyCells * 15 + nbEnnemyAround * 50 - nnEntitesAround * 25;
            fprintf(stderr, "- %d %d [%d]\n", x, y, score);
            if (score < 75)
                continue;

            if (score > best_score)
            {
                best_score = score;
                best_pos = pos;
            }
        }
    }
    return (best_pos);

    /*
    vec_t best_pos = {-1, -1};
    int best_score = 0;

    int x = 0, y = 0;
    FOR_EACH_CELLS(x, y)
    {
     vec_t pos = {x, y};
     int score = 0;
     cell_t *cell = &game.map[y][x];
     if (cell->can_build)
     {
         for (int i = -1; i <= 1; i++)
         {
             for (int j = -1; j <= 1; j++)
             {
                 if (i == 0 && j == 0)
                     continue;
                 if (isValidPos((vec_t){x + i, y + j}))
                 {
                     cell_t *cell = &game.map[y + j][x + i];
                     score += cell->scrap_amount;
                     score += cell->owner == EMPTY_CELL ? 1 : 0;
                     score += cell->owner == OPP_CELL ? 1 : 0;
                     score += cell->units;
                     score += getNbEnnemyCellsInDist(pos, 2);
                     score -= getDistanceToNearestEnnemyCell(pos);
                     score += MAP_RATIO;

                     if (cell->in_range_of_recycler || recyclerInRange(pos, 2) || checkValidCellsAround(pos, 1) < 3)
                         score = 0;
                     // || cellIsInIsland(pos) == true

                     // fprintf(stderr, "Check is island [%d:%d]\n", pos.x, pos.y);
                     // if (pos.x == 1 && pos.y == 8)
                     if ((cellIsInIsland(pos)) == true)
                     {
                         fprintf("Island [%d:%d]\n", pos.x, pos.y);
                         score = 0;
                     }

                     if (score > best_score)
                     {
                         best_score = score;
                         best_pos = pos;
                     }
                 }
             }
         }
     }
    }

    return (best_pos);
    */
}

bool shouldWaitForRecycler()
{
    // if (game.my_matter < SCRAPS_TO_BUILD)
    //     return (true);
    // if (game.recycler_count - game.units_count < 1)
    //     return (true);

    if (game.units_count - 10 > game.recycler_count) // 10: arbitrary value
        return (false);

    /* Check if units is in opp side */
    {
        int x = 0, y = 0;
        FOR_EACH_CELLS(x, y)
        {
            cell_t *cell = &game.map[y][x];
            if (cell->owner == OWN_CELL && cell->units > 0)
            {
                float dist_to_opp = getDistanceTo((vec_t){x, y}, game.opp_base_location);
                float dist_to = getDistanceTo((vec_t){x, y}, game.base_location);

                if (dist_to_opp < dist_to)
                    return (true);
            }
        }
    }
    return (false);
}

/*
** AI Spawn:
*/

vec_t *findBestSpawnLocation()
{
    if (game.my_matter < SCRAPS_TO_SPAWN)
        return (NULL);
    else
    {
        fprintf(stderr, "\n### Find best spawn location ###\n\n");
        int nb_spawnable = game.my_matter / SCRAPS_TO_SPAWN;
        vec_t *pos = calloc(nb_spawnable + 1, sizeof(vec_t));

        for (int i = 0; i < nb_spawnable + 1; i++)
        {
            pos[i].x = -1;
            pos[i].y = -1;
        }

        /*
        ** Best Spawn:
        ** 1. Cell with the most empty cells around
        ** 2. Cell with the most scrap
        ** 3. Cell with the most opponent units around
        */

        for (int ccell = 0; ccell < nb_spawnable; ccell++)
        {
            cell_t *best_cell = NULL;
            int best_score = 0;

            int x = 0, y = 0;
            FOR_EACH_CELLS(x, y)
            {
                cell_t *cell = &game.map[y][x];
                if (cell->can_spawn && isValidCell(cell))
                {
                    int nb_empty_cells = 0;
                    int nb_scrap = 0;
                    int nb_opp_units = 0;
                    int nb_opp_cells = 0;
                    int nb_units = 0;

                    int current_score = 0;

                    for (int i = -1; i <= 1; i++)
                    {
                        for (int j = -1; j <= 1; j++)
                        {
                            if (i == 0 && j == 0 || i == 1 && j == 1 || i == -1 && j == -1 || i == 1 && j == -1 || i == -1 && j == 1)
                            {
                                if (cell->owner == OWN_CELL)
                                    nb_units += cell->units;
                                continue;
                            }
                            vec_t pos = {x + i, y + j};
                            if (isValidPos(pos))
                            {
                                cell_t *cell = &game.map[pos.y][pos.x];
                                if (cell->owner == EMPTY_CELL && cell->scrap_amount > 0)
                                    nb_empty_cells++;
                                else if (cell->owner == OPP_CELL)
                                {
                                    nb_opp_cells++;
                                    nb_opp_units += cell->units;
                                }
                                else if (cell->owner == OWN_CELL)
                                {
                                    nb_units += cell->units;
                                }
                                nb_scrap += cell->scrap_amount;
                            }
                        }
                    }
                    current_score = nb_empty_cells * 40 + nb_scrap * 1 + nb_opp_cells * 100 + nb_opp_units * 60 + nb_units * 10;
                    current_score -= cellWillBeDestroyed(cell) ? 1000 : 0;
                    // current_score -= shouldWaitForRecycler() ? 1000 : 0; // Todo: Improve
                    fprintf(stderr, "Cell %d %d: %d %d %d | %d\n", x, y, nb_empty_cells, nb_scrap, nb_opp_cells, current_score);
                    if (current_score > best_score)
                    {
                        best_score = current_score;
                        best_cell = cell;
                    }
                }
            }
            if (best_cell != NULL)
            {
                pos[ccell].x = best_cell->pos.x;
                pos[ccell].y = best_cell->pos.y;
            }
        }
        return (pos);
    }
    return (NULL);
}

/*
** AI Movement:
*/

cell_t *target_cells[MAP_MAX_X * MAP_MAX_Y + 1] = {NULL};
int ai_units_focus_top = 0, ai_units_focus_bottom = 0, ai_units_focus_center = 0;

bool cellAlreadyTarget(cell_t *cell)
{
    for (int i = 0; i < MAP_MAX_X * MAP_MAX_Y + 1; i++)
    {
        if (target_cells[i] == cell)
            return (true);
    }
    return (false);
}

// Todo: Improve this function
// Use a BFS to find the best path or A* algorithm
bool cellIsReacheable(vec_t pos, cell_t *cell)
{
    vec_t target_pos = cell->pos;

    if (target_pos.x == pos.x && target_pos.y == pos.y)
        return (true);
    else
    {
        if (target_pos.x > pos.x)
        {
            pos.x++;
            if (isValidPos(pos) == false)
                return (false);
            if (game.map[pos.y][pos.x].scrap_amount <= 0 || game.map[pos.y][pos.x].recycler == 1)
                return (false);
            if (pos.x == target_pos.x && pos.y == target_pos.y)
            {
                return (true);
            }
            return (cellIsReacheable(pos, cell));
        }
        else if (target_pos.x < pos.x)
        {
            pos.x--;
            if (isValidPos(pos) == false)
                return (false);
            if (game.map[pos.y][pos.x].scrap_amount <= 0 || game.map[pos.y][pos.x].recycler == 1)
                return (false);
            if (pos.x == target_pos.x && pos.y == target_pos.y)
            {
                return (true);
            }
            return (cellIsReacheable(pos, cell));
        }
        else if (target_pos.y > pos.y)
        {
            pos.y++;
            if (isValidPos(pos) == false)
                return (false);
            if (game.map[pos.y][pos.x].scrap_amount <= 0 || game.map[pos.y][pos.x].recycler == 1)
                return (false);
            if (pos.x == target_pos.x && pos.y == target_pos.y)
            {
                return (true);
            }
            return (cellIsReacheable(pos, cell));
        }
        else if (target_pos.y < pos.y)
        {
            pos.y--;
            if (isValidPos(pos) == false)
                return (false);
            if (game.map[pos.y][pos.x].scrap_amount <= 0 || game.map[pos.y][pos.x].recycler == 1)
                return (false);
            if (pos.x == target_pos.x && pos.y == target_pos.y)
            {
                return (true);
            }
            return (cellIsReacheable(pos, cell));
        }
    }
    return (false);

    // if (target_pos.y > pos.y)
    // {
    //     pos.y++;
    //     if (isValidPos(pos) == false)
    //         return (false);
    //     if (game.map[pos.y][pos.x].scrap_amount <= 0 || game.map[pos.y][pos.x].recycler == 1)
    //         return (false);
    //     if (pos.x == target_pos.x && pos.y == target_pos.y)
    //     {
    //         return (true);
    //     }
    //     return (cellIsReacheable(pos, cell));
    // }
    // else if (target_pos.y < pos.y)
    // {
    //     pos.y--;
    //     if (isValidPos(pos) == false)
    //         return (false);
    //     if (game.map[pos.y][pos.x].scrap_amount <= 0 || game.map[pos.y][pos.x].recycler == 1)
    //         return (false);
    //     if (pos.x == target_pos.x && pos.y == target_pos.y)
    //     {
    //         return (true);
    //     }
    //     return (cellIsReacheable(pos, cell));
    // }
    // else if (target_pos.x > pos.x)
    // {
    //     pos.x++;
    //     if (isValidPos(pos) == false)
    //         return (false);
    //     if (game.map[pos.y][pos.x].scrap_amount <= 0 || game.map[pos.y][pos.x].recycler == 1)
    //         return (false);
    //     if (pos.x == target_pos.x && pos.y == target_pos.y)
    //     {
    //         return (true);
    //     }
    //     return (cellIsReacheable(pos, cell));
    // }
    // else if (target_pos.x < pos.x)
    // {
    //     pos.x--;
    //     if (isValidPos(pos) == false)
    //         return (false);
    //     if (game.map[pos.y][pos.x].scrap_amount <= 0 || game.map[pos.y][pos.x].recycler == 1)
    //         return (false);
    //     if (pos.x == target_pos.x && pos.y == target_pos.y)
    //     {
    //         return (true);
    //     }
    //     return (cellIsReacheable(pos, cell));
    // }

    // return (false);
}

vec_t findBestMoveLocation(unit_t unit)
{
    int ai_type = 0;
    /*
    ** AI Type:
    ** 0. Explorer: Move to the closest empty cell
    ** 1. Attack: Move to the closest opponent cell
    ** 2. Rusher: Move to ennemy base and attack (place recycler)
    */

    /* Select AI Type */
    {
        int nb_opp_units = 0;
        int nb_ally_units = 0;
        int nb_empty_cells = 0;
        int nb_scrap = 0;
        int nb_opp_cells = 0;

        int __range = 2;

        for (int i = -__range; i <= __range; i++)
        {
            for (int j = -__range; j <= __range; j++)
            {
                if (i == 0 && j == 0)
                    continue;
                vec_t pos = {unit.pos.x + i, unit.pos.y + j};
                if (isValidPos(pos))
                {
                    cell_t *cell = &game.map[pos.y][pos.x];

                    if (isValidCell(cell) == false)
                        continue;
                    if (cell->owner == EMPTY_CELL)
                        nb_empty_cells++;
                    else if (cell->owner == OWN_CELL)
                        nb_ally_units += cell->units;
                    else if (cell->owner == OPP_CELL)
                    {
                        nb_opp_units += cell->units;
                        if (cellIsReacheable(unit.pos, cell) == true)
                            nb_opp_cells++;
                    }
                    nb_scrap += cell->scrap_amount;
                }
            }
        }

        // Todo: Improve
        if (nbEntitiesInOppBase() < 3 ||game.game_turn < 15)
            ai_type = 2;
        else if (nb_opp_units > 0)
            ai_type = 1;
        else if (nb_opp_cells > 1)
            ai_type = 1;
        else
            ai_type = 0;
    }

switch_ai:

    fprintf(stderr, "AI %d %d: Type: %d\n", unit.pos.x, unit.pos.y, ai_type);

    vec_t best_pos = {-1, -1};
    float nearest_distance = INT_MAX;

    switch (ai_type)
    {

    /*
    ** AI Explorer
    ** Move to the closest empty cell to get all cells around
    */
    case 0:
    {
        int x = 0, y = 0;

        FOR_EACH_CELLS(x, y)
        {
            for (int i = -1; i <= 1; i++)
            {
                for (int j = -1; j <= 1; j++)
                {
                    if (i == 0 && j == 0)
                        continue;
                    vec_t pos = {x + i, y + j};
                    cell_t *cell = &game.map[pos.y + j][pos.x + i];

                    if (isValidPos(pos) && isValidCell(cell))
                    {
                        if (cell->owner == EMPTY_CELL)
                        {
                            if (cellAlreadyTarget(cell) == true)
                            {
                                if (getDistanceTo(unit.pos, cell->pos) >= 2)
                                    continue;
                                if (cellWillBeDestroyed(cell) == true)
                                    continue;
                            }

                            float distance = getDistanceTo(unit.pos, cell->pos);
                            if (distance < nearest_distance)
                            {
                                nearest_distance = distance;
                                best_pos = cell->pos;
                            }
                        }
                    }
                }
            }
        }
        fprintf(stderr, "AI EXPLORER %d %d: Move to %d %d, nearest_distance: %f\n", unit.pos.x, unit.pos.y, best_pos.x, best_pos.y, nearest_distance);
        if (best_pos.x == -1 && best_pos.y == -1)
        {
            ai_type = 1;
            goto switch_ai;
        }
        break;
    }

    /*
    ** AI Attack
    ** Move to the closest opponent cell to attack
    */
    case 1:
    {
        int x = 0, y = 0;

        FOR_EACH_CELLS(x, y)
        {
            cell_t *cell = &game.map[y][x];
            if (cell->owner == OPP_CELL && isValidCell(cell) && cell->recycler == 0)
            {
                // fprintf(stderr, "Focus: %d %d\n", cell->pos.x, cell->pos.y);
                bool __res = true;
                if (getDistanceTo(unit.pos, cell->pos) <= 1)
                    __res = cellIsReacheable(unit.pos, cell);
                // fprintf(stderr, "AI ATTACK %d %d: Cell %d %d is reacheable: %d\n", unit.pos.x, unit.pos.y, cell->pos.x, cell->pos.y, __res);

                if (__res == false)
                {
                    // fprintf(stderr, "AI ATTACK %d %d: Cell %d %d is not reacheable\n", unit.pos.x, unit.pos.y, cell->pos.x, cell->pos.y);
                    continue;
                }
                // fprintf(stderr, "[%d] [%d] to %d %d Distance: %d\n", unit.pos.x, unit.pos.y, cell->pos.x, cell->pos.y, distToPos(unit.pos, cell->pos));

                float distance = getDistanceTo(unit.pos, cell->pos);
                // float distance = distToPos(unit.pos, cell->pos);
                if (distance < nearest_distance)
                {
                    nearest_distance = distance;
                    best_pos = cell->pos;
                }
            }
        }
        fprintf(stderr, "\t- AI ATTACK %d %d: Move to %d %d, nearest_distance: %f\n", unit.pos.x, unit.pos.y, best_pos.x, best_pos.y, nearest_distance);
        break;
    }

    /*
    ** AI Rush:
    ** Split cells to rush entities.
    ** Split units, top center -> center center -> bottom center
    */
    case 2:
    {
        /*
        ** 0: Top
        ** 1: Center
        ** 2: Bottom
        */
        int __focus_side = -1;

        if (ai_units_focus_top == 0 && ai_units_focus_center == 0 && ai_units_focus_bottom == 0)
            __focus_side = 0;
        else if (ai_units_focus_center == 0 && ai_units_focus_bottom == 0)
            __focus_side = 1;
        else if (ai_units_focus_bottom == 0)
            __focus_side = 2;
        else
        {
            if (ai_units_focus_top < ai_units_focus_center && ai_units_focus_top < ai_units_focus_bottom)
                __focus_side = 0;
            else if (ai_units_focus_center < ai_units_focus_bottom)
                __focus_side = 1;
            else
                __focus_side = 2;
        }

        /* Check if unit is near to the focus side */
        switch (__focus_side)
        {
        case 0:
            if (getDistanceTo(unit.pos, getTopCenterCell()->pos) <= 1)
            {
                ai_type = 1;
                goto switch_ai;
            }
            break;
        case 1:
            if (getDistanceTo(unit.pos, getCenterCell()->pos) <= 1)
            {
                ai_type = 1;
                goto switch_ai;
            }
            break;
        case 2:
            if (getDistanceTo(unit.pos, getBottomCenterCell()->pos) <= 1)
            {
                ai_type = 1;
                goto switch_ai;
            }
            break;
        }

        fprintf(stderr, "AI RUSH: __focus_side: %d\n", __focus_side);

        int x = 0, y = 0;

        // Focus Side
        {
            cell_t *cell = NULL;

            switch (__focus_side)
            {
            case 0:
                cell = getTopCenterCell();
                break;
            case 1:
                cell = getCenterCell();
                break;
            case 2:
                cell = getBottomCenterCell();
                break;
            }

            if (isValidCell(cell) == false)
                cell = getNearestValidCell(cell);
            FOR_EACH_CELLS(x, y)
            {
                if (cellAlreadyTarget(cell) == true)
                {
                    cell = getNearestValidCell(cell);
                }
                // else if (cellIsReacheable(unit.pos, cell) == false)
                //     continue;
                if (isValidCell(cell) == true)
                {
                    best_pos = cell->pos;

                    switch (__focus_side)
                    {
                    case 0:
                        fprintf(stderr, "\t- Unit %d %d move to Top Center -> %d %d\n", unit.pos.x, unit.pos.y, best_pos.x, best_pos.y);
                        ai_units_focus_top++;
                        break;
                    case 1:
                        fprintf(stderr, "\t- Unit %d %d move to Center Center -> %d %d\n", unit.pos.x, unit.pos.y, best_pos.x, best_pos.y);
                        ai_units_focus_center++;
                        break;
                    case 2:
                        fprintf(stderr, "\t- Unit %d %d move to Bottom Center -> %d %d\n", unit.pos.x, unit.pos.y, best_pos.x, best_pos.y);
                        ai_units_focus_bottom++;
                        break;
                    }
                    goto ai_rush_end;
                }
            }
        }

    ai_rush_end:
        if (best_pos.x == -1 && best_pos.y == -1)
        {
            ai_type = 1;
            goto switch_ai;
        }
        break;
    }

    default:
        break;
    }

    // for (int y = 0; y < game.height; y++)
    // {
    //     for (int x = 0; x < game.width; x++)
    //     {
    //         cell_t *cell = &game.map[y][x];
    //         if (cell->owner == 0 || cell->owner == -1)
    //         {
    //             float distance = getDistanceTo(unit.pos, cell->pos);
    //             if (distance > nearest_distance)
    //             {
    //                 nearest_distance = distance;
    //                 best_pos = cell->pos;
    //             }
    //         }
    //     }
    // }

    int i = 0;

    while (target_cells[i] != NULL)
        i++;
    target_cells[i] = &game.map[best_pos.y][best_pos.x];
    target_cells[i + 1] = NULL;
    return (best_pos);
}

/*******************************************************************************
 *                                MAIN FUNCTION                                *
 ******************************************************************************/

int main()
{
    static bool __init = false;

    /* Get Map Size */
    scanf("%d%d", &game.width, &game.height);

    game.game_turn = 0;
    // game loop
    while (1)
    {
        scanf("%d%d", &game.my_matter, &game.opp_matter);

        bzero(game.map, sizeof(game.map));
        for (int i = 0; i < game.height; i++)
            bzero(game.map[i], sizeof(game.map[i]));

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
                        game.units_count += 1;
                }
            }
        }

        /* Init base location turn 1 */
        if (__init == false)
        {
            game.base_location = getBaseLocation();
            game.opp_base_location = getOppBaseLocation();

            fprintf(stderr, "Base Location: %d %d\n", game.base_location.x, game.base_location.y);
            fprintf(stderr, "Opp Base Location: %d %d\n", game.opp_base_location.x, game.opp_base_location.y);

            __init = true;
        }

        fprintf(stderr, "Dist -> %d\n", distToPos(game.base_location, game.opp_base_location));

        // vec_t build = findBestBuildLocation();

        // if (isValidPos(build) == true)
        // {
        //     printf("BUILD %d %d;", build.x, build.y);
        //     game.my_matter -= 10;
        // }

        vec_t build = (vec_t){-1, -1};
        do
        {
            vec_t build = findBestBuildLocation();

            /* No Build Location */
            if (isValidPos(build) == false)
                break;

            printf("BUILD %d %d;", build.x, build.y);
            game.my_matter -= 10;
        } while (isValidPos(build) == true);

        updateUnits();
        for (int i = 0; i < getUnitsCount(); i++)
        {
            vec_t unit_pos = units[i].pos;
            vec_t best_pos = findBestMoveLocation(units[i]);

            printf("MOVE 1 %d %d %d %d;", unit_pos.x, unit_pos.y, best_pos.x, best_pos.y);
        }

        vec_t *spawn = findBestSpawnLocation();

        if (spawn != NULL)
        {
            for (int i = 0; isValidPos(spawn[i]); i++)
            {
                printf("SPAWN 1 %d %d;", spawn[i].x, spawn[i].y);
                game.my_matter -= 10;
            }
        }

        /* Reset Ai Units Focus */
        bzero(target_cells, sizeof(target_cells));
        target_cells[0] = NULL;
        ai_units_focus_top = 0, ai_units_focus_bottom = 0, ai_units_focus_center = 0;

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
- peut être en laisser 1, 2 pour capturer notre coté de la map.

- Si un ennemie est proche, vérifier si les unités peuvent ensemble le tuer / capturer le point
- Si oui, les unités continuent d'avancer ver la base ennemie.
- Si non, ...

- S'il y a trop d'ennemie a coter, placer full recycler, terran strat pour faire un mur vertical

- grandes map, placer des recyclers au début pour agrandir l'armée et placer plus de recyclers.

- les unités esquivent les zones qui vont être détruites par le recycler le prochain tour.

*/