#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <random>

using namespace std;

/*******************************************************************************
 *                                 STRUCTURES                                  *
 ******************************************************************************/

typedef enum e_ai_type
{
    AI_EXPLORER, // Explore the map to get all scraps
    AI_ATTACKER, // Attack the opponent
    AI_DEFENDER, // Defend the base limits
    AI_TRAPPER,  // Trap the opponent
} ai_type_t;

typedef struct s_vec
{
    int x;
    int y;
} vec_t;

typedef struct s_unit
{
    int x;
    int y;
} unit_t;

typedef struct s_ai
{
    vec_t pos;
    ai_type_t type;
} ai_t;

typedef struct s_map
{
    int scrap_amount;         // amount of scrap
    int owner;                // 1 = me, 0 = foe, -1 = neutral
    int units;                // number of units
    int recycler;             // 1 = recycler, 0 = no recycler
    int can_build;            // 1 = can build, 0 = can't build
    int can_spawn;            // 1 = can spawn, 0 = can't spawn
    int in_range_of_recycler; // 1 = in range of recycler, 0 = not in range of recycler

    int x; // x position
    int y; // y position
} map_t;

/*******************************************************************************
 *                              GLOBAL VARIABLES                               *
 ******************************************************************************/

int map_width = 0;
int map_height = 0;

int matter = 0;
int opponent_matter = 0;

int units = 0;
int opponent_units = 0;

int owning_cells = 0;

int turn = 0;

/* All Actions for the turn */
vector<string> actions;

/* All AI */
vector<ai_t> ai;

#define MAX_ENTITY_SPAWN() (matter / 10)

/*******************************************************************************
 *                               UTILS FUNCTONS                                *
 ******************************************************************************/

static float getDistanceTo(vec_t const &a, vec_t const &b)
{
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}

static int random(int min, int max)
{
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(min, max);
    return dis(gen);
}

static void addAction(string const &action)
{
    actions.push_back(action);
}

static void processActions(void)
{
    if (actions.empty() == true)
    {
        cout << "WAIT" << endl;
        return;
    }

    uint32_t i = 0;
    for (string action : actions)
    {
        cout << action;
        if (i < actions.size() - 1)
            cout << ";";
        i++;
    }
    cout << endl;
    actions.clear();
}

/*******************************************************************************
 *                                  CLASS MAP                                  *
 ******************************************************************************/

class Map
{
private:
    vector<vector<map_t>> map;

public:
    Map()
    {
        map.resize(map_height);
        for (int i = 0; i < map_height; i++)
            map[i].resize(map_width);
    }

    void updateMap(void)
    {
        for (int i = 0; i < map_height; i++)
        {
            for (int j = 0; j < map_width; j++)
            {
                cin >> map[i][j].scrap_amount >> map[i][j].owner >> map[i][j].units >> map[i][j].recycler >> map[i][j].can_build >> map[i][j].can_spawn >> map[i][j].in_range_of_recycler;
                cin.ignore();

                map[i][j].x = j;
                map[i][j].y = i;
            }
        }
        owning_cells = getOwningCells();
        cerr << "Owning cells: " << owning_cells << endl;
    }

    uint32_t getOwningCells(void)
    {
        uint32_t __owning_cells = 0;

        for (auto &line : map)
        {
            for (auto &cell : line)
            {
                if (cell.owner == 1)
                    __owning_cells++;
            }
        }
        return __owning_cells;
    }

    vector<vector<map_t>> getMap(void)
    {
        return map;
    }

    map_t getMapCell(vec_t const &vec)
    {
        return map[vec.y][vec.x];
    }

    void updateMapCell(map_t const &cell, vec_t vec)
    {
        map[vec.y][vec.x] = cell;
    }

    vec_t findNearestOpponentCase(vec_t const &vec)
    {
        vector<vector<map_t>> cmap = getMap();
        vector<vec_t> __opponent_cases;
        vec_t __nearest = {-1, -1};
        float __distance = 0;

        for (auto &line : cmap)
        {
            for (auto &cell : line)
            {
                if (cell.owner == 0)
                {
                    __opponent_cases.push_back({cell.x, cell.y});
                }
            }
        }

        for (auto &opponent_case : __opponent_cases)
        {
            if (__distance == 0)
            {
                __distance = getDistanceTo(vec, opponent_case);
                __nearest = opponent_case;
            }
            else
            {
                float __tmp_distance = getDistanceTo(vec, opponent_case);
                if (__tmp_distance < __distance)
                {
                    __distance = __tmp_distance;
                    __nearest = opponent_case;
                }
            }
        }
        return __nearest;
    }

    vector<map_t> findNearestCells(vec_t const &vec, uint32_t range)
    {
        vector<vector<map_t>> cmap = getMap();
        vector<map_t> __nearest_cells;
        uint32_t __nb = 0;

        for (auto &line : cmap)
        {
            for (auto &cell : line)
            {
                if (getDistanceTo(vec, {cell.x, cell.y}) <= range)
                {
                    __nearest_cells.push_back(cell);
                    __nb++;
                }
            }
        }
        return __nearest_cells;
    }

    uint32_t getNBEntityAround(vec_t const &vec, uint32_t range)
    {
        vector<vector<map_t>> cmap = getMap();
        uint32_t __nb = 0;

        for (auto &line : cmap)
        {
            for (auto &cell : line)
            {
                if (getDistanceTo(vec, {cell.x, cell.y}) <= range && cell.owner == 1)
                {
                    __nb += cell.units;
                }
            }
        }
        return __nb;
    }

    uint32_t getNBOpponentEntityAround(vec_t const &vec, uint32_t range)
    {
        vector<vector<map_t>> cmap = getMap();
        uint32_t __nb = 0;

        for (auto &line : cmap)
        {
            for (auto &cell : line)
            {
                if (getDistanceTo(vec, {cell.x, cell.y}) <= range && cell.owner == 0)
                {
                    __nb += cell.units;
                }
            }
        }
        return __nb;
    }
};

/*******************************************************************************
 *                                 CLASS BUILD                                 *
 ******************************************************************************/

class Build
{
private:
    Map _map;
    uint32_t _nb_recycler;

public:
    Build(Map const &map)
    {
        _map = map;
        _nb_recycler = getNBRecycler();
    }
    uint32_t getNBRecycler(void)
    {
        vector<vector<map_t>> cmap = _map.getMap();
        uint32_t __nb = 0;

        for (auto &line : cmap)
        {
            for (auto &cell : line)
            {
                if (cell.recycler == 1)
                    __nb++;
            }
        }
        return __nb;
    }

    uint32_t getBuildTotalScrap(vec_t const &placement)
    {
        vector<vector<map_t>> cmap = _map.getMap();

        uint32_t __total = 0;

        __total += cmap[placement.y][placement.x].scrap_amount;
        if (placement.x + 1 < map_width)
            __total += cmap[placement.y][placement.x + 1].scrap_amount;
        if (placement.x - 1 >= 0)
            __total += cmap[placement.y][placement.x - 1].scrap_amount;
        if (placement.y + 1 < map_height)
            __total += cmap[placement.y + 1][placement.x].scrap_amount;
        if (placement.y - 1 >= 0)
            __total += cmap[placement.y - 1][placement.x].scrap_amount;

        return __total;
    }

    vec_t getBestBuildPlacement(void)
    {
        vector<vector<map_t>> cmap = _map.getMap();

        vec_t __placement = {-1, -1};
        uint32_t __total = 0;
        float __distance = 0;

        for (auto &line : cmap)
        {
            for (auto &cell : line)
            {
                vec_t __tmp = {cell.x, cell.y};
                if (cell.can_build == 1)
                {
                    uint32_t __best_total = getBuildTotalScrap(__tmp);
                    if (__total > __best_total)
                        continue;
                    __total = __best_total;
                    __placement = __tmp;
                }
            }
        }
        std::cerr << "Best build placement: " << __placement.x << " " << __placement.y << " total: " << __total << std::endl;
        return __placement;
    }

    vec_t buildPlacement(void)
    {
        return (getBestBuildPlacement());
    }

    bool shouldBuild(void)
    {
        if (matter >= 10 && _nb_recycler < 1)
            return true;
        return false;
    }
};

/*******************************************************************************
 *                                CLASS SPAWNER                                *
 ******************************************************************************/

class Spawn
{
private:
    Map _map;

    bool canSpawn(map_t const &cell)
    {
        if (cell.can_spawn == 1 && cell.owner == 1 && cell.units == 0 && cell.recycler == 0)
            return true;
        return false;
    }

    /*
    ** Find best location
    ** 1. Find nearest opponent case
    ** 2. Get number of 0 cases around
    ** 3. Number of current entity
    */
    vec_t getBestSpawnLocation(void)
    {
        vector<vector<map_t>> cmap = _map.getMap();
        vector<pair<vec_t, uint32_t>> __best_locations;

        for (auto const &line : cmap)
        {
            for (auto const &cell : line)
            {
                if (canSpawn(cell) == true)
                {
                    vec_t __placement = {cell.x, cell.y};
                    // vec_t __nearest = _map.findNearestOpponentCase(__placement);
                    vector<map_t> nb_zero = _map.findNearestCells(__placement, 1);
                    uint32_t __nb_0 = 0;

                    for (auto const &zero : nb_zero)
                    {
                        if (zero.owner == 0)
                            __nb_0++;
                    }

                    uint32_t __nb_entity = _map.getNBOpponentEntityAround(__placement, 1);
                    uint32_t __score = __nb_0 + __nb_entity;
                    __best_locations.push_back({__placement, __score});
                }
            }
        }

        pair<vec_t, uint32_t> __best = {vec_t{-1, -1}, 0};
        for (auto const &cell : __best_locations)
        {
            if (cell.second > 0)
                cerr << "Best location: " << cell.first.x << " " << cell.first.y << " score: " << cell.second << endl;
            if (cell.second > __best.second || __best.second == 0)
                __best = cell;
        }
        return __best.first;
    }

public:
    Spawn(Map const &map)
    {
        _map = map;
    }

    vector<vec_t> spawnEntities(void)
    {
        vector<vector<map_t>> cmap = _map.getMap();
        vector<vec_t> __placements;

        cerr << "MAX_ENTITY_SPAWN: " << MAX_ENTITY_SPAWN() << endl;
        for (uint32_t i = 0; i < MAX_ENTITY_SPAWN(); i++)
        {
            vec_t __placement = getBestSpawnLocation();
            if (__placement.x != -1 && __placement.y != -1)
                __placements.push_back(__placement);
        }
        return __placements;

        for (auto const &line : cmap)
        {
            for (auto const &cell : line)
            {
                cerr << "Can Spawn: " << canSpawn(cell) << endl;
                if (canSpawn(cell) == true)
                {
                    vec_t __placement = {cell.x, cell.y};
                    // return __placement;
                }
            }
        }
    }

    bool shouldSpawn(void)
    {
        return (true);
    }
};

/*******************************************************************************
 *                                 CLASS UNIT                                  *
 ******************************************************************************/

class Units
{
private:
    vector<unit_t> _units;
    vector<unit_t> _opponent_units;

    Map _map;

    uint32_t getNBOpponentCellAtLocation(vec_t const &location, uint32_t range)
    {
        auto const &cmap = _map.getMap();
        uint32_t __nb = 0;

        for (auto const &line : cmap)
        {
            for (auto const &cell : line)
            {
                if (cell.owner == 0 && cell.units > 0)
                {
                    vec_t __cell = {cell.x, cell.y};
                    if (getDistanceTo(__cell, location) <= range)
                        __nb++;
                }
            }
        }
        return __nb;
    }

public:
    Units(Map const &map) : _map(map)
    {
        int owner_unit_count = 0;
        int opponent_unit_count = 0;

        vector<vector<map_t>> cmap = _map.getMap();

        uint32_t x = 0, y = 0;
        for (vector<map_t> row : cmap)
        {
            x = 0;
            for (map_t cell : row)
            {
                if (cell.units == 0)
                {
                    x++;
                    continue;
                }
                if (cell.owner == 1)
                {
                    _units.push_back({x, y});
                    owner_unit_count += cell.units;
                }
                else if (cell.owner == 0)
                {
                    _opponent_units.push_back({x, y});
                    opponent_unit_count += cell.units;
                }
                x++;
            }
            y++;
        }
        units = owner_unit_count;
        opponent_units = opponent_unit_count;
    }

    /*
    ** Find best next movement
    ** 1. Find nearest cell with huge opponent cases around
    */
    vec_t findBestNextMovement(unit_t const &unit)
    {
        return _map.findNearestOpponentCase({unit.x, unit.y});

        auto const &cmap = _map.getMap();
        vector<pair<vec_t, int64_t>> __best_locations;

        for (auto const &line : cmap)
        {
            for (auto const &cell : line)
            {
                if (cell.scrap_amount == 0)
                    continue;

                int64_t __score = 0;
                double __dist_to_cell = getDistanceTo(vec_t{cell.x, cell.y}, vec_t{unit.x, unit.y});

                __score += getNBOpponentCellAtLocation(vec_t{cell.x, cell.y}, 1) * 2;
                __score -= __dist_to_cell;
                // __score += _map.findNearestCells(vec_t{cell.x, cell.y}, 1).size();

                __best_locations.push_back({vec_t{cell.x, cell.y}, __score});
                cerr << "Cell: " << cell.x << " " << cell.y << " nb: " << __score << endl;
            }
        }

        pair<vec_t, int64_t> __best = {vec_t{-1, -1}, 0};
        for (auto const &cell : __best_locations)
        {
            if (cell.second > 0)
                cerr << "Best location: " << cell.first.x << " " << cell.first.y << " score: " << cell.second << endl;
            if (cell.second > __best.second || __best.second == 0)
                __best = cell;
        }
        return __best.first;
    }

    vector<unit_t> getUnits(void)
    {
        return _units;
    }

    vector<unit_t> getOpponentUnits(void)
    {
        return _opponent_units;
    }
};

/*******************************************************************************
 *                                  CLASS AI                                   *
 ******************************************************************************/

class AI
{
private:
    Map _map;
    vector<ai_t> _ais;

    const uint32_t _ai_type_00_percent = 15;
    const uint32_t _ai_type_01_percent = 50;
    const uint32_t _ai_type_02_percent = 35;
    const uint32_t _ai_type_03_percent = 15;

    /*
    ** Assign AI type
    ** 0: Explorer
    ** 1: Attacker
    ** 2: Defender
    ** 3: Trapper
    */
    ai_type_t assignAiType(void)
    {
        if (rand() % 100 < _ai_type_00_percent)
            return AI_EXPLORER;
        else if (rand() % 100 < _ai_type_01_percent)
            return AI_ATTACKER;
        else if (rand() % 100 < _ai_type_02_percent)
            return AI_DEFENDER;
        else if (rand() % 100 < _ai_type_03_percent)
            return AI_TRAPPER;
        else
            return (assignAiType());
    }

public:
    AI() {}

    void assignMap(Map const &map)
    {
        _map = map;
    }

    void updateAi(vector<unit_t> const &units)
    {
        for (auto const &unit : units)
        {
            vec_t __pos = {unit.x, unit.y};
            bool _is_valid = false;
            for (auto const &ai : _ais)
            {
                if (ai.pos.x == __pos.x && ai.pos.y == __pos.y)
                {
                    _is_valid = true;
                    break;
                }
            }
            if (!_is_valid)
                _ais.erase(unit);
        }
    }

    void assignAI(vec_t const &pos)
    {
        // _ai.pos = pos;
        // _ai.type = assignAiType();
        // cerr << "Unit: " << pos.x << "-" << pos.y << "AI type: " << (_ai.type == AI_EXPLORER ? "Explorer" : _ai.type == AI_ATTACKER ? "Attacker"
        //                                                                                                 : _ai.type == AI_DEFENDER   ? "Defender"
        //                                                                                                 : _ai.type == AI_TRAPPER    ? "Trapper"
        //                                                                                                                             : "Unknown")
        //      << endl;
    }
};

/*******************************************************************************
 *                                MAIN FUNCTION                                *
 ******************************************************************************/

int main()
{
    /* Init Map Size */
    cin >> map_width >> map_height;
    cin.ignore();

    vector<AI> ais;

    /* Game Loop */
    while (1)
    {
        cin >> matter >> opponent_matter;
        cin.ignore();

        Map map;
        map.updateMap();

        for (AI const &ai : ais)
        {
            ai.assignMap(map);
        }
        Units units(map);

        for (auto unit : units.getUnits())
        {
            ai.assignAI({unit.x, unit.y});
        }

        // Build build(map);
        // if ((build.shouldBuild()) == true)
        // {
        //     vec_t placement = build.buildPlacement();
        //     cerr << "Placement: " << placement.x << " " << placement.y << endl;
        //     if (placement.x != -1 && placement.y != -1)
        //     {
        //         addAction("BUILD " + to_string(placement.x) + " " + to_string(placement.y));

        //         map_t cell = map.getMapCell({placement.x, placement.y});
        //         cell.recycler = 1;
        //         map.updateMapCell(cell, {placement.x, placement.y});
        //     }
        // }

        Spawn spawn(map);
        if ((spawn.shouldSpawn()) == true)
        {
            vector<vec_t> placements = spawn.spawnEntities();
            if (placements.empty() == true)
                cerr << "No Placement" << endl;
            for (auto placement : placements)
            {
                cerr << "Placement: " << placement.x << " " << placement.y << endl;
                if (placement.x != -1 && placement.y != -1)
                {
                    addAction("SPAWN 1 " + to_string(placement.x) + " " + to_string(placement.y));

                    // map_t cell = map.getMapCell({placement.x, placement.y});
                    // cell.units += 1;
                    // map.updateMapCell(cell, {placement.x, placement.y});
                }
            }
            // vec_t placement = spawn.spawnEntities();
            // cerr << "Placement: " << placement.x << " " << placement.y << endl;
            // if (placement.x != -1 && placement.y != -1)
            // {
            //     addAction("SPAWN 3 " + to_string(placement.x) + " " + to_string(placement.y));

            //     map_t cell = map.getMapCell({placement.x, placement.y});
            //     cell.units = 1;
            //     map.updateMapCell(cell, {placement.x, placement.y});
            // }
        }

        auto opponent_units = units.getOpponentUnits();
        uint32_t __i = 0;
        if (opponent_units.size() > 0)
        {
            for (auto unit : units.getUnits())
            {
                ai.assignAI({unit.x, unit.y});
                // const auto &opponent_unit = opponent_units[__i];

                // addAction("MOVE 1 " + to_string(unit.x) + " " + to_string(unit.y) + " " + to_string(opponent_unit.x) + " " + to_string(opponent_unit.y));
                // __i++;
                // if (__i >= opponent_units.size())
                //     __i = 0;
                // if (opponent_units.size() == 0)
                //     break;
                // vec_t __location = map.findNearestOpponentCase({unit.x, unit.y});

                // for (uint32_t i = 0; i < map.getMapCell({unit.x, unit.y}).units; i++)
                // {

                vec_t __location = units.findBestNextMovement(unit);

                addAction("MOVE 1 " + to_string(unit.x) + " " + to_string(unit.y) + " " + to_string(__location.x) + " " + to_string(__location.y));
                // }
            }
        }

        processActions();
        turn++;
    }
}