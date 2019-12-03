#include "MyStrategy.hpp"
#include "utils/Geometry.h"
#include <iostream>


using namespace std;

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

MyStrategy::MyStrategy() {}

double distSqr(Vec2Double a, Vec2Double b) {
    return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
}

UnitAction MyStrategy::getAction(const Unit &unit, const Game &game,
                                 Debug &debug) {

    const Unit *nearestEnemy = nullptr;

    for (const Unit &other : game.units) {
        if (other.playerId != unit.playerId) {
            if (nearestEnemy == nullptr ||
                    unit.position.distSqr(other.position) < unit.position.distSqr(nearestEnemy->position)) {
                nearestEnemy = &other;
            }
        }
    }

    const LootBox *nearestWeapon = nullptr;
    const LootBox *nearestAidKit = nullptr;

    for (const LootBox &lootBox : game.lootBoxes) {
        if (std::dynamic_pointer_cast<Item::Weapon>(lootBox.item)) {
            if (nearestWeapon == nullptr ||
                    unit.position.distSqr(lootBox.position) < unit.position.distSqr(nearestWeapon->position)) {
                nearestWeapon = &lootBox;
            }
        }

        if (std::dynamic_pointer_cast<Item::HealthPack>(lootBox.item)) {
            if (nearestAidKit == nullptr ||
                unit.position.distSqr(lootBox.position) < unit.position.distSqr(nearestAidKit->position)) {
                nearestAidKit = &lootBox;
            }
        }
    }

    const Mine *nearestMine = nullptr;

    for (const Mine &mine : game.mines) {
        if (mine.state == MineState::IDLE or mine.state == MineState::TRIGGERED) {
            if (nearestMine == nullptr or mine.position.distSqr(unit.position) < nearestMine->position.distSqr(unit.position)) {
                nearestMine = &mine;
            }
        }
    }

    Vec2Double targetPos = unit.position;

    if (unit.weapon == nullptr && nearestWeapon != nullptr) {
        targetPos = nearestWeapon->position;
    } else if (nearestEnemy != nullptr and unit.health < nearestEnemy->health and nearestAidKit != nullptr) {
        targetPos = nearestAidKit->position;
    } else if (nearestEnemy != nullptr) {
        targetPos = nearestEnemy->position;
    }

    
    debug.draw(CustomData::Log(
            std::string("Target pos: ") + targetPos.toString()));

    Vec2Double aim = Vec2Double(0, 0);

    UnitAction action;
    action.shoot = true;

    if (nearestEnemy != nullptr) {
        aim = Vec2Double(nearestEnemy->position.x - unit.position.x,
                         nearestEnemy->position.y - unit.position.y);

        Vec2Float unitCenter = Vec2Float(unit.position + Vec2Double(0, game.properties.unitSize.y / 2.0));
        Vec2Float enemyCenter = Vec2Float(nearestEnemy->position + Vec2Double(0, game.properties.unitSize.y / 2.0));

        if (game.level.crossWall(unitCenter, enemyCenter)) {
            action.shoot = false;
        }
    }



    bool jump = targetPos.y > unit.position.y;

    if (
            targetPos.x > unit.position.x
            &&
            game.level.tiles[size_t(unit.position.x + 1)][size_t(unit.position.y)] == Tile::WALL
            ) {
        jump = true;
    }

    if (targetPos.x < unit.position.x &&
        game.level.tiles[size_t(unit.position.x - 1)][size_t(unit.position.y)] ==
        Tile::WALL) {
        jump = true;
    }

    cout << "Tick : " << game.currentTick << unit.position.toString() << " " << (game.currentTick > 0 ? (unit.position - prevPos).toString() : "" ) << endl;


    for (auto & bullet : game.bullets) {
        cout << "Bullet " << bullet.position.toString() << " " << (bulletPrevPos != ZERO_VEC_2_DOUBLE ? (bullet.position - bulletPrevPos).toString() : "") << endl;
        bulletPrevPos = bullet.position;
    }

    prevPos = unit.position;

    action.velocity = sgn(targetPos.x - unit.position.x) * game.properties.unitMaxHorizontalSpeed;
    action.jump = jump;
    action.jumpDown = !action.jump;
    action.aim = aim;

    action.swapWeapon = false;
    action.plantMine = false;
    return action;
}