//
// Created by lookuut on 29.11.19.
//

#ifndef AICUP2019_ARENA_HPP
#define AICUP2019_ARENA_HPP

#include "../Stream.hpp"
#include <string>
#include "../model/Properties.hpp"

#include "../model/Vec2Double.hpp"
#include "../model/Vec2Double.hpp"
#include <unordered_map>
#include "../model/WeaponType.hpp"
#include "../model/WeaponParams.hpp"
#include "../model/BulletParams.hpp"
#include <memory>
#include "../model/ExplosionParams.hpp"
#include "../model/Level.hpp"
#include "../model/Tile.hpp"
#include "../model/Player.hpp"
#include "../model/Unit.hpp"
#include "../model/JumpState.hpp"
#include "../model/Weapon.hpp"
#include "../model/Bullet.hpp"
#include "../model/Mine.hpp"
#include "../model/LootBox.hpp"
#include "../model/UnitAction.hpp"
#include "../Consts.h"
#include "../model/Game.hpp"
#include <math.h>
#include <vector>

class Simulation {
private:
    int currentTick = 0;
    double microTicksPerSecond;
public:
    Properties * properties;
    Level * level;
    Game * game;
    Debug * debug;

    int enemyPoints = 0;
    int allyPoints = 0;

    vector<Unit> units;

    std::vector<Bullet> bullets;
    std::vector<Mine> mines;

    std::vector<LootBox> lootHealthPacks;
    std::vector<LootBox> lootWeapons;
    std::vector<LootBox> lootMines;

    Simulation(Game * game, Debug * debug);

    void update();

    void tick(const vector<UnitAction> & actions, int ticks);

    void moveBullets();
    void mineMicrotick();
    void mineActicate(const Unit & unit);
    void bulletOverlapWithUnit(Unit &unit);
    void bulletWallOverlap();
    void bulletMineOverlap();
    void shootAction(Unit & unit, const Vec2Double & aim);

    std::string toString() const;
    void pickUpLoots(Unit & unit);
};

#endif //AICUP2019_ARENA_HPP
