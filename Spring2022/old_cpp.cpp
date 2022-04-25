#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

#include <cstdlib>
#include <cmath>

#define MIN_X 0
#define MIN_Y 0
#define MAX_X 17630
#define MAX_Y 9000

using namespace std;

typedef struct s_point
{
    size_t x;
    size_t y;
} t_point;

enum e_entity_type
{
    E_MONSTER,
    E_HERO,
    E_OPPONENT_HERO
};
enum e_entity_threat_for
{
    E_BASE,
    E_OPPONENT_BASE,
    E_NEITHER
};

#define Point t_point

class Entity
{
protected:
    int _id;                         // Unique identifier
    e_entity_type _type;             // Type of entity
    Point _location;                 // Location of entity
    int _health;                     // Remaining health of this monster
    int shield_life;                 // Ignore for this league; Count down until shield spell fades
    int is_controlled;               // Ignore for this league; Equals 1 when this entity is under a control spell
    Point _trajectory;               // Trajectory of this monster
    bool _targeting_a_base;          // 0=monster with no target yet, 1=monster targeting a base
    e_entity_threat_for _threat_for; // Given this monster's trajectory, is it a threat to 1=your base, 2=your opponent's base, 0=neither

public:
    Entity();
    Entity(int id, e_entity_type type, Point location, int health, int shield_life, int is_controlled, Point trajectory, bool targeting_a_base, e_entity_threat_for threat_for);
    Entity(const Entity &other)
    {
        _id = other._id;
        _type = other._type;
        _location = other._location;
        _health = other._health;
        shield_life = other.shield_life;
        is_controlled = other.is_controlled;
        _trajectory = other._trajectory;
        _targeting_a_base = other._targeting_a_base;
        _threat_for = other._threat_for;
    }
    virtual ~Entity(){};

    Entity &operator=(const Entity &other)
    {
        this->_id = other._id;
        this->_type = other._type;
        this->_location = other._location;
        this->_health = other._health;
        this->shield_life = other.shield_life;
        this->is_controlled = other.is_controlled;
        this->_trajectory = other._trajectory;
        this->_targeting_a_base = other._targeting_a_base;
        this->_threat_for = other._threat_for;
        return *this;
    }

    Point getLocation() { return _location; }
    int getId() { return _id; }
    e_entity_type getType() { return _type; }
    int getHealth() { return _health; }
    int getShieldLife() { return shield_life; }
    int getIsControlled() { return is_controlled; }
    Point getTrajectory() { return _trajectory; }
    bool getTargetingABase() { return _targeting_a_base; }
    e_entity_threat_for getThreatFor() { return _threat_for; }
};

Entity::Entity(int id, e_entity_type type, Point location, int health, int shield_life, int is_controlled, Point trajectory, bool targeting_a_base, e_entity_threat_for threat_for)
{
    _id = id;
    _type = type;
    _location = location;
    _health = health;
    this->shield_life = shield_life;
    this->is_controlled = is_controlled;
    _trajectory = trajectory;
    _targeting_a_base = targeting_a_base;
    _threat_for = threat_for;
}

class Hero : public Entity
{
private:
    map<Entity, bool> _target_mob; // Entity -> bool (true if mob is target)

public:
    Hero(){};
    Hero(Entity &entity) : Entity(entity){};
    Hero(int id, e_entity_type type, Point location, int health, int shield_life, int is_controlled, Point trajectory, bool targeting_a_base, e_entity_threat_for threat_for)
    {
        _id = id;
        _type = type;
        _location = location;
        _health = health;
        this->shield_life = shield_life;
        this->is_controlled = is_controlled;
        _trajectory = trajectory;
        _targeting_a_base = targeting_a_base;
        _threat_for = threat_for;
    }
    virtual ~Hero(){};

    Hero(const Hero &other) : Entity(other){};
    Hero &operator=(const Hero &other)
    {
        Entity::operator=(other);
        return *this;
    };

public:
    // AI Functions

    void setTargetMob(Entity entity);

    // Getters

    map<Entity, bool> get_target_mob() const { return _target_mob; };
};

void Hero::setTargetMob(Entity entity)
{
    _target_mob.insert(_target_mob.begin(), pair<Entity, bool>(entity, true));
}

class Game
{
private:
    // Own variables
    Point _base_location;
    size_t _nb_heroes;

    size_t _life;
    size_t _mana;

    // Ennemy variables
    Point _ennemy_base_location;
    size_t _ennemy_life;
    size_t _ennemy_mana;

    // Game variables
    vector<Entity> _entity_mobs;
    vector<Hero> _entity_heroes;
    vector<Entity> _entity_ennemy_heroes;

public:
    // Game Default Functions
    Game();
    virtual ~Game();
    void init_values();
    void init_game_values();
    void clear_game();

    // Getters

    Point getBaseLocation() { return _base_location; }
    size_t getLife() { return _life; }
    size_t getMana() { return _mana; }
    size_t getNbHeroes() { return _nb_heroes; }
    Point getEnnemyBaseLocation() { return _ennemy_base_location; }
    size_t getEnnemyLife() { return _ennemy_life; }
    size_t getEnnemyMana() { return _ennemy_mana; }

    vector<Entity> getEntityMobs() { return _entity_mobs; }
    vector<Hero> getEntityHeroes() { return _entity_heroes; }
    vector<Entity> getEntityEnnemyHeroes() { return _entity_ennemy_heroes; }

    // IA Functions
    Entity &FindNearestMob(Entity &hero);

public:
    size_t getNbHeros() { return _nb_heroes; }
};

//*******************************//
//     GAME DEFAULT FUNCTIONS    //
//*******************************//

// Init values of the game are called once at the beginning of the game
Game::Game()
{
    init_values();
}
Game::~Game() {}
// Initializes the game for the first time
void Game::init_values()
{
    cerr << "init_values" << endl;
    cin >> _base_location.x >> _base_location.y;
    cin.ignore();
    cin >> _nb_heroes;
    cin.ignore();
    cerr << "Base: " << _base_location.x << " " << _base_location.y << endl;
    cerr << "NbHeroes: " << _nb_heroes << endl;
}
// Initializes the game for each round
void Game::init_game_values()
{
    for (int i = 0; i < 2; i++)
    {
        if (i == 0)
        {
            cin >> _life >> _mana;
            cin.ignore();
            cerr << "Health: " << _life << " Mana: " << _mana << endl;
        }
        else
        {
            cin >> _ennemy_life >> _ennemy_mana;
            cin.ignore();
            cerr << "Ennemy Health: " << _ennemy_life << " Ennemy Mana: " << _ennemy_mana << endl;
        }
    }

    int entity_count; // Amount of heros and monsters you can see
    cin >> entity_count;
    cin.ignore();
    for (int i = 0; i < entity_count; i++)
    {
        int id;
        int type;
        int x;
        int y;
        int shield_life;
        int is_controlled;
        int health;
        int vx;
        int vy;
        int near_base;
        int threat_for;

        cin >> id >> type >> x >> y >> shield_life >> is_controlled >> health >> vx >> vy >> near_base >> threat_for;
        cin.ignore();

        Point location = {size_t(x), size_t(y)};
        Point trajectory = {size_t(vx), size_t(vy)};
        Entity new_entity(id, (e_entity_type)type, location, health, shield_life, is_controlled, trajectory, (bool)near_base, (e_entity_threat_for)threat_for);
        Hero new_hero(new_entity);

        if (type == E_MONSTER)
            _entity_mobs.push_back(new_entity);
        else if (type == E_HERO)
            _entity_heroes.push_back(new_hero);
        else if (type == E_OPPONENT_HERO)
            _entity_ennemy_heroes.push_back(new_entity);
    }
}
// Clear game values for the next round
void Game::clear_game()
{
    _entity_mobs.clear();
    _entity_heroes.clear();
    _entity_ennemy_heroes.clear();
}

//*******************************//
//         IA FUNCTIONS          //
//*******************************//

Entity &Game::FindNearestMob(Entity &hero)
{
    Entity &nearest_mob = _entity_mobs[0];
    int nearest_distance = MAX_X + MAX_Y;

    for (Entity mob : _entity_mobs)
    {
        int distance = mob.getLocation().x - hero.getLocation().x + mob.getLocation().y - hero.getLocation().y;
        if (distance < nearest_distance)
        {
            nearest_distance = distance;
            nearest_mob = mob;
        }
    }
    return (nearest_mob);
}

int main()
{
    // game loop
    Game *game = new Game();
    while (1)
    {
        game->init_game_values();

        for (int i = 0; i < game->getNbHeros(); i++)
        {
            if (game->getEntityMobs().size() > 0)
            {
                Hero hero = game->getEntityHeroes()[i];
                Entity &nearest_mob = game->FindNearestMob(hero);

                cerr << "Mob location: " << nearest_mob.getLocation().x << " " << nearest_mob.getLocation().y << endl;
                cout << "MOVE " << nearest_mob.getLocation().x << " " << nearest_mob.getLocation().y << endl;
            }
            else
                cout << "WAIT" << endl;
        }
        game->clear_game();
    }
    delete game;
}