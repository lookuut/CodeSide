#include "Unit.hpp"
#include "../arena/Arena.hpp"
#include "../utils/Geometry.h"
#include "../Consts.h"
#include <iostream>

Unit::Unit() { }
Unit::Unit(int playerId, int id, int health, Vec2Double position, Vec2Double size, JumpState jumpState, bool walkedRight, bool stand, bool onGround, bool onLadder, int mines, std::shared_ptr<Weapon> weapon) : playerId(playerId), id(id), health(health), position(position), size(size), jumpState(jumpState), walkedRight(walkedRight), stand(stand), onGround(onGround), onLadder(onLadder), mines(mines), weapon(weapon) { }

Unit Unit::readFrom(InputStream& stream, const Level &level) {
    Unit result;
    result.playerId = stream.readInt();
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
    result.level = level;

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

void Unit::laddering(double vel) {
    position.y += vel;

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

    centerPosTileY = (int)(position.y + size.y / 2.0);

    leftPosTileX = (int)(position.x - size.x / 2.0);
    rightPosTileX = (int)(position.x + size.x / 2.0);

    topTileY = (int)(position.y + size.y);
    meanTileY = (int)(position.y + size.y / 2.0);
    minDownDeltaTileY = (int)(position.y + Properties::getProperty().unitFallSpeed / (Arena::microticks * Properties::getProperty().ticksPerSecond));
}

bool Unit::isOnLadder() {
    return level.tiles[posTileX][posTileY] == Tile::LADDER or level.tiles[posTileX][posTileY] == Tile::LADDER;
}

bool Unit::isOnGround() {
    return isOnLadder() or isOnPlatform() or level.tiles[leftPosTileX][minDownDeltaTileY] == Tile::WALL or level.tiles[rightPosTileX][minDownDeltaTileY] == Tile::WALL;
}

bool Unit::isInGround() {
    return isOnLadder() or isOnPlatform() or level.tiles[leftPosTileX][posTileY] == Tile::WALL or level.tiles[rightPosTileX][posTileY] == Tile::WALL;
}

bool Unit::isOnWall() {
    return level.tiles[leftPosTileX][posTileY] == Tile::WALL or level.tiles[rightPosTileX][posTileY] == Tile::WALL;
}

bool Unit::isOnPlatform() {

    return (level.tiles[leftPosTileX][minDownDeltaTileY] != Tile::PLATFORM or level.tiles[rightPosTileX][minDownDeltaTileY] != Tile::PLATFORM)
            and (level.tiles[leftPosTileX][posTileY] == Tile::PLATFORM or level.tiles[rightPosTileX][posTileY] == Tile::PLATFORM);
}

bool Unit::isOnJumpPad() {
    return level.tiles[leftPosTileX][posTileY] == Tile::JUMP_PAD or level.tiles[rightPosTileX][posTileY] == Tile::JUMP_PAD;
}

void Unit::horizontalWallCollision(double velocity) {
    if (velocity == 0.0) {
        return;
    }

    int tileX = velocity < 0 ? leftPosTileX : rightPosTileX;

    Tile upTile = level.tiles[tileX][topTileY];
    Tile meanTile = level.tiles[tileX][meanTileY];
    Tile downTile = level.tiles[tileX][posTileY];

    if (upTile == Tile::WALL or meanTile == Tile::WALL or downTile == Tile::WALL) {
        position.x = tileX - (velocity < 0 ? -1 : 1) * (1  +  size.x / 2.0);
    }

    updateTilePos();
    updateMoveState();
}

bool Unit::isHeatRoof() {
    return level.tiles[leftPosTileX][topTileY] == Tile::WALL or level.tiles[rightPosTileX][topTileY] == Tile::WALL;
}

void Unit::verticalWallCollision() {

    applyOnGround();
    position.y = posTileY + 1.0;

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
    jumpState.speed = Properties::getProperty().unitJumpSpeed;
    jumpState.maxTime = Properties::getProperty().unitJumpTime;

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
    double leftX = position.x - size.x / 2.0;
    double rightX = position.x + size.x / 2.0;

    double bottomY = position.y;
    double topY = position.y + size.y;

    return leftX <= point.x and point.x <= rightX and bottomY <= point.y and point.y <= topY;
}

bool Unit::isOverlap(const Bullet &bullet) const {
    double leftX = position.x - size.x / 2.0;
    double rightX = position.x + size.x / 2.0;

    double bottomY = position.y;
    double topY = position.y + size.y;


    Vec2Double bl = Vec2Double(bullet.position.x - bullet.size / 2.0, bullet.position.y + bullet.size / 2.0);
    Vec2Double br = Vec2Double(bullet.position.x + bullet.size / 2.0, bullet.position.y - bullet.size / 2.0);

    return Geometry::isRectOverlap(Vec2Double(leftX, topY), Vec2Double(rightX, bottomY), bl, br);
}

UnitAction& Unit::getAction() {
    return action;
}

void Unit::setAction(UnitAction & action) {
    this->action = action;
}