#include "Unit.hpp"
#include "../arena/Simulation.hpp"
#include "../utils/Geometry.h"
#include "../Consts.h"
#include <iostream>
#include <math.h>
#include "Game.hpp"
#include "../utils/Segment.h"


template <typename T> int sgn(T val) {
    return (T(0) <= val) - (val < T(0));
}

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
    onGroundLadderTicks = 0;
}

Unit::Unit(const Unit &unit) :
playerId(unit.playerId),
id(unit.id),
health(unit.health),
position(unit.position),
prevPosition(unit.prevPosition),
size(unit.size),
jumpState(unit.jumpState),
walkedRight(unit.walkedRight),
stand(unit.stand),
level(unit.level),
properties(unit.properties),
onGround(unit.onGround),
onLadder(unit.onLadder),
mines(unit.mines),
onGroundLadderTicks(unit.onGroundLadderTicks),
lastTouchedJumpPad(unit.lastTouchedJumpPad),
lastTouchedJumpPadPart(unit.lastTouchedJumpPadPart),
leftTop(unit.leftTop),
rightDown(unit.rightDown),
widthHalf(unit.widthHalf),
posTileX(unit.posTileX),
posTileY(unit.posTileY),
leftPosTileX(unit.leftPosTileX),
rightPosTileX(unit.rightPosTileX),
topTileY(unit.topTileY),
meanTileY(unit.meanTileY),
minUpDeltaTileY(unit.minUpDeltaTileY)
{
    if (unit.weapon) {
        weapon = make_shared<Weapon>(
                Weapon(
                        unit.weapon.get()->type,
                        unit.weapon.get()->params,
                        unit.weapon.get()->magazine,
                        unit.weapon.get()->wasShooting,
                        unit.weapon.get()->spread,
                        unit.weapon.get()->fireTimer,
                        unit.weapon.get()->lastAngle,
                        unit.weapon.get()->lastAngle));
    }
}

void Unit::init(InputStream &stream, Properties *properties, Level *level) {
    this->properties = properties;
    this->level = level;
    health = stream.readInt();
    position = Vec2Double::readFrom(stream);
    size = Vec2Double::readFrom(stream);
    jumpState = JumpState::readFrom(stream);
    walkedRight = stream.readBool();
    stand = stream.readBool();
    onGround = stream.readBool();
    onLadder = stream.readBool();
    mines = stream.readInt();
    widthHalf = size.x / 2.0;
    onGroundLadderTicks = 0;
    updateTilePos();

    if (stream.readBool()) {
        weapon = std::shared_ptr<Weapon>(new Weapon());
        *weapon = Weapon::readFrom(stream);
    } else {
        weapon = std::shared_ptr<Weapon>();
    }
}

void Unit::update(InputStream &stream) {

    health = stream.readInt();
    position = Vec2Double::readFrom(stream);
    Vec2Double::readFrom(stream);
    jumpState = JumpState::readFrom(stream);
    walkedRight = stream.readBool();
    stand = stream.readBool();
    onGround = stream.readBool();
    onLadder = stream.readBool();
    mines = stream.readInt();
    onGroundLadderTicks = 0;
    updateTilePos();

    if (stream.readBool()) {
        weapon = std::shared_ptr<Weapon>(new Weapon());
        *weapon = Weapon::readFrom(stream);
    } else {
        weapon = std::shared_ptr<Weapon>();
    }
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
        stream.write(true);
        (*weapon).writeTo(stream);
    } else {
        stream.write(false);
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
    return unit.mines == mines and unit.onLadder == onLadder and unit.health == health and (unit.position - position).len() <= eps and unit.jumpState.equal(jumpState , eps);
}


void Unit::updateTilePos() {
    posTileX = (int)position.x;
    posTileY = (int)position.y;

    leftPosTileX = (int)(position.x - widthHalf);
    rightPosTileX = (int)(position.x + widthHalf);

    topTileY = (int)(position.y + size.y);
    meanTileY = (int)(position.y + size.y / 2.0);
    minUpDeltaTileY = (int)(position.y + properties->unitFallSpeed / ((double)Consts::microticks * properties->ticksPerSecond));

    leftTop.x = position.x - widthHalf;
    leftTop.y = position.y + size.y;

    rightDown.x = position.x + widthHalf;
    rightDown.y = position.y;
}

bool Unit::isOnLadder() {
    return level->tiles[posTileX][posTileY] == Tile::LADDER or level->tiles[posTileX][posTileY] == Tile::LADDER;
}

bool Unit::isOnGround() {
    return isOnLadder() or isOnPlatform() or level->tiles[leftPosTileX][minUpDeltaTileY] == Tile::WALL or level->tiles[rightPosTileX][minUpDeltaTileY] == Tile::WALL;
}

bool Unit::isInGround() {
    return isOnLadder() or isOnPlatform() or level->tiles[leftPosTileX][posTileY] == Tile::WALL or level->tiles[rightPosTileX][posTileY] == Tile::WALL;
}

bool Unit::isOnWall() {
    return level->tiles[leftPosTileX][posTileY] == Tile::WALL or level->tiles[rightPosTileX][posTileY] == Tile::WALL;
}

bool Unit::isOnPlatform() const {

    return (level->tiles[leftPosTileX][minUpDeltaTileY] != Tile::PLATFORM or level->tiles[rightPosTileX][minUpDeltaTileY] != Tile::PLATFORM)
            and (level->tiles[leftPosTileX][posTileY] == Tile::PLATFORM or level->tiles[rightPosTileX][posTileY] == Tile::PLATFORM) and
            (prevPosition.y >= posTileY + 1) or (jumpState.canJump and level->tiles[leftPosTileX][(int)(posTileY - Consts::eps)] == Tile::PLATFORM and level->tiles[rightPosTileX][(int)(posTileY - Consts::eps)] == Tile::PLATFORM);
}

bool Unit::isOnJumpPad() {

    if (level->tiles[leftPosTileX][posTileY] == Tile::JUMP_PAD) {
        lastTouchedJumpPad = level->tileIndex(leftPosTileX, posTileY);
        lastTouchedJumpPadPart = (position.y - posTileY) * Consts::ppFieldSize;
        return true;
    }

    if (level->tiles[rightPosTileX][posTileY] == Tile::JUMP_PAD) {
        lastTouchedJumpPad = level->tileIndex(rightPosTileX, posTileY);
        lastTouchedJumpPadPart = (position.y - posTileY) * Consts::ppFieldSize;
        return true;
    }

    if (level->tiles[leftPosTileX][meanTileY] == Tile::JUMP_PAD) {
        lastTouchedJumpPad = level->tileIndex(leftPosTileX, meanTileY);
        lastTouchedJumpPadPart = (position.y + size.y / 2.0 - meanTileY) * Consts::ppFieldSize;
        return true;
    }

    if (level->tiles[rightPosTileX][meanTileY] == Tile::JUMP_PAD) {
        lastTouchedJumpPad = level->tileIndex(rightPosTileX, meanTileY);
        lastTouchedJumpPadPart = (position.y + size.y / 2.0 - meanTileY) * Consts::ppFieldSize;
        return true;
    }

    if (level->tiles[leftPosTileX][topTileY] == Tile::JUMP_PAD) {
        lastTouchedJumpPad = level->tileIndex(leftPosTileX, topTileY);
        lastTouchedJumpPadPart = (position.y + size.y - topTileY) * Consts::ppFieldSize;
        return true;
    }

    if (level->tiles[rightPosTileX][topTileY] == Tile::JUMP_PAD) {
        lastTouchedJumpPad = level->tileIndex(rightPosTileX, topTileY);
        lastTouchedJumpPadPart = (position.y + size.y - topTileY) * Consts::ppFieldSize;
        return true;
    }

    return false;
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
        double distance = abs((double)tileX - direction - prevPosition.x - direction * widthHalf);

        if (distance < Consts::eps) {
            position.x = prevPosition.x;
        } else {
            position.x = (double)tileX - direction * ((direction < 0 ? 1.0 : 0)  +  size.x / 2.0) + -direction * (abs(distance) > Consts::eps ? Consts::eps : .0);
        }
    }

    updateTilePos();
    updateMoveState();
}

void Unit::heatRoofRoutine() {
    if (level->tiles[leftPosTileX][topTileY] == Tile::WALL or level->tiles[rightPosTileX][topTileY] == Tile::WALL) {
        applyJumpCancel();

        double distance = (double)topTileY - size.y - prevPosition.y;

        if (distance > Consts::eps) {
            position.y = topTileY - (size.y + Consts::eps);
        } else {
            position.y = prevPosition.y;
        }

        updateTilePos();
    }
}

void Unit::verticalWallCollision() {

    applyOnGround();

    double distance = prevPosition.y - (double)posTileY - 1.0;

    if (distance > Consts::eps) {
        position.y = (double)posTileY + 1.0 + ((distance > Consts::eps) ? Consts::eps : .0);
    } else {
        position.y = prevPosition.y;
    }

    updateTilePos();
    updateMoveState();
}

void Unit::platformCollision() {
    double distance = prevPosition.y - (double)posTileY - 1.0;

    if (distance > Consts::eps) {
        position.y = (double)posTileY + 1.0 + ((distance > Consts::eps) ? Consts::eps : .0);
    } else {
        position.y = prevPosition.y;
    }

    applyOnGround();
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
                        properties->weaponParams[w.get()->weaponType].minSpread,
                        properties->weaponParams[w.get()->weaponType].reloadTime,
                        Consts::noLastAngleValue,
                        -1));
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

bool Unit::isPickUpLootbox(const LootBox &lootBox) const {
    return Geometry::isRectOverlap(leftTop, rightDown, lootBox.leftTop, lootBox.rightDown);
}


void Unit::applyActionMicroticks(const UnitAction &action, vector<Unit> &units, int microticks) {
    double microTicksPerSecond =  properties->updatesPerSecond * microticks;

    weaponRoutine(microTicksPerSecond, action.aim);

    prevPosition = position;

    moveHor(action.velocity * microTicksPerSecond);
    horizontalWallCollision(action.velocity);

    if (isOnJumpPad()) {
        applyJumpPad(properties->jumpPadJumpSpeed, properties->jumpPadJumpTime);
    }

    for (Unit & nearUnit : units) {
        if (nearUnit.id != id) {
            unitHorCollide(nearUnit);
        }
    }

    if (jumpState.canJump and jumpState.canCancel) {//Jumping
        if (action.jump) {
            jumping(jumpState.speed * microTicksPerSecond, microTicksPerSecond);
        }
    }

    if (!jumpState.canJump and jumpState.maxTime <= .0) {//Down
        downing(properties->unitFallSpeed * microTicksPerSecond);
    }

    if (jumpState.canCancel) {//Down
        if (!action.jump) {
            downing(properties->unitFallSpeed * microTicksPerSecond);
            applyJumpCancel();
        }
    } else if (jumpState.maxTime > 0) {
        jumping(jumpState.speed * microTicksPerSecond, microTicksPerSecond);
    }

    heatRoofRoutine();

    if (jumpState.maxTime <= .0) {
        applyJumpCancel();
    }

    if (isOnJumpPad()) {
        applyJumpPad(properties->jumpPadJumpSpeed, properties->jumpPadJumpTime);
    }

    if (isOnWall()) {
        verticalWallCollision();
    } else if (isOnPlatform() and !jumpState.canJump and !action.jumpDown) {
        platformCollision();
    }

    if (onLadder) {
        applyOnGround();
    }

    for (Unit & nearUnit : units) {
        if (nearUnit.id != id) {
            unitVerCollide(nearUnit);
        }
    }
}

int Unit::collisionWithWallMicrotick(const UnitAction &action) {
    double moveLeftX = position.x + sgn(action.velocity) * properties->unitMaxHorTickDistance - widthHalf;
    double moveRightX = position.x + sgn(action.velocity) * properties->unitMaxHorTickDistance + widthHalf;

    double prevLeftX = position.x - widthHalf;
    double prevRightX = position.x + widthHalf;

    if ((jumpState.canJump and action.jump) or (jumpState.canJump and !jumpState.canCancel)) {//heat with roof
        double move_y = position.y + size.y + jumpState.speed / properties->ticksPerSecond;
        double prev_y = position.y + size.y;

        if ((int)move_y != (int)prev_y) {
            if (
                    level->tiles[(int)prevLeftX][(int)move_y] == Tile::WALL
                    or
                    level->tiles[(int)moveLeftX][(int)move_y] == Tile::WALL
                    or
                    level->tiles[(int)prevRightX][(int)move_y] == Tile::WALL
                    or
                    level->tiles[(int)moveRightX][(int)move_y] == Tile::WALL
                    or
                    level->tiles[(int)prevLeftX][(int)move_y] == Tile::JUMP_PAD
                    or
                    level->tiles[(int)moveLeftX][(int)move_y] == Tile::JUMP_PAD
                    or
                    level->tiles[(int)prevRightX][(int)move_y] == Tile::JUMP_PAD
                    or
                    level->tiles[(int)moveRightX][(int)move_y] == Tile::JUMP_PAD
                    ) {
                return (int)properties->ticksPerSecond * properties->updatesPerTick * ((int)move_y - (position.y + size.y)) / jumpState.speed;
            }
        }
    } else if (!jumpState.canJump and !jumpState.canCancel or jumpState.canCancel and !action.jump) {//Down
        double move_y = position.y - properties->unitFallSpeed / properties->ticksPerSecond;
        double prev_y = position.y;

        if ((int)move_y != (int)prev_y) {
            if (
                    level->tiles[(int)prevLeftX][(int)move_y] == Tile::WALL
                    or
                    level->tiles[(int)moveLeftX][(int)move_y] == Tile::WALL
                    or
                    level->tiles[(int)prevRightX][(int)move_y] == Tile::WALL
                    or
                    level->tiles[(int)moveRightX][(int)move_y] == Tile::WALL
                    or
                    level->tiles[(int)prevLeftX][(int)move_y] == Tile::JUMP_PAD
                    or
                    level->tiles[(int)moveLeftX][(int)move_y] == Tile::JUMP_PAD
                    or
                    level->tiles[(int)prevRightX][(int)move_y] == Tile::JUMP_PAD
                    or
                    level->tiles[(int)moveRightX][(int)move_y] == Tile::JUMP_PAD
                    or
                    level->tiles[(int)prevLeftX][(int)move_y] == Tile::PLATFORM
                    or
                    level->tiles[(int)moveLeftX][(int)move_y] == Tile::PLATFORM
                    or
                    level->tiles[(int)prevRightX][(int)move_y] == Tile::PLATFORM
                    or
                    level->tiles[(int)moveRightX][(int)move_y] == Tile::PLATFORM
                    ) {
                return (int)properties->ticksPerSecond * properties->updatesPerTick * (position.y - ((int)move_y + 1.0)) / properties->unitFallSpeed;
            }
        }
    }

    return properties->updatesPerTick;
}

void Unit::applyAction(const UnitAction &action, vector<Unit> & units) {

    int microtick = collisionWithWallMicrotick(action) + 1;

    if (microtick > 0 and microtick < properties->updatesPerTick) {
        applyActionMicroticks(action, units, microtick);
        applyActionMicroticks(action, units, properties->updatesPerTick - microtick);
    } else {
        applyActionMicroticks(action, units, properties->updatesPerTick);
    }
}

void Unit::weaponRoutine(double time, const Vec2Double & aim) {
    if (weapon) {
        Weapon * weapon = this->weapon.get();

        double angle = atan2(aim.y, aim.x);

        weapon->fireTimer -= time;

        if (weapon->fireTimer < .0) {
            weapon->fireTimer = .0;
        }


        weapon->spread += (weapon->lastAngle != Consts::noLastAngleValue ? abs(angle - weapon->lastAngle) : 0);
        if (weapon->spread > weapon->params.maxSpread) {
            weapon->spread = weapon->params.maxSpread;
        }

        weapon->spread -= (weapon->params.aimSpeed * time);

        if (weapon->spread < weapon->params.minSpread) {
            weapon->spread = weapon->params.minSpread;
        }

        weapon->lastAngle = atan2(aim.y, aim.x);
    }
}

void Unit::plantMine(vector<Mine> &mines) {
    if (onGround and this->mines >= 1
        and
        (
            (level->tiles[posTileX][posTileY - 1] == Tile::WALL or level->tiles[posTileX][posTileY - 1] == Tile::PLATFORM)
        )
        and
        (
            (level->tiles[leftPosTileX][posTileY - 1] == Tile::WALL or level->tiles[leftPosTileX][posTileY - 1] == Tile::PLATFORM)
            or
            (level->tiles[rightPosTileX][posTileY - 1] == Tile::WALL or level->tiles[rightPosTileX][posTileY - 1] == Tile::PLATFORM)
        )
     ) {

        Mine mine = Mine(
                playerId,
                position,
                properties->mineSize,
                MineState::PREPARING,
                properties->minePrepareTime,
                properties->mineTriggerRadius,
                properties->mineExplosionParams
                );

        mines.push_back(mine);

        --this->mines;
    }
}

void Unit:: unitHorCollide(Unit & unit) {

    if (Geometry::isRectOverlap(unit.leftTop, unit.rightDown, leftTop, rightDown)) {
        if (unit.position.x < position.x) {
            position.x = unit.position.x + 2 * widthHalf;

            int rightTileX = (int)(position.x + widthHalf);

            Tile upTile = level->tiles[rightTileX][topTileY];
            Tile meanTile = level->tiles[rightTileX][meanTileY];
            Tile downTile = level->tiles[rightTileX][posTileY];

            if (upTile == Tile::WALL or meanTile == Tile::WALL or downTile == Tile::WALL) {
                position.x = rightTileX - widthHalf;
                unit.position.x = position.x - 2 * widthHalf;

                updateTilePos();
                updateMoveState();

                unit.updateTilePos();
                unit.updateMoveState();
             }

        } else {
            position.x = unit.position.x - 2 * widthHalf;

            int leftTileX = (int)(position.x - widthHalf);

            Tile upTile = level->tiles[leftTileX][topTileY];
            Tile meanTile = level->tiles[leftTileX][meanTileY];
            Tile downTile = level->tiles[leftTileX][posTileY];

            if (upTile == Tile::WALL or meanTile == Tile::WALL or downTile == Tile::WALL) {
                position.x = leftTileX + 1.0 + widthHalf;
                unit.position.x = position.x + 2 * widthHalf;

                updateTilePos();
                updateMoveState();
                unit.updateTilePos();
                unit.updateMoveState();
            }
        }
    }
}

void Unit::unitVerCollide(Unit &unit) {
    if (Geometry::isRectOverlap(unit.leftTop, unit.rightDown, leftTop, rightDown)) {
        if (position.y > unit.position.y) {
            position.y = unit.position.y + unit.size.y;
            applyOnGround();
            unit.applyJumpCancel();
        } else {
            position.y = unit.position.y - unit.size.y;
            applyJumpCancel();
            unit.applyOnGround();
        }
    }
}

int Unit::crossBulletTick(Bullet &bullet, int microticks, const Vec2Double & unitVelocity, const Vec2Double & position) {

    vector<Vec2Double> bullets = {
            Vec2Double(bullet.halfSize, -bullet.halfSize) + bullet.position,
            Vec2Double(-bullet.halfSize, bullet.halfSize) + bullet.position,
            Vec2Double(bullet.halfSize, bullet.halfSize) + bullet.position,
            Vec2Double(-bullet.halfSize, -bullet.halfSize) + bullet.position
    };

    return min(
            crossWithFrontPoint(Vec2Double(position.x + widthHalf, position.y + size.y), bullets, bullet, unitVelocity, make_pair(-size.x, -size.y) , microticks),
            crossWithFrontPoint(Vec2Double(position.x - widthHalf, position.y), bullets, bullet, unitVelocity, make_pair(size.x, size.y) , microticks)
            );
}

int Unit::crossWithFrontPoint(
        const Vec2Double &frontUnitPoint,
        const vector<Vec2Double> & bullets,
        const Bullet & bullet,
        const Vec2Double & unitVelocity,
        const pair<double, double> & unitSegments,
        int microticks
        ) {
    int heatWithBulletMicrotick = microticks + 1;

    for (Vec2Double bPoint : bullets) {
        bPoint -= frontUnitPoint;

        if ((bullet.velocity.x - unitVelocity.x) != 0) {//@TODO optimzied it!!!
            int microticksToHeat = (int)((bPoint.x / (unitVelocity.x - bullet.velocity.x)) * properties->ticksPerSecond * properties->updatesPerTick);

            if (microticksToHeat >= 0 and microticksToHeat <= microticks)  {
                double timeToHeat = microticksToHeat / (properties->ticksPerSecond * properties->updatesPerTick);

                double unitY = unitVelocity.y * timeToHeat;
                double segmentStart = unitY + (unitSegments.second > 0 ? 0 : unitSegments.second);
                double segmentEnd = unitY + (unitSegments.second > 0 ? unitSegments.second : 0);
                double bulletY = bullet.velocity.y * timeToHeat + bPoint.y;

                if (segmentStart <= bulletY and bulletY <= segmentEnd) {//heat with bullet
                    heatWithBulletMicrotick = min(heatWithBulletMicrotick, microticksToHeat);
                }
            }
        }

        if ((bullet.velocity.y - unitVelocity.y) != 0) {//@TODO optimzied it!!!
            int microticksToHeat = (int)((bPoint.y / (unitVelocity.y - bullet.velocity.y)) * properties->ticksPerSecond * properties->updatesPerTick);

            if (microticksToHeat >= 0 and microticksToHeat <= microticks)  {
                double timeToHeat = microticksToHeat / (properties->ticksPerSecond * properties->updatesPerTick);

                double unitX = unitVelocity.x * timeToHeat;
                double segmentStart = unitX + (unitSegments.first > 0 ? 0 : unitSegments.first);
                double segmentEnd = unitX + (unitSegments.first > 0 ? unitSegments.first : 0);
                double bulletX = bullet.velocity.x * timeToHeat + bPoint.x;

                if (segmentStart <= bulletX and bulletX <= segmentEnd) {//heat with bullet
                    heatWithBulletMicrotick = min(heatWithBulletMicrotick, microticksToHeat);
                }
            }
        }
    }
    return heatWithBulletMicrotick;
}

pair<double, double> Unit::setFrontSegments(const Vec2Double & dir, Vec2Double & frontPoint, const Vec2Double & position) const {
    if (dir.x >= 0 and dir.y >= 0) {
        frontPoint.x = position.x + widthHalf;
        frontPoint.y = position.y + size.y;
        return make_pair(-size.x, -size.y);
    } else if (dir.x >= 0 and dir.y < 0) {
        frontPoint.x = position.x - widthHalf;
        frontPoint.y = position.y + size.y;
        return make_pair(size.x, -size.y);
    } else if (dir.x < 0 and dir.y < 0) {
        frontPoint.x = position.x + widthHalf;
        frontPoint.y = position.y + size.y;
        return make_pair(-size.x, -size.y);
    } else {
        frontPoint.x = position.x + widthHalf;
        frontPoint.y = position.y;
        return make_pair(-size.x, size.y);
    }
}

bool Unit::checkAim(const Unit & enemy, Vec2Double & aim, const list<int> & allies, const vector<Unit> & units) const {

    Vec2Float unitCenter(position.x, position.y + size.y / 2.0);
    Vec2Double targetPoint = position + Vec2Double(0, size.y / 2.0) + aim;

    if (level->crossWall(unitCenter, targetPoint.toFloat())) {
        return false;
    }

    Vec2Double lowLineFromBulletCenter = aim.rotate(weapon.get()->spread);
    Vec2Double topLineFromBulletCenter = aim.rotate(-weapon.get()->spread);

    Vec2Double bulletLowLineDelta = lowLineFromBulletCenter.getOpponentAngle(weapon.get()->params.bullet.size / 2.0, false);
    Vec2Double bulletTopLineDelta = topLineFromBulletCenter.getOpponentAngle(weapon.get()->params.bullet.size / 2.0, true);

    Vec2Double lowLine = bulletLowLineDelta + lowLineFromBulletCenter;
    Vec2Double topLine = bulletTopLineDelta + topLineFromBulletCenter;

    auto lowLineCross = level->crossMiDistanceWall(unitCenter + bulletLowLineDelta, unitCenter + lowLine);
    auto topLineCross = level->crossMiDistanceWall(unitCenter + bulletTopLineDelta, unitCenter + topLine);


    if (
        weapon.get()->type == WeaponType ::ROCKET_LAUNCHER
        and
        !((lowLineCross ? (lowLineCross.value() - targetPoint).sqrLen() < (lowLineCross.value() - unitCenter).sqrLen() : true)
            and
        (topLineCross ? (topLineCross.value() - targetPoint).sqrLen() < (topLineCross.value() - unitCenter).sqrLen() : true))

        ) {
        return false;
    }

    for (int allyUnitId : allies) {

        const Unit & allyUnit = units[Game::unitIndexById(allyUnitId)];

        if (allyUnit.id != id) {

            vector<vector<Vec2Double>> allyBorders = {
                    {
                            Vec2Double(allyUnit.position.x - allyUnit.widthHalf, allyUnit.position.y), Vec2Double(allyUnit.position.x + allyUnit.widthHalf, allyUnit.position.y)
                    },
                    {
                            Vec2Double(allyUnit.position.x - allyUnit.widthHalf, allyUnit.position.y), Vec2Double(allyUnit.position.x - allyUnit.widthHalf, allyUnit.position.y + allyUnit.size.y)
                    },
                    {
                            Vec2Double(allyUnit.position.x + allyUnit.widthHalf, allyUnit.position.y), Vec2Double(allyUnit.position.x + allyUnit.widthHalf, allyUnit.position.y + allyUnit.size.y)
                    },
                    {
                            Vec2Double(allyUnit.position.x - allyUnit.widthHalf, allyUnit.position.y + allyUnit.size.y), Vec2Double(allyUnit.position.x + allyUnit.widthHalf, allyUnit.position.y + allyUnit.size.y)
                    }
            };


            for (const vector<Vec2Double> & border : allyBorders) {
                if (
                        Geometry::doIntersect(border[0], border[1], unitCenter, targetPoint)
                        or
                        Geometry::doIntersect(border[0], border[1], (unitCenter + bulletLowLineDelta),  (unitCenter + lowLine))
                        or
                        Geometry::doIntersect(border[0], border[1], (unitCenter + bulletTopLineDelta),  (unitCenter + topLine))
                        ) {
                    return false;
                }
            }
        }
    }

    return true;
}

int Unit::actionType() const {

    if (isOnPlatform()) {
        return 3;
    }

    if (onGround) {
        return 1;
    }

    if (jumpState.maxTime > 0) {
        return 0;
    }

    return 2;
}


bool Unit::shootable(const Unit &unit) const {
    Vec2Float targetPoint(unit.position.x, unit.position.y + unit.size.y / 2.0);
    Vec2Float unitCenter(position.x, position.y + size.y / 2.0);


    return level->crossWall(unitCenter, targetPoint) == nullopt;
}

pair<int, Vec2Double> Unit::chooseEnemy(const list<int> &enemyIds, const vector<Unit> &enemies, int choosenEnemyId) const {

    int minEnemyValueId = enemyIds.front();
    double minEnemyValue = INT32_MAX;

    Vec2Double enemyAim;

    for (int enemyId : enemyIds) {
        const Unit & enemy = enemies[Game::unitIndexById(enemyId)];

        Vec2Double aim = (Vec2Double(enemy.position.x, enemy.position.y + enemy.size.y / 2.0)  - Vec2Double(position.x, position.y + size.y / 2.0));

        double enemyVal = enemyValue(enemy, enemyId == choosenEnemyId, aim);

        if (minEnemyValue > enemyVal) {
            minEnemyValue = enemyVal;
            minEnemyValueId = enemyId;
            enemyAim = aim;
        }
    }

    return make_pair(minEnemyValueId, enemyAim);
}

double Unit::enemyValue(const Unit &enemy, bool alreadyChoosed, const Vec2Double & aim) const {

    return (1.0  - 1.0 / aim.sqrLen()) + (double)alreadyChoosed + enemy.health / (double)health + weapon.get()->lastAngle != Consts::noLastAngleValue * abs(aim.angle(X_AXIS) - weapon.get()->lastAngle);
}

int Unit::stateIndex() {
    if (jumpState.canJump and jumpState.canCancel) {//onGround or jumping
        return 0;
    }

    if (!jumpState.canJump and !jumpState.canCancel) {//Fall
        return 1;
    }

    if (isOnJumpPad()) {//Is ON Jump PAD
        return 2;
    }

    if (jumpState.canJump and !jumpState.canCancel) {//Jump pad
        return level->jumpPads[lastTouchedJumpPad] * Consts::ppFieldSize + lastTouchedJumpPadPart + Consts::predefinitionStates;
    }
}