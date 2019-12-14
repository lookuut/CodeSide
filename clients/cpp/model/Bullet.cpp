#include "Bullet.hpp"
#include "../utils/Geometry.h"
#include "Unit.hpp"
#include "Game.hpp"

Bullet::Bullet() { }
Bullet::Bullet(
        WeaponType weaponType,
        int unitId,
        int playerId,
        Vec2Double position,
        Vec2Double velocity,
        int damage,
        double size,
        std::shared_ptr<ExplosionParams> explosionParams
        ) :
        weaponType(weaponType),
        unitId(unitId),
        playerId(playerId),
        position(position),
        velocity(velocity),
        damage(damage),
        size(size),
        halfSize(size / 2.0),
        explosionParams(explosionParams) {
    leftTop.x = position.x - halfSize;
    leftTop.y = position.y + halfSize;

    rightDown.x = position.x + halfSize;
    rightDown.y = position.y - halfSize;
}
Bullet Bullet::readFrom(InputStream& stream) {
    Bullet result;
    switch (stream.readInt()) {
    case 0:
        result.weaponType = WeaponType::PISTOL;
        break;
    case 1:
        result.weaponType = WeaponType::ASSAULT_RIFLE;
        break;
    case 2:
        result.weaponType = WeaponType::ROCKET_LAUNCHER;
        break;
    default:
        throw std::runtime_error("Unexpected discriminant value");
    }
    result.unitId = stream.readInt();
    result.playerId = stream.readInt();
    result.position = Vec2Double::readFrom(stream);

    result.velocity = Vec2Double::readFrom(stream);
    result.damage = stream.readInt();
    result.size = stream.readDouble();
    result.halfSize = result.size / 2.0;
    result.leftTop.x = result.position.x - result.halfSize;
    result.leftTop.y = result.position.y + result.halfSize;

    result.rightDown.x = result.position.x + result.halfSize;
    result.rightDown.y = result.position.y - result.halfSize;


    if (stream.readBool()) {
        result.explosionParams = std::shared_ptr<ExplosionParams>(new ExplosionParams());
        *result.explosionParams = ExplosionParams::readFrom(stream);
    } else {
        result.explosionParams = std::shared_ptr<ExplosionParams>();
    }
    return result;
}
void Bullet::writeTo(OutputStream& stream) const {
    stream.write((int)(weaponType));
    stream.write(unitId);
    stream.write(playerId);
    position.writeTo(stream);
    velocity.writeTo(stream);
    stream.write(damage);
    stream.write(size);
    if (explosionParams) {
        stream.write(true);
        (*explosionParams).writeTo(stream);
    } else {
        stream.write(false);
    }
}
std::string Bullet::toString() const {
    return std::string("Bullet") + "(" +
        "TODO" + 
        std::to_string(unitId) +
        std::to_string(playerId) +
        position.toString() +
        velocity.toString() +
        std::to_string(damage) +
        std::to_string(size) +
        "TODO" + 
        ")";
}

void Bullet::explossion(vector<Unit> &units) const {

    if (weaponType != WeaponType::ROCKET_LAUNCHER) {
        return;
    }

    for (Unit & unit : units) {
        double r = explosionParams.get()->radius;

        Vec2Double explossionLeft = Vec2Double(position.x - r, position.y + r);
        Vec2Double explossionRight = Vec2Double(position.x + r, position.y - r);

        Vec2Double unitLeft = Vec2Double(unit.position.x - unit.widthHalf, unit.position.y + unit.size.y);
        Vec2Double unitRight = Vec2Double(unit.position.x + unit.widthHalf, unit.position.y);

        if (Geometry::isRectOverlap(explossionLeft, explossionRight, unitLeft, unitRight)) {
            unit.health -= explosionParams.get()->damage;
        }
    }
}


bool Bullet::equal(const Bullet &bullet, double eps) const {
    return bullet.weaponType == bullet.weaponType and (bullet.position - position).len() <= eps;
}


void Bullet::move(const Vec2Double & vel) {//@TODO fix this boolshit
    position.x += (vel.x * Game::getProperties()->microticksPerSecond);
    position.y += (vel.y * Game::getProperties()->microticksPerSecond);

    leftTop.x = position.x - halfSize;
    leftTop.y = position.y + halfSize;

    rightDown.x = position.x + halfSize;
    rightDown.y = position.y - halfSize;
}

vector<Vec2Double> Bullet::getFrontPoints() const {

    if (velocity.x >= 0 and velocity.y >= 0) {
        return {Vec2Double(halfSize, -halfSize) + position, Vec2Double(-halfSize, halfSize) + position, Vec2Double(halfSize, halfSize) + position};
    } else if (velocity.x >= 0 and velocity.y < 0) {
        return {Vec2Double(halfSize, halfSize) + position, Vec2Double(-halfSize, -halfSize) + position, Vec2Double(halfSize, -halfSize) + position};
    } else if (velocity.x < 0 and velocity.y < 0) {
        return {Vec2Double(halfSize, -halfSize) + position, Vec2Double(-halfSize, halfSize) + position, Vec2Double(-halfSize, -halfSize) + position};
    } else {
        return {Vec2Double(halfSize, halfSize) + position, Vec2Double(-halfSize, -halfSize) + position, Vec2Double(-halfSize, halfSize) + position};
    }
}
