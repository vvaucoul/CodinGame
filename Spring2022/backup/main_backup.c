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

#define HERO_VIEW_DISTANCE 2200

#define ENNEMY_SPEED 400
#define HERO_SPEED 800
#define HERO_REACH_DISTANCE 10

#define BASE_IN_TOP_LEFT(point) (point.x < (MAX_X / 2) && point.y < (MAX_Y / 2))
#define DISTANCE(point, second_point) sqrt(pow(point.x - second_point.x, 2) + pow(point.y - second_point.y, 2))
#define RANDOM(min, max) (rand() % (max - min + 1) + min)

#define IS_VALID_ENTITY(e) e.id >= 0
#define EMPTY_ENTITY \
    {                \
        -1           \
    }

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
    return (point);
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
Entity **sortEntitiesNotFocuses(Entity *entities, Hero *heroes)
{
    Entity **sorted_entities = malloc(sizeof(Entity *) * _game.nb_entities);
    int nb_entities = 0;

    for (size_t i = 0; IS_VALID_ENTITY(entities[i]); i++)
    {
        bool add = false;

        for (size_t j = 0; j < _game.nb_heroes; j++)
        {
            if (entities[i].id == heroes[j].task.target_id)
                add = true;
        }
        if (add)
        {
            sorted_entities[nb_entities] = &entities[i];
            nb_entities++;
        }
    }
    return (sorted_entities);
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
// Heros Functions
void heroAssignTask(Hero *hero, Task task)
{
    if (hero->uninterupt_task == false)
        hero->task = task;
}
void heroSetTaskUninterrupted(Hero *hero)
{
    hero->uninterupt_task = true;
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
    location.x = RANDOM(MIN_X, MAX_X);
    location.y = RANDOM(MIN_Y, MAX_Y);
    return (location);

    switch (id)
    {
    case 0:
    {
        Point aggressive_location;

        if (_game.nb_turn < 10)
        {
            aggressive_location.x = RANDOM((int)(MAX_X / 2.5), (int)(MAX_X / 1.25));
            aggressive_location.y = RANDOM((int)(MAX_Y / 3), (int)(MAX_Y / 1.5));
        }
        else
        {
            aggressive_location.x = RANDOM((int)(MAX_X / 1.75), (int)(MAX_X / 1.15));
            aggressive_location.y = RANDOM((int)(MAX_Y / 2.25), (int)(MAX_Y / 1.15));
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
            deffensive_location.x = RANDOM((int)(MAX_X / 2.5), (int)(MAX_X / 1.75));
            deffensive_location.y = RANDOM((int)(MAX_Y / 8), (int)(MAX_Y / 3));
        }
        else
        {
            Point random = generateRandomPointInCircle((Point){6500, 2000}, HERO_VIEW_DISTANCE);
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
            deffensive_location.x = RANDOM((int)(MAX_X / 4), (int)(MAX_X / 2));
            deffensive_location.y = RANDOM((int)(MAX_Y / 1.75), (int)(MAX_Y / 1.25));
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
    _game.monsters[0].id = -1;
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

            _game.monsters[i_monsters + 1].id = -1;
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
    }
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

            // todo ajouter id 0 && 1 / 2 pour les 2 types d IA
            if (getNBEntitiesInBase() > 0)
            {
                Entity *entity_nearest_to_base = findNearestEntityByLocation(_game.monsters, _game.base_location, VIEW_DISTANCE_BASE);
                if (entity_nearest_to_base != NULL)
                {
                    // isEntityFocuses(entity_nearest_to_base);
                    fprintf(stderr, " - Entity nearest to base: %d\n", entity_nearest_to_base->id);
                    heroForceInterruptTask(hero);
                    heroAssignTask(hero, (Task){E_TASK_ATTACK, entity_nearest_to_base->id, 0, 0});
                }
            }
            else
            {
                fprintf(stderr, "Distance to base: %f > %f\n", DISTANCE(_game.base_location, hero->location), VIEW_DISTANCE_BASE * 1.5);
                if (DISTANCE(_game.base_location, hero->location) > VIEW_DISTANCE_BASE * 1.5)
                {
                    fprintf(stderr, " - Hero is too far from base\n");
                    Point destination = heroGetRandomIADistance(hero);
                    heroAssignTask(hero, (Task){E_TASK_MOVE, destination.x, destination.y, 0});
                    heroSetTaskUninterrupted(hero);
                    continue;
                }

                Entity *nearest_entity = findNearestEntity(hero, _game.monsters, HERO_VIEW_DISTANCE);
                fprintf(stderr, "Nearest Entity: %d\n", nearest_entity ? nearest_entity->id : -1);

                Task task = {0};
                if (nearest_entity == NULL)
                {
                    if (hero->task.type != E_TASK_MOVE)
                    {
                        fprintf(stderr, "No entity found, moving location\n");
                        task.destination = heroGetRandomIADistance(hero);
                        heroAssignTask(hero, (Task){E_TASK_MOVE, 0, task.destination.x, task.destination.y});
                    }
                    else
                        task = hero->task;
                }
                else
                {
                    task.type = E_TASK_ATTACK;
                    task.target_id = nearest_entity->id;
                    heroAssignTask(hero, task);
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