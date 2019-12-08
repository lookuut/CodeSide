//
// Created by lookuut on 29.11.19.
//

#include "Arena.hpp"

#include <iostream>
using namespace std;

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

Arena::Arena() {}


Arena::Arena(
        const Properties &properties,
        const Level &level,
        const std::vector<Player> &players,
        const std::vector<Unit> &units,
        const std::vector<Bullet> &bullets,
        const std::vector<Mine> &mines,
        const std::vector<LootBox> &lootBoxes
) : properties(properties), level(level), players(players), units(units), bullets(bullets),
    mines(mines), lootBoxes(lootBoxes) {
    microTicksPerSecond = 1.0 / (properties.ticksPerSecond * microticks);
}


void Arena::bulletMicrotick() {

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

        if (level.tiles[xl][yd] == Tile::WALL or level.tiles[xr][yd] == Tile::WALL or level.tiles[xl][yt] == Tile::WALL or level.tiles[xr][yt] == Tile::WALL) {
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

    auto it = lootBoxes.begin();

    while (it != lootBoxes.end()) {
        LootBox & lootBox = *it;

        if (std::dynamic_pointer_cast<Item::HealthPack>(lootBox.item) and unit.health < Properties::getProperty().unitMaxHealth) {
            if (unit.position.x - unit.size.x / 2.0 <= lootBox.position.x and lootBox.position.x <= unit.position.x + unit.size.x / 2.0
                and
                unit.position.y <= lootBox.position.y + lootBox.size.y / 2.0 and unit.position.y + unit.size.y >= lootBox.position.y + lootBox.size.y
                    ) {
                unit.health += Properties::getProperty().healthPackHealth;
                it = lootBoxes.erase(it);
                cout << "pick up loot" << endl;
                continue;
            }
        }

        ++it;
    }
}


void Arena::tick() {


    for (int i = 0; i < microticks; ++i) {

        bulletMicrotick();

        for (Unit & unit : units) {

            UnitAction action = unit.getAction();

            double horSpeed = sgn(action.velocity) * min(abs(action.velocity), properties.unitMaxHorizontalSpeed);

            unit.moveHor(horSpeed  * microTicksPerSecond);

            unit.horizontalWallCollision(horSpeed);

            if (unit.jumpState.canJump) {//Jumping
                if (action.jump) {
                    unit.jumping(unit.jumpState.speed * microTicksPerSecond, microTicksPerSecond);
                }
            }

            if (!unit.jumpState.canJump and unit.jumpState.maxTime <= .0) {//Down
                unit.downing(properties.unitFallSpeed * microTicksPerSecond);
            }

            if (unit.jumpState.canCancel /*and unit.onGround*/) {//Down
                if (!action.jump) {
                    unit.downing(properties.unitFallSpeed * microTicksPerSecond);
                    unit.applyJumpCancel();
                }
            } else if (unit.jumpState.maxTime > 0) {
                unit.jumping(unit.jumpState.speed * microTicksPerSecond, microTicksPerSecond);
            }

            if (unit.isHeatRoof()) {
                unit.applyJumpCancel();
                unit.position.y = unit.topTileY - unit.size.y;
                unit.downing(properties.unitFallSpeed * microTicksPerSecond);
            }

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
                unit.applyJumpPad(properties.jumpPadJumpSpeed, properties.jumpPadJumpTime);
            }

            bulletOverlapWithUnit(unit);
            pickUpLoots(unit);
        }

        bulletWallOverlap();
    }

    ++currentTick;
}
