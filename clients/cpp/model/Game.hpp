#ifndef _MODEL_GAME_HPP_
#define _MODEL_GAME_HPP_


#include <string>
#include <stdexcept>
#include <vector>

#include "../Stream.hpp"
#include "Properties.hpp"
#include "Player.hpp"
#include "Level.hpp"
#include "Unit.hpp"
#include "Bullet.hpp"
#include "Mine.hpp"
#include "Item.hpp"
#include "../Consts.h"


/**
 * Singleton class
 */
class Game {

private:
    Game();

public:
    int currentTick;

    Properties properties;
    Level level;

    int allyPlayerId;
    int enemyPlayerId;

    std::map<int, Player> players;
    vector<Unit> units;

    list<int> aliveAllyUnits;
    list<int> aliveEnemyUnits;

    std::vector<Bullet> bullets;
    std::vector<std::vector<Bullet*>> unitBullets;
    std::vector<Mine> mines;

    std::vector<LootBox> lootHealthPacks;
    std::vector<LootBox> lootWeapons;
    std::vector<LootBox> lootMines;

    static Game * init(InputStream &stream, int allyPlayerId);
    static Game * updateTick(InputStream& stream);
    static Properties * getProperties();
    static Level * getLevel();

    static std::list<int> & getPlayerUnits(int playerId);

    void writeTo(OutputStream& stream) const;
    std::string toString() const;

    static std::unique_ptr<Game> game;

    inline static int unitIndexById(int id) {
        return id - 3;
    }

    inline static int allyUnitIndexById(int id) {
        return (id - 3) / 2;
    }

    ~Game();
};

#endif
