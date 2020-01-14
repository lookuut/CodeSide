#include "Bullet.hpp"
#include "../utils/Geometry.h"
#include "Unit.hpp"
#include "Game.hpp"
#include <math.h>
#include "../arena/Simulation.hpp"

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

    updateFrontPoint();
    widthBorderSign = (frontPoint.x > position.x) ? -1 : 1;
    heightBorderSign = (frontPoint.y > position.y) ? -1 : 1;

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
    result.updateFrontPoint();

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

void Bullet::explossion(Unit & unit, const Vec2Double & unitPosition, Simulation & simulation) const {

    if (weaponType != WeaponType::ROCKET_LAUNCHER) {
        return;
    }

    double r = explosionParams.get()->radius;

    Vec2Double explossionLeft = Vec2Double(position.x - r, position.y + r);
    Vec2Double explossionRight = Vec2Double(position.x + r, position.y - r);

    Vec2Double unitLeft = Vec2Double(unitPosition.x - unit.widthHalf, unitPosition.y + unit.size.y);
    Vec2Double unitRight = Vec2Double(unitPosition.x + unit.widthHalf, unitPosition.y);

    if (Geometry::isRectOverlap(explossionLeft, explossionRight, unitLeft, unitRight)) {
        unit.health -= explosionParams.get()->damage;

        if (unit.playerId == simulation.game->allyPlayerId) {
            simulation.enemyPoints += explosionParams.get()->damage + (unit.health <= 0 ? simulation.properties->killScore : 0);
        } else {
            simulation.allyPoints += explosionParams.get()->damage + (unit.health <= 0 ? simulation.properties->killScore : 0);
        }
    }
}


bool Bullet::equal(const Bullet &bullet, double eps) const {
    return bullet.weaponType == bullet.weaponType and (bullet.position - position).len() <= eps;
}


void Bullet::move(const Vec2Double & vel, int microTicks, Properties * properties) {//@TODO fix this boolshit
    position.x += (vel.x * microTicks / (properties->updatesPerTick * properties->ticksPerSecond));
    position.y += (vel.y * microTicks / (properties->updatesPerTick * properties->ticksPerSecond));

    leftTop.x = position.x - halfSize;
    leftTop.y = position.y + halfSize;

    rightDown.x = position.x + halfSize;
    rightDown.y = position.y - halfSize;
    updateFrontPoint();
}

int Bullet::overlapWithSegmentMicroTicks(const Segment &segment, int microticks, Properties *properties) const {

    Vec2Double rectBackPoint = segment.rectBackPoint(velocity);

    double width = segment.rightDown.x - segment.leftTop.x;
    double height = segment.leftTop.y - segment.rightDown.y;

    int minTime = microticks + 1;

    vector<Vec2Double> bulletPoints = {
            frontPoint,
            Vec2Double(frontPoint.x, frontPoint.y + size * heightBorderSign),
            Vec2Double(frontPoint.x + size * widthBorderSign, frontPoint.y)
    };

    for (Vec2Double & frontPoint : bulletPoints) {

        if (abs(height) <= Consts::eps) {
            if (auto horTime = Geometry::crossHorSegmentTime(frontPoint, velocity, rectBackPoint, (rectBackPoint.x > leftTop.x) ? -width : width)) {
                minTime = min((int)floor(horTime.value() * properties->ticksPerSecond * properties->updatesPerTick) , minTime);
            }
        } else {
            if (auto verTime = Geometry::crossVerSegmentTime(frontPoint, velocity, rectBackPoint, (rectBackPoint.y > rightDown.y) ? -height : height)) {
                minTime = min((int)floor(verTime.value() * properties->ticksPerSecond * properties->updatesPerTick), minTime);
            }
        }
    }

    return minTime;
}


int Bullet::overlapWithRectMicroTicks(const Segment &rect, int microticks, Properties *properties) const {

    Vec2Double rectBackPoint = rect.rectBackPoint(velocity);

    double width = rect.rightDown.x - rect.leftTop.x;
    double height = rect.leftTop.y - rect.rightDown.y;

    int minTime = microticks + 1;

    vector<Vec2Double> bulletPoints = {
            frontPoint,
            Vec2Double(frontPoint.x, frontPoint.y + size * heightBorderSign),
            Vec2Double(frontPoint.x + size * widthBorderSign, frontPoint.y)
    };

    for (Vec2Double & frontPoint : bulletPoints) {
        if (auto horTime = Geometry::crossHorSegmentTime(frontPoint, velocity, rectBackPoint, (rectBackPoint.x > leftTop.x) ? -width : width)) {
            minTime = min((int)floor(horTime.value() * properties->ticksPerSecond * properties->updatesPerTick) , minTime);
        }

        if (auto verTime = Geometry::crossVerSegmentTime(frontPoint, velocity, rectBackPoint, (rectBackPoint.y > rightDown.y) ? -height : height)) {
            minTime = min((int)floor(verTime.value() * properties->ticksPerSecond * properties->updatesPerTick), minTime);
        }
    }

    return minTime;
}

void Bullet::updateFrontPoint() {
    if (velocity.x >= 0 and velocity.y >= 0) {
        frontPoint.x = position.x + halfSize;
        frontPoint.y = position.y + halfSize;
    } else if (velocity.x >= 0 and velocity.y < 0) {
        frontPoint.x = position.x + halfSize;
        frontPoint.y = position.y - halfSize;
    } else if (velocity.x < 0 and velocity.y < 0) {
        frontPoint.x = position.x - halfSize;
        frontPoint.y = position.y - halfSize;
    } else {
        frontPoint.x = position.x - halfSize;
        frontPoint.y = position.y + halfSize;
    }
}