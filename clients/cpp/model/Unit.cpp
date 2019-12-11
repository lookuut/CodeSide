#include "Unit.hpp"
#include "../arena/Arena.hpp"
#include "../utils/Geometry.h"
#include "../Consts.h"
#include <iostream>
#include <math.h>
#include "Game.hpp"

Unit::Unit() { }
Unit::Unit(
        int playerId,
        int id,
        int health,
        Vec2Double position,
        Vec2Double size,
        JumpState jumpState,
        bool walkedRight,
        bool stand,
        bool onGround,
        bool onLadder,
        int mines,
        std::shared_ptr<Weapon> weapon
        ) : playerId(playerId), id(id), health(health), position(position), size(size), jumpState(jumpState), walkedRight(walkedRight), stand(stand), onGround(onGround), onLadder(onLadder), mines(mines), weapon(weapon) {

    leftTop.x = position.x - size.x / 2.0;
    leftTop.y = position.y + size.y;

    rightDown.x = position.x + size.x / 2.0;
    rightDown.y = position.y;
    widthHalf = size.x / 2.0;
}

Unit Unit::readFrom(InputStream& stream, Properties * properties, Level *level) {
    Unit result;
    result.playerId = stream.readInt();
    result.properties = properties;
    result.level = level;
    result.id = stream.readInt();
    result.health = stream.readInt();
    result.position = Vec2Double::readFrom(stream);
    result.size = Vec2Double::readFrom(stream);
    result.jumpState = JumpState::readFrom(stream);
    result.walkedRight = stream.readBool();
    result.stand = stream.readBool();
    result.onGround = stream.readBool();
    result.onLadder = stream.readBool();
    result.mines = stream.readInt();
    result.widthHalf = result.size.x / 2.0;

    result.updateTilePos();

    if (stream.readBool()) {
        result.weapon = std::shared_ptr<Weapon>(new Weapon());
        *result.weapon = Weapon::readFrom(stream);
    } else {
        result.weapon = std::shared_ptr<Weapon>();
    }
    return result;
}
void Unit::writeTo(OutputStream& stream) const {
    stream.write(playerId);
    stream.write(id);
    stream.write(health);
    position.writeTo(stream);
    size.writeTo(stream);
    jumpState.writeTo(stream);
    stream.write(walkedRight);
    stream.write(stand);
    stream.write(onGround);
    stream.write(onLadder);
    stream.write(mines);
    if (weapon) {
        stream.write(false);
    } else {
        stream.write(true);
        (*weapon).writeTo(stream);
    }
}
std::string Unit::toString() const {
    return std::string("Unit") + "(" +
        std::to_string(playerId) +
        std::to_string(id) +
        std::to_string(health) +
        position.toString() +
        size.toString() +
        jumpState.toString() +
        (walkedRight ? "true" : "false") + 
        (stand ? "true" : "false") + 
        (onGround ? "true" : "false") + 
        (onLadder ? "true" : "false") + 
        std::to_string(mines) +
        "TODO" + 
        ")";
}

void Unit::moveHor(double velocity) {
    position.x += velocity;
    updateTilePos();
    updateMoveState();
}


void Unit::jumping(double velocity, double time) {
    position.y += velocity;
    jumpState.maxTime -= time;

    updateTilePos();
    updateMoveState();
    onGround = false;
}

void Unit::downing(double velocity) {
    position.y -= velocity;
    onGround = false;

    updateTilePos();
    updateMoveState();
}

void Unit::updateMoveState() {
    //Ladder state
    onLadder = isOnLadder();
}

bool Unit::equal(const Unit &unit, double eps) const {
    return unit.onLadder == onLadder and unit.health == health and unit.onGround == onGround and (unit.position - position).len() <= eps and unit.jumpState.equal(jumpState , eps);
}


void Unit::updateTilePos() {
    posTileX = (int)position.x;
    posTileY = (int)position.y;

    leftPosTileX = (int)(position.x - widthHalf);
    rightPosTileX = (int)(position.x + widthHalf);

    topTileY = (int)(position.y + size.y);
    meanTileY = (int)(position.y + widthHalf);
    minDownDeltaTileY = (int)(position.y + properties->unitFallSpeed / (Consts::microticks * properties->ticksPerSecond));

    leftTop.x = position.x - widthHalf;
    leftTop.y = position.y + size.y;

    rightDown.x = position.x + widthHalf;
    rightDown.y = position.y;
}

bool Unit::isOnLadder() {
    return level->tiles[posTileX][posTileY] == Tile::LADDER or level->tiles[posTileX][posTileY] == Tile::LADDER;
}

bool Unit::isOnGround() {
    return isOnLadder() or isOnPlatform() or level->tiles[leftPosTileX][minDownDeltaTileY] == Tile::WALL or level->tiles[rightPosTileX][minDownDeltaTileY] == Tile::WALL;
}

bool Unit::isInGround() {
    return isOnLadder() or isOnPlatform() or level->tiles[leftPosTileX][posTileY] == Tile::WALL or level->tiles[rightPosTileX][posTileY] == Tile::WALL;
}

bool Unit::isOnWall() {
    return level->tiles[leftPosTileX][posTileY] == Tile::WALL or level->tiles[rightPosTileX][posTileY] == Tile::WALL;
}

bool Unit::isOnPlatform() {

    return (level->tiles[leftPosTileX][minDownDeltaTileY] != Tile::PLATFORM or level->tiles[rightPosTileX][minDownDeltaTileY] != Tile::PLATFORM)
            and (level->tiles[leftPosTileX][posTileY] == Tile::PLATFORM or level->tiles[rightPosTileX][posTileY] == Tile::PLATFORM);
}

bool Unit::isOnJumpPad() {
    return level->tiles[leftPosTileX][posTileY] == Tile::JUMP_PAD or level->tiles[rightPosTileX][posTileY] == Tile::JUMP_PAD;
}

void Unit::horizontalWallCollision(double velocity) {
    if (velocity == 0.0) {
        return;
    }

    int tileX = velocity < 0 ? leftPosTileX : rightPosTileX;

    Tile upTile = level->tiles[tileX][topTileY];
    Tile meanTile = level->tiles[tileX][meanTileY];
    Tile downTile = level->tiles[tileX][posTileY];

    int direction = (velocity < 0 ? -1 : 1);
    if (upTile == Tile::WALL or meanTile == Tile::WALL or downTile == Tile::WALL) {
        position.x = (double)tileX - direction * ((direction < 0 ? 1.0 : 0)  +  size.x / 2.0) + -direction * (abs(position.x - prevPosition.x) > Consts::eps ? Consts::eps : .0);
    }

    updateTilePos();
    updateMoveState();
}

void Unit::heatRoofRoutine() {
    if (level->tiles[leftPosTileX][topTileY] == Tile::WALL or level->tiles[rightPosTileX][topTileY] == Tile::WALL) {
        applyJumpCancel();

        double distance = (double)topTileY - size.y - prevPosition.y;
        position.y = (double)topTileY - size.y - ((distance > Consts::eps) ? Consts::eps : .0);
        updateTilePos();
    }
}

void Unit::verticalWallCollision() {

    applyOnGround();

    double distance = prevPosition.y - (double)posTileY - 1.0;

    position.y = (double)posTileY + 1.0 + ((distance > Consts::eps) ? Consts::eps : .0);

    updateTilePos();
    updateMoveState();
}

void Unit::applyJumpPad(double speed, double maxTime) {
    jumpState.canJump = true;
    jumpState.canCancel = false;
    jumpState.speed = speed;
    jumpState.maxTime = maxTime;
    onGround = false;
}


void Unit::applyOnGround() {
    jumpState.canJump = true;
    jumpState.canCancel = true;
    jumpState.speed = properties->unitJumpSpeed;
    jumpState.maxTime = properties->unitJumpTime;

    onGround = true;
    onLadder = isOnLadder();
}


void Unit::applyJump(double speed, double maxTime) {
    jumpState.speed = speed;
    jumpState.maxTime = maxTime;
    onGround = false;
}


void Unit::applyJumpCancel() {
    jumpState.canJump = false;
    jumpState.canCancel = false;
    jumpState.maxTime = .0;
    jumpState.speed = .0;
}

bool Unit::isInside(const Vec2Double &point) const {
    return leftTop.x <= point.x and point.x <= rightDown.x and rightDown.y <= point.y and point.y <= leftTop.y;
}

bool Unit::isOverlap(const Bullet &bullet) const {
    return Geometry::isRectOverlap(leftTop, rightDown, bullet.leftTop, bullet.rightDown);
}

bool Unit::picUpkHealthPack(const LootBox &lootbox) {
    if (health >= properties->unitMaxHealth) {
        return false;
    }

    if(isPickUpLootbox(lootbox)) {
        health = min(health + std::dynamic_pointer_cast<Item::HealthPack>(lootbox.item).get()->health, properties->unitMaxHealth);
        return true;
    }

    return false;
}

bool Unit::pickUpWeapon(const LootBox &lootBox) {
    if (weapon != nullptr) {
        return false;
    }

    if(isPickUpLootbox(lootBox)) {
        shared_ptr<Item::Weapon> w = std::dynamic_pointer_cast<Item::Weapon>(lootBox.item);

        weapon = make_shared<Weapon>(
                Weapon(
                        w.get()->weaponType,
                        properties->weaponParams[w.get()->weaponType],
                        properties->weaponParams[w.get()->weaponType].magazineSize,
                        false,
                        properties->weaponParams[w.get()->weaponType].maxSpread,
                        0,
                        0,
                        0));
        return true;
    }

    return false;
}

bool Unit::picUpkMine(const LootBox &lootbox) {

    if(isPickUpLootbox(lootbox)) {
        this->mines++;
        return true;
    }

    return false;
}

bool Unit::isPickUpLootbox(const LootBox &lootBox) {
    return Geometry::isRectOverlap(leftTop, rightDown, lootBox.leftTop, lootBox.rightDown);
}