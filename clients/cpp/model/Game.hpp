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
#include <limits>

typedef struct AstarNode{
    int x;
    int y;

    double vel;

    bool jump;
    bool jumpDown;

    mutable int timeCostFromStart;//cost from start node
    float distanceCostToFinish;//cost to end node

    mutable Unit unit;

    mutable int parentNodeIndex;
    mutable int parentNodeStateIndex;

    bool operator==(const AstarNode & node) const{
        return (node.x + node.y * node.unit.level->width * Consts::ppFieldSize) == (x + y * unit.level->width * Consts::ppFieldSize);
    }

    int nodeIndex() const {
        return x + y * unit.level->width * Consts::ppFieldSize;
    }

} AstarNode;


struct AstarNodeHasher {
    size_t operator() (const AstarNode &node) const
    {
        return node.x + node.y * node.unit.level->width * Consts::ppFieldSize + node.unit.stateIndex() * node.unit.level->height * Consts::ppFieldSize * node.unit.level->width * Consts::ppFieldSize;
    }
};

struct SortedAstarNodeComparator {
    bool operator() (const AstarNode &leftNode, const AstarNode &rightNode) const
    {

        if (leftNode.timeCostFromStart + leftNode.distanceCostToFinish < rightNode.timeCostFromStart + rightNode.distanceCostToFinish) {
            return true;
        }

        if (leftNode.timeCostFromStart + leftNode.distanceCostToFinish == rightNode.timeCostFromStart + rightNode.distanceCostToFinish and leftNode.timeCostFromStart < rightNode.timeCostFromStart) {
            return true;
        }

        if (leftNode.timeCostFromStart + leftNode.distanceCostToFinish == rightNode.timeCostFromStart + rightNode.distanceCostToFinish and leftNode.timeCostFromStart == rightNode.timeCostFromStart and leftNode.unit.stateIndex() < rightNode.unit.stateIndex()) {
            return true;
        }

        if (leftNode.timeCostFromStart + leftNode.distanceCostToFinish == rightNode.timeCostFromStart + rightNode.distanceCostToFinish and leftNode.timeCostFromStart == rightNode.timeCostFromStart and leftNode.unit.stateIndex() == rightNode.unit.stateIndex() and leftNode.nodeIndex() < rightNode.nodeIndex()) {
            return true;
        }

        return false;

    }
};



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
    list<int> aliveUnits;

    std::vector<Bullet> bullets;
    std::vector<std::vector<Bullet*>> unitBullets;
    std::vector<Mine> mines;

    std::vector<LootBox> lootHealthPacks;
    vector<int> lootHealPacksIds;


    std::vector<LootBox> lootWeapons;
    vector<int> lootWeaponIds;
    vector<int> lootWeaponPistolIds;
    vector<int> lootWeaponAssultIds;

    std::vector<LootBox> lootMines;

    vector<vector<AstarNode>> unitAstarPath;

    int pp_width;
    int pp_height;

    static Game * init(InputStream &stream, int allyPlayerId);
    static Game * updateTick(InputStream& stream);
    static Properties * getProperties();
    static Level * getLevel();

    static std::list<int> & getPlayerUnits(int playerId);


    optional<int> getNearestPistol(const Vec2Double & unitPos) const;
    optional<int> getNearestAssult(const Vec2Double & unitPos) const;

    int getNearestWeapon(const Vec2Double & unitPos, const vector<int> weaponIds) const;;


    void writeTo(OutputStream& stream) const;
    std::string toString() const;

    static std::unique_ptr<Game> game;

    inline static int unitIndexById(int id) {
        return id - 3;
    }

    inline static int allyUnitIndexById(int id) {
        return (id - 3) / 2;
    }

    inline static int nearestLootBox(const vector<LootBox> & loots, const Vec2Double & pos) {
        double minSqrDist = Vec2Double::sqrLen(loots[0].position, pos);
        int minDistLootId = 0;

        for (int i = loots.size() - 1; i > 0; --i) {

            if (minSqrDist > Vec2Double::sqrLen(loots[i].position, pos)) {
                minSqrDist = Vec2Double::sqrLen(loots[i].position, pos);
                minDistLootId = i;
            }
        }

        return minDistLootId;
    }

    void buildPotentionalField(const Vec2Double & pos, vector<vector<int>> & field);


    void updateLootBoxes(InputStream &stream);
    int aStarPathInit(const Unit & unit, const vector<LootBox> & loots, Debug & debug, vector<AstarNode> & allyUnitPath);

    ~Game();

    static float fCost(int x, int y, int x1, int y1);

};

#endif
