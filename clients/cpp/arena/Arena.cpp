//
// Created by lookuut on 29.11.19.
//

#include "Arena.hpp"
#include <chrono>
#include <ctime>

#include <iostream>
using namespace std;

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

Arena::Arena(Properties * properties, Level * level): properties(properties), level(level) {
    microTicksPerSecond = 1.0 / (properties->ticksPerSecond * Consts::microticks);
}


void Arena::update(
        const std::vector<Unit> &units,
        const std::vector<Bullet> &bullets,
        const std::vector<Mine> &mines,
        const std::vector<LootBox> &lootHealthPacks,
        const std::vector<LootBox> &lootWeapons,
        const std::vector<LootBox> &lootMines
) {
    this->units = units;
    this->bullets = bullets;
    this->mines = mines;
    this->lootHealthPacks = lootHealthPacks;
    this->lootWeapons = lootWeapons;
    this->lootMines = lootMines;
}


void Arena::bulletMicrotick() {

    auto it = bullets.begin();
    while (it != bullets.end()) {
        Bullet & bullet = *it;

        bullet.move(bullet.velocity);

        for (Unit & unit: units) {

            if (unit.id != bullet.unitId and unit.isOverlap(bullet)) {

                if (bullet.weaponType == WeaponType::ROCKET_LAUNCHER) {
                    bullet.explossion(units);
                } else {
                    unit.health -= bullet.damage;
                }

                it = bullets.erase(it);
            } else {
                ++it;
            }
        }
    }
}

void Arena::bulletOverlapWithUnit(Unit &unit) {
    auto it = bullets.begin();

    while (it != bullets.end()) {
        Bullet & bullet = *it;

        if (unit.id != bullet.unitId) {

            bullet.velocity *= -1.0;
            bullet.move(bullet.velocity);

            if (unit.id != bullet.unitId and unit.isOverlap(bullet)) {

                if (bullet.weaponType == WeaponType::ROCKET_LAUNCHER) {
                    bullet.explossion(units);
                } else {
                    unit.health -= bullet.damage;
                }

                it = bullets.erase(it);
                continue;
            }

            bullet.velocity *= -1.0;
            bullet.move(bullet.velocity);

            if (unit.id != bullet.unitId and unit.isOverlap(bullet)) {

                if (bullet.weaponType == WeaponType::ROCKET_LAUNCHER) {
                    bullet.explossion(units);
                } else {
                    unit.health -= bullet.damage;
                }

                it = bullets.erase(it);
                continue;
            }
        }

        ++it;
    }
}

void Arena::bulletWallOverlap() {
    auto it = bullets.begin();

    while (it != bullets.end()) {
        Bullet & bullet = *it;

        int xl = (int)(bullet.position.x - bullet.size / 2.0);
        int yd = (int)(bullet.position.y - bullet.size / 2.0);
        int xr = (int)(bullet.position.x + bullet.size / 2.0);
        int yt = (int)(bullet.position.y + bullet.size / 2.0);

        if (level->tiles[xl][yd] == Tile::WALL or level->tiles[xr][yd] == Tile::WALL or level->tiles[xl][yt] == Tile::WALL or level->tiles[xr][yt] == Tile::WALL) {
            if (bullet.weaponType == WeaponType::ROCKET_LAUNCHER) {
                bullet.explossion(units);
            }
            it = bullets.erase(it);
        } else {
            ++it;
        }
    }
}

void Arena::pickUpLoots(Unit & unit) {//@TODO how it works?

    for (auto it = lootHealthPacks.begin(); it != lootHealthPacks.end(); ) {
        if (unit.picUpkHealthPack(*it)) {
            it = lootHealthPacks.erase(it);
        } else {
            ++it;
        }
    }

    for (auto it = lootWeapons.begin(); it != lootWeapons.end(); ) {
        if (unit.pickUpWeapon(*it)) {
            it = lootWeapons.erase(it);
        } else {
            ++it;
        }
    }

    for (auto it = lootMines.begin(); it != lootMines.end(); ) {
        if (unit.picUpkMine(*it)) {
            it = lootMines.erase(it);
        } else {
            ++it;
        }
    }
}


void Arena::tick(const vector<UnitAction> & actions) {

    /*for (int ii = 0; ii < 100; ++ii) {

        chrono::system_clock::time_point start = chrono::system_clock::now();

        for (int j = 0; j < 10000; ++j) {
    */        for (Unit &unit : units) {
                const UnitAction &action = actions[unit.id];
                double horSpeed = sgn(action.velocity) * min(abs(action.velocity), properties->unitMaxHorizontalSpeed);

                for (int i = 0; i < Consts::microticks; ++i) {

                    bulletMicrotick();

                    unit.prevPosition = unit.position;

                    unit.moveHor(horSpeed * microTicksPerSecond);
                    unit.horizontalWallCollision(horSpeed);

                    if (unit.jumpState.canJump and unit.jumpState.canCancel) {//Jumping
                        if (action.jump) {
                            unit.jumping(unit.jumpState.speed * microTicksPerSecond, microTicksPerSecond);
                        }
                    }

                    if (!unit.jumpState.canJump and unit.jumpState.maxTime <= .0) {//Down
                        unit.downing(properties->unitFallSpeed * microTicksPerSecond);
                    }

                    if (unit.jumpState.canCancel) {//Down
                        if (!action.jump) {
                            unit.downing(properties->unitFallSpeed * microTicksPerSecond);
                            unit.applyJumpCancel();
                        }
                    } else if (unit.jumpState.maxTime > 0) {
                        unit.jumping(unit.jumpState.speed * microTicksPerSecond, microTicksPerSecond);
                    }

                    unit.heatRoofRoutine();

                    if (unit.jumpState.maxTime <= .0) {
                        unit.applyJumpCancel();
                    }

                    if (unit.isOnWall() or (unit.isOnPlatform() and !unit.jumpState.canJump and !action.jumpDown)) {
                        unit.verticalWallCollision();
                    }

                    if (unit.onLadder) {
                        unit.applyOnGround();
                    }

                    if (unit.isOnJumpPad()) {
                        unit.applyJumpPad(properties->jumpPadJumpSpeed, properties->jumpPadJumpTime);
                    }

                    bulletOverlapWithUnit(unit);

                    pickUpLoots(unit);

                    unit.prevPosition = unit.position;

                    bulletWallOverlap();
                }
            }

            ++currentTick;
        /*}

        chrono::system_clock::time_point end = chrono::system_clock::now();
        long micros = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        cout << micros << endl;
    }*/
}

/*for (Unit & unit : units) {
        UnitAction action = unit.actions.front();

        Weapon * currentWeapon = unit.weapon.get();

        if (action.shoot and currentWeapon != nullptr and currentWeapon->lastFireTick != nullptr and action.aim != ZERO_VEC_2_DOUBLE) {//can shoot

            action.aim.normalize() *= currentWeapon->params.bullet.speed;

            Bullet bullet = Bullet(
                    currentWeapon->type,
                    unit.id,
                    unit.playerId,
                    Vec2Double(unit.position.x, unit.position.y + unit.size.y / 2.0),
                    action.aim,
                    currentWeapon->params.bullet.damage,
                    currentWeapon->params.bullet.size,
                    currentWeapon->params.explosion
            );

            bullets.push_back(bullet);

    }

    }*/