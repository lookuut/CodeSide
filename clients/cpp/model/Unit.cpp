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
onGroundLadderTicks(unit.onGroundLadderTicks)
{
    leftTop.x = position.x - size.x / 2.0;
    leftTop.y = position.y + size.y;

    rightDown.x = position.x + size.x / 2.0;
    rightDown.y = position.y;
    widthHalf = size.x / 2.0;

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
    updateTilePos();
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
    isAlive = true;
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

bool Unit::isOnPlatform() {

    return (level->tiles[leftPosTileX][minUpDeltaTileY] != Tile::PLATFORM or level->tiles[rightPosTileX][minUpDeltaTileY] != Tile::PLATFORM)
            and (level->tiles[leftPosTileX][posTileY] == Tile::PLATFORM or level->tiles[rightPosTileX][posTileY] == Tile::PLATFORM) and
            (prevPosition.y >= posTileY + 1);
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
    onPlatform = true;
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

bool Unit::isPickUpLootbox(const LootBox &lootBox) {
    return Geometry::isRectOverlap(leftTop, rightDown, lootBox.leftTop, lootBox.rightDown);
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


int Unit::horFirstEvent(double velocity, int microTicksLimit, const Vec2Double & position) const {

    int rightPosTileX = (int)(position.x + widthHalf);
    int leftPosTileX = (int)(position.x - widthHalf);
    int posTileX = (int)position.x;
    int posTileY = (int)position.y;

    double x = velocity * microTicksLimit / properties->microticksPerSecond + position.x + (velocity >= 0 ? widthHalf : -widthHalf);
    int frontTile = (int)x;

    int prevTile = (velocity >= 0 ? rightPosTileX : leftPosTileX);

    if (velocity >= 0 and frontTile == leftPosTileX or velocity < 0 and frontTile == rightPosTileX)  {
        return microTicksLimit;
    }

    int backTile = velocity * microTicksLimit / properties->microticksPerSecond + position.x + (velocity >= 0 ? -widthHalf : widthHalf);

    bool onGround = level->tiles[posTileX][ (int)ceil(position.y - 1)] == Tile::WALL or level->tiles[rightPosTileX][ (int)ceil(position.y - 1)] == Tile::WALL or level->tiles[leftPosTileX][ (int)ceil(position.y - 1)] == Tile::WALL;

    if (onGround and level->tiles[backTile][posTileY - 1] == Tile::EMPTY) {//Fall from edge
        double posX = position.x + (velocity >= 0 ? -widthHalf : widthHalf);
        double edgeTile = (velocity >= 0 ? ceil(posX) :  floor(posX));
        double distance = abs(edgeTile - posX);

        return (int)ceil((distance / velocity) * properties->updatesPerTick * properties->ticksPerSecond);
    }

    int meanTileY = (int)(position.y + size.y / 2.0);
    int topTileY = (int)(position.y + size.y);

    if (level->tiles[frontTile][topTileY] == level->tiles[prevTile][topTileY]
        and
        level->tiles[frontTile][meanTileY] == level->tiles[prevTile][meanTileY]
        and
        level->tiles[frontTile][posTileY] == level->tiles[prevTile][posTileY]
    ) {
        return microTicksLimit;
    }

    double distance = (-sgn(velocity)) * (position.x - (velocity >= 0 ? ceil(position.x) : floor(position.x) ) );

    return (int)ceil((distance / velocity) * properties->updatesPerTick * properties->ticksPerSecond);
}


int Unit::verFirstEvent(double velocity, int microTicksLimit, const Vec2Double & position, const UnitAction & action) const {

    int rightPosTileX = (int)(position.x + widthHalf);
    int leftPosTileX = (int)(position.x - widthHalf);
    int posTileX = (int)position.x;

    if (jumpState.canJump and jumpState.canCancel and action.jump) {//Jumping
        //End jumping time
        int endJumpMicroTicks = min(microTicksLimit, (int)ceil(jumpState.maxTime * properties->updatesPerTick * properties->ticksPerSecond));

        double y = jumpState.speed * microTicksLimit * properties->microticksPerSecond + position.y;

        //Roof case
        double topY = y + size.y;

        int topTileY = (int)topY;

        if (
                level->tiles[posTileX][topTileY] == Tile::WALL
                or
                level->tiles[leftPosTileX][topTileY] == Tile::WALL
                or
                level->tiles[rightPosTileX][topTileY] == Tile::WALL
                ) {

            int microTicksToRoof = (int)ceil(((topTileY - position.y - size.y) / jumpState.speed) * properties->updatesPerTick * properties->ticksPerSecond);
            return min(endJumpMicroTicks, microTicksToRoof);
        }

        //Ladder case

        double middleY = y + size.y / 2.0;
        int middleTileY = (int)middleY;
        int prevMiddleTileY = (int)(position.y + size.y / 2.0);

        if (level->tiles[posTileX][middleTileY] == Tile::LADDER and level->tiles[posTileX][prevMiddleTileY] != Tile::LADDER) {
            int microTicksToLadder = (int)ceil(((middleTileY - position.y - size.y / 2.0) / jumpState.speed) * properties->updatesPerTick * properties->ticksPerSecond);
            return min(endJumpMicroTicks, microTicksToLadder);
        }

        //Platform case

        int downTileY = (int)y;

        if (level->tiles[rightPosTileX][downTileY] == Tile::EMPTY) {

        }

    }

}
