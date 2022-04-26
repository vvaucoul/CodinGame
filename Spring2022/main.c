#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#define MIN_X 0
#define MIN_Y 0
#define MAX_X 17630
#define MAX_Y 9000

#define PLAYER_OWN 0
#define PLAYER_ENEMY 1

#define BASE_RANGE 5000
#define VIEW_DISTANCE_BASE 6000
#define CRITICAL_BASE_RANGE 300 + 200

#define HERO_VIEW_DISTANCE 2200

#define MONSTER_SPEED 400
#define HERO_SPEED 800
#define HERO_REACH_DISTANCE 10
#define HERO_DAMAGES 2

#define DISTANCE_SPELL_WIND 1280
#define DISTANCE_SPELL_CONTROL 2200
#define DISTANCE_SPELL_SHIELD 2200

#define MANA_TO_CAST_SPELL 10

#define TURN_TO_DEFFENCE_BASE 25
#define TURN_TO_ATTACK_ENNEMY_BASE 50

#define BASE_IN_TOP_LEFT(point) (point.x < (MAX_X / 2) && point.y < (MAX_Y / 2))
#define ENNEMY_BASE_LOCATION(point) (BASE_IN_TOP_LEFT(point) == true ? (Point){MAX_X, MAX_Y} : (Point){0, 0})
#define DISTANCE(point, second_point) sqrt(pow(point.x - second_point.x, 2) + pow(point.y - second_point.y, 2))
#define RANDOM(min, max) (rand() % (max - min + 1) + min)
#define ENTITY_LEVEL(entity) (entity.health < 14 ? 0 : entity.health < 19 ? 1 \
                                                                          : 2)
#define HAS_ENOUGH_MANA() (_game.players[PLAYER_OWN].mana >= MANA_TO_CAST_SPELL)

#define IS_VALID_ENTITY(e) e.id > 0
#define NO_MONSTERS_VIEWED() _game.nb_monsters <= 0
#define EMPTY_ENTITY() \
    (Entity) { -1 }

enum e_task
{
    E_TASK_WAIT,
    E_TASK_MOVE,
    E_TASK_ATTACK,
    E_TASK_SPELL_WIND,
    E_TASK_SPELL_CONTROL,
    E_TASK_SPELL_SHIELD,
};
#define E_Task enum e_task

typedef struct s_point
{
    int x;
    int y;
} t_point;
#define Point t_point

typedef struct s_task
{
    E_Task type;
    int target_id;
    Point destination;
    Point spell_vector;
} t_task;
#define Task t_task

typedef struct s_task_pair
{
    Task task;
    float value;
} t_task_pair;
#define TPair t_task_pair

typedef struct s_player
{
    int health;
    int mana;
} t_player;
#define Player t_player

typedef struct s_entity
{
    int id;
    int type;
    Point location;
    int shield_life;
    int is_controlled;
    int health;
    int vx;
    int vy;
    int near_base;
    int threat_for;
} t_entity;
#define Entity t_entity

typedef struct s_hero
{
    int id;
    int type;
    Point location;

    int shield_life;
    int is_controlled;

    Task task;
    int ia_type;

    bool uninterupt_task;
    int uninterupt_task_turn;
} t_hero;
#define Hero t_hero

typedef struct s_game
{
    Point base_location;
    bool base_in_top_left;

    int nb_heroes;
    int nb_entities;
    int nb_monsters;
    int nb_turn;
    Player players[2];

    Entity monsters[1024];
    Hero heroes[3];
    Hero ennemy_heroes[3];
} t_game;
#define Game t_game

Game _game;

// Functions Declaration
Hero *findNearestHeroToEntity(Entity *entity);

// Utility Functions
Point invert_location(Point *location, int min_x, int max_x, int min_y, int max_y)
{
    location->x = (max_x + min_x) - location->x;
    location->y = (max_y + min_y) - location->y;
    return (*location);
}
Point generateRandomPointInCircle(Point center, int radius)
{
    Point point;
    double angle = (double)rand() / RAND_MAX * 2 * M_PI;
    point.x = (int)(cos(angle) * radius) + center.x;
    point.y = (int)(sin(angle) * radius) + center.y;
    if (point.x < MIN_X)
        point.x = MIN_X + HERO_VIEW_DISTANCE;
    if (point.x > MAX_X)
        point.x = MAX_X - HERO_VIEW_DISTANCE;
    if (point.y < MIN_Y)
        point.y = MIN_Y + HERO_VIEW_DISTANCE;
    if (point.y > MAX_Y)
        point.y = MAX_Y - HERO_VIEW_DISTANCE;
    return (point);
}
Point generateRandomPointInCircleWithMinMax(Point center, int radius, float dist_min)
{
    Point point = generateRandomPointInCircle(center, radius);

    if (DISTANCE(point, center) < dist_min)
    {
        return (generateRandomPointInCircleWithMinMax(center, radius, dist_min));
    }
    return (point);
}
Point findBestPointByEntities(Entity *entities, Hero *hero)
{
    Point best_point = {0, 0};
    size_t i = 0;
    for (i = 0; i < IS_VALID_ENTITY(entities[i]); i++)
    {
        best_point.x += entities[i].location.x;
        best_point.y += entities[i].location.y;
    }
    best_point.x /= i;
    best_point.y /= i;
    return (best_point);
}
float findDistanceByEntities(Entity *entities, Hero *hero)
{
    float distance = 0;
    size_t i = 0;
    for (i = 0; i < IS_VALID_ENTITY(entities[i]); i++)
    {
        distance += DISTANCE(entities[i].location, hero->location);
    }
    distance /= i;
    return (distance);
}
float findDistanceByEntitiesWithLocation(Entity *entities, Point location)
{
    float distance = 0;
    size_t i = 0;
    for (i = 0; i < IS_VALID_ENTITY(entities[i]); i++)
    {
        distance += DISTANCE(entities[i].location, location);
    }
    distance /= i;
    return (distance);
}

// Entity Functions
Entity *findEntityByID(Entity *entities, int id)
{
    for (size_t i = 0; IS_VALID_ENTITY(entities[i]); i++)
    {
        if (entities[i].id == id)
            return (&entities[i]);
    }
    return (NULL);
}
Entity *findNearestEntity(Hero *hero, Entity *entities, float distance)
{
    Entity *nearest_entity = NULL;
    float max_distance = distance;

    if (!entities || NO_MONSTERS_VIEWED())
        return (NULL);
    for (size_t i = 0; IS_VALID_ENTITY(entities[i]); i++)
    {
        float distance = DISTANCE(hero->location, entities[i].location);
        if (distance < max_distance)
        {
            max_distance = distance;
            nearest_entity = &entities[i];
        }
    }
    return (nearest_entity);
}
Entity *findNearestEntityByLocation(Entity *entities, Point location, float distance)
{
    Entity *nearest_entity = NULL;
    float max_distance = distance;

    for (size_t i = 0; IS_VALID_ENTITY(entities[i]); i++)
    {
        float distance = DISTANCE(location, entities[i].location);
        if (distance < max_distance)
        {
            max_distance = distance;
            nearest_entity = &entities[i];
        }
    }
    return (nearest_entity);
}
Entity *sortEntitiesNotFocuses(Entity *entities, Hero *heroes, Hero *hero)
{
    if (!entities || NO_MONSTERS_VIEWED())
        return (NULL);

    Entity *sorted_entities = malloc(sizeof(Entity) * (_game.nb_monsters + 1));
    memset(sorted_entities, 0, sizeof(Entity) * (_game.nb_monsters + 1));
    int nb_entities = 0;

    for (size_t i = 0; IS_VALID_ENTITY(entities[i]); i++)
    {
        bool add = true;

        for (size_t j = 0; j < _game.nb_heroes; j++)
        {
            if (entities[i].id == heroes[j].task.target_id && hero->task.target_id != entities[i].id)
                add = false;
        }
        if (add)
        {
            sorted_entities[nb_entities] = entities[i];
            nb_entities++;
        }
    }
    return (sorted_entities);
}
Entity *sortEntitiesWithLocationByDistance(Entity *entities, Point location, float dist_max)
{
    if (!entities || NO_MONSTERS_VIEWED())
        return (NULL);

    Entity *sorted_entities = malloc(sizeof(Entity) * (_game.nb_monsters + 1));
    memset(sorted_entities, 0, sizeof(Entity) * (_game.nb_monsters + 1));
    int nb_entities = 0;
    for (size_t i = 0; IS_VALID_ENTITY(entities[i]); i++)
    {
        float distance = DISTANCE(location, entities[i].location);
        if (distance < dist_max)
        {
            sorted_entities[nb_entities] = entities[i];
            nb_entities++;
        }
    }
    return (sorted_entities);
}
Entity *sortEntitiesReachBase(Entity *entities)
{
    if (!entities || NO_MONSTERS_VIEWED())
        return (NULL);

    Entity *sorted_entities = malloc(sizeof(Entity) * (_game.nb_monsters + 1));
    memset(sorted_entities, 0, sizeof(Entity) * (_game.nb_monsters + 1));
    int nb_entities = 0;
    for (size_t i = 0; IS_VALID_ENTITY(entities[i]); i++)
    {
        if (entities[i].threat_for == 1 || entities[i].near_base == 1)
        {
            sorted_entities[nb_entities] = entities[i];
            nb_entities++;
        }
    }
    return (sorted_entities);
}
Entity *sortEntitiesNotShielded(Entity *entities)
{
    if (!entities || NO_MONSTERS_VIEWED())
        return (NULL);

    Entity *sorted_entities = malloc(sizeof(Entity) * (_game.nb_monsters + 1));
    memset(sorted_entities, 0, sizeof(Entity) * (_game.nb_monsters + 1));
    int nb_entities = 0;
    for (size_t i = 0; IS_VALID_ENTITY(entities[i]); i++)
    {
        if (entities[i].shield_life <= 0)
        {
            sorted_entities[nb_entities] = entities[i];
            nb_entities++;
        }
    }
    return (sorted_entities);
}
float *sortHeroDistanceToEntity(Entity *entity)
{
    float *distances = calloc(sizeof(float), _game.nb_heroes);
    memset(distances, 0, sizeof(float) * 3);

    for (size_t i = 0; i < _game.nb_heroes; i++)
    {
        (distances)[i] = DISTANCE(_game.heroes[i].location, entity->location);
    }
    return (distances);
}
size_t getNBEntitiesInBase()
{
    Entity *entities = _game.monsters;
    int nb_entities = 0;
    for (size_t i = 0; IS_VALID_ENTITY(entities[i]); i++)
    {
        if (DISTANCE(entities[i].location, _game.base_location) < VIEW_DISTANCE_BASE)
            nb_entities++;
    }
    return (nb_entities);
}
size_t getNBHeroesFocusEntity(Entity *entity)
{
    int nb_heroes = 0;
    for (size_t i = 0; i < _game.nb_heroes; i++)
    {
        if (_game.heroes[i].task.target_id == entity->id)
            nb_heroes++;
    }
    return (nb_heroes);
}
size_t getNBHeroesViewEntity(Entity *entity)
{
    int nb_heroes = 0;
    for (size_t i = 0; i < _game.nb_heroes; i++)
    {
        if (DISTANCE(_game.heroes[i].location, entity->location) < HERO_VIEW_DISTANCE)
            nb_heroes++;
    }
    return (nb_heroes);
}
size_t getNBEntitiesHeroSee(Entity *entities, Hero *hero)
{
    int nb_entities = 0;
    for (size_t i = 0; IS_VALID_ENTITY(entities[i]); i++)
    {
        if (DISTANCE(entities[i].location, hero->location) <= HERO_VIEW_DISTANCE)
            nb_entities++;
    }
    return (nb_entities);
}
size_t getEntityLength(Entity *entities)
{
    size_t length = 0;
    for (size_t i = 0; IS_VALID_ENTITY(entities[i]); i++)
        length++;
    return (length);
}
size_t getTotalEntityLevelByDistance(Entity *entities, Point location, float dis_max)
{
    size_t total_level = 0;
    size_t i = 0;
    for (i = 0; IS_VALID_ENTITY(entities[i]); i++)
    {
        if (DISTANCE(entities[i].location, location) < dis_max)
            total_level += ENTITY_LEVEL(entities[i]);
    }
    return (total_level / i);
}
bool isEntityFocuses(Entity *entity)
{
    for (size_t i = 0; i < _game.nb_heroes; i++)
    {
        Hero *hero = &_game.heroes[i];

        if (hero->task.target_id == entity->id)
            return (true);
    }
    return (false);
}
bool isEntityRequiertMoreTarget(Entity *entity, Hero *hero)
{
    int nb_heroes_focus_entity = getNBHeroesFocusEntity(entity);
    float distance_entity_to_base = DISTANCE(entity->location, _game.base_location);
    int entity_life = entity->health;

    int nb_turn_to_monster_reach_base = (int)(distance_entity_to_base / MONSTER_SPEED);
    int nb_heroes_near_to_entity = getNBHeroesViewEntity(entity);

    float distance_to_monster = DISTANCE(hero->location, entity->location);
    int nb_turn_hero_to_reach_entity = (int)(DISTANCE(hero->location, entity->location) / HERO_SPEED);
    int nb_turn_hero_to_kill_entity = (int)((float)entity_life / (float)HERO_DAMAGES) + nb_turn_hero_to_reach_entity;

    Hero *nearest_hero_to_entity = findNearestHeroToEntity(entity);

    float *distances = sortHeroDistanceToEntity(entity);
    bool hero_is_the_nearest_to_ennemy = true;
    for (size_t i = 0; i < _game.nb_heroes; i++)
    {
        if (i == hero->id)
            continue;
        if (distances[i] < distances[hero->id])
            hero_is_the_nearest_to_ennemy = false;
    }
    free(distances);

    fprintf(stderr, "Hero is the nearest to ennemy : %d\n", hero_is_the_nearest_to_ennemy);
    fprintf(stderr, "Nearest Hero to entity : %d\n", nearest_hero_to_entity->id);
    fprintf(stderr, "Distance to monster : %f < %d\n", distance_to_monster, HERO_VIEW_DISTANCE);
    fprintf(stderr, "Nb turn hero to reach entity : %d\n", nb_turn_hero_to_reach_entity);
    fprintf(stderr, "Nb turn hero to kill entity : %d\n", nb_turn_hero_to_kill_entity);
    fprintf(stderr, "Nb turn to monster reach base : %d\n", nb_turn_to_monster_reach_base);
    fprintf(stderr, "Nb heroes near to entity : %d\n", nb_heroes_near_to_entity);
    if (hero_is_the_nearest_to_ennemy)
        return (true);
    else if (nearest_hero_to_entity->id == hero->id && distance_to_monster < HERO_VIEW_DISTANCE)
        return (true);
    if (nb_turn_hero_to_kill_entity < nb_turn_to_monster_reach_base)
    {
        if (nb_heroes_near_to_entity > 1)
            return (true);
    }
    return (false);
}

// Heros Functions
void heroAssignTask(Hero *hero, Task task)
{
    if (hero->uninterupt_task == false)
        hero->task = task;
}
void heroSetTaskUninterrupted(Hero *hero, int nb_turn_max)
{
    hero->uninterupt_task = true;
    hero->uninterupt_task_turn = nb_turn_max > 0 ? nb_turn_max : -1;
}
void heroInterruptTask(Hero *hero)
{
    switch (hero->task.type)
    {
    case E_TASK_WAIT:
    {
        hero->uninterupt_task = false;
        break;
    }
    case E_TASK_ATTACK:
    {
        bool find = findEntityByID(_game.monsters, hero->task.target_id);

        if (!find)
            hero->uninterupt_task = false;
        break;
    }
    case E_TASK_MOVE:
    {
        bool reached_destination = DISTANCE(hero->location, hero->task.destination) < HERO_REACH_DISTANCE;

        if (reached_destination)
            hero->uninterupt_task = false;
        break;
    }
    default:
        break;
    }
}
void heroForceInterruptTask(Hero *hero)
{
    hero->uninterupt_task = false;
    hero->uninterupt_task_turn = -1;
}
void heroDownUninteruptTaskTurn(Hero *hero)
{
    if (hero->uninterupt_task_turn > 0)
        hero->uninterupt_task_turn--;
    if (hero->uninterupt_task_turn == 0)
        heroForceInterruptTask(hero);
}
void heroDoTask(Hero *hero)
{
    switch (hero->task.type)
    {
    case E_TASK_WAIT:
    {
        printf("WAIT [%d] Zzz...\n", hero->id);
        break;
    }
    case E_TASK_MOVE:
    {
        printf("MOVE %d %d [%d] Move\n", hero->task.destination.x, hero->task.destination.y, hero->id);
        break;
    }
    case E_TASK_ATTACK:
    {
        Entity *target = findEntityByID(_game.monsters, hero->task.target_id);
        printf("MOVE %d %d [%d] -> [%d]\n", target->location.x, target->location.y, hero->id, target->id);
        break;
    }
    case E_TASK_SPELL_WIND:
    {
        printf("SPELL WIND %d %d [%d] Woosh...\n", hero->task.spell_vector.x, hero->task.spell_vector.y, hero->id);
        break;
    }
    case E_TASK_SPELL_SHIELD:
    {
        printf("SPELL SHIELD %d [%d] Shield\n", hero->task.target_id, hero->id);
        break;
    }
    case E_TASK_SPELL_CONTROL:
    {
        printf("SPELL CONTROL %d %d %d [%d] Wololo\n", hero->task.target_id, hero->task.destination.x, hero->task.destination.y, hero->id);
        break;
    }
    default:
        break;
    }
}
void heroReachedDestination(Hero *hero)
{
    if (DISTANCE(hero->location, hero->task.destination) < HERO_REACH_DISTANCE)
    {
        heroAssignTask(hero, (Task){E_TASK_WAIT, -1, {-1, -1}, {-1, -1}});
        fprintf(stderr, "Hero %d reached destination\n", hero->id);
    }
}
Point heroGetRandomIADistance(Hero *hero)
{
    int id = hero->id;
    int ia_type = hero->ia_type;
    Point location;

    switch (id)
    {
    case 0:
    {
        Point aggressive_location;

        if (_game.nb_turn < 10)
        {
            Point random = generateRandomPointInCircle((Point){MAX_X / 2, MAX_Y / 2}, HERO_VIEW_DISTANCE / 2);
            aggressive_location = random;

            // aggressive_location.x = RANDOM((int)(MAX_X / 2.5), (int)(MAX_X / 1.25));
            // aggressive_location.y = RANDOM((int)(MAX_Y / 3), (int)(MAX_Y / 1.5));
        }
        else
        {
            bool random_location = RANDOM(0, 1);

            if (random_location == true)
            {
                Point random = generateRandomPointInCircle((Point){11530, 7000}, HERO_VIEW_DISTANCE);
                aggressive_location.x = random.x;
                aggressive_location.y = random.y;
            }
            else
            {
                Point random = generateRandomPointInCircle((Point){15500, 3000}, HERO_VIEW_DISTANCE);
                aggressive_location.x = random.x;
                aggressive_location.y = random.y;
            }
        }
        if (_game.base_in_top_left == false)
            invert_location(&aggressive_location, MIN_X, MAX_X, MIN_Y, MAX_Y);
        return (aggressive_location);
    }
    case 1:
    {
        Point deffensive_location;
        if (_game.nb_turn < 10)
        {

            Point random = generateRandomPointInCircle((Point){6500, 1500}, (HERO_VIEW_DISTANCE));
            deffensive_location.x = random.x;
            deffensive_location.y = random.y;
            // deffensive_location.x = RANDOM((int)(MAX_X / 2.5), (int)(MAX_X / 1.75));
            // deffensive_location.y = RANDOM((int)(MAX_Y / 8), (int)(MAX_Y / 3));
        }
        else
        {
            Point random = generateRandomPointInCircle((Point){6500, 1500}, HERO_VIEW_DISTANCE);
            deffensive_location.x = random.x;
            deffensive_location.y = random.y;
            // deffensive_location.x = RANDOM((int)(MAX_X / 4), (int)(MAX_X / 2));
            // deffensive_location.y = RANDOM((int)(MAX_Y / 6), (int)(MAX_Y / 2));
        }
        if (_game.base_in_top_left == false)
            invert_location(&deffensive_location, MIN_X, MAX_X, MIN_Y, MAX_Y);
        return (deffensive_location);
    }
    case 2:
    {
        Point deffensive_location;
        if (_game.nb_turn < 10)
        {

            Point random = generateRandomPointInCircle((Point){3000, 6000}, (HERO_VIEW_DISTANCE));
            deffensive_location.x = random.x;
            deffensive_location.y = random.y;
            // deffensive_location.x = RANDOM((int)(MAX_X / 4), (int)(MAX_X / 2));
            // deffensive_location.y = RANDOM((int)(MAX_Y / 1.75), (int)(MAX_Y / 1.25));
        }
        else
        {
            Point random = generateRandomPointInCircle((Point){3000, 6000}, HERO_VIEW_DISTANCE);
            deffensive_location.x = random.x;
            deffensive_location.y = random.y;

            // deffensive_location.x = RANDOM((int)(MAX_X / 6), (int)(MAX_X / 3));
            // deffensive_location.y = RANDOM((int)(MAX_Y / 2), (int)(MAX_Y / 1.25));
        }

        if (_game.base_in_top_left == false)
            invert_location(&deffensive_location, MIN_X, MAX_X, MIN_Y, MAX_Y);
        return (deffensive_location);
    }
    default:
        break;
    }
    return ((Point){0, 0});
}
Hero *findNearestHeroToEntity(Entity *entity)
{
    Hero *nearest_hero = NULL;
    int min_distance = MAX_X + MAX_Y;
    for (int i = 0; i < _game.nb_heroes; i++)
    {
        Hero *hero = &_game.heroes[i];
        int distance = DISTANCE(entity->location, hero->location);
        if (distance < min_distance)
        {
            min_distance = distance;
            nearest_hero = hero;
        }
    }
    return (nearest_hero);
}
bool heroAnotherHeroWillDoTask(Hero *hero, int *heroes_id_to_find, enum e_task task)
{
    for (int i = 0; i < _game.nb_heroes; i++)
    {
        Hero *hero_to_check = &_game.heroes[i];

        for (size_t j = 0; heroes_id_to_find[j] >= 0; j++)
        {
            if (hero_to_check->id == heroes_id_to_find[j])
            {
                if (hero_to_check->task.type == task)
                {
                    return (true);
                }
            }
        }
    }
    return (false);
}

// Game Functions
static void init_startup_game_values()
{
    fprintf(stderr, "Init startup game values\n");
    scanf("%d%d", &_game.base_location.x, &_game.base_location.y);
    scanf("%d", &_game.nb_heroes);
    _game.base_in_top_left = BASE_IN_TOP_LEFT(_game.base_location);

    memset(_game.heroes, 0, sizeof(_game.heroes));
    memset(_game.ennemy_heroes, 0, sizeof(_game.ennemy_heroes));
    memset(_game.monsters, 0, sizeof(_game.monsters));
    _game.monsters[0].id = -1;
    _game.heroes[0].ia_type = 1;
    _game.heroes[1].ia_type = 0;
    _game.heroes[2].ia_type = 0;

    _game.nb_turn = 0;
}
static void init_game_values()
{
    fprintf(stderr, "Init game values\n");
    for (int i = 0; i < 2; i++)
        scanf("%d%d", &_game.players[i].health, &_game.players[i].mana);
    scanf("%d", &_game.nb_entities);
    _game.nb_monsters = _game.nb_entities - _game.nb_heroes;

    memset(_game.ennemy_heroes, 0, sizeof(_game.ennemy_heroes));
    memset(_game.monsters, 0, sizeof(_game.monsters));
    _game.nb_turn++;

    int i_monsters = 0, i_heroes = 0, i_ennemy_heroes = 0;
    for (int i = 0; i < _game.nb_entities; i++)
    {
        int id, type, x, y, shield_life, is_controlled, health, vx, vy, near_base, threat_for;
        scanf("%d%d%d%d%d%d%d%d%d%d%d", &id, &type, &x, &y, &shield_life, &is_controlled, &health, &vx, &vy, &near_base, &threat_for);

        if (type == 0)
        {
            _game.monsters[i_monsters].id = id;
            _game.monsters[i_monsters].type = type;
            _game.monsters[i_monsters].location.x = x;
            _game.monsters[i_monsters].location.y = y;
            _game.monsters[i_monsters].shield_life = shield_life;
            _game.monsters[i_monsters].is_controlled = is_controlled;
            _game.monsters[i_monsters].health = health;
            _game.monsters[i_monsters].vx = vx;
            _game.monsters[i_monsters].vy = vy;
            _game.monsters[i_monsters].near_base = near_base;
            _game.monsters[i_monsters].threat_for = threat_for;
            i_monsters++;
        }
        else if (type == 1)
        {
            if (id >= 3)
                id -= 3;
            _game.heroes[i_heroes].id = id;
            _game.heroes[i_heroes].type = type;
            _game.heroes[i_heroes].location.x = x;
            _game.heroes[i_heroes].location.y = y;
            _game.heroes[i_heroes].shield_life = shield_life;
            _game.heroes[i_heroes].is_controlled = is_controlled;
            i_heroes++;
        }
        else if (type == 2)
        {
            if (id < 3)
                id += 3;
            _game.ennemy_heroes[i_ennemy_heroes].id = id;
            _game.ennemy_heroes[i_ennemy_heroes].type = type;
            _game.ennemy_heroes[i_ennemy_heroes].location.x = x;
            _game.ennemy_heroes[i_ennemy_heroes].location.y = y;
            _game.ennemy_heroes[i_ennemy_heroes].shield_life = shield_life;
            _game.ennemy_heroes[i_ennemy_heroes].is_controlled = is_controlled;
            i_ennemy_heroes++;
        }
    }
    for (size_t i = 0; i < _game.nb_heroes; i++)
    {
        heroReachedDestination(&_game.heroes[i]);
        heroInterruptTask(&_game.heroes[i]);
        heroDownUninteruptTaskTurn(&_game.heroes[i]);
    }
}

// IA GAME Functions
static bool IA_DefenseBase(Hero *hero)
{
    // Code - Defense Base systeme poids
    TPair best_actions[6];

    for (size_t i = 0; i < 6; i++)
    {
        memset(&best_actions[i], 0, sizeof(TPair));
        best_actions[i].task.type = i;
    }

    Entity *entities_in_base = sortEntitiesWithLocationByDistance(_game.monsters, _game.base_location, VIEW_DISTANCE_BASE);
    Entity *entities_in_view_range = sortEntitiesWithLocationByDistance(_game.monsters, hero->location, HERO_VIEW_DISTANCE);

    Entity *entity_nearest_to_base = findNearestEntityByLocation(entities_in_base, _game.base_location, VIEW_DISTANCE_BASE);

    for (size_t i = 0; i < 6; i++)
    {
        if (i == E_TASK_WAIT) // ignore (inutile de wait)
        {
            best_actions[i].value = 0;
            continue;
        }
        else if (i == E_TASK_MOVE) // ignore (les movements sont effectues apres)
        {
            best_actions[i].value = 0;
            continue;
        }
        else if (i == E_TASK_ATTACK)
        {
            int nb_monster_in_range = getNBEntitiesHeroSee(entities_in_base, hero);
            Entity *target_entity = findNearestEntityByLocation(entities_in_base, _game.base_location, VIEW_DISTANCE_BASE);

            best_actions[i].value = nb_monster_in_range;
            best_actions[i].task.target_id = target_entity->id;
            continue;
        }
        else if (i == E_TASK_SPELL_WIND)
        {
            Entity *entities = sortEntitiesWithLocationByDistance(entities_in_base, hero->location, DISTANCE_SPELL_WIND);
            entities = sortEntitiesNotShielded(entities);

            int nb_ennemies_affected_by_wind = getEntityLength(entities);
            best_actions[i].value = nb_ennemies_affected_by_wind;
            continue;
        }
        else if (i == E_TASK_SPELL_SHIELD)
        {
            best_actions[i].value = 0;
            continue;
        }
        else if (i == E_TASK_SPELL_CONTROL)
        {
            int nb_ennemies_heroes_in_range = 0;
            best_actions[i].value = 0;
            continue;
        }
    }
    TPair best_action = {0, 0};
    for (size_t i = 0; i < 6; i++)
    {
        fprintf(stderr, "Task %ld : %f\n", i, best_actions[i].value);
        if (best_actions[i].value > best_action.value)
            best_action = best_actions[i];
    }
    heroAssignTask(hero, best_action.task);
    if (best_action.task.type == E_TASK_WAIT || best_action.value <= 0)
        return (false);
    return (true);
    // do best action

    // Code - Defense Base

    /*
    Entity *entities_in_base = sortEntitiesWithLocationByDistance(_game.monsters, _game.base_location, VIEW_DISTANCE_BASE);
    Entity *entities_in_view_range = sortEntitiesWithLocationByDistance(_game.monsters, hero->location, HERO_VIEW_DISTANCE);
    Entity *entity_nearest_to_base = findNearestEntityByLocation(entities_in_base, _game.base_location, VIEW_DISTANCE_BASE);

    if (entity_nearest_to_base->near_base != 1 && entity_nearest_to_base->threat_for != 1)
    {
        fprintf(stderr, " - Entity [%d] nearest to base will not reach base\n", entity_nearest_to_base->id);
        if (isEntityFocuses(entity_nearest_to_base))
            entity_nearest_to_base = NULL;
    }
    else if (entity_nearest_to_base->threat_for == 1)
    {
        fprintf(stderr, " - Entity [%d] is threat for base\n", entity_nearest_to_base->id);
        // if (hero->task.type != E_TASK_ATTACK)
        //     if (isEntityRequiertMoreTarget(entity_nearest_to_base, hero) == false)
        //         entity_nearest_to_base = NULL;
    }
    if (entity_nearest_to_base != NULL)
    {
        if (DISTANCE(entity_nearest_to_base->location, _game.base_location) < CRITICAL_BASE_RANGE)
        {
            heroForceInterruptTask(hero);
            heroAssignTask(hero, (Task){E_TASK_ATTACK, entity_nearest_to_base->id, 0, 0});
            return (true);
        }

        int nb_ennemies_in_range = getNBEntitiesHeroSee(_game.monsters, hero);
        fprintf(stderr, "\n - Spell Wind Defense\n");
        fprintf(stderr, "   - Nb ennemies in range: %d\n", nb_ennemies_in_range);
        if (nb_ennemies_in_range > 1)
        {
            float distance_entities = findDistanceByEntities(entities_in_view_range, hero);
            Point best_location = findBestPointByEntities(entities_in_view_range, hero);
            int entities_affected_by_wind = getEntityLength(sortEntitiesWithLocationByDistance(entities_in_view_range, hero->location, DISTANCE_SPELL_WIND));
            int entities_reach_base = getEntityLength(sortEntitiesReachBase(entities_in_view_range));
            float distance_entities_to_base = findDistanceByEntitiesWithLocation(entities_in_base, _game.base_location);

            fprintf(stderr, "   - Distance entities: %f\n", distance_entities);
            fprintf(stderr, "   - Distance entities to base: %f\n", distance_entities_to_base);
            fprintf(stderr, "   - Entities affected by wind: %d\n", entities_affected_by_wind);
            fprintf(stderr, "   - Entities reach base: %d\n", entities_reach_base);
            fprintf(stderr, "   - Best location: (%d, %d)\n", best_location.x, best_location.y);

            if (heroAnotherHeroWillDoTask(hero, (int[3]){1, 2, -1}, E_TASK_SPELL_WIND) == false)
            {
                if (entities_affected_by_wind > 1 &&
                    entities_reach_base > 1 &&
                    HAS_ENOUGH_MANA())
                {
                    fprintf(stderr, "   - Spell Wind Defense\n");
                    Point destination = ENNEMY_BASE_LOCATION(_game.base_location);
                    heroAssignTask(hero, (Task){E_TASK_SPELL_WIND, 0, 0, 0, destination.x, destination.y});
                    return (true);
                }
                else if (distance_entities < DISTANCE_SPELL_WIND && distance_entities_to_base < BASE_RANGE)
                {
                    fprintf(stderr, "   - Moving to best location: [%d] [%d]\n", best_location.x, best_location.y);
                    heroAssignTask(hero, (Task){E_TASK_MOVE, 0, best_location.x, best_location.y, 0, 0});
                    return (true);
                }
            }
        }
        else
        {
            float distance_entity_to_base = DISTANCE(entity_nearest_to_base->location, _game.base_location);
            fprintf(stderr, "   - Distance entity to base: %f\n", distance_entity_to_base);
            if (distance_entity_to_base > BASE_RANGE &&
                HAS_ENOUGH_MANA() &&
                DISTANCE(hero->location, entity_nearest_to_base->location) < DISTANCE_SPELL_CONTROL &&
                heroAnotherHeroWillDoTask(hero, (int[3]){1, 2, -1}, E_TASK_SPELL_CONTROL) == false)
            {
                Point destination = ENNEMY_BASE_LOCATION(_game.base_location);
                fprintf(stderr, "   - Spell Control [%d]\n", entity_nearest_to_base->id);
                heroAssignTask(hero, (Task){E_TASK_SPELL_CONTROL, entity_nearest_to_base->id, destination.x, destination.y, 0, 0});
                return (true);
            }
        }
        fprintf(stderr, " - Entity nearest to base: %d\n", entity_nearest_to_base->id);
        heroForceInterruptTask(hero);
        heroAssignTask(hero, (Task){E_TASK_ATTACK, entity_nearest_to_base->id, 0, 0});
    }
    else
    {
        Task task = {0};

        if (hero->task.type != E_TASK_MOVE)
        {
            fprintf(stderr, "No entity found, moving location\n");
            task.destination = heroGetRandomIADistance(hero);
            heroAssignTask(hero, (Task){E_TASK_MOVE, 0, task.destination.x, task.destination.y});
        }
        else
            task = hero->task;
    }
    return (false);
    */
}

int main()
{
    init_startup_game_values();
    // game loop
    while (1)
    {
        init_game_values();
        fprintf(stderr, "--- [TURN %d] ---\n", _game.nb_turn);
        for (int i = 0; i < _game.nb_heroes; i++)
        {
            Hero *hero = &_game.heroes[i];

            fprintf(stderr, "--- [HERO %d] ---\n", hero->id);
            fprintf(stderr, " - Current Task: %d\n", hero->task.type);

            fprintf(stderr, " - Nb Monsters in Base: %ld\n", getNBEntitiesInBase());
            // IA Defense Base [1 / 2]
            if (getNBEntitiesInBase() > 0 && hero->id != 0)
            {
                if ((IA_DefenseBase(hero)) == true)
                    continue;
                else
                {
                    fprintf(stderr, "NB MONSTERS: %d\n", _game.nb_monsters);
                    Entity *entities = sortEntitiesNotFocuses(_game.monsters, _game.heroes, hero);
                    for (size_t i = 0; !NO_MONSTERS_VIEWED() && IS_VALID_ENTITY(entities[i]); i++)
                        fprintf(stderr, "Not focused Entity %d\n", entities[i].id);
                    fprintf(stderr, "Try to find a monster to attack\n");
                    Entity *nearest_entity = findNearestEntity(hero, entities, HERO_VIEW_DISTANCE);
                    fprintf(stderr, "Nearest Entity: %d\n", nearest_entity ? nearest_entity->id : -1);

                    if (nearest_entity == NULL)
                    {
                        if (hero->task.type != E_TASK_MOVE)
                        {
                            fprintf(stderr, "No entity found, moving location\n");
                            Point destination = heroGetRandomIADistance(hero);
                            fprintf(stderr, "Moving to %d, %d\n", destination.x, destination.y);
                            heroAssignTask(hero, (Task){E_TASK_MOVE, 0, destination.x, destination.y});
                        }
                    }
                    else
                    {
                        heroAssignTask(hero, (Task){E_TASK_ATTACK, nearest_entity->id, 0, 0});
                        continue;
                    }
                }
            }
            else
            {
                fprintf(stderr, "Distance to base: %f > %f\n", DISTANCE(_game.base_location, hero->location), VIEW_DISTANCE_BASE * 1.5);
                if (DISTANCE(_game.base_location, hero->location) > (VIEW_DISTANCE_BASE * 1.5) && _game.nb_turn >= TURN_TO_DEFFENCE_BASE && hero->id != 0)
                {
                    fprintf(stderr, " - Hero is too far from base\n");
                    Point destination = heroGetRandomIADistance(hero);
                    heroAssignTask(hero, (Task){E_TASK_MOVE, 0, destination.x, destination.y});
                    heroSetTaskUninterrupted(hero, 4);
                    continue;
                }
                // IA ATTACK
                fprintf(stderr, " Hero ID %d | Game Turn: %d >= %d\n", hero->id, _game.nb_turn, TURN_TO_ATTACK_ENNEMY_BASE);
                if (hero->id == 0 && _game.nb_turn >= TURN_TO_ATTACK_ENNEMY_BASE)
                {
                    float distance_to_ennemy_base = DISTANCE(ENNEMY_BASE_LOCATION(_game.base_location), hero->location);
                    int nb_ennemies_hero_see = getNBEntitiesHeroSee(_game.monsters, hero);

                    fprintf(stderr, "NB ennemies hero see: %d\n", nb_ennemies_hero_see);
                    fprintf(stderr, "Distance to ennemy base: %f\n", distance_to_ennemy_base);
                    if (distance_to_ennemy_base > (VIEW_DISTANCE_BASE + HERO_VIEW_DISTANCE) || nb_ennemies_hero_see <= 0)
                    {
                        Point destination = generateRandomPointInCircleWithMinMax(ENNEMY_BASE_LOCATION(_game.base_location),
                                                                                  VIEW_DISTANCE_BASE + HERO_VIEW_DISTANCE, BASE_RANGE);

                        fprintf(stderr, "Destination: %d, %d\n", destination.x, destination.y);
                        heroAssignTask(hero, (Task){E_TASK_MOVE, 0, destination.x, destination.y});
                        heroSetTaskUninterrupted(hero, 0);
                        continue;
                    }
                    else
                    {
                        if (nb_ennemies_hero_see == 1)
                        {
                            Entity *monster = findNearestEntity(hero, _game.monsters, HERO_VIEW_DISTANCE);

                            fprintf(stderr, "Monster: %d\n", monster ? monster->id : -1);
                            if (monster != NULL)
                            {
                                if (monster->threat_for != 2 && monster->near_base != 2 && HAS_ENOUGH_MANA())
                                {
                                    Point destination = ENNEMY_BASE_LOCATION(_game.base_location);
                                    heroAssignTask(hero, (Task){E_TASK_SPELL_CONTROL, monster->id, destination.x, destination.y, destination.x, destination.y});
                                    continue;
                                }
                            }
                        }
                        else if (nb_ennemies_hero_see > 1)
                        {
                            Entity *entities = sortEntitiesWithLocationByDistance(_game.monsters, hero->location, HERO_VIEW_DISTANCE);
                            Point best_location = findBestPointByEntities(entities, hero);
                            float distance_entities = findDistanceByEntities(entities, hero);

                            fprintf(stderr, "Best location: %d, %d\n", best_location.x, best_location.y);
                            fprintf(stderr, "Distance entities: %f\n", distance_entities);

                            for (size_t i = 0; IS_VALID_ENTITY(entities[i]); i++)
                                fprintf(stderr, "Distance to entity %d: %f\n", entities[i].id, DISTANCE(hero->location, entities[i].location));

                            float distance_wind = DISTANCE(best_location, hero->location);
                            int nb_ennemies_affected_by_wind = getEntityLength(sortEntitiesWithLocationByDistance(entities, hero->location, DISTANCE_SPELL_WIND));
                            int entity_level_moyenne = getTotalEntityLevelByDistance(entities, hero->location, DISTANCE_SPELL_WIND);
                            fprintf(stderr, "Distance wind: %f\n", distance_wind);
                            fprintf(stderr, "Nb ennemies affected by wind: %d\n", nb_ennemies_affected_by_wind);
                            fprintf(stderr, "Entity level moyenne [%d]\n", entity_level_moyenne);
                            if ((nb_ennemies_affected_by_wind > 1 || entity_level_moyenne > 2) && HAS_ENOUGH_MANA())
                            {
                                Point destination = ENNEMY_BASE_LOCATION(_game.base_location);
                                heroAssignTask(hero, (Task){E_TASK_SPELL_WIND, 0, 0, 0, destination.x, destination.y});
                                continue;
                            }
                            else
                            {
                                heroAssignTask(hero, (Task){E_TASK_MOVE, 0, best_location.x, best_location.y, 0, 0});
                                continue;
                            }
                        }
                        continue;
                    }
                    // Si aucune action n'a été faites, le héro bouge entre 2 points de la base ennemie
                    // static int movement = 0;

                    fprintf(stderr, "No entity found, moving location\n");
                    Point destination = heroGetRandomIADistance(hero);
                    heroAssignTask(hero, (Task){E_TASK_MOVE, 0, destination.x, destination.y});
                    continue;
                }
                else
                {
                    fprintf(stderr, "NB MONSTERS: %d\n", _game.nb_monsters);
                    Entity *entities = sortEntitiesNotFocuses(_game.monsters, _game.heroes, hero);
                    for (size_t i = 0; !NO_MONSTERS_VIEWED() && IS_VALID_ENTITY(entities[i]); i++)
                        fprintf(stderr, "Not focused Entity %d\n", entities[i].id);
                    fprintf(stderr, "Try to find a monster to attack\n");
                    Entity *nearest_entity = findNearestEntity(hero, entities, HERO_VIEW_DISTANCE);
                    fprintf(stderr, "Nearest Entity: %d\n", nearest_entity ? nearest_entity->id : -1);

                    if (nearest_entity == NULL)
                    {
                        if (hero->task.type != E_TASK_MOVE)
                        {
                            fprintf(stderr, "No entity found, moving location\n");
                            Point destination = heroGetRandomIADistance(hero);
                            fprintf(stderr, "Moving to %d, %d\n", destination.x, destination.y);
                            heroAssignTask(hero, (Task){E_TASK_MOVE, 0, destination.x, destination.y});
                        }
                    }
                    else
                    {
                        heroAssignTask(hero, (Task){E_TASK_ATTACK, nearest_entity->id, 0, 0});
                        continue;
                    }
                }
            }
        }

        for (int i = 0; i < _game.nb_heroes; i++)
        {
            Hero *hero = &_game.heroes[i];

            heroDoTask(hero);
        }
    }

    return 0;
}