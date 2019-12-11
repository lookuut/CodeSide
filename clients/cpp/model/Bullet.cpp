#include "Bullet.hpp"
#include "../utils/Geometry.h"
#include "Unit.hpp"
#include "Game.hpp"

Bullet::Bullet() { }
Bullet::Bullet(WeaponType weaponType, int unitId, int playerId, Vec2Double position, Vec2Double velocity, int damage, double size, std::shared_ptr<ExplosionParams> explosionParams) : weaponType(weaponType), unitId(unitId), playerId(playerId), position(position), velocity(velocity), damage(damage), size(size), explosionParams(explosionParams) {
    leftTop.x = position.x - size / 2.0;
    leftTop.y = position.y + size / 2.0;

    rightDown.x = position.x + size / 2.0;
    rightDown.y = position.y - size / 2.0;
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

    result.leftTop.x = result.position.x - result.size / 2.0;
    result.leftTop.y = result.position.y + result.size / 2.0;

    result.rightDown.x = result.position.x + result.size / 2.0;
    result.rightDown.y = result.position.y - result.size / 2.0;


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
        stream.write(false);
    } else {
        stream.write(true);
        (*explosionParams).writeTo(stream);
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

        Vec2Double unitLeft = Vec2Double(unit.position.x - unit.size.x / 2.0, unit.position.y + unit.size.y);
        Vec2Double unitRight = Vec2Double(unit.position.x + unit.size.x / 2.0, unit.position.y);

        if (Geometry::isRectOverlap(explossionLeft, explossionRight, unitLeft, unitRight)) {
            unit.health -= explosionParams.get()->damage;
        }
    }
}


bool Bullet::equal(const Bullet &bullet, double eps) const {
    return bullet.weaponType == bullet.weaponType and (bullet.position - position).len() <= eps;
}


void Bullet::move(const Vec2Double & vel) {
    position.x += (vel.x * Game::getProperties()->microticksPerSecond);
    position.y += (vel.y * Game::getProperties()->microticksPerSecond);

    leftTop.x = position.x - size / 2.0;
    leftTop.y = position.y + size / 2.0;

    rightDown.x = position.x + size / 2.0;
    rightDown.y = position.y - size / 2.0;
}