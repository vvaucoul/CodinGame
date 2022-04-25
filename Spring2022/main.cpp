#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <map>

using namespace std;

#define MIN_X 0
#define MIN_Y 0
#define MAX_X 17630
#define MAX_Y 9000
#define MAX_DISTANCE MAX_X + MAX_Y

#define DISTANCE(x1, y1, x2, y2) sqrt(pow(abs(x1 - x2), 2) + pow(abs(y1 - y2), 2))
#define DISTANCE_LOCATIONS(p1, p2) DISTANCE(p1.x, p1.y, p2.x, p2.y)

#define ENNEMIE_BASE_LOCATION(p1) ((p1.x < MAX_X / 2 && p1.y < MAX_Y / 2) ? (Point){MAX_X, MAX_Y} : (Point){0, 0})
#define OWN_BASE_IS_BOTTOM_RIGHT(p1) (p1.x > MAX_X / 2 && p1.y > MAX_Y / 2)
#define GENERATE_RANDOM(min, max) (rand() % (max - min + 1) + min)

#define VIEW_DISTANCE 2200
#define BASE_VIEW_DISTANCE 6000

#define MONSTER_SPEED 400
#define MONSTER_KILL_DISTANCE 300
#define HERO_SPEED 800
#define HERO_MOVEMENT_DISTANCE_TO_OTHER_HERO (VIEW_DISTANCE / 1.5)

#define DISTANCE_SPELL_WIND 1280
#define DISTANCE_SPELL_SHIELD 2200
#define DISTANCE_SPELL_CONTROL 2200
#define DISTANCE_SPELL_MAX 2200

#define MANA_TO_TARGET_BASE 150
#define MANA_TO_DEFENSE_BASE 100
#define ENNEMY_BASE_TARGET_VIEW_SCALE 2.15

#define HERO_END_DESTINATION_TOLERANCE 4

#define ENTITY_MONSTER_LEVEL(e) (e < 14 ? 0 : e < 20 ? 1 \
                                                     : 2)
#define GAME_TURN_MONSTERS_LEVEL_1 42
#define GAME_TURN_MONSTERS_LEVEL_2 94

typedef struct s_point
{
    int x;
    int y;
} t_point;
#define Point t_point

enum e_hero_action
{
    E_HA_WAIT,
    E_HA_MOVE_TO_LOCATION,
    E_HA_MOVE_TO_MONSTER,
    E_HA_CAST_SPELL,
};
enum e_spells
{
    E_SPELL_WIND = 0,
    E_SPELL_SHIELD = 1,
    E_SPELL_CONTROL = 2,
    E_SPELL_NONE = 3,
};
enum e_best_action
{
    E_BA_WAIT,
    E_BA_MOVE_TO_MONSTER,
    E_BA_MOVE_TO_ENNEMY_HERO,
    E_BA_SPELL_WIND,
    E_BA_SPELL_SHIELD,
    E_BA_SPELL_CONTROL,
    E_BA_NONE,
};

#define NB_BEST_ACTIONS 7

inline void test_debug(const string s)
{
    fprintf(stderr, "%s\n", s.c_str());
}

inline float map_range_clamped(float value, float min, float max, float new_min, float new_max)
{
    return ((value - min) / (max - min) * (new_max - new_min) + new_min);
}
inline float invert_value(float value, float min, float max)
{
    return ((max + min) - value);
}
inline Point invert_location(Point *location, int min_x, int max_x, int min_y, int max_y)
{
    location->x = (max_x + min_x) - location->x;
    location->y = (max_y + min_y) - location->y;
    return (*location);
}
inline bool point_is_in_circle(Point location, Point center, float radius)
{
    return (pow(location.x - center.x, 2) + pow(location.y - center.y, 2) <= pow(radius, 2));
    // return (DISTANCE_LOCATIONS(location, center) <= radius);
}

class Game;

class Entity
{
private:
    // Unique identifier
    int _id;
    // 0=monster, 1=your hero, 2=opponent hero
    int _type;
    // Position of this entity
    Point _location;
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
    Entity(int id, int type, int x, int y, int shield_life, int is_controlled, int health, int vx, int vy, int near_base, int threat_for)
    {
        _id = id;
        _type = type;
        _location.x = x;
        _location.y = y;
        _shield_life = shield_life;
        _is_controlled = is_controlled;
        _health = health;
        _vx = vx;
        _vy = vy;
        _near_base = near_base;
        _threat_for = threat_for;
    };
    Entity(){};
    Entity(Entity const &src)
    {
        _id = src._id;
        _type = src._type;
        _location.x = src._location.x;
        _location.y = src._location.y;
        _shield_life = src._shield_life;
        _is_controlled = src._is_controlled;
        _health = src._health;
        _vx = src._vx;
        _vy = src._vy;
        _near_base = src._near_base;
        _threat_for = src._threat_for;
    };
    Entity &operator=(Entity const &rhs)
    {
        _id = rhs._id;
        _type = rhs._type;
        _location.x = rhs._location.x;
        _location.y = rhs._location.y;
        _shield_life = rhs._shield_life;
        _is_controlled = rhs._is_controlled;
        _health = rhs._health;
        _vx = rhs._vx;
        _vy = rhs._vy;
        _near_base = rhs._near_base;
        _threat_for = rhs._threat_for;
        return *this;
    };
    ~Entity() { delete this; }

    const int get_id() const
    {
        return _id;
    }
    const int get_type() const
    {
        return _type;
    }
    const Point get_location() const
    {
        return _location;
    }
    const int get_shield_life() const
    {
        return _shield_life;
    }
    const int get_is_controlled() const
    {
        return _is_controlled;
    }
    const int get_health() const
    {
        return _health;
    }
    const int get_vx() const
    {
        return _vx;
    }
    const int get_vy() const
    {
        return _vy;
    }
    const int get_near_base() const
    {
        return _near_base;
    }
    const int get_threat_for() const
    {
        return _threat_for;
    }
};
class Hero
{
private:
    int _id;
    Point _location;
    int _is_controlled;

    // Hero Actions
    enum e_hero_action _action;
    enum e_spells _spell;
    bool _did_action;
    Point _destination;

    int _target_entity_id;
    Entity *_target_entity;
    Hero *_target_hero;
    Point _spell_control_vector;

public:
    Hero(){};
    Hero(int id, Point location)
    {
        _id = id;
        _location = location;
        _is_controlled = false;
    };
    ~Hero() { delete this; };

    const int getId() const
    {
        return _id;
    }
    const Point getLocation() const
    {
        return _location;
    }
    const bool isControlled() const
    {
        return _is_controlled;
    }

    void updateHero(int id, Point location, int is_controlled);
    void updateHeroTargetEntity(Game *game);
    // Actions

    void applyAction()
    {
        if (_action == E_HA_WAIT)
        {
            cout << "WAIT [" << _id << "] "
                 << "ZZzz" << endl;
        }
        else if (_action == E_HA_MOVE_TO_LOCATION)
        {
            cout << "MOVE " << _destination.x << " " << _destination.y << " [" << _id << "] "
                 << "Move" << endl;
        }
        else if (_action == E_HA_MOVE_TO_MONSTER)
        {
            cout << "MOVE " << _target_entity->get_location().x << " " << _target_entity->get_location().y << " [" << _id << "] "
                 << "-> [" << _target_entity->get_id() << "]" << endl;
        }
        else if (_action == E_HA_CAST_SPELL)
        {
            switch (_spell)
            {
            case E_SPELL_WIND:
            {
                cout << "SPELL "
                     << "WIND " << _spell_control_vector.x << " " << _spell_control_vector.y << " [" << _id << "] "
                     << "Woosh" << endl;
                break;
            }
            case E_SPELL_CONTROL:
            {
                if (_target_entity != NULL)
                {
                    cout << "SPELL "
                         << "CONTROL " << _target_entity->get_id() << " " << _spell_control_vector.x << " " << _spell_control_vector.y << " [" << _id << "] "
                         << "Wololoo" << endl;
                }
                else if (_target_hero != NULL)
                {
                    cout << "SPELL "
                         << "CONTROL " << _target_hero->getId() << " " << _spell_control_vector.x << " " << _spell_control_vector.y << " [" << _id << "] "
                         << "Wololoo" << endl;
                }
                break;
            }
            case E_SPELL_SHIELD:
            {
                cout << "SPELL "
                     << "SHIELD " << _target_entity->get_id() << " [" << _id << "] "
                     << "Protection" << endl;
                break;
            }

            default:
                break;
            }
        }
    }
    void moveToEntity(Entity *entity)
    {
        if (_did_action == true)
            return;
        _action = E_HA_MOVE_TO_MONSTER;
        _destination = entity->get_location();
        _target_entity = entity;
        _target_entity_id = entity->get_id();
        _did_action = true;
    }
    void move(int x, int y)
    {
        if (_did_action == true)
            return;
        _destination = {x, y};
        _action = E_HA_MOVE_TO_LOCATION;
        _target_entity = NULL;
        _target_entity_id = -1;
        _did_action = true;
    }
    void move(Point p)
    {
        move(p.x, p.y);
    }
    void wait()
    {
        if (_did_action == true)
            return;
        _action = E_HA_WAIT;
        _destination = {-1, -1};
        _target_entity = NULL;
        _target_entity_id = -1;
        _did_action = true;
    }
    void cast_spell_wind(int vx, int vy)
    {
        if (_did_action == true)
            return;
        _action = E_HA_CAST_SPELL;
        _spell = E_SPELL_WIND;
        _spell_control_vector = {vx, vy};
        _destination = {-1, -1};
        _did_action = true;
    }
    void cast_spell_wind(Point p)
    {
        cast_spell_wind(p.x, p.y);
    }
    void cast_spell_control(Entity *entity, int vx, int vy)
    {
        if (_did_action == true)
            return;

        _action = E_HA_CAST_SPELL;
        _spell = E_SPELL_CONTROL;
        _spell_control_vector = {vx, vy};
        _target_entity = entity;
        _target_entity_id = entity->get_id();
        _destination = {-1, -1};
        _did_action = true;
    }
    void cast_spell_control(Entity *entity, Point p)
    {
        cast_spell_control(entity, p.x, p.y);
    }
    void cast_spell_control(Hero *hero, int vx, int vy)
    {
        if (_did_action == true)
            return;
        _action = E_HA_CAST_SPELL;
        _spell = E_SPELL_CONTROL;
        _target_hero = hero;
        _destination = {-1, -1};
        _spell_control_vector = {vx, vy};
        _did_action = true;
        hero->updateHero(hero->getId(), {hero->getLocation().x, hero->getLocation().y}, true);
    }
    void cast_spell_shield(Entity *entity)
    {
        if (_did_action == true)
            return;
        _action = E_HA_CAST_SPELL;
        _spell = E_SPELL_SHIELD;
        _target_entity = entity;
        _target_entity_id = entity->get_id();
        _destination = {-1, -1};
        _did_action = true;
    }

    Entity *findNearestEntity(vector<Entity *> entities)
    {
        Entity *nearestEntity = NULL;
        int minDistance = MAX_DISTANCE;

        for (Entity *entity : entities)
        {
            float distance = DISTANCE_LOCATIONS(_location, entity->get_location());
            if (distance < minDistance)
            {
                minDistance = distance;
                nearestEntity = entity;
            }
        }
        return nearestEntity;
    }
    Entity *findNearestEntity_by_location(vector<Entity *> entities, Point location, float distance_max)
    {
        Entity *nearestEntity = NULL;
        int minDistance = MAX_DISTANCE;

        for (Entity *entity : entities)
        {
            float distance = DISTANCE_LOCATIONS(location, entity->get_location());
            if (distance < minDistance && distance < distance_max)
            {
                minDistance = distance;
                nearestEntity = entity;
            }
        }
        return nearestEntity;
    }
    Entity *findNearestEntity_by_life(vector<Entity *> entities, int life_max)
    {
        Entity *nearestEntity = NULL;
        int minLife = 0;

        for (Entity *entity : entities)
        {
            if (entity->get_health() > minLife && entity->get_health() > life_max)
            {
                minLife = entity->get_health();
                nearestEntity = entity;
            }
        }
        return (nearestEntity);
    }
    int predictionGetEntityNbTurnToReachBase(Entity *entity, Point destination)
    {
        if (entity->get_threat_for() != 1)
            return (0);
        Point entity_location = entity->get_location();

        int nb_turn = 0;
        Point location_entity_next_turn = entity_location;
        Point previous_location_entity;
        float previous_distance_to_base;
        do
        {
            previous_location_entity = location_entity_next_turn;
            previous_distance_to_base = DISTANCE_LOCATIONS(entity_location, destination);
            int vx = entity->get_vx();
            int vy = entity->get_vy();
            location_entity_next_turn = {entity_location.x + (vx * nb_turn), entity_location.y + (vy * nb_turn)};
            ++nb_turn;

            if (DISTANCE_LOCATIONS(destination, location_entity_next_turn) > previous_distance_to_base)
                return (false);
        } while (DISTANCE_LOCATIONS(location_entity_next_turn, destination) > MONSTER_KILL_DISTANCE);
        return (nb_turn);
    }
    int predictionGetHeroNbTurnToReachBase(Hero *hero, Point destination)
    {
        Point hero_location = hero->getLocation();

        int nb_turn = 0;
        nb_turn = ((hero_location.x + hero_location.y) / 2) / HERO_SPEED;
        return (nb_turn);
        // Point location_hero_next_turn = hero_location;
        // Point previous_location_hero;
        // float previous_distance_to_base;

        // // VX = d * x / d * t
        // // VY = d * y / d * t

        // // Norme = sqrt(vx² + vy²)

        // // Angle = arctan(vy / vx)

        // do
        // {
        //     previous_location_hero = location_hero_next_turn;
        //     previous_distance_to_base = DISTANCE_LOCATIONS(hero_location, destination);

        //     // C'est de la merde, ça marche pas..
        //     float distance = DISTANCE_LOCATIONS(hero_location, destination);
        //    // int vx = distance * hero_location.x / distance * HERO_SPEED ; //* HERO_SPEED;
        //    // int vy = distance * hero_location.y / distance * HERO_SPEED ; //* HERO_SPEED;

        //     int vx = 0;
        //     int vy = 0;

        //     int v = sqrt(pow(hero_location.x, 2) + pow(hero_location.y, 2));
        //     fprintf(stderr, "v = %d\n", v);
        //    // float delta = hero_location.x * sin
        //     //float angle =

        //     float norme = sqrt(pow(vx * vx, 2) + pow(vy * vy, 2));
        //     float angle = tan((float)vy / (float)vx);

        //     location_hero_next_turn = {hero_location.x + (vx * nb_turn), hero_location.y + (vy * nb_turn)};
        //     ++nb_turn;

        //     fprintf(stderr, "Hero: [%d]\n", hero->getId());
        //     fprintf(stderr, "Hero location: %d, %d\n", hero_location.x, hero_location.y);
        //     fprintf(stderr, "Hero Vectors: vx = %d, vy = %d\n", vx, vy);
        //     fprintf(stderr, "Hero next location: %d, %d\n", location_hero_next_turn.x, location_hero_next_turn.y);
        //     fprintf(stderr, "Hero previous location: %d, %d\n", previous_location_hero.x, previous_location_hero.y);
        //     fprintf(stderr, "\n");

        //     if (DISTANCE_LOCATIONS(destination, location_hero_next_turn) > previous_distance_to_base)
        //         return (false);
        // } while (DISTANCE_LOCATIONS(location_hero_next_turn, destination) > MONSTER_KILL_DISTANCE);
    }
    bool entityWillBeKilledInTime(Entity *entity, Point base_location)
    {
        int nb_turn_ennemy = predictionGetEntityNbTurnToReachBase(entity, base_location);
        fprintf(stderr, "Entity %d : nb_turn_ennemy = %d\n", entity->get_id(), nb_turn_ennemy);
        int nb_turn_hero = predictionGetHeroNbTurnToReachBase(this, base_location);
        fprintf(stderr, "Hero %d : nb_turn_hero = %d\n", this->getId(), nb_turn_hero);
        if (nb_turn_ennemy <= 0)
            return (true);
        return (nb_turn_ennemy > nb_turn_hero);
        return (false);
    }
    size_t getNbEnnemies_by_location(vector<Entity *> entities, Point location, float distance_max)
    {
        size_t nbEnnemies = 0;
        for (Entity *entity : entities)
        {
            if (DISTANCE_LOCATIONS(location, entity->get_location()) < distance_max)
            {
                nbEnnemies++;
            }
        }
        return nbEnnemies;
    }
    size_t getNbEnnemies_by_distance(vector<Entity *> entities, float distance_max)
    {
        size_t nbEnnemies = 0;
        for (Entity *entity : entities)
        {
            if (DISTANCE_LOCATIONS(_location, entity->get_location()) < distance_max)
            {
                nbEnnemies++;
            }
        }
        return nbEnnemies;
    }

    vector<Entity *> sortEntities_by_distance(vector<Entity *> entities, float distance_max)
    {
        vector<Entity *> sortedEntities;
        for (Entity *entity : entities)
        {
            fprintf(stderr, "Entity %d : distance = %f\n", entity->get_id(), DISTANCE_LOCATIONS(_location, entity->get_location()));
            if (DISTANCE_LOCATIONS(_location, entity->get_location()) < distance_max)
            {
                sortedEntities.push_back(entity);
            }
        }
        return (sortedEntities);
    }
    vector<Entity *> sortEntities_by_distance_with_location(vector<Entity *> entities, Point location, float distance_max)
    {
        vector<Entity *> sortedEntities;
        for (Entity *entity : entities)
        {
            if (DISTANCE_LOCATIONS(location, entity->get_location()) < distance_max)
                sortedEntities.push_back(entity);
        }
        return (sortedEntities);
    }
    vector<Entity *> sortEntities_by_life(vector<Entity *> entities, int life_max)
    {
        vector<Entity *> sortedEntities;
        for (Entity *entity : entities)
        {
            if (entity->get_health() > life_max)
            {
                sortedEntities.push_back(entity);
            }
        }
        return (sortedEntities);
    }
    vector<Entity *> sortEntities_by_threat_for(vector<Entity *> entities, int threat_for)
    {
        vector<Entity *> sortedEntities;
        for (Entity *entity : entities)
        {
            if (entity->get_threat_for() == threat_for)
            {
                sortedEntities.push_back(entity);
            }
        }
        return (sortedEntities);
    }
    vector<Entity *> sortEntities_by_not_shielded(vector<Entity *> entities)
    {
        vector<Entity *> sortedEntities;
        for (Entity *entity : entities)
        {
            if (entity->get_shield_life() <= 0)
            {
                sortedEntities.push_back(entity);
            }
        }
        return (sortedEntities);
    }
    vector<Entity *> sortEntities_by_not_focused(vector<Entity *> entities, vector<Hero *> heroes)
    {
        vector<Entity *> sortedEntities;
        for (Entity *entity : entities)
        {
            // for (Hero *hero : heroes)
            // {
            //     if (hero->getTargetEntity() == NULL)
            //         continue;
            //     else if (hero->getTargetEntity()->get_id() == entity->get_id())
            //         continue;
            //     else
            //         sortedEntities.push_back(entity);
            // }
            if (entityIsAlreadyFocuses(entity, heroes) == false)
                sortedEntities.push_back(entity);
        }
        return (sortedEntities);
    }
    vector<Entity *> sortEntities_by_level(vector<Entity *> entities, int level)
    {
        vector<Entity *> sortedEntities;
        for (Entity *entity : entities)
        {
            int lvl = ENTITY_MONSTER_LEVEL(entity->get_health());
            if (lvl >= level)
            {
                sortedEntities.push_back(entity);
            }
        }
        return (sortedEntities);
    }

    Hero *findNearestHeroByDistance(vector<Hero *> heroes, float distance_max)
    {
        Hero *nearestHero = NULL;
        float distance_min = distance_max;
        for (Hero *hero : heroes)
        {
            float distance = DISTANCE_LOCATIONS(_location, hero->getLocation());
            if (distance < distance_min)
            {
                distance_min = distance;
                nearestHero = hero;
            }
        }
        return (nearestHero);
    }
    Hero *findNearestHeroByDistanceWithLocation(vector<Hero *> heroes, Point location, float distance_max)
    {
        Hero *nearestHero = NULL;
        float distance_min = distance_max;
        for (Hero *hero : heroes)
        {
            float distance = DISTANCE_LOCATIONS(location, hero->getLocation());
            if (distance < distance_min)
            {
                distance_min = distance;
                nearestHero = hero;
            }
        }
        return (nearestHero);
    }
    Hero *findNearestHeroToEntity(vector<Hero *> heroes, Entity *entity, float distance_max)
    {
        Hero *nearestHero = NULL;
        float distance_min = distance_max;
        for (Hero *hero : heroes)
        {
            float distance = DISTANCE_LOCATIONS(entity->get_location(), hero->getLocation());
            if (distance < distance_min)
            {
                distance_min = distance;
                nearestHero = hero;
            }
        }
        return (nearestHero);
    }
    vector<Hero *> sortEnnemyHeroes_by_distance(vector<Hero *> heroes, float distance)
    {
        vector<Hero *> sortedHeroes;

        for (Hero *hero : heroes)
        {
            if (DISTANCE_LOCATIONS(_location, hero->getLocation()) <= distance)
                sortedHeroes.push_back(hero);
        }
        return (sortedHeroes);
    }
    vector<Hero *> sortEnnemyHeroes_by_distance_with_location(vector<Hero *> heroes, Point location, float distance_max)
    {
        vector<Hero *> sortedHeroes;

        for (Hero *hero : heroes)
        {
            if (DISTANCE_LOCATIONS(location, hero->getLocation()) <= distance_max)
                sortedHeroes.push_back(hero);
        }
        return (sortedHeroes);
    }
    vector<Hero *> sortEnnemyHeroes_by_not_controlled(vector<Hero *> heroes)
    {
        vector<Hero *> sortedHeroes;

        for (Hero *hero : heroes)
        {
            if (hero->isControlled() == false)
                sortedHeroes.push_back(hero);
        }
        return (sortedHeroes);
    }

    const enum e_hero_action getAction() const
    {
        return _action;
    }
    const Point getDestination() const
    {
        return _destination;
    }
    const Entity *getTargetEntity() const
    {
        return _target_entity;
    }
    void heroAsReachedDestination()
    {
        if (DISTANCE_LOCATIONS(_destination, _location) < HERO_END_DESTINATION_TOLERANCE)
        {
            _action = E_HA_WAIT;
            _destination = {-1, -1};
            _target_entity = NULL;
        }
    }
    const bool getDidAction() const
    {
        return _did_action;
    }

    bool entityIsAlreadyFocuses(const Entity *entity, const vector<Hero *> heroes)
    {
        for (Hero *hero : heroes)
        {
            if (_target_entity != NULL)
            {
                if (_target_entity->get_id() == entity->get_id() && _id == hero->getId()) // Ignore Self
                    return (false);
            }
            if (hero->getAction() != E_HA_MOVE_TO_MONSTER)
                continue;
            if (entity->get_id() == hero->getTargetEntity()->get_id())
            {
                return (true);
            }
        }
        return (false);
    }
    bool entityRequiertMoreTarget(const Entity *entity, const vector<Hero *> heroes, Point base_location, int nb_turn)
    {
        int entity_level = ENTITY_MONSTER_LEVEL(entity->get_health());

        for (Hero *hero : heroes)
        {
            if (hero->getId() == 0) //&& nb_turn >= GAME_TURN_MONSTERS_LEVEL_1)
                continue;
            if (entity_level >= 1 && (entity->get_threat_for() == 1 || DISTANCE_LOCATIONS(entity->get_location(), base_location) <= BASE_VIEW_DISTANCE))
            {
                return (true);
            }
        }
        return (false);
    }
    bool targetEntityCanBeKilledAlone()
    {
        return (false);
    }
    Point getRandomIALocation(Game *game);
    Point getPointSeparateToHeroes(Game *game, float dist_max);
    // to do ( a finaliser )
    pair<enum e_best_action, int> weightGetBestDefenseAction(vector<Entity *> entities, vector<Hero *> ennemy_heroes, Point base_location, size_t mana_ennemy)
    {
        vector<pair<enum e_best_action, float>> best_actions; // Best actions and their weight
        int target_entity = -1;

        for (int i = 0; i < NB_BEST_ACTIONS; i++)
            best_actions.push_back(make_pair((enum e_best_action)i, 0));

        Entity *entity_nearest_to_base = findNearestEntity_by_location(entities, base_location, BASE_VIEW_DISTANCE);
        Hero *ennemy_hero_nearest_to_base = findNearestHeroByDistanceWithLocation(ennemy_heroes, base_location, BASE_VIEW_DISTANCE);
        Entity *entity_nearest_to_ennemy = NULL;
        if (ennemy_hero_nearest_to_base != NULL)
            entity_nearest_to_ennemy = findNearestEntity_by_location(entities, entity_nearest_to_ennemy->get_location(), VIEW_DISTANCE);

        int nb_ennemy_in_base = sortEntities_by_distance_with_location(entities, base_location, BASE_VIEW_DISTANCE).size();
        int nb_ennemy_heroes_in_base = sortEnnemyHeroes_by_distance_with_location(ennemy_heroes, base_location, BASE_VIEW_DISTANCE).size();

        fprintf(stderr, "Entity Nearest to Base: %d\n", entity_nearest_to_base != NULL ? entity_nearest_to_base->get_id() : -1);
        fprintf(stderr, "Ennemy Hero Nearest to Base: %d\n", ennemy_hero_nearest_to_base != NULL ? ennemy_hero_nearest_to_base->getId() : -1);
        fprintf(stderr, "Entity Nearest to Ennemy: %d\n", entity_nearest_to_ennemy != NULL ? entity_nearest_to_ennemy->get_id() : -1);
        fprintf(stderr, "Nb Ennemy in Base: %d\n", nb_ennemy_in_base);
        fprintf(stderr, "Nb Ennemy Heroes in Base: %d\n", nb_ennemy_heroes_in_base);

        test_debug("00");

        bool any_ennemy_hero_can_use_wind_spell = false;
        {
            for (Hero *ennemy_hero : ennemy_heroes)
            {
                Entity *entity = ennemy_hero->findNearestEntity(entities);
                float distance = DISTANCE_LOCATIONS(ennemy_hero->getLocation(), entity->get_location());
                if (distance <= DISTANCE_SPELL_WIND)
                {
                    any_ennemy_hero_can_use_wind_spell = true;
                    break;
                }
            }
        }
        bool any_ennemy_hero_can_use_control_spell = false;
        {
            for (Hero *ennemy_hero : ennemy_heroes)
            {
                Entity *entity = ennemy_hero->findNearestEntity(entities);
                float distance = DISTANCE_LOCATIONS(ennemy_hero->getLocation(), entity->get_location());
                if (distance <= DISTANCE_SPELL_CONTROL)
                {
                    any_ennemy_hero_can_use_control_spell = true;
                    break;
                }
            }
        }
        bool any_ennemy_hero_can_use_shild_spell = false;
        {
            for (Hero *ennemy_hero : ennemy_heroes)
            {
                Entity *entity = ennemy_hero->findNearestEntity(entities);
                float distance = DISTANCE_LOCATIONS(ennemy_hero->getLocation(), entity->get_location());
                if (distance <= DISTANCE_SPELL_SHIELD)
                {
                    any_ennemy_hero_can_use_shild_spell = true;
                    break;
                }
            }
        }

        test_debug("01");
        for (int i = 0; i < NB_BEST_ACTIONS; i++)
        {
            if (i == E_BA_MOVE_TO_MONSTER)
            {
                float weight = 0;
            }
            else if (i == E_BA_MOVE_TO_ENNEMY_HERO)
            {
                float weight = 0;
            }
            else if (i == E_BA_SPELL_WIND)
            {
                float weight = 0;
            }
            else if (i == E_BA_SPELL_CONTROL)
            {
                float weight = 0;
            }
            else if (i == E_BA_SPELL_SHIELD)
            {
                float weight = 0;
            }
        }
        test_debug("02");

        // Select best defense action
        pair<enum e_best_action, int> best_action; // Best action and entity ID
        best_action = make_pair(E_BA_NONE, -1);
        float best_weight = 0;
        for (vector<pair<enum e_best_action, float>>::iterator it = best_actions.begin(); it != best_actions.end(); ++it)
        {
            if (it->second > best_weight)
            {
                best_weight = it->second;
                best_action = make_pair(it->first, target_entity);
            }
        }
        test_debug("03");

        return (best_action);
    }
};
class Player
{
private:
    int _health;
    int _mana;

public:
    const int getHealth() const
    {
        return _health;
    }
    const int getMana() const
    {
        return _mana;
    }

    const void setHealth(int health)
    {
        _health = health;
    }
    const void setMana(int mana)
    {
        _mana = mana;
    }
};
class Game
{
private:
    Point _base_location;

    Player _players[2];

    int _nb_entities;
    int _nb_heroes;
    int _nb_monsters;
    int _nb_turn;

    std::vector<Hero *> _heroes;
    std::vector<Hero *> _ennemy_heroes;
    std::vector<Entity *> _monsters;

public:
    const Point &getBaseLocation() const
    {
        return (_base_location);
    }
    const Player &getOwnPlayer() const
    {
        return (_players[0]);
    }
    const Player &getEnnemyPlayer() const
    {
        return (_players[1]);
    }
    const int getNbEntities() const
    {
        return (_nb_entities);
    }
    const int getNbHeroes() const
    {
        return (_nb_heroes);
    }
    const int getNbMonsters() const
    {
        return (_nb_monsters);
    }
    const int getNbTurn() const
    {
        return (_nb_turn);
    }

    std::vector<Hero *> &getHeroes()
    {
        return (_heroes);
    }
    std::vector<Hero *> &getEnnemyHeroes()
    {
        return (_ennemy_heroes);
    }
    std::vector<Entity *> &getMonsters()
    {
        return (_monsters);
    }

public:
    Game(){};
    ~Game(){};
    void init_game_values()
    {
        fprintf(stderr, "Init game values !\n");
        cin >> _base_location.x >> _base_location.y;
        cin.ignore();
        cin >> _nb_heroes;
        cin.ignore();
        _nb_turn = 0;
        for (int i = 0; i < 3; i++)
        {
            Hero *newHero = new Hero();
            _heroes.push_back(newHero);
        }
        srand(static_cast<unsigned int>(time(0)));
    }
    void clear_game_values()
    {
        fprintf(stderr, "Clear game values !\n");
        //_heroes.clear();
        _ennemy_heroes.clear();
        _monsters.clear();
    }
    void init_game_entities()
    {
        fprintf(stderr, "Init game entities !\n");
        for (int i = 0; i < 2; i++)
        {
            int health;
            int mana;

            cin >> health >> mana;
            cin.ignore();

            _players[i].setHealth(health);
            _players[i].setMana(mana);
        }

        cin >> _nb_entities;
        cin.ignore();
        _nb_turn++;
        for (int i = 0; i < _nb_entities; i++)
        {
            int id, type, x, y, shield_life, is_controlled, health, vx, vy, near_base, threat_for;
            cin >> id >> type >> x >> y >> shield_life >> is_controlled >> health >> vx >> vy >> near_base >> threat_for;
            cin.ignore();

            Point location;
            location.x = x;
            location.y = y;

            if (type == 0)
            {
                Entity *newEntity = new Entity(id, type, x, y, shield_life, is_controlled, health, vx, vy, near_base, threat_for);
                _monsters.push_back(newEntity);
            }
            else if (type == 1)
            {
                if (id >= 3)
                    id -= 3;
                _heroes[id]->updateHero(id, location, is_controlled);
            }
            else if (type == 2)
            {
                Hero *newHero = new Hero(id, location);
                _ennemy_heroes.push_back(newHero);
            }
        }
        _nb_monsters = _monsters.size();
        for (Hero *hero : _heroes)
            hero->updateHeroTargetEntity(this);
    }
};

// Hero Functions wich need Game class

void Hero::updateHeroTargetEntity(Game *game)
{
    bool found = false;

    fprintf(stderr, "<HERO %d> Update target entity !\n", _id);
    for (Entity *entity : game->getMonsters())
    {
        fprintf(stderr, "Entity ID : %d\n", entity->get_id());
        if (entity->get_id() == _target_entity_id)
        {
            fprintf(stderr, "Found target entity ! [%d]\n", entity->get_id());
            _target_entity = entity;
            found = true;
            break;
        }
    }
    if (!found)
    {
        _target_entity = NULL;
        _target_entity_id = -1;
    }
}
void Hero::updateHero(int id, Point location, int is_controlled)
{
    _id = id;
    _location = location;
    _did_action = false;
    _is_controlled = is_controlled;
    heroAsReachedDestination(); // Check if hero has reached its destination
}
Point Hero::getPointSeparateToHeroes(Game *game, float dist_max)
{
    Point new_point;
    vector<Hero *> heroes = game->getHeroes();

    size_t count = 0;
    bool found = false;

    do
    {
        found = true;
        new_point = getRandomIALocation(game);
        for (Hero *hero : heroes)
        {
            if (DISTANCE_LOCATIONS(new_point, hero->getLocation()) < dist_max)
            {
                found = false;
                break;
            };
        }
        fprintf(stderr, "Separate Points: [%d] | [%d]\n", new_point.x, new_point.y);
        if (found == true)
            break;
    } while (1);
    return (new_point);
}
Point Hero::getRandomIALocation(Game *game)
{
    int game_turn = game->getNbTurn();
    Point base_location = game->getBaseLocation();

    switch (_id)
    {
    case 0:
    {
        Point aggressive_location;

        if (game_turn < 10)
        {
            aggressive_location.x = GENERATE_RANDOM(int(MAX_X / 2.5), int(MAX_X / 1.25));
            aggressive_location.y = GENERATE_RANDOM(int(MAX_Y / 3), int(MAX_Y / 1.5));
        }
        else
        {
            aggressive_location.x = GENERATE_RANDOM(int(MAX_X / 1.75), int(MAX_X / 1.15));
            aggressive_location.y = GENERATE_RANDOM(int(MAX_Y / 2.25), int(MAX_Y / 1.15));
        }

        if (OWN_BASE_IS_BOTTOM_RIGHT(base_location))
            invert_location(&aggressive_location, MIN_X, MAX_X, MIN_Y, MAX_Y);
        return (aggressive_location);
    }

    case 1:
    {
        Point deffensive_location;
        if (game_turn < 10)
        {
            deffensive_location.x = GENERATE_RANDOM(int(MAX_X / 2.5), int(MAX_X / 1.75));
            deffensive_location.y = GENERATE_RANDOM(int(MAX_Y / 8), int(MAX_Y / 3));
        }
        else
        {
            deffensive_location.x = GENERATE_RANDOM(int(MAX_X / 4), int(MAX_X / 2));
            deffensive_location.y = GENERATE_RANDOM(int(MAX_Y / 6), int(MAX_Y / 2));
        }

        if (OWN_BASE_IS_BOTTOM_RIGHT(base_location))
            invert_location(&deffensive_location, MIN_X, MAX_X, MIN_Y, MAX_Y);
        return (deffensive_location);
    }

    case 2:
    {
        Point deffensive_location;
        if (game_turn < 10)
        {
            deffensive_location.x = GENERATE_RANDOM(int(MAX_X / 4), int(MAX_X / 2));
            deffensive_location.y = GENERATE_RANDOM(int(MAX_Y / 1.75), int(MAX_Y / 1.25));
        }
        else
        {
            deffensive_location.x = GENERATE_RANDOM(int(MAX_X / 6), int(MAX_X / 3));
            deffensive_location.y = GENERATE_RANDOM(int(MAX_Y / 2), int(MAX_Y / 1.25));
        }

        if (OWN_BASE_IS_BOTTOM_RIGHT(base_location))
            invert_location(&deffensive_location, MIN_X, MAX_X, MIN_Y, MAX_Y);
        return (deffensive_location);
    }

    default:
        break;
    }
    return ((Point){0, 0});
}

int main()
{

    // game loop
    Game game;

    game.init_game_values();
    while (1)
    {
        fprintf(stderr, "Turn [%d]\n", game.getNbTurn());
        game.init_game_entities();

        // Select Heroes actions
        for (int i = 0; i < game.getNbHeroes(); i++)
        {
            Hero *hero = game.getHeroes()[i];

            fprintf(stderr, "--- HERO [%d] ---\n\n", hero->getId());
            vector<Entity *> entities_hero_see = hero->sortEntities_by_distance(game.getMonsters(), VIEW_DISTANCE);
            vector<Entity *> entities_in_base = hero->sortEntities_by_distance_with_location(game.getMonsters(), game.getBaseLocation(), BASE_VIEW_DISTANCE);
            for (Entity *entity : entities_in_base)
                entities_hero_see.push_back(entity);

            int nbEntitiesViewed = entities_hero_see.size();
            int nbEntitiesInBase = entities_in_base.size();
            fprintf(stderr, "NB Monsters: [%d]\n", game.getNbMonsters());
            fprintf(stderr, "NB Monsters in base: [%d]\n", nbEntitiesInBase);
            fprintf(stderr, "NB Monsters viewed: [%d]\n", nbEntitiesViewed);

            if (hero->getId() == 0)
            {
            }
            else
            {
                test_debug("Test 01");
                if (hero->getTargetEntity() != NULL)
                    continue;
                if (nbEntitiesViewed > 0)
                {
                    Entity *targetEntity = NULL;

                    if (hero->getTargetEntity() != NULL)
                        continue;
                    test_debug("Test 02");
                    fprintf(stderr, "NB heroes: [%d]\n", game.getNbHeroes());
                    entities_hero_see = hero->sortEntities_by_not_focused(entities_hero_see, game.getHeroes());
                    test_debug("Test 03");
                    targetEntity = hero->findNearestEntity(entities_hero_see);
                    test_debug("Test 04");

                    if (targetEntity != NULL)
                    {
                        hero->moveToEntity(targetEntity);
                        test_debug("Test 05");
                        fprintf(stderr, "Target Entity: [%d]\n", targetEntity->get_id());
                    }
                }
                else
                {
                    Point random_location = {0, 0};
                    random_location = hero->getPointSeparateToHeroes(&game, VIEW_DISTANCE / 2);
                    hero->move(random_location);
                }
            }
            fprintf(stderr, "\n");
        }

        // Apply Heroes actions
        for (int i = 0; i < game.getNbHeroes(); i++)
        {
            Hero *hero = game.getHeroes()[i];
            hero->applyAction();
        }

        /*
        for (int i = 0; i < game.getNbHeroes(); i++)
        {
            Hero *hero = game.getHeroes()[i];
            Entity *entity = NULL;

            if (hero->getNbEnnemies_by_location(game.getMonsters(), game.getBaseLocation(), BASE_VIEW_DISTANCE * 1.2) > 0 && hero->getId() > 0)
            {
                vector<Entity *> entities = hero->sortEntities_by_distance_with_location(game.getMonsters(), game.getBaseLocation(), BASE_VIEW_DISTANCE * 1.2, true);

                entities = hero->sortEntities_by_not_focused(entities, game.getHeroes());
                entity = hero->findNearestEntity_by_location(entities, game.getBaseLocation(), BASE_VIEW_DISTANCE * 1.2);

                // hero->weightGetBestDefenseAction(entities, game.getEnnemyHeroes(), game.getBaseLocation(), game.getEnnemyPlayer().getMana());

                // SPELL DEFENSES
                // if (entity != NULL)
                // {
                //     bool killed_in_time = hero->entityWillBeKilledInTime(entity, game.getBaseLocation());
                //     fprintf(stderr, "Entity [%d] will be killed in time: [%d]\n", entity->get_id(), killed_in_time);
                //     if (killed_in_time == false)
                //     {
                //         float distance_to_ennemie = DISTANCE_LOCATIONS(hero->getLocation(), entity->get_location());

                //         if (distance_to_ennemie < DISTANCE_SPELL_WIND)
                //         {
                //             hero->cast_spell_wind(ENNEMIE_BASE_LOCATION(game.getBaseLocation()));
                //             hero->setDidAction(true);
                //         }
                //         else if (distance_to_ennemie < DISTANCE_SPELL_CONTROL)
                //         {
                //             hero->cast_spell_control(entity, ENNEMIE_BASE_LOCATION(game.getBaseLocation()));
                //             hero->setDidAction(true);
                //         }
                //     }
                // }
            }
            else
            {
                bool already_focused = false, requiert_more_target = false;
                vector<Entity *> entities = game.getMonsters();

                // Target Entity
                do
                {
                    Entity *entity_tmp = NULL;
                    entity = hero->findNearestEntity(game.getMonsters());
                    if (entity == NULL)
                        break;

                    if (hero->getId() > 0)
                    {
                        if (game.getOwnPlayer().getMana() >= MANA_TO_DEFENSE_BASE)
                        {
                            vector<Entity *> new_entities = hero->sortEntities_by_not_focused(entities, game.getHeroes());
                            new_entities = hero->sortEntities_by_distance_with_location(new_entities, game.getBaseLocation(), BASE_VIEW_DISTANCE, true);

                            if (new_entities.size() > 0)
                            {
                                entity = hero->findNearestEntity(new_entities);
                            }

                            for (Entity *ce : new_entities)
                            {
                                if (hero->entityWillBeKilledInTime(entity, game.getBaseLocation()) == false)
                                {
                                    entity = ce;
                                    break;
                                }
                            }
                            if (hero->getNbEnnemies_by_location(new_entities, game.getBaseLocation(), BASE_VIEW_DISTANCE) > 0 && hero->getId() > 0)
                            {
                                entity = hero->findNearestEntity_by_location(new_entities, game.getBaseLocation(), BASE_VIEW_DISTANCE);
                            }
                        }
                        else
                        {
                            vector<Entity *> new_entities = hero->sortEntities_by_not_focused(entities, game.getHeroes());
                            fprintf(stderr, "Hero [%d] will focus entity [%d]\n", hero->getId(), entity != NULL ? entity->get_id() : -1);
                            if (new_entities.size() > 0)
                            {
                                bool hero_is_near_to_ennemy = false;

                                entity = hero->findNearestEntity(new_entities);
                                if (entity == NULL)
                                    break;
                                Hero *nearest_hero;
                                if ((nearest_hero = hero->findNearestHeroByDistanceWithLocation(game.getHeroes(), entity->get_location(), VIEW_DISTANCE)) != NULL)
                                {

                                    hero_is_near_to_ennemy = true;
                                    entity = NULL;
                                    fprintf(stderr, "Hero [%d] is near will focus entity [%d]\n", nearest_hero->getId(), entity != NULL ? entity->get_id() : -1);
                                }
                                break;
                                fprintf(stderr, "Hero [%d] will focus entity [%d]\n", hero->getId(), entity != NULL ? entity->get_id() : -1);
                            }
                        }
                    }
                    else if (hero->getId() == 0)
                    {
                        vector<Entity *> new_entities = hero->sortEntities_by_not_shielded(entities);
                        new_entities = hero->sortEntities_by_distance_with_location(new_entities, ENNEMIE_BASE_LOCATION(game.getBaseLocation()), BASE_VIEW_DISTANCE, false);

                        if (game.getOwnPlayer().getMana() >= MANA_TO_TARGET_BASE)
                        {
                            if ((entity_tmp = hero->findNearestEntity_by_location(new_entities, ENNEMIE_BASE_LOCATION(game.getBaseLocation()), int(BASE_VIEW_DISTANCE * ENNEMY_BASE_TARGET_VIEW_SCALE))) != NULL)
                            {
                                entity = entity_tmp;
                                break;
                            }
                            else
                            {
                                entity = NULL;
                                break;
                            }
                        }
                    }

                    already_focused = hero->entityIsAlreadyFocuses(entity, game.getHeroes());
                    requiert_more_target = hero->entityRequiertMoreTarget(entity, game.getHeroes(), game.getBaseLocation(), game.getNbTurn());

                    fprintf(stderr, "Entity [%d] requiert more target : %d\n", entity->get_id(), requiert_more_target);
                    if (requiert_more_target == true)
                        break;
                    if (already_focused == true)
                    {
                        entities.erase(std::remove(entities.begin(), entities.end(), entity), entities.end());
                        entity = NULL;
                        if (entities.size() == 0)
                            break;
                        continue;
                    }
                    else
                        break;
                } while (entity != NULL && entities.size() > 0);
            }

            // Cast Spells Attack / Movement Spells IA
            if (hero->getId() == 0 && game.getOwnPlayer().getMana() >= MANA_TO_TARGET_BASE &&
                DISTANCE_LOCATIONS(hero->getLocation(), ENNEMIE_BASE_LOCATION(game.getBaseLocation())) > BASE_VIEW_DISTANCE * ENNEMY_BASE_TARGET_VIEW_SCALE &&
                hero->getTargetEntity() == NULL)
            {
                // Target Ennemy Monster in Ennemy Base
                if (hero->getAction() != E_HA_MOVE_TO_LOCATION)
                    hero->move(hero->getRandomIALocation(game.getNbTurn(), game.getBaseLocation()));
                else
                    hero->move(hero->getDestination());
                hero->setDidAction(true);
            }
            else if (hero->getId() == 0 && game.getOwnPlayer().getMana() >= MANA_TO_TARGET_BASE &&
                     DISTANCE_LOCATIONS(hero->getLocation(), ENNEMIE_BASE_LOCATION(game.getBaseLocation())) <= BASE_VIEW_DISTANCE * ENNEMY_BASE_TARGET_VIEW_SCALE)
            {
                // Cast Spells

                int spell_priority = E_SPELL_NONE;
                vector<pair<enum e_spells, float>> spells_priority; // <spell_id, priority / with weight>
                vector<Entity *> entities;

                // Init spells
                {
                    spells_priority.push_back(make_pair(E_SPELL_WIND, 0));
                    spells_priority.push_back(make_pair(E_SPELL_SHIELD, 0));
                    spells_priority.push_back(make_pair(E_SPELL_CONTROL, 0));
                    spells_priority.push_back(make_pair(E_SPELL_NONE, -1));
                }

                entities = hero->sortEntities_by_distance(game.getMonsters(), DISTANCE_SPELL_MAX);
                entities = hero->sortEntities_by_life(entities, 10);
                entities = hero->sortEntities_by_not_shielded(entities);
                // entities = hero->sortEntities_by_threat_for(entities, 2);

                // Entity *tmp = hero->findNearestEntity_by_location(entities, ENNEMIE_BASE_LOCATION(game.getBaseLocation()), DISTANCE_SPELL_SHIELD);

                int nearest_to_ennemy_base_ennemies = hero->getNbEnnemies_by_location(entities, ENNEMIE_BASE_LOCATION(game.getBaseLocation()), BASE_VIEW_DISTANCE);

                for (vector<pair<enum e_spells, float>>::iterator it = spells_priority.begin(); it != spells_priority.end(); it++)
                {
                    if (it->first == E_SPELL_WIND)
                    {
                        float weight = 0;

                        Entity *nearest_entity = hero->findNearestEntity(entities);
                        if (nearest_entity == NULL)
                        {
                            it->second = -1;
                            continue;
                        }
                        bool is_in_distance_to_wind = DISTANCE_LOCATIONS(nearest_entity->get_location(), hero->getLocation()) <= DISTANCE_SPELL_WIND ? true : false;
                        int nb_ennemies_for_wind = hero->getNbEnnemies_by_location(entities, hero->getLocation(), DISTANCE_SPELL_WIND);
                        int nb_ennemies_with_good_health = hero->sortEntities_by_life(entities, 10).size();

                        if (is_in_distance_to_wind == false)
                        {
                            it->second = -1;
                            continue;
                        }
                        weight += nb_ennemies_for_wind * 3 + nb_ennemies_with_good_health * 2;
                        it->second = weight;
                        fprintf(stderr, "SPELLS | WIND | WEIGHT: [%f]\n", weight);
                    }
                    else if (it->first == E_SPELL_SHIELD)
                    {
                        float weight = 0;

                        Entity *nearest_entity = hero->findNearestEntity_by_location(entities, hero->getLocation(), DISTANCE_SPELL_SHIELD);
                        if (nearest_entity == NULL)
                        {
                            it->second = -1;
                            continue;
                        }
                        else if (DISTANCE_LOCATIONS(nearest_entity->get_location(), ENNEMIE_BASE_LOCATION(game.getBaseLocation())) > BASE_VIEW_DISTANCE)
                        {
                            it->second = -1;
                            continue;
                        }
                        bool ennemy_is_moving_to_ennemy_base = nearest_entity->get_threat_for() == 2 ? true : false;
                        float ennemy_is_near_to_enney_base = DISTANCE_LOCATIONS(nearest_entity->get_location(), ENNEMIE_BASE_LOCATION(game.getBaseLocation())) / BASE_VIEW_DISTANCE;

                        weight += (ennemy_is_near_to_enney_base * 1) + (ennemy_is_moving_to_ennemy_base * 2);
                        it->second = weight;
                        fprintf(stderr, "SPELLS | SHIELD | WEIGHT: [%f]\n", weight);
                    }
                    else if (it->first == E_SPELL_CONTROL)
                    {
                        float weight = 0;

                        Entity *nearest_entity = hero->findNearestEntity_by_location(entities, hero->getLocation(), DISTANCE_SPELL_CONTROL);
                        if (nearest_entity == NULL)
                        {
                            it->second = -1;
                            continue;
                        }
                        int ennemy_value = ENTITY_MONSTER_LEVEL(nearest_entity->get_health());
                        int threat_for = nearest_entity->get_threat_for() == 2 ? false : true;
                        int ennemy_health = nearest_entity->get_health() / 25;

                        weight += ennemy_value * 2 + threat_for * 2 + ennemy_health * 2;
                        it->second = weight;
                        fprintf(stderr, "SPELLS | CONTROL | WEIGHT: [%f]\n", weight);
                    }
                    else
                        continue;
                }

                float weight = 0;
                for (vector<pair<enum e_spells, float>>::iterator it = spells_priority.begin(); it != spells_priority.end(); it++)
                {
                    if (it->second > weight)
                    {
                        spell_priority = it->first;
                        weight = it->second;
                    }
                }
                fprintf(stderr, "SPELLS | PRIORITY: [%d]\n", spell_priority);
                if (spell_priority == E_SPELL_WIND)
                {
                    fprintf(stderr, "Try to cast Spell Wind : has ennemy in distance : [%d]\n", hero->findNearestEntity_by_location(entities, hero->getLocation(), DISTANCE_SPELL_WIND) == NULL ? 0 : 1);
                    hero->cast_spell_wind(ENNEMIE_BASE_LOCATION(game.getBaseLocation()).x, ENNEMIE_BASE_LOCATION(game.getBaseLocation()).y);
                    hero->setDidAction(true);
                }
                else if (spell_priority == E_SPELL_SHIELD)
                {
                    Entity *tmp = hero->findNearestEntity_by_location(entities, hero->getLocation(), DISTANCE_SPELL_SHIELD);

                    fprintf(stderr, "Try to cast Spell Shield : ha ennemy in distance : [%d]\n", tmp == NULL ? 0 : 1);
                    if (tmp != NULL)
                    {
                        hero->cast_spell_shield(tmp);
                        hero->setDidAction(true);
                    }
                }
                else if (spell_priority == E_SPELL_CONTROL)
                {
                    Entity *tmp = hero->findNearestEntity_by_location(entities, hero->getLocation(), DISTANCE_SPELL_CONTROL);

                    fprintf(stderr, "Try to cast Spell Control : ha ennemy in distance : [%d]\n", tmp == NULL ? 0 : 1);
                    if (tmp != NULL)
                    {
                        hero->cast_spell_control(tmp, ENNEMIE_BASE_LOCATION(game.getBaseLocation()).x, ENNEMIE_BASE_LOCATION(game.getBaseLocation()).y);
                        hero->setDidAction(true);
                    }
                }
            }

            // Cast Spells Defense / Movement Spells IA
            if (hero->getId() > 0 && game.getOwnPlayer().getMana() >= MANA_TO_DEFENSE_BASE)
            {
                // Control entity
                vector<Entity *> entities = game.getMonsters();

                if (hero->getNbEnnemies_by_distance(game.getMonsters(), int(DISTANCE_SPELL_WIND)) > 1)
                {
                    hero->cast_spell_wind(ENNEMIE_BASE_LOCATION(game.getBaseLocation()).x, ENNEMIE_BASE_LOCATION(game.getBaseLocation()).y);
                    hero->setDidAction(true);
                }
                entities = hero->sortEntities_by_distance(entities, VIEW_DISTANCE);
                vector<Entity *> entities_lvl = hero->sortEntities_by_level(entities, 2);
                entities_lvl = hero->sortEntities_by_not_shielded(entities_lvl);
                entities_lvl = hero->sortEntities_by_threat_for(entities_lvl, 1);

                fprintf(stderr, "Cast spell control : [%zu]\n", entities_lvl.size());
                if (entities_lvl.size() > 0)
                {
                    Entity *tmp = hero->findNearestEntity_by_location(entities_lvl, hero->getLocation(), BASE_VIEW_DISTANCE * 1.2);
                    if (tmp != NULL)
                    {
                        hero->cast_spell_control(tmp, ENNEMIE_BASE_LOCATION(game.getBaseLocation()).x, ENNEMIE_BASE_LOCATION(game.getBaseLocation()).y);
                        hero->setDidAction(true);
                    }
                }

                // Control Ennemy hero

                vector<Hero *> ennemy_heroes = game.getEnnemyHeroes();
                ennemy_heroes = hero->sortEnnemyHeroes_by_distance(ennemy_heroes, VIEW_DISTANCE);
                ennemy_heroes = hero->sortEnnemyHeroes_by_not_controlled(ennemy_heroes);

                Hero *ennemy_hero = hero->findNearestHeroByDistance(ennemy_heroes, VIEW_DISTANCE);
                if (ennemy_hero != NULL && entity != NULL)
                {
                    float distance_to_target = DISTANCE_LOCATIONS(entity->get_location(), ennemy_hero->getLocation());

                    if (distance_to_target <= (DISTANCE_SPELL_WIND * 1.5))
                    {
                        hero->cast_spell_control(ennemy_hero, ENNEMIE_BASE_LOCATION(game.getBaseLocation()).x, ENNEMIE_BASE_LOCATION(game.getBaseLocation()).y);
                        hero->setDidAction(true);
                    }
                }
            }

            // Movement to entity
            if (entity != NULL)
            {
                hero->moveToEntity(entity);
                hero->setDidAction(true);
            }
            // Movement Randoms IA
            else
            {
                if (hero->getAction() != E_HA_MOVE_TO_LOCATION)
                {
                    Point location = hero->getPointSeparateToHeroes(&game, HERO_MOVEMENT_DISTANCE_TO_OTHER_HERO); // hero->getRandomIALocation(game.getNbTurn(), game.getBaseLocation())
                    hero->move(location);
                }
                else
                    hero->move(hero->getDestination());
                hero->setDidAction(true);
            }
        }
        */
        game.clear_game_values();
    }
}