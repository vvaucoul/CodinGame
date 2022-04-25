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

#define BASE_IN_TOP_LEFT(point) (point.x < (MAX_X / 2) && point.y < (MAX_Y / 2))
#define DISTANCE(point, second_point) sqrt(pow(point.x - second_point.x, 2) + pow(point.y - second_point.y, 2))

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
} t_hero;
#define Hero t_hero

typedef struct s_game
{
    Point base_location;
    bool base_in_top_left;

    int nb_heroes;
    int nb_entities;
    int nb_monsters;
    Player players[2];

    Entity monsters[1024];
    Hero heroes[3];
    Hero ennemy_heroes[3];
} t_game;
#define Game t_game

Game _game;

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

// Heros Functions
void heroAssignTask(Hero *hero, Task task)
{
    hero->task = task;
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
        printf("MOVE %d %d [%d] Attack [%d]\n", target->location.x, target->location.y, hero->id, target->id);
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

// Game Functions
static void init_startup_game_values()
{
    scanf("%d%d", &_game.base_location.x, &_game.base_location.y);
    scanf("%d", &_game.nb_heroes);
    _game.base_in_top_left = BASE_IN_TOP_LEFT(_game.base_location);

    memset(_game.heroes, 0, sizeof(_game.heroes));
    memset(_game.ennemy_heroes, 0, sizeof(_game.ennemy_heroes));
    memset(_game.monsters, 0, sizeof(_game.monsters));
    _game.monsters[0].id = -1;
}
static void init_game_values()
{
    for (int i = 0; i < 2; i++)
        scanf("%d%d", &_game.players[i].health, &_game.players[i].mana);
    scanf("%d", &_game.nb_entities);
    _game.nb_monsters = _game.nb_entities - _game.nb_heroes;

    memset(_game.ennemy_heroes, 0, sizeof(_game.ennemy_heroes));
    memset(_game.monsters, 0, sizeof(_game.monsters));
    _game.monsters[0].id = -1;

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
            _game.ennemy_heroes[i_ennemy_heroes].id = id;
            _game.ennemy_heroes[i_ennemy_heroes].type = type;
            _game.ennemy_heroes[i_ennemy_heroes].location.x = x;
            _game.ennemy_heroes[i_ennemy_heroes].location.y = y;
            _game.ennemy_heroes[i_ennemy_heroes].shield_life = shield_life;
            _game.ennemy_heroes[i_ennemy_heroes].is_controlled = is_controlled;
            i_ennemy_heroes++;
        }
    }

    for (int i = 0; i < _game.nb_heroes; i++)
    {
        fprintf(stderr, "Hero [%d]\n", _game.heroes[i].id);
    }
    for (int i = 0; i < _game.nb_monsters; i++)
    {
        fprintf(stderr, "Monster [%d]\n", _game.monsters[i].id);
    }
}

int main()
{
    init_startup_game_values();
    // game loop
    while (1)
    {
        init_game_values();
        for (int i = 0; i < _game.nb_heroes; i++)
        {
            Hero *hero = &_game.heroes[i];

            fprintf(stderr, "--- [HERO %d] ---\n", hero->id);
            Entity *nearest_entity = findNearestEntity(hero, _game.monsters, HERO_VIEW_DISTANCE);
            fprintf(stderr, "Nearest Entity: %d\n", nearest_entity ? nearest_entity->id : -1);

            Task task = {0};
            if (nearest_entity == NULL)
            {
                task.type = E_TASK_MOVE;
                task.destination = (Point){MAX_X / 2, MAX_Y / 2};
            }
            else
            {
                task.type = E_TASK_ATTACK;
                task.target_id = nearest_entity->id;
            }
            heroAssignTask(hero, task);
        }

        for (int i = 0; i < _game.nb_heroes; i++)
        {
            Hero *hero = &_game.heroes[i];

            heroDoTask(hero);
        }
    }

    return 0;
}