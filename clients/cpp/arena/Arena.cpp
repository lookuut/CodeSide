//
// Created by lookuut on 29.11.19.
//

#include "Arena.hpp"
#include "../utils/Geometry.h"
#include <chrono>
#include <ctime>
#include <math.h>
#include <iostream>
#include "../model/Game.hpp"

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

void Arena::mineMicrotick() {

    for (int i = mines.size() - 1; i >= 0; --i) {
        Mine & mine = mines[i];

        if (mine.timer) {
            *mine.timer.get() -= microTicksPerSecond;

            if (*mine.timer <= 0) {
                switch (mine.state) {
                    case MineState::PREPARING:
                        mine.state = MineState::IDLE;
                        mine.timer = nullptr;
                        break;
                    case MineState::TRIGGERED://explode
                        mine.explode(units, mines, i);
                        break;
                    default:
                        throw std::runtime_error("Unexpected discriminant value");
                }
            }
        }
    }
}

void Arena::bulletMicrotick() {

    auto it = bullets.begin();
    while (it != bullets.end()) {
        Bullet & bullet = *it;

        bullet.move(bullet.velocity);

        bool isHeatUnit = false;
        for (Unit & unit: units) {

            if (unit.id != bullet.unitId and unit.isOverlap(bullet)) {

                if (bullet.weaponType == WeaponType::ROCKET_LAUNCHER) {
                    bullet.explossion(units);
                } else {
                    unit.health -= bullet.damage;
                }
                isHeatUnit = true;
            }
        }

        if (isHeatUnit) {
            it = bullets.erase(it);
        } else {
            ++it;
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
                }
                unit.health -= bullet.damage;

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

void Arena::bulletMineOverlap() {
    auto it = bullets.begin();

    while (it != bullets.end()) {
        Bullet & bullet = *it;

        bool heatMine = false;
        for (int i = mines.size() - 1; i >= 0; --i) {
            Mine & mine = mines[i];

            if (Geometry::isRectOverlap(bullet.leftTop, bullet.rightDown, mine.leftTopAngle, mine.rightDownAngle)) {
                mine.explode(units, mines, i);
                heatMine = true;
                break;
            }
        }

        if (heatMine) {
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

void Arena::shootAction(Unit &unit, const Vec2Double & aim) {

    if (unit.weapon != nullptr and aim != ZERO_VEC_2_DOUBLE) {
        Weapon * weapon = unit.weapon.get();

        if (weapon->fireTimer <= .0 and weapon->magazine > 0) {
            double randomSpread = weapon->spread * (double)rand() / RAND_MAX - weapon->spread / 2.0;

            Vec2Double vel = Vec2Double(aim.x * weapon->params.bullet.speed, aim.y * weapon->params.bullet.speed);
            Bullet bullet = Bullet(
                    weapon->type,
                    unit.id,
                    unit.playerId,
                    Vec2Double(unit.position.x, unit.position.y + unit.size.y / 2.0),
                    vel.rotate(randomSpread),
                    weapon->params.bullet.damage,
                    weapon->params.bullet.size,
                    weapon->params.explosion
            );
            bullets.push_back(bullet);

            double angle = atan2(aim.y, aim.x);
            weapon->fireTimer = weapon->params.fireRate;
            weapon->lastFireTick = currentTick;

            weapon->spread += weapon->params.recoil + (weapon->lastAngle != Consts::noLastAngleValue ? abs(angle - weapon->lastAngle) : 0);

            if (weapon->spread > weapon->params.maxSpread) {
                weapon->spread = weapon->params.maxSpread;
            }

            weapon->lastAngle = angle;

            --weapon->magazine;

            if (weapon->magazine <= 0) {
                weapon->fireTimer = weapon->params.reloadTime;
                weapon->magazine = weapon->params.magazineSize;
            }
        }
    }
}

void Arena::mineActicate(const Unit &unit) {

    for (Mine & mine : mines) {
        if (mine.state == MineState::IDLE and Geometry::isRectOverlap(unit.leftTop, unit.rightDown, mine.activateLeftTopAngle, mine.activateRightDownAngle)) {
            mine.state = MineState ::TRIGGERED;
            mine.timer = shared_ptr<double>(new double(properties->mineTriggerTime + microTicksPerSecond));
        }
    }
}

void Arena::tick(const vector<UnitAction> & actions) {

    /*for (int ii = 0; ii < 100; ++ii) {

        chrono::system_clock::time_point start = chrono::system_clock::now();

        for (int j = 0; j < 10000; ++j) {
    */


    for (Unit &unit : units) {
        const UnitAction &action = actions[Game::unitIndexById(unit.id)];

        if (action.plantMine) {
            unit.plantMine(mines);
        }
    }

    for (int i = 0; i < Consts::microticks; ++i) {

        bulletMicrotick();

        for (Unit &unit : units) {
            const UnitAction &action = actions[Game::unitIndexById(unit.id)];
            double horSpeed = sgn(action.velocity) * min(abs(action.velocity), properties->unitMaxHorizontalSpeed);

            if (action.plantMine) {
                unit.plantMine(mines);
            }

            if (action.shoot) {
                shootAction(unit, action.aim);
            }

            unit.weaponRoutine(microTicksPerSecond, action.aim);
            pickUpLoots(unit);

            unit.prevPosition = unit.position;

            unit.moveHor(horSpeed * microTicksPerSecond);
            unit.horizontalWallCollision(horSpeed);

            for (Unit & opponentUnit : units) {
                if (opponentUnit.id != unit.id) {
                    unit.unitHorCollide(opponentUnit);
                }
            }

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

            if (unit.isOnWall()) {
                unit.verticalWallCollision();
            } else if (unit.isOnPlatform() and !unit.jumpState.canJump and !action.jumpDown) {
                unit.platformCollision();
            }

            if (unit.onLadder) {
                unit.applyOnGround();
            }

            if (unit.isOnJumpPad()) {
                unit.applyJumpPad(properties->jumpPadJumpSpeed, properties->jumpPadJumpTime);
            }

            for (Unit & opponentUnit : units) {
                if (opponentUnit.id != unit.id) {
                    unit.unitVerCollide(opponentUnit);
                }
            }

            bulletOverlapWithUnit(unit);
            mineActicate(unit);

            unit.prevPosition = unit.position;
        }

        mineMicrotick();
        bulletMineOverlap();
        bulletWallOverlap();
    }

    ++currentTick;
        /*}

        chrono::system_clock::time_point end = chrono::system_clock::now();
        long micros = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        cout << micros << endl;
    }*/
}
