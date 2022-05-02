#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cmath>

using namespace std;

#define MIN_X 0
#define MIN_Y 0
#define MAX_X 17630
#define MAX_Y 9000
#define MAX_DISTANCE sqrt(pow(MAX_X, 2) + pow(MAX_Y, 2))
#define HERO_VIEW_DISTANCE 2200
#define BASE_VIEW_DISTANCE 6000

#define POIDS_DISTANCE 1
#define POIDS_DISTANCE_BASE 2
#define POIDS_DISTANCE_GUARD 2

#define POIDS_DISTANCE_OUTSIDE 4
#define POIDS_ALREADY_TARGET 10

enum e_player_type
{
    E_OWN,
    E_ENNEMY
};
enum e_entity_type
{
    E_MONSTER = 0,
    E_HERO = 1,
    E_ENNEMY_HERO = 2
};

typedef struct s_point
{
    int x;
    int y;
} t_point;
#define Point t_point

class Monster
{
public:
    // Unique identifier
    int _id;
    // 0=monster, 1=your hero, 2=opponent hero
    enum e_entity_type _type;
    // Position of this entity
    int _x;
    int _y;
    // Ignore for this league; Count down until shield spell fades
    int _shield_life;
    // Ignore for this league; Equals 1 when this entity is under a control spell
    int _is_controlled;
    // Remaining health of this monster
    int _health;
    // Trajectory of this monster
    int _vx;
    int _vy;
    // 0=monster with no target yet, 1=monster targeting a base
    int _near_base;
    // Given this monster's trajectory, is it a threat to 1=your base, 2=your opponent's base, 0=neither
    int _threat_for;

public:
    Monster(int id, int type, int x, int y, int shield_life, int is_controlled, int health, int vx, int vy, int near_base, int threat_for)
    {
        _id = id;
        _type = (e_entity_type)type;
        _x = x;
        _y = y;
        _shield_life = shield_life;
        _is_controlled = is_controlled;
        _health = health;
        _vx = vx;
        _vy = vy;
        _near_base = near_base;
        _threat_for = threat_for;
    };
};
class Hero
{
public:
    enum e_entity_type _type;
    short _ia_type; // 0 = defense, 1 = attack, 2 = all, 3 = none
    int _id;
    int _x;
    int _y;

    bool _is_targeting_entity;
    Monster *target_monster;

    bool _is_moving_to_location;
    Point _destination;

public:
    Hero(int id, int type, int x, int y, short ia_type)
    {
        _id = id;
        _type = (e_entity_type)type;
        _x = x;
        _y = y;
        _ia_type = ia_type;
        _is_targeting_entity = false;
        target_monster = nullptr;
    };
    void updateHero(int x, int y)
    {
        _x = x;
        _y = y;
    };
    void move_to_location(int x, int y)
    {
        cout << "MOVE " << x << " " << y << " [" << _id << "] Move" << endl;
        _is_moving_to_location = true;
    }
    void move_to_monster(Monster &monster)
    {
        cout << "MOVE " << monster._x << " " << monster._y << " [" << _id << "] [" << monster._id << "]" << endl;
    }
    void cast_spell(Monster &monster)
    {
        cout << "SPELL WIND " << MAX_X << " " << MAX_Y << " WOOSH !" << endl;
        // fprintf(stderr, "NB Monster in distance: %zu\n", monster_filter_by_distance(_game._monsters, *hero, 1000).size());
        // if (monster_filter_by_distance(_game._monsters, *hero, 1000).size() >= 2 && _game._players[E_OWN]._mana > 25)
        // {
        //     continue;
        // }
    }
    void wait()
    {
        cout << "WAIT" << endl;
    }
    void set_target_monster(Monster *monster)
    {
        if (monster == nullptr)
        {
            _is_targeting_entity = false;
            if (target_monster != nullptr)
                delete target_monster;
            target_monster = nullptr;
        }
        else
        {
            _is_targeting_entity = true;
            target_monster = monster;

            _is_moving_to_location = false;
        }
    }
};
class Player
{
public:
    enum e_player_type _type;
    int _health;
    int _mana;
};
class Game
{
public:
    int _base_x;
    int _base_y;

    Player _players[2];

    int _nb_entities;
    int _nb_heroes;
    int _nb_monsters;

    std::vector<Hero> _heroes;
    std::vector<Hero> _ennemy_heroes;
    std::vector<Monster> _monsters;
};

Game _game;

// Utils

float scale(float A, float A1, float A2, float Min, float Max)
{
    long double percentage = (A - A1) / (A1 - A2);
    return (percentage) * (Min - Max) + Min;
}

// Initialize the game

static void init_startup_values()
{
    cin >> _game._base_x >> _game._base_y;
    cin.ignore();
    cin >> _game._nb_heroes; // Always 3
    cin.ignore();
    srand(time(NULL));
}
static void init_game_entities()
{
    for (int i = 0; i < 2; i++)
    {
        _game._players[i]._type = E_OWN;
        cin >> _game._players[i]._health >> _game._players[i]._mana;
        cin.ignore();
    }
    cin >> _game._nb_entities;
    cin.ignore();
    _game._nb_monsters = _game._nb_entities - (_game._nb_heroes * 2);

    _game._monsters.clear();
    _game._ennemy_heroes.clear();

    size_t i_monster = 0, i_hero = 0, i_ennemy_hero = 0;
    short ia_type = 0;

    for (int i = 0; i < _game._nb_entities; i++)
    {
        int id, type, x, y, shield_life, is_controlled, health, vx, vy, near_base, threat_for;

        cin >> id >> type >> x >> y >> shield_life >> is_controlled >> health >> vx >> vy >> near_base >> threat_for;
        cin.ignore();

        if (type == E_MONSTER)
        {
            Monster monster(id, type, x, y, shield_life, is_controlled, health, vx, vy, near_base, threat_for);
            _game._monsters.push_back(monster);
            ++i_monster;
        }
        else if (type == E_ENNEMY_HERO)
        {
            Hero hero(id, type, x, y, 4);
            _game._ennemy_heroes.push_back(hero);
            ++i_ennemy_hero;
        }
        else if (type == E_HERO)
        {
            if (_game._heroes.size() < 3)
            {
                Hero hero(id, type, x, y, ia_type);
                _game._heroes.push_back(hero);
                ia_type += 1;
            }
            else
            {
                for (Hero &hero : _game._heroes)
                {
                    hero.updateHero(x, y);
                    ++i_hero;
                }
            }
        }
    }
}
static void update_heros_values()
{
    // Update current Monster
    for (Hero &hero : _game._heroes)
    {
        if (hero._is_targeting_entity == true)
        {
            for (Monster &monster : _game._monsters)
            {
                if (monster._id == hero.target_monster->_id)
                {
                    if (hero.target_monster != nullptr)
                        delete (hero.target_monster);
                    hero.target_monster = new Monster(monster);
                }
            }
        }
    }

    // Check if target monster is dead
    for (Hero &hero : _game._heroes)
    {
        if (hero._is_targeting_entity == false)
            continue;
        bool is_invalid = true;
        for (Monster &monster : _game._monsters)
        {
            if (monster._id == hero.target_monster->_id)
            {
                is_invalid = false;
                break;
            }
        }
        if (is_invalid == true)
            hero.set_target_monster(nullptr);
    }

    // Check if hero has arrived to destination
    for (Hero &hero : _game._heroes)
    {
        if (hero._is_moving_to_location == true)
        {
            int distance = sqrt(pow(hero._x - hero._destination.x, 2) + pow(hero._y - hero._destination.y, 2));
            if (distance < 4)
            {
                hero._is_moving_to_location = false;
            }
        }
    }
}
// IA Functions

static vector<Monster> monster_filter_by_distance(vector<Monster> &monster, Hero &hero, size_t distance_max)
{
    vector<Monster> filtered_monsters;

    for (Monster &monster : monster)
    {
        int distance = abs(monster._x - hero._x) + abs(monster._y - hero._y);
        if (distance <= distance_max)
        {
            filtered_monsters.push_back(monster);
        }
    }
    return (filtered_monsters);
}
static vector<Monster> monster_filter_by_threat_for(vector<Monster> &monster, int threat_for)
{
    vector<Monster> filtered_monsters;

    for (Monster &monster : monster)
    {
        if (monster._threat_for == threat_for)
        {
            filtered_monsters.push_back(monster);
        }
    }
    return (filtered_monsters);
}
static vector<Monster> monster_filter_by_not_focuses(vector<Monster> &monster)
{
    vector<Monster> filtered_monsters;
    bool push_monster = true;

    for (Monster &monster : monster)
    {
        push_monster = true;
        for (Hero &hero : _game._heroes)
        {
            if (hero._is_targeting_entity == true && monster._id == hero.target_monster->_id)
            {
                fprintf(stderr, "Hero %d targeting entity %d | id: %d\n", hero._id, hero._is_targeting_entity, monster._id);
                push_monster = false;
            }
        }
        if (push_monster == true)
            filtered_monsters.push_back(monster);
    }
    return (filtered_monsters);
}

static size_t monster_get_distance_to(Monster &monster, Hero &hero)
{
    return (abs(monster._x - hero._x) + abs(monster._y - hero._y));
}
static size_t nb_monster_in_distance(Hero &hero, size_t distance)
{
    size_t nb_monsters = 0;

    for (Monster &monster : _game._monsters)
    {
        if (monster_get_distance_to(monster, hero) <= distance)
            ++nb_monsters;
    }
    return (nb_monsters);
}
static size_t nb_monster_in_distance_by_location(Point location, size_t distance)
{
    size_t nb_monsters = 0;

    for (Monster &monster : _game._monsters)
    {
        if (abs(monster._x - location.x) <= distance && abs(monster._y - location.y) <= distance)
            ++nb_monsters;
    }
    return (nb_monsters);
}

static Monster *find_nearest_monster(vector<Monster> &monsters, Hero &hero)
{
    Monster *closest_monster = nullptr;
    size_t distance_min = MAX_X * MAX_Y;

    for (Monster &monster : monsters)
    {
        size_t distance = monster_get_distance_to(monster, hero);
        if (distance < distance_min)
        {
            distance_min = distance;
            closest_monster = new Monster(monster);
        }
    }
    return (closest_monster);
}
static Monster *find_nearest_monster_by_location(Point point)
{
    Monster *closest_monster = nullptr;
    size_t distance_min = MAX_X * MAX_Y;

    for (Monster &monster : _game._monsters)
    {
        size_t distance = abs(monster._x - point.x) + abs(monster._y - point.y);
        if (distance < distance_min)
        {
            distance_min = distance;
            closest_monster = new Monster(monster);
        }
    }
    return (closest_monster);
}
// IA Deplacements [FOG]

static Point ia_hero_move_random(Hero *hero)
{
    Point point;

    if (hero->_is_moving_to_location)
        return (hero->_destination);

    switch (hero->_ia_type)
    {
        // IA 0 down
        // IA 1 right
        // IA 2 to center map

    case 0:
    {
        int add_x = 0, add_y = 0;

        add_x = rand() % int(MAX_DISTANCE / 8) - 1;
        add_y = rand() % int(MAX_DISTANCE / 2) - 1;
        point.x = hero->_x + add_x;
        point.y = hero->_y + add_y;
        break;
    }
    case 1:
    {
        int add_x = 0, add_y = 0;

        add_x = rand() % int(MAX_DISTANCE / 2) - 1;
        add_y = rand() % int(MAX_DISTANCE / 8) - 1;
        point.x = hero->_x + add_x;
        point.y = hero->_y + add_y;
        break;
    }
    case 2:
    {
        int add_x = 0, add_y = 0;

        add_x = rand() % int(MAX_DISTANCE / 4) - 1;
        add_y = rand() % int(MAX_DISTANCE / 4) - 1;

        point.x = MAX_X / 2 + add_x;
        point.y = MAX_Y / 2 + add_y;
        break;
    }
    default:
    {
        break;
    }
    }
    if (point.x < 0)
        point.x = 0;
    if (point.x > MAX_X - 1)
        point.x = MAX_X - 1;
    if (point.y < 0)
        point.y = 0;
    if (point.y > MAX_Y - 1)
        point.y = MAX_Y - 1;
    return (point);
}

// IA Functions Systeme poids

static vector<pair<float, Monster *>> ia_monster_init_poids()
{
    vector<pair<float, Monster *>> poids;

    for (Monster &monster : _game._monsters)
    {
        poids.push_back(make_pair(0.0f, new Monster(monster)));
    }
    return (poids);
}

// Adders
static void ia_monster_poids_by_distance(vector<pair<float, Monster *>> &poids, Point location, float poids_value)
{
    for (pair<float, Monster *> &poid : poids)
    {
        float distance = abs(poid.second->_x - location.x) + abs(poid.second->_y - location.y);
        float newValue = (MAX_DISTANCE / distance) * poids_value;
        fprintf(stderr, "DISTANCE - Poids: %f\n", newValue);
        poid.first += newValue;
    }
}
static void ia_hero_guard_location(vector<pair<float, Monster *>> &poids, Hero &hero, Point location, float poids_value)
{
    for (pair<float, Monster *> &poid : poids)
    {
        float distance = abs(hero._x - location.x) + abs(hero._y - location.y);
        float newValue = (MAX_DISTANCE / distance) * poids_value;
        fprintf(stderr, "DISTANCE - Poids: %f\n", newValue);
        poid.first += newValue;
    }
}

// Subtracters
static void ia_monster_poids_by_is_already_target(vector<pair<float, Monster *>> &poids)
{
    for (pair<float, Monster *> &poid : poids)
    {
        for (Hero &hero : _game._heroes)
        {
            if (hero._is_targeting_entity == true && poid.second->_id == hero.target_monster->_id)
            {
                poid.first -= POIDS_ALREADY_TARGET;
                fprintf(stderr, "DISTANCE - Poids: %d\n", POIDS_ALREADY_TARGET);
                break;
            }
        }
    }
}
static void ia_monster_poids_by_outside_distance(vector<pair<float, Monster *>> &poids, Hero &hero, size_t distance_max)
{
    for (pair<float, Monster *> &poid : poids)
    {
        size_t distance = monster_get_distance_to(*poid.second, hero);
        if (distance <= distance_max)
        {
            poid.first -= POIDS_DISTANCE_OUTSIDE;
            fprintf(stderr, "DISTANCE - Poids: %d\n", POIDS_DISTANCE_OUTSIDE);
        }
    }
}

static Monster *ia_hero_select_monster(Hero &hero)
{
    // todo, systeme de poids

    // IA a faire:
    // Selectionner le monstre le plus proche suivant s'il est loin ou non de la base
    // Les monstres proches de la base sont prioritaires ainsi que ceux qui vont vers la base
    // Ecarter le plus possible les heros

    fprintf(stderr, "--- Hero: [%d] ---\n\n", hero._id);
    fprintf(stderr, "IA: [%d]\n", hero._ia_type);

    vector<pair<float, Monster *>> _monsters_poids;
    Monster *targetMonster = nullptr;
    float add_poids_by_ia = 0;

    _monsters_poids = ia_monster_init_poids();

    Point hero_location = {hero._x, hero._y};
    Point base_location = {_game._base_x, _game._base_y};

    add_poids_by_ia = hero._ia_type == 2 ? 4 : hero._ia_type == 1 ? 2
                                                                  : 1;
    ia_monster_poids_by_distance(_monsters_poids, hero_location, POIDS_DISTANCE * add_poids_by_ia);
    add_poids_by_ia = hero._ia_type == 2 ? 1.1 : hero._ia_type == 1 ? 2
                                                                    : 4;
    ia_monster_poids_by_distance(_monsters_poids, base_location, POIDS_DISTANCE_BASE * add_poids_by_ia);
    ia_monster_poids_by_outside_distance(_monsters_poids, hero, 2200);
    ia_monster_poids_by_is_already_target(_monsters_poids);

    for (vector<pair<float, Monster *>>::iterator it = _monsters_poids.begin(); it != _monsters_poids.end(); ++it)
    {
        fprintf(stderr, "Monster: %d | poids: %f\n", (*it).second->_id, (*it).first);
    }

    size_t max_poids = 0;
    for (vector<pair<float, Monster *>>::iterator it = _monsters_poids.begin(); it != _monsters_poids.end(); ++it)
    {
        if ((*it).first > max_poids)
        {
            targetMonster = (*it).second;
            max_poids = (*it).first;
        }
    }
    return (targetMonster);

    vector<Monster> monsters = _game._monsters;
    monsters = monster_filter_by_not_focuses(monsters);

    for (Monster &monster : monsters)
    {
        fprintf(stderr, "Monster: %d\n", monster._id);
    }
    fprintf(stderr, "\n");

    vector<Monster> monsters_distance = monster_filter_by_distance(monsters, hero, 10000);
    // vector<Monster> monsters_threat_for = monster_filter_by_threat_for(monsters, 1);

    return (find_nearest_monster(monsters, hero));

    if (monster_get_distance_to(*find_nearest_monster(monsters, hero), hero) <= 5000)
        targetMonster = find_nearest_monster(monsters, hero);
    else
        targetMonster = find_nearest_monster(monsters, hero);

    return (targetMonster);
    return (nullptr);
}
static short ia_select_best_movement(Hero *hero)
{
    // 0 -> WAIT
    // 1 -> MOVE TO LOCATION
    // 2 -> MOVE TO MONSTER
    // 3 -> CAST SPELL

    int ia_type = hero->_ia_type;
    vector<pair<float, short>> poids_movements;
    short best_movement = 0;

    // Monster *target_monster = ia_hero_select_monster(*hero);

    for (size_t i = 0; i < 4; i++)
        poids_movements.push_back(make_pair(0, i));

    float poids_protect_base = 1, poids_attack_ennemy = 1;

    switch (ia_type)
    {
    case 0: // Defense
    {
        poids_protect_base = 4;
        poids_attack_ennemy = 1;
        break;
    }
    case 1: // Attack
    {
        poids_protect_base = 2;
        poids_attack_ennemy = 2;
        break;
    }
    case 2: // All in, peut aller jusque dans la base ennemie
    {
        poids_protect_base = 1;
        poids_attack_ennemy = 4;
        break;
    }
    }

    // Si aucun monstre n'es a portee -> Move
    if (nb_monster_in_distance(*hero, HERO_VIEW_DISTANCE) <= 0)
    {
        hero->_destination = ia_hero_move_random(hero);
        return (1);
    }

    Point base_location;

    base_location.x = _game._base_x;
    base_location.y = _game._base_y;
    if (nb_monster_in_distance_by_location(base_location, BASE_VIEW_DISTANCE) > (4 - poids_protect_base))
    {
        hero->target_monster = find_nearest_monster_by_location(base_location);
        poids_movements[2].second += 1 * poids_protect_base;
    }
    else if (nb_monster_in_distance(*hero, HERO_VIEW_DISTANCE) > 0)
    {
        Point hero_location;

        hero_location.x = hero->_x;
        hero_location.y = hero->_y;
        hero->target_monster = find_nearest_monster_by_location(hero_location);
        poids_movements[2].second += 1 * poids_attack_ennemy;
    }

    float current_poids = 0.0f;
    for (vector<pair<float, short>>::iterator it = poids_movements.begin(); it != poids_movements.end(); ++it)
    {
        fprintf(stderr, "Movement: [%d] | poids: %f\n", (*(it)).second, (*(it)).first);
        if (poids_movements[best_movement].first > current_poids)
        {
            best_movement = (*it).second;
            current_poids = (*it).first;
        }
    }

    return (best_movement);
}

int main()
{
    init_startup_values();
    // game loop
    while (1)
    {
        init_game_entities();
        update_heros_values();
        for (int i = 0; i < _game._nb_heroes; i++)
        {
            Hero *hero = &_game._heroes[i];
            short best_movement = ia_select_best_movement(hero);

            fprintf(stderr, "Hero: [%d] | best_movement: %d\n", hero->_id, best_movement);
            switch (best_movement)
            {
            case 0:
            {
                hero->wait();
                break;
            }
            case 1:
            {
                fprintf(stderr, "Hero: [%d] Move to location: [%d, %d]\n", hero->_id, hero->_destination.x, hero->_destination.y);
                hero->set_target_monster(nullptr);
                hero->move_to_location(hero->_destination.x, hero->_destination.y);
                break;
            }
            case 2:
            {
                hero->_is_moving_to_location = false;
                fprintf(stderr, "Monster is valid: %d\n", hero->target_monster != nullptr);
                // if (hero->_is_targeting_entity == false)
                // {
                hero->set_target_monster(hero->target_monster);
                hero->move_to_monster(*hero->target_monster);
                //}
                break;
            }
            case 3:
            {
                hero->cast_spell(*hero->target_monster);
                break;
            }
            default:
                break;
            }
        }
    }
}