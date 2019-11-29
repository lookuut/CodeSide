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

class Arena {
private:
    int currentTick = 0;

public:
    Properties properties;
    Level level;
    std::vector<Player> players;
    std::vector<Unit> units;
    std::vector<Bullet> bullets;
    std::vector<Mine> mines;
    std::vector<LootBox> lootBoxes;
    Arena();

    Arena(
            Properties &properties,
            Level &level,
            std::vector<Player> &players,
            std::vector<Unit> &units,
            std::vector<Bullet> &bullets,
            std::vector<Mine> &mines,
            std::vector<LootBox> &lootBoxes
            );


    void tick();

    static Arena readFrom(InputStream& stream);

    void writeTo(OutputStream& stream) const;

    std::string toString() const;
};

#endif //AICUP2019_ARENA_HPP
