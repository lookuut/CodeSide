//
// Created by lookuut on 29.11.19.
//

#include "Simulation.hpp"
#include "../utils/Geometry.h"
#include <chrono>
#include <ctime>
#include <math.h>
#include <iostream>
#include "../model/Game.hpp"

using namespace std;

template <typename T> int sgn(T val) {
    return (T(0) <= val) - (val < T(0));
}

Simulation::Simulation(Game * game, Debug * debug): properties(&game->properties), level(&game->level), debug(debug), game(game) {
    microTicksPerSecond = 1.0 / (properties->ticksPerSecond * Consts::microticks);
}


void Simulation::update() {

    this->allyPoints = game->players[game->allyPlayerId].score;
    this->enemyPoints = game->players[game->enemyPlayerId].score;

    this->units = game->units;

    this->bullets = game->bullets;
    this->mines = game->mines;
    this->lootHealthPacks = game->lootHealthPacks;
    this->lootWeapons = game->lootWeapons;
    this->lootMines = game->lootMines;

    currentTick = 0;
}

void Simulation::mineMicrotick() {

    for (int i = mines.size() - 1; i >= 0; --i) {
        Mine & mine = mines[i];

        if (mine.timer >= 0) {
            mine.timer -= microTicksPerSecond;

            if (mine.timer <= 0 and mine.timer != -1) {
                map<int, bool> deletedMines;
                vector<Mine> newMines;

                switch (mine.state) {
                    case MineState::PREPARING:
                        mine.state = MineState::IDLE;
                        mine.timer = -1;
                        break;
                    case MineState::TRIGGERED://explode

                        mine.explode(units, mines, i, deletedMines);

                        for (int mineId = mines.size() - 1; mineId >= 0; --mineId) {
                            if (deletedMines.find(mineId) != deletedMines.end()) {
                                newMines.push_back(mines[mineId]);
                            }
                        }

                        mines = newMines;
                        break;
                    default:
                        throw std::runtime_error("Unexpected discriminant value");
                }
            }
        }
    }
}

void Simulation::moveBullets() {

    for (Bullet & bullet: bullets) {
        bullet.move(bullet.velocity, properties->updatesPerTick, properties);
    }
}

void Simulation::bulletOverlapWithUnit(Unit &unit) {
    auto it = bullets.begin();

    while (it != bullets.end()) {
        Bullet & bullet = *it;

        if (unit.id != bullet.unitId) {
            Vec2Double unitVelocity = (unit.position - unit.prevPosition) * (properties->ticksPerSecond * Consts::microticks);

            if (unit.crossBulletTick(bullet, properties->updatesPerTick, unitVelocity, unit.prevPosition) <= properties->updatesPerTick) {
                if (bullet.weaponType == WeaponType::ROCKET_LAUNCHER) {

                    for (Unit & unit : units) {
                        bullet.explossion(unit, unit.position);
                    }
                }
                unit.health -= bullet.damage;

                it = bullets.erase(it);
                continue;
            }
        }

        ++it;
    }
}

void Simulation::bulletWallOverlap() {
    auto it = bullets.begin();

    while (it != bullets.end()) {
        Bullet & bullet = *it;

        double deltax = bullet.velocity.x / properties->ticksPerSecond;
        double deltay = bullet.velocity.y / properties->ticksPerSecond;

        int prevTileX = (int)bullet.frontPoint.x;

        int xl = (int)(bullet.frontPoint.x + deltax);
        int yd = (int)(bullet.frontPoint.y + deltay);

        int xr = (int)(bullet.frontPoint.x + bullet.widthBorderSign * bullet.size + deltax);
        int yt = (int)(bullet.frontPoint.y + bullet.heightBorderSign * bullet.size + deltay);

        if (level->tiles[xl][yd] == Tile::WALL or level->tiles[xr][yd] == Tile::WALL or level->tiles[xl][yt] == Tile::WALL or level->tiles[xr][yt] == Tile::WALL) {
            int heatTileX = xr;
            int heatTileY = yt;

            if (level->tiles[xl][yd] == Tile::WALL) {
                heatTileX = xl;
                heatTileY = yd;
            } else if (level->tiles[xr][yd] == Tile::WALL) {
                heatTileX = xr;
                heatTileY = yd;
            } else if (level->tiles[xl][yt] == Tile::WALL) {
                heatTileX = xl;
                heatTileY = yt;
            }

            int dirX = (bullet.velocity.x < 0 ? 1 : 0);
            int dirY = (bullet.velocity.y < 0 ? 1 : 0);

            Segment segment;
            if (prevTileX == heatTileX) {
                segment = Segment(Vec2Double(heatTileX, heatTileY + dirY), Vec2Double(heatTileX + 1, heatTileY + dirY));
            } else {
                segment = Segment(Vec2Double(heatTileX + dirX, heatTileY + 1), Vec2Double(heatTileX + dirX, heatTileY));
            }


            int microticksToWall = bullet.overlapWithSegmentMicroTicks(segment, properties->updatesPerTick, properties);

            bullet.move(bullet.velocity, microticksToWall, properties);

            if (bullet.weaponType == WeaponType::ROCKET_LAUNCHER) {
                for (Unit & unit : units) {
                    Vec2Double unitVelocity = (unit.position - unit.prevPosition);
                    Vec2Double unitPos = unit.prevPosition + unitVelocity * ( microticksToWall / (double)properties->updatesPerTick);
                    bullet.explossion(unit, unitPos);
                }
            }
            it = bullets.erase(it);
        } else {
            ++it;
        }
    }
}

void Simulation::bulletMineOverlap() {
    auto it = bullets.begin();

    while (it != bullets.end()) {
        Bullet & bullet = *it;

        bool heatMine = false;
        for (int i = mines.size() - 1; i >= 0; --i) {//@TODO do it well
            Mine & mine = mines[i];

            if (Geometry::isRectOverlap(bullet.leftTop, bullet.rightDown, mine.leftTopAngle, mine.rightDownAngle)) {

                map<int, bool> deletedMines;
                mine.explode(units, mines, i, deletedMines);
                vector<Mine> newMines;

                for (int mineId = mines.size() - 1; mineId >= 0; --mineId) {
                    if (deletedMines.find(mineId) != deletedMines.end()) {
                        newMines.push_back(mines[mineId]);
                    }
                }

                mines = newMines;
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

void Simulation::pickUpLoots(Unit & unit) {//@TODO how it works?

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

void Simulation::shootAction(Unit &unit, const Vec2Double & aim) {

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

void Simulation::mineActicate(const Unit &unit) {

    for (Mine & mine : mines) {
        if (mine.state == MineState::IDLE and Geometry::isRectOverlap(unit.leftTop, unit.rightDown, mine.activateLeftTopAngle, mine.activateRightDownAngle)) {
            mine.state = MineState ::TRIGGERED;
            mine.timer = properties->mineTriggerTime + microTicksPerSecond;
        }
    }
}

void Simulation::tick(const vector<UnitAction> & actions, int ticks) {

    /*for (int ii = 0; ii < 100; ++ii) {

        chrono::system_clock::time_point start = chrono::system_clock::now();

        for (int j = 0; j < 1000000; ++j) {*/


    for (int & unitId: game->aliveAllyUnits) {
        Unit & unit = units[Game::unitIndexById(unitId)];
        const UnitAction &action = actions[Game::allyUnitIndexById(unit.id)];

        if (action.plantMine) {
            unit.plantMine(mines);
        }
    }

    for (int tick = 0; tick < ticks; tick++){
        for (int i = 0; i < Consts::microticks; ++i) {

            for (int & unitId: game->aliveAllyUnits) {
                Unit & unit = units[Game::unitIndexById(unitId)];
                const UnitAction &action = actions[Game::allyUnitIndexById(unit.id)];
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

                for (int unitId : game->aliveAllyUnits) {
                    Unit & nearUnit = game->units[Game::unitIndexById(unitId)];
                    if (nearUnit.id != unit.id) {
                        unit.unitHorCollide(nearUnit);
                    }
                }

                for (int unitId : game->aliveEnemyUnits) {
                    Unit & nearUnit = game->units[Game::unitIndexById(unitId)];
                    unit.unitHorCollide(nearUnit);
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

                for (int unitId : game->aliveAllyUnits) {
                    Unit & nearUnit = game->units[Game::unitIndexById(unitId)];
                    if (nearUnit.id != unit.id) {
                        unit.unitVerCollide(nearUnit);
                    }
                }

                for (int unitId : game->aliveEnemyUnits) {
                    Unit & nearUnit = game->units[Game::unitIndexById(unitId)];
                    unit.unitVerCollide(nearUnit);
                }

                bulletOverlapWithUnit(unit);
                mineActicate(unit);

                if (unit.onGround or unit.onLadder) {
                    ++unit.onGroundLadderTicks;
                }

                unit.prevPosition = unit.position;
            }

            mineMicrotick();
            bulletMineOverlap();
            bulletWallOverlap();

            moveBullets();
        }
    }

    currentTick += ticks;
     /*   }

        chrono::system_clock::time_point end = chrono::system_clock::now();
        long micros = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        cout << micros << endl;
    }*/
}
