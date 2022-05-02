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
#define CRITICAL_BASE_RANGE 300 + 1000

#define HERO_VIEW_DISTANCE 2200

#define MONSTER_SPEED 400
#define HERO_SPEED 800
#define HERO_REACH_DISTANCE 10
#define HERO_DAMAGES 2

#define DISTANCE_SPELL_WIND 1280
#define SPELL_WIND_PUISSANCE 2200
#define DISTANCE_SPELL_CONTROL 2200
#define DISTANCE_SPELL_SHIELD 2200

#define MANA_TO_CAST_SPELL 10

#define TURN_TO_DEFFENCE_BASE 25
#define TURN_TO_ATTACK_ENNEMY_BASE 50
#define TURN_TO_ATTACK_BY_CENTER 100 // Strat, attack base en regroupant des mobs via le centre (comme le boss)

#define MIN_VALUE_TO_ACCEPT_WEIGHT 0.5

#define BASE_IN_TOP_LEFT(point) (point.x < (MAX_X / 2) && point.y < (MAX_Y / 2))
#define ENNEMY_BASE_LOCATION(point) (BASE_IN_TOP_LEFT(point) == true ? (Point){MAX_X, MAX_Y} : (Point){0, 0})
#define DISTANCE(point, second_point) sqrt(pow(point.x - second_point.x, 2) + pow(point.y - second_point.y, 2))
#define RANDOM(min, max) (rand() % (max - min + 1) + min)
#define ENTITY_LEVEL(entity) (entity.health < 14 ? 0 : entity.health < 19 ? 1 \
                                                                          : 2)
#define HAS_ENOUGH_MANA() (_game.players[PLAYER_OWN].mana >= MANA_TO_CAST_SPELL)
#define TURN_TO_REACH_BASE(entity) (DISTANCE(entity->location, _game.base_location) / MONSTER_SPEED)

#define IS_VALID_ENTITY(e) e.id > 0
#define IS_VALID_HERO(e) e.id >= 0
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

    bool saw_ennemy_first_time;
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
float invert_value(float value, float min, float max)
{
    return (max + min) - value;
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
Point generateRandomAttackPoint()
{
    static size_t i = 0;
    Point location;

    if (i >= 3)
        i = 0;
    switch (i)
    {
    case 0:
    {
        location = (Point){15000, 2500};
        break;
    }
    case 1:
    {
        location = (Point){11500, 7500};
        break;
    }
    case 2:
    {
        location = (Point){10000, 4500};
        break;
    }
    }
    ++i;
    if (_game.base_in_top_left == false)
        invert_location(&location, MIN_X, MAX_X, MIN_Y, MAX_Y);
    fprintf(stderr, "Attack Point: %d %d\n", location.x, location.y);
    return (location);
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
Entity *findNearestEntityWitHighestLife(Entity *entities, Point location, float distance)
{
    Entity *nearest_entity = NULL;
    float max_distance = distance;
    float max_life = 0;

    for (size_t i = 0; IS_VALID_ENTITY(entities[i]); i++)
    {
        float distance = DISTANCE(location, entities[i].location);
        if (distance < max_distance)
        {
            max_distance = distance;
            nearest_entity = &entities[i];
        }
        if (entities[i].health > max_life)
        {
            max_life = entities[i].health;
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
Entity *sortEntitiesNotControled(Entity *entities)
{
    if (!entities || NO_MONSTERS_VIEWED())
        return (NULL);

    Entity *sorted_entities = malloc(sizeof(Entity) * (_game.nb_monsters + 1));
    memset(sorted_entities, 0, sizeof(Entity) * (_game.nb_monsters + 1));
    int nb_entities = 0;
    for (size_t i = 0; IS_VALID_ENTITY(entities[i]); i++)
    {
        if (entities[i].is_controlled == 0)
        {
            sorted_entities[nb_entities] = entities[i];
            nb_entities++;
        }
    }
    return (sorted_entities);
}
Entity *getEntityById(size_t id)
{
    for (size_t i = 0; i < _game.nb_monsters; i++)
    {
        if (_game.monsters[i].id == id)
            return (&_game.monsters[i]);
    }
    return (NULL);
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
Point getEntityLocationAfterWind(Entity *entity, Point wind_destination, float distance)
{

    // todo end
    Point destination;
    Point start = entity->location;
    float angle = atan2(start.y - wind_destination.y, start.x - wind_destination.x) * 180 / M_PI;
    // float angle = fmod(atan2(start.x * wind_destination.y - wind_destination.x * start.y, start.x * wind_destination.x + start.y * wind_destination.y), 2 * M_PI) * 180 / M_PI;
    fprintf(stderr, "\t - Angle : %f\n", angle);
    fprintf(stderr, "\t - Distance : %f\n", distance);
    fprintf(stderr, "\t - Angle [0 - 360] : %f\n", angle * M_PI / 180);
    fprintf(stderr, "\t - Cos : %f\n", cos(angle));
    fprintf(stderr, "\t - Sin : %f\n", sin(angle));
    fprintf(stderr, "\t - Distance with Cos : %f\n", distance * cos(angle));
    fprintf(stderr, "\t - Distance with Sin : %f\n", distance * sin(angle));

    destination.x = start.x + distance * cos(angle);
    destination.y = start.y + distance * sin(angle);
    fprintf(stderr, "\t - Destination : %d, %d\n", destination.x, destination.y);
    return (destination);
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
Point heroFirstMovementTillSeeEntity(Hero *hero)
{
    Point location;
    switch (hero->id)
    {
    case 0:
    {
        location.x = (MAX_X / 2) + HERO_VIEW_DISTANCE;
        location.y = 1500;
        break;
    }
    case 1:
    {
        location.x = 3130;
        location.y = 5800;
        break;
    }
    case 2:
    {
        location.x = 5730;
        location.y = 3000;
        break;
    }
    default:
        break;
    }
    if (BASE_IN_TOP_LEFT(_game.base_location) == false)
        invert_location(&location, MIN_X, MAX_X, MIN_Y, MAX_Y);
    return (location);
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
Hero *findNearestEnnemyHeroToEntity(Entity *entity)
{
    Hero *nearest_hero = NULL;
    int min_distance = MAX_X + MAX_Y;
    for (int i = 0; i < _game.nb_heroes; i++)
    {
        Hero *hero = &_game.ennemy_heroes[i];
        int distance = DISTANCE(entity->location, hero->location);
        if (distance < min_distance)
        {
            min_distance = distance;
            nearest_hero = hero;
        }
    }
    return (nearest_hero);
}
Hero *findNearestEnnemyHeroByLocationWithDistance(Point location, float dist_max)
{
    Hero *nearest_hero = NULL;
    int min_distance = MAX_X + MAX_Y;
    for (int i = 0; i < _game.nb_heroes; i++)
    {
        Hero *hero = &_game.ennemy_heroes[i];
        float distance = DISTANCE(location, hero->location);
        if (distance < min_distance && distance < dist_max)
        {
            min_distance = distance;
            nearest_hero = hero;
        }
    }
    return (nearest_hero);
}
Hero *sortEnnemyHeroesByDistanceWithLocation(Point location, float dist_max)
{
    Hero *sorted_heroes = malloc(sizeof(Hero) * _game.nb_heroes + 1);
    memset(sorted_heroes, 0, sizeof(Hero) * _game.nb_heroes + 1);
    sorted_heroes[0].id = -1;
    int nb_heroes = 0;
    for (int i = 0; i < _game.nb_heroes; i++)
    {
        Hero *hero = &_game.ennemy_heroes[i];
        int distance = DISTANCE(location, hero->location);
        if (distance < dist_max)
        {
            sorted_heroes[nb_heroes] = *hero;
            sorted_heroes[nb_heroes + 1].id = -1;
            nb_heroes++;
        }
    }
    return (sorted_heroes);
}
Hero *sortHeroesByDistanceWithLocation(Point location, float dist_max)
{
    Hero *sorted_heroes = malloc(sizeof(Hero) * _game.nb_heroes + 1);
    memset(sorted_heroes, 0, sizeof(Hero) * _game.nb_heroes + 1);
    sorted_heroes[0].id = -1;
    int nb_heroes = 0;
    for (int i = 0; i < _game.nb_heroes; i++)
    {
        Hero *hero = &_game.heroes[i];
        int distance = DISTANCE(location, hero->location);
        if (distance < dist_max)
        {
            sorted_heroes[nb_heroes] = *hero;
            sorted_heroes[nb_heroes + 1].id = -1;
            nb_heroes++;
        }
    }
    return (sorted_heroes);
}
Hero *findNearestHeroByLocationWithDistance(Hero *heroes, Point location, float dist_max)
{
    Hero *nearest_hero = NULL;
    int min_distance = MAX_X + MAX_Y;

    if (!heroes)
        return (NULL);
    for (int i = 0; i < IS_VALID_HERO(heroes[i]) && i < _game.nb_heroes; i++)
    {
        if (heroes[i].id == -1)
            break;
        Hero *hero = &heroes[i];
        int distance = DISTANCE(location, hero->location);
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
size_t getHeroesLength(Hero *heroes)
{
    size_t length = 0;
    for (size_t i = 0; heroes[i].id >= 0; i++)
        length++;
    return (length);
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
    _game.ennemy_heroes[0].id = -1;
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
            _game.ennemy_heroes[i_ennemy_heroes + 1].id = -1;
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
            TPair best_monster = {0, 0};

            Entity *entities_in_base_to_attack = entities_in_base;
            Entity *entity_to_target = NULL;
            int nb_monster_in_base = getEntityLength(entities_in_base);

            for (size_t j = 0; IS_VALID_ENTITY(entities_in_base_to_attack[j]); j++)
            {
                // fprintf(stderr, "--- ATTACK ---\n");
                Entity *ce = &entities_in_base_to_attack[j];
                float value = 0;

                Hero *neh = findNearestEnnemyHeroToEntity(ce);

                float distance_entity = DISTANCE(hero->location, ce->location);
                bool monster_is_in_range = distance_entity <= HERO_VIEW_DISTANCE;

                float ennemy_hero_distance_to_monster = neh == NULL ? 0 : DISTANCE(neh->location, ce->location);
                bool ennemy_hero_is_in_range = ennemy_hero_distance_to_monster <= HERO_VIEW_DISTANCE;

                bool monster_threat_to_base = ce->threat_for == 1;
                bool monster_near_to_base = ce->near_base == 1;

                float distance_to_border_base = DISTANCE(ce->location, _game.base_location) - BASE_RANGE < 0 ? 0 : DISTANCE(ce->location, _game.base_location) - BASE_RANGE;
                float distance_to_base = DISTANCE(ce->location, _game.base_location);

                size_t nb_turn_entity_will_reach_base = TURN_TO_REACH_BASE(ce);

                // fprintf(stderr, "\t - Distance to monster %d : %f\n", ce->id, distance_entity);
                // fprintf(stderr, "\t - Monster is in range : %d\n", monster_is_in_range);
                // fprintf(stderr, "\t - Ennemy hero distance to monster %d : %f\n", ce->id, ennemy_hero_distance_to_monster);
                // fprintf(stderr, "\t - Ennemy hero is in range : %d\n", ennemy_hero_is_in_range);
                // fprintf(stderr, "\t - Monster threat to base : %d\n", monster_threat_to_base);
                // fprintf(stderr, "\t - Monster near to base : %d\n", monster_near_to_base);
                // fprintf(stderr, "\t - Distance to border base : %f\n", distance_to_border_base);
                // fprintf(stderr, "\t - Distance to base : %f\n", distance_to_base);
                // fprintf(stderr, "\t - Ennemy hero : %d\n", neh == NULL ? -1 : neh->id);
                // fprintf(stderr, "\t - Nb turn entity will reach base : %ld\n", nb_turn_entity_will_reach_base);

                value += invert_value(distance_entity / (HERO_VIEW_DISTANCE / 2), 0, 2); // 0 - 2
                value += (monster_is_in_range ? 1 : 0) * 1;
                value += (ennemy_hero_is_in_range ? 1 : 0) * 1;
                value += ennemy_hero_distance_to_monster > 0 && ennemy_hero_distance_to_monster < HERO_VIEW_DISTANCE ? invert_value(ennemy_hero_distance_to_monster / (HERO_VIEW_DISTANCE / 2), 0, 2) * 1.25 : 0; // 0 - 2
                value += (monster_threat_to_base ? 1 : 0) * 2;
                value += (monster_near_to_base ? 1 : 0) * 2;
                value += invert_value(distance_to_border_base, 0, 1000) / 1000;            // 0 - 1
                value += (invert_value(distance_to_base, 0, BASE_RANGE) / BASE_RANGE) * 2; // 0 - 1
                value += (invert_value(nb_turn_entity_will_reach_base, 0, 12) / 12) * 2;   // a debug
                value = value < 0 ? value * -1 : value;

                // fprintf(stderr, "\n\t - Value : %f\n", value);
                if (value > best_monster.value && value > MIN_VALUE_TO_ACCEPT_WEIGHT)
                {
                    best_monster.task.type = E_TASK_ATTACK;
                    best_monster.value = value;
                    best_monster.task.target_id = ce->id;
                    // fprintf(stderr, "\t - Assign value : %f\n", value);
                }
            }

            best_actions[i].value = best_monster.value;
            best_actions[i].task = best_monster.task;
            // fprintf(stderr, "\t - Continue : %f\n", best_monster.value);
            continue;
        }
        else if (i == E_TASK_SPELL_WIND)
        {
            continue;
            // fprintf(stderr, "--- SPELL WIND ---\n");
            Entity *entities_in_range_to_wind = sortEntitiesWithLocationByDistance(_game.monsters, hero->location, DISTANCE_SPELL_WIND);
            Entity *entities_not_shielded = sortEntitiesNotShielded(entities_in_range_to_wind);

            Hero *A_neh = sortEnnemyHeroesByDistanceWithLocation(hero->location, DISTANCE_SPELL_WIND);

            float distance_entities_to_base = 0;
            size_t level_total_entities = 0;

            size_t j = 0;
            for (j = 0; IS_VALID_ENTITY(entities_in_range_to_wind[j]); j++)
            {
                distance_entities_to_base += DISTANCE(entities_in_range_to_wind[j].location, _game.base_location);
                level_total_entities += ENTITY_LEVEL(entities_in_range_to_wind[j]);
            }
            if (distance_entities_to_base > 0)
                distance_entities_to_base /= j;
            if (level_total_entities != 0)
                level_total_entities /= j;

            float value = 0;

            int nb_ennemies_affected_by_wind = getEntityLength(entities_in_range_to_wind);
            int nb_ennemies_affected_by_wind_not_shielded = getEntityLength(entities_not_shielded);

            // fprintf(stderr, "\t - Distance to base : %f\n", distance_entities_to_base);
            // fprintf(stderr, "\t - Number of monsters affected by wind : %d\n", nb_ennemies_affected_by_wind);
            // fprintf(stderr, "\t - Number of monsters affected by wind not shielded : %d\n", nb_ennemies_affected_by_wind_not_shielded);
            // fprintf(stderr, "\t - Level total entities : %ld\n", level_total_entities);

            if (nb_ennemies_affected_by_wind == 0)
            {
                best_actions[i].task.type = E_TASK_SPELL_WIND;
                best_actions[i].task.target_id = -1;
                best_actions[i].task.spell_vector = ENNEMY_BASE_LOCATION(_game.base_location);
                best_actions[i].value = 0;
                continue;
            }

            value += (nb_ennemies_affected_by_wind);
            value += nb_ennemies_affected_by_wind_not_shielded;
            // value += log(invert_value(distance_entities_to_base, 300, BASE_RANGE) / (BASE_RANGE / 2) * 2);
            value += log(distance_entities_to_base);
            // fprintf(stderr, "\t - Value: %f\n", invert_value(distance_entities_to_base, 300, BASE_RANGE) / (BASE_RANGE / 2) * 2);
            // fprintf(stderr, "\t - Log : %f\n", log(invert_value(distance_entities_to_base, 300, BASE_RANGE) / (BASE_RANGE / 2) * 2));
            // fprintf(stderr, "\t - Log without invert: %f\n", log(distance_entities_to_base)); // checker si le monstre est bien dans la base
            value += (level_total_entities / 4) * 2;
            value = value < 0 ? value * -1 : value;

            // UTILISER LES LOG ! log(value)

            // fprintf(stderr, "\n\t - Value : %f\n", value);

            // tmp le temps de changer
            value = 0;
            best_actions[i].task.type = E_TASK_SPELL_WIND;
            best_actions[i].task.target_id = -1;
            best_actions[i].task.spell_vector = ENNEMY_BASE_LOCATION(_game.base_location);

            if (value > MIN_VALUE_TO_ACCEPT_WEIGHT)
                best_actions[i].value = value;
            else
                best_actions[i].value = 0;
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
    // fprintf(stderr, "End find best ennemy\n");
    //  Spell Wind
    do
    {
        if (HAS_ENOUGH_MANA())
        {
            fprintf(stderr, "--- SPELL WIND ---\n");
            Entity *entities_can_be_winded = sortEntitiesWithLocationByDistance(_game.monsters, hero->location, DISTANCE_SPELL_WIND);
            // fprintf(stderr, "NB entities can be winded: %ld\n", getEntityLength(entities_can_be_winded));
            entities_can_be_winded = sortEntitiesNotShielded(entities_can_be_winded);
            // fprintf(stderr, "NB entities can be winded without shield: %ld\n", getEntityLength(entities_can_be_winded));
            Hero *ennemy_heroes_can_be_winded = sortEnnemyHeroesByDistanceWithLocation(hero->location, DISTANCE_SPELL_WIND);
            // fprintf(stderr, "NB ennemy heroes can be winded: %ld\n", getHeroesLength(ennemy_heroes_can_be_winded));
            size_t nb_entities_can_be_winded = getEntityLength(entities_can_be_winded) + getHeroesLength(ennemy_heroes_can_be_winded);
            bool another_hero_will_wind = false;
            for (size_t i = 0; i < _game.nb_heroes; i++)
            {
                Hero *ce = &_game.heroes[i];

                if (ce->task.type == E_TASK_SPELL_WIND)
                {
                    if (DISTANCE(ce->location, hero->location) <= DISTANCE_SPELL_WIND / 2)
                    {
                        another_hero_will_wind = true;
                        break;
                    }
                }
            }
            // fprintf(stderr, "Another hero will wind: %d\n", another_hero_will_wind);
            bool ennemy_is_in_critical_base_range = (findNearestEntityByLocation(entities_can_be_winded, _game.base_location, CRITICAL_BASE_RANGE) != NULL);
            if (another_hero_will_wind == false || (ennemy_is_in_critical_base_range == true && nb_entities_can_be_winded > 1))
            {
                // fprintf(stderr, "\t - Number of entities in range : %ld\n", nb_entities_can_be_winded);
                if (nb_entities_can_be_winded >= 3)
                {
                    Point location = ENNEMY_BASE_LOCATION(_game.base_location);
                    heroAssignTask(hero, (Task){E_TASK_SPELL_WIND, 0, 0, 0, location.x, location.y});
                    return (true);
                }

                for (size_t i = 0; IS_VALID_ENTITY(entities_can_be_winded[i]); i++)
                {
                    float distance_to_hero = DISTANCE(entities_can_be_winded[i].location, hero->location);
                    float distance_to_base = DISTANCE(entities_can_be_winded[i].location, _game.base_location);

                    // fprintf(stderr, "Distance Entity [%d] to base : %f\n", entities_can_be_winded[i].id, distance_to_base);
                    if (distance_to_base < CRITICAL_BASE_RANGE && nb_entities_can_be_winded > 0)
                    {
                        Point location = ENNEMY_BASE_LOCATION(_game.base_location);
                        heroAssignTask(hero, (Task){E_TASK_SPELL_WIND, 0, 0, 0, location.x, location.y});

                        return (true);
                    }
                }
            }
        }
        break;
    } while (1);

    // Spell Shield
    do
    {
        if (HAS_ENOUGH_MANA())
        {
            fprintf(stderr, "--- SPELL SHIELD ---\n");
            // fprintf(stderr, "Hero [%d]\n", hero->id);
            Hero *heroes_can_be_shielded = sortHeroesByDistanceWithLocation(hero->location, DISTANCE_SPELL_SHIELD);
            size_t nb_heroes_can_be_shielded = getHeroesLength(heroes_can_be_shielded);
            // fprintf(stderr, "Sorted heroes [0 = %d] [1 = %d] \n", heroes_can_be_shielded[0].id, heroes_can_be_shielded[1].id);

            // fprintf(stderr, "\t - Number of heroes in range : %ld\n", nb_heroes_can_be_shielded);
            if (nb_heroes_can_be_shielded > 0)
            {
                Hero *nearestHero = findNearestHeroByLocationWithDistance(heroes_can_be_shielded, hero->location, DISTANCE_SPELL_SHIELD);
                // fprintf(stderr, "\t - Nearest hero : %d\n", nearestHero == NULL ? -1 : nearestHero->id);
                if (nearestHero == NULL)
                    break;
                Entity *entitiesNearestToHero = sortEntitiesWithLocationByDistance(_game.monsters, nearestHero->location, HERO_VIEW_DISTANCE);
                // fprintf(stderr, "\t - Enties nearest to hero : %ld\n", getEntityLength(entitiesNearestToHero));
                Hero *ennemyHeroes = sortEnnemyHeroesByDistanceWithLocation(hero->location, DISTANCE_SPELL_SHIELD);
                // fprintf(stderr, "\t - Ennemy heroes : %ld\n", getHeroesLength(ennemyHeroes));
                size_t nb_ennemy_heroes = getHeroesLength(ennemyHeroes);
                // fprintf(stderr, "\t - Number of ennemy heroes in range : %ld\n", nb_ennemy_heroes);
                Hero *nearestEnnemyHero = findNearestEnnemyHeroByLocationWithDistance(hero->location, DISTANCE_SPELL_SHIELD);
                // fprintf(stderr, "\t - Nearest ennemy hero : %d\n", nearestEnnemyHero ? nearestEnnemyHero->id : -1);
                if (nearestEnnemyHero == NULL)
                    break;
                Entity *entitiesNearestToEnnemyHero = sortEntitiesWithLocationByDistance(_game.monsters, nearestEnnemyHero->location, HERO_VIEW_DISTANCE);
                // fprintf(stderr, "\t - Enties nearest to ennemy hero : %ld\n", getEntityLength(entitiesNearestToEnnemyHero));
                size_t nbEntitiesNearToEnnemyHero = getEntityLength(entitiesNearestToEnnemyHero);
                // fprintf(stderr, "\t - Number of entities near to ennemy hero : %ld\n", nbEntitiesNearToEnnemyHero);
                Hero *nearestHeroToEnnemyHero = findNearestHeroByLocationWithDistance(_game.heroes, nearestEnnemyHero->location, DISTANCE_SPELL_SHIELD);
                // fprintf(stderr, "\t - Nearest hero to ennemy hero : %d\n", nearestHeroToEnnemyHero ? nearestHeroToEnnemyHero->id : -1);

                if (nearestEnnemyHero != NULL)
                {
                    if (nbEntitiesNearToEnnemyHero > 1)
                    {
                        if (DISTANCE(nearestEnnemyHero->location, hero->location) <= DISTANCE_SPELL_SHIELD / 2)
                        {
                            if (nearestHeroToEnnemyHero->shield_life <= 0)
                            {
                                heroAssignTask(hero, (Task){E_TASK_SPELL_SHIELD, nearestHeroToEnnemyHero->id, 0, 0, 0, 0});
                                return (true);
                            }
                        }
                    }
                }
            }
        }
        break;
    } while (1);

    // Spell Control
    do
    {
        if (HAS_ENOUGH_MANA())
        {
            fprintf(stderr, "--- SPELL CONTROL ---\n");
            Hero *ennemyHeroesCanBeControlled = sortEnnemyHeroesByDistanceWithLocation(hero->location, DISTANCE_SPELL_CONTROL);
            Hero *nearestEnnemyHero = findNearestEnnemyHeroByLocationWithDistance(hero->location, DISTANCE_SPELL_CONTROL);
            // fprintf(stderr, "Nearest ennemy hero : %d\n", nearestEnnemyHero ? nearestEnnemyHero->id : -1);
            if (nearestEnnemyHero == NULL || ennemyHeroesCanBeControlled == NULL)
                break;
            else if (IS_VALID_HERO((*nearestEnnemyHero)) == false)
                break;
            Entity *entitiesNearestToEnnemyHero = sortEntitiesWithLocationByDistance(_game.monsters, nearestEnnemyHero->location, HERO_VIEW_DISTANCE);
            // fprintf(stderr, "Enties nearest to ennemy hero : %ld\n", getEntityLength(entitiesNearestToEnnemyHero));
            size_t nbEntitiesNearestToEnnemyHero = getEntityLength(entitiesNearestToEnnemyHero);

            if (nbEntitiesNearestToEnnemyHero > 2)
            {
                if (DISTANCE(nearestEnnemyHero->location, hero->location) <= DISTANCE_SPELL_CONTROL / 1.5)
                {
                    if (nearestEnnemyHero->shield_life <= 0 && nearestEnnemyHero->is_controlled == false)
                    {
                        // fprintf(stderr, "Control ennemy hero %d\n", nearestEnnemyHero->id);
                        Point location = ENNEMY_BASE_LOCATION(_game.base_location);
                        heroAssignTask(hero, (Task){E_TASK_SPELL_CONTROL, nearestEnnemyHero->id, location.x, location.y, 0, 0});
                        return (true);
                    }
                }
            }
        }
        break;
    } while (1);

    TPair best_action = {0, 0};
    for (size_t i = 0; i < 6; i++)
    {
        // fprintf(stderr, "Task %ld : %f\n", i, best_actions[i].value);
        if (best_actions[i].value > best_action.value)
            best_action = best_actions[i];
    }
    // fprintf(stderr, "Best action : %d\n", best_action.task.type);
    // fprintf(stderr, "Best action value : %f\n", best_action.value);
    if (best_action.task.type == E_TASK_WAIT || best_action.value <= 0)
        return (false);
    heroAssignTask(hero, best_action.task);
    // fprintf(stderr, "Hero [%d] assigned task : %d\n", hero->id, best_action.task.type);
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
static bool IA_Attack(Hero *hero)
{
    // if (_game.nb_turn >= TURN_TO_ATTACK_BY_CENTER)
    // {
    //     return (false);
    // }
    // else
    // {

    static size_t targetEntityID = -1;
    static Point lastEntityLocation = {-1, -1};
    static int turn_entity = 0;

    fprintf(stderr, "\t- TargetEntity ID: %ld\n", targetEntityID);
    if (targetEntityID >= 0)
    {
        Entity *targetEntity = getEntityById(targetEntityID);
        fprintf(stderr, "\t- TargetEntity: %d\n", targetEntity ? targetEntity->id : -1);
        if (targetEntity == NULL && targetEntityID > 0 &&
            (lastEntityLocation.x != -1 && lastEntityLocation.y != -1))
        {
            ++turn_entity;
            fprintf(stderr, "\t- TargetEntity is not visible, moving to last location : %d %d\n", lastEntityLocation.x, lastEntityLocation.y);
            heroAssignTask(hero, (Task){E_TASK_MOVE, 0, lastEntityLocation.x, lastEntityLocation.y, 0, 0});

            if (turn_entity >= 3 - 1)
            {
                targetEntityID = -1;
                turn_entity = 0;
                lastEntityLocation = (Point){-1, -1};
            }
            return (true);
            // targetEntityID = -1;
        }
        else if (targetEntity)
        {
            float distance = DISTANCE(hero->location, targetEntity->location);
            size_t nb_ennemies_in_range = getHeroesLength(sortEnnemyHeroesByDistanceWithLocation(targetEntity->location, HERO_VIEW_DISTANCE));
            Hero *nearestEnnemyHero = findNearestEnnemyHeroByLocationWithDistance(targetEntity->location, HERO_VIEW_DISTANCE);

            Entity *entitiesInRange = sortEntitiesWithLocationByDistance(_game.monsters, hero->location, HERO_VIEW_DISTANCE);
            entitiesInRange = sortEntitiesNotShielded(entitiesInRange);

            fprintf(stderr, "\t- Distance: %f\n", distance);
            fprintf(stderr, "\t- Nb ennemies in range: %ld\n", nb_ennemies_in_range);
            fprintf(stderr, "\t- Nearest ennemy hero: %d\n", nearestEnnemyHero ? nearestEnnemyHero->id : -1);

            if (getEntityLength(entitiesInRange) > 1)
            {
                Entity *bestEntityToShield = findNearestEntityWitHighestLife(entitiesInRange, hero->location, HERO_VIEW_DISTANCE);
                if (bestEntityToShield)
                {
                    if (DISTANCE(bestEntityToShield->location, ENNEMY_BASE_LOCATION(_game.base_location)) < BASE_RANGE &&
                        bestEntityToShield->health > 10)
                    {
                        fprintf(stderr, "\t- Best entity to shield: %d\n", bestEntityToShield->id);
                        heroAssignTask(hero, (Task){E_TASK_SPELL_SHIELD, bestEntityToShield->id, 0, 0, 0, 0});
                        return (true);
                    }
                }
            }
            if (distance > DISTANCE_SPELL_WIND)
            {
                heroForceInterruptTask(hero);
                heroAssignTask(hero, (Task){E_TASK_MOVE, 0, targetEntity->location.x, targetEntity->location.y, 0, 0});
                return (true);
            }
            if (nb_ennemies_in_range == 1)
            {
                heroForceInterruptTask(hero);
                heroAssignTask(hero, (Task){E_TASK_SPELL_CONTROL, nearestEnnemyHero->id, 0, 0, 0, 0});
                return (true);
            }
            bool entityWillBeInBaseAfterWind = (DISTANCE(targetEntity->location, ENNEMY_BASE_LOCATION(_game.base_location)) < (BASE_RANGE + DISTANCE_SPELL_WIND));
            if (DISTANCE(hero->location, targetEntity->location) < DISTANCE_SPELL_WIND &&
                entityWillBeInBaseAfterWind)
            {
                heroForceInterruptTask(hero);
                Point destination = ENNEMY_BASE_LOCATION(_game.base_location);
                lastEntityLocation = targetEntity->location;
                lastEntityLocation.x += HERO_VIEW_DISTANCE / 2;
                lastEntityLocation.y += HERO_VIEW_DISTANCE / 2;
                heroAssignTask(hero, (Task){E_TASK_SPELL_WIND, 0, 0, 0, destination.x, destination.y});
                return (true);
            }
        }
    }
    fprintf(stderr, "\t- No target entity found, moving location\n");

    Entity *nearestEntity = findNearestEntityByLocation(_game.monsters, hero->location, DISTANCE_SPELL_WIND);
    Entity *entitiesNearestToHero = sortEntitiesWithLocationByDistance(_game.monsters, hero->location, HERO_VIEW_DISTANCE);
    
    if (entitiesNearestToHero && getEntityLength(entitiesNearestToHero) == 1)
    {
        entitiesNearestToHero = sortEntitiesNotShielded(entitiesNearestToHero);
        entitiesNearestToHero = sortEntitiesNotControled(entitiesNearestToHero);
        nearestEntity = findNearestEntityByLocation(entitiesNearestToHero, hero->location, DISTANCE_SPELL_CONTROL);
        fprintf(stderr, "\t- Nearest entity to hero: %d\n", nearestEntity ? nearestEntity->id : -1);
        if (nearestEntity != NULL)
        {
            if (nearestEntity->threat_for != 2 && nearestEntity->health > 10)
            {
                heroForceInterruptTask(hero);
                Point location = ENNEMY_BASE_LOCATION(_game.base_location);
                heroAssignTask(hero, (Task){E_TASK_SPELL_CONTROL, nearestEntity->id, location.x, location.y, 0, 0});
                return (true);
            }
        }
    }
    else
    {
        fprintf(stderr, "\t- Nearest entity: %d\n", nearestEntity ? nearestEntity->id : -1);
        if (nearestEntity != NULL)
        {
            bool entityWillBeInBaseAfterWind = (DISTANCE(nearestEntity->location, ENNEMY_BASE_LOCATION(_game.base_location)) < (BASE_RANGE + DISTANCE_SPELL_WIND));
            if (DISTANCE(nearestEntity->location, hero->location) < DISTANCE_SPELL_WIND)
            {
                if (HAS_ENOUGH_MANA())
                {
                    targetEntityID = nearestEntity->id;
                    lastEntityLocation = nearestEntity->location;
                    lastEntityLocation.x += HERO_VIEW_DISTANCE / 2;
                    lastEntityLocation.y += HERO_VIEW_DISTANCE / 2;
                    Point destination = ENNEMY_BASE_LOCATION(_game.base_location);
                    heroAssignTask(hero, (Task){E_TASK_SPELL_WIND, 0, 0, 0, destination.x, destination.y});
                    return (true);
                }
            }
        }
    }

    float distance_to_ennemie_base = DISTANCE(hero->location, ENNEMY_BASE_LOCATION(_game.base_location));
    fprintf(stderr, "\t- Distance to ennemie base: %f\n", distance_to_ennemie_base);
    if (distance_to_ennemie_base > (VIEW_DISTANCE_BASE + HERO_VIEW_DISTANCE))
    {
        Point destination = generateRandomAttackPoint();
        fprintf(stderr, "Destination: %d, %d\n", destination.x, destination.y);
        heroAssignTask(hero, (Task){E_TASK_MOVE, 0, destination.x, destination.y});
        heroSetTaskUninterrupted(hero, 0);
        return (true);
    }
    // Old IA
    /*
    float distance_to_ennemy_base = DISTANCE(ENNEMY_BASE_LOCATION(_game.base_location), hero->location);
    Hero *ennemy_hero_see = sortEnnemyHeroesByDistanceWithLocation(hero->location, HERO_VIEW_DISTANCE);
    Entity *monsters = sortEntitiesWithLocationByDistance(_game.monsters, hero->location, HERO_VIEW_DISTANCE);
    size_t nb_ennemy_heroes_hero_see = getHeroesLength(ennemy_hero_see);
    int nb_monsters_hero_see = getEntityLength(monsters);

    fprintf(stderr, "NB ennemies hero see: %ld\n", nb_ennemy_heroes_hero_see);
    fprintf(stderr, "NB monsters hero see: %d\n", nb_monsters_hero_see);
    fprintf(stderr, "Distance to ennemy base: %f\n", distance_to_ennemy_base);
    if (distance_to_ennemy_base > (VIEW_DISTANCE_BASE + HERO_VIEW_DISTANCE) || nb_monsters_hero_see <= 0)
    {
        // Point destination = generateRandomPointInCircleWithMinMax(ENNEMY_BASE_LOCATION(_game.base_location),
        //   VIEW_DISTANCE_BASE + HERO_VIEW_DISTANCE, BASE_RANGE);
        Point destination = generateRandomAttackPoint();

        fprintf(stderr, "Destination: %d, %d\n", destination.x, destination.y);
        heroAssignTask(hero, (Task){E_TASK_MOVE, 0, destination.x, destination.y});
        heroSetTaskUninterrupted(hero, 0);
        return (true);
    }
    else
    {
        if (nb_ennemy_heroes_hero_see > 0)
        {
            Hero *nearest_ennemy_hero = findNearestEnnemyHeroByLocationWithDistance(hero->location, VIEW_DISTANCE_BASE);
            Entity *nearest_monster_to_ennemy_hero = findNearestEntityByLocation(_game.monsters, nearest_ennemy_hero->location, HERO_VIEW_DISTANCE);
            size_t nb_monsters_near_to_hero = getNBEntitiesHeroSee(_game.monsters, nearest_ennemy_hero);

            if (nb_monsters_near_to_hero >= 1 && DISTANCE(nearest_ennemy_hero->location, nearest_monster_to_ennemy_hero->location) < DISTANCE_SPELL_WIND &&
                nearest_ennemy_hero->is_controlled == false && (nearest_monster_to_ennemy_hero->threat_for == 2 || nearest_monster_to_ennemy_hero->near_base == 2))
            {
                Point destination = _game.base_location;
                heroAssignTask(hero, (Task){E_TASK_SPELL_CONTROL, nearest_ennemy_hero->id, destination.x, destination.y, destination.x, destination.y});
                return (true);
            }
        }
        if (nb_monsters_hero_see == 1)
        {
            Entity *monster = findNearestEntity(hero, _game.monsters, HERO_VIEW_DISTANCE);

            fprintf(stderr, "Monster: %d\n", monster ? monster->id : -1);
            fprintf(stderr, "Monster is controlled: %d\n", monster ? monster->is_controlled : -1);
            if (monster != NULL)
            {
                if (monster->threat_for != 2 && monster->near_base != 2 &&
                    HAS_ENOUGH_MANA() && monster->is_controlled == false)
                {
                    Point destination = ENNEMY_BASE_LOCATION(_game.base_location);
                    heroAssignTask(hero, (Task){E_TASK_SPELL_CONTROL, monster->id, destination.x, destination.y, destination.x, destination.y});
                    return (true);
                }
            }
        }
        else if (nb_monsters_hero_see > 1)
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
                return (true);
            }
            else
            {
                heroAssignTask(hero, (Task){E_TASK_MOVE, 0, best_location.x, best_location.y, 0, 0});
                return (true);
            }
        }
        return (true);
    }
    // Si aucune action n'a été faites, le héro bouge entre 2 points de la base ennemie
    // static int movement = 0;

    fprintf(stderr, "No entity found, moving location\n");
    // Point destination = heroGetRandomIADistance(hero);
    Point destination = generateRandomAttackPoint();
    heroAssignTask(hero, (Task){E_TASK_MOVE, 0, destination.x, destination.y});
    */
    // }
    // Si aucune action n'a été faites, le héro bouge entre 3 points de la base ennemie
    Point destination = generateRandomAttackPoint();
    heroAssignTask(hero, (Task){E_TASK_MOVE, 0, destination.x, destination.y});
    heroSetTaskUninterrupted(hero, 0);
    return (true);
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

            if (findNearestEntityByLocation(_game.monsters, hero->location, HERO_VIEW_DISTANCE) != NULL)
                hero->saw_ennemy_first_time = true;
            if (hero->saw_ennemy_first_time == false)
            {
                Point location;
                location = heroFirstMovementTillSeeEntity(hero);
                heroAssignTask(hero, (Task){E_TASK_MOVE, 0, location.x, location.y});
                continue;
            }

            // fprintf(stderr, " - Nb Monsters in Base: %ld\n", getNBEntitiesInBase());
            // IA Defense Base [1 / 2]
            if (getNBEntitiesInBase() > 0 && hero->id != 0)
            {
                if ((IA_DefenseBase(hero)) == true)
                    continue;
                else
                {
                    // fprintf(stderr, "NB MONSTERS: %d\n", _game.nb_monsters);
                    Entity *entities = sortEntitiesNotFocuses(_game.monsters, _game.heroes, hero);
                    // for (size_t i = 0; !NO_MONSTERS_VIEWED() && IS_VALID_ENTITY(entities[i]); i++)
                    // fprintf(stderr, "Not focused Entity %d\n", entities[i].id);
                    // fprintf(stderr, "Try to find a monster to attack\n");
                    Entity *nearest_entity = findNearestEntity(hero, entities, HERO_VIEW_DISTANCE);
                    // fprintf(stderr, "Nearest Entity: %d\n", nearest_entity ? nearest_entity->id : -1);

                    if (nearest_entity == NULL)
                    {
                        if (hero->task.type != E_TASK_MOVE)
                        {
                            // fprintf(stderr, "No entity found, moving location\n");
                            Point destination = heroGetRandomIADistance(hero);
                            // fprintf(stderr, "Moving to %d, %d\n", destination.x, destination.y);
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
                if (hero->id == 0 && (_game.nb_turn >= TURN_TO_ATTACK_ENNEMY_BASE ||
                                      DISTANCE(hero->location, ENNEMY_BASE_LOCATION(_game.base_location)) <= (VIEW_DISTANCE_BASE + HERO_VIEW_DISTANCE)))
                {
                    if ((IA_Attack(hero)) == true)
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