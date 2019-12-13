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
    Game(
            int currentTick,
            Properties properties,
            Level level,
            std::vector<Player> players,
            std::vector<Unit> units,
            std::vector<Bullet> bullets,
            std::vector<Mine> mines
            );

public:
    int currentTick;
    int maxUnitId;
    Properties properties;
    Level level;

    std::vector<Player> players;

    std::vector<Unit> units;
    std::vector<int> unitsIndex = vector<int>(Consts::maxUnitCount, 0);

    std::map<int, vector<Unit*>> playerUnits;
    std::vector<Bullet> bullets;
    std::vector<std::vector<Bullet*>> unitBullets;
    std::vector<Mine> mines;

    std::vector<LootBox> lootHealthPacks;
    std::vector<LootBox> lootWeapons;
    std::vector<LootBox> lootMines;

    static Game * init(InputStream &stream);
    static Game * updateTick(InputStream& stream);
    static Properties * getProperties();
    static Level * getLevel();

    static std::vector<Unit*> & getPlayerUnits(int playerId);

    void writeTo(OutputStream& stream) const;
    std::string toString() const;

    static std::unique_ptr<Game> game;

    inline static int unitIndexById(int id) {
        return id - 1;
    }
};

#endif
