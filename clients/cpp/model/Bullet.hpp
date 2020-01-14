#ifndef _MODEL_BULLET_HPP_
#define _MODEL_BULLET_HPP_

#include "../Stream.hpp"
#include <string>
#include <stdexcept>
#include "WeaponType.hpp"
#include "Vec2Double.hpp"
#include <memory>
#include "ExplosionParams.hpp"
#include "../utils/Segment.h"
#include <math.h>
#include <vector>
#include "Properties.hpp"

using namespace std;

class Unit;
class Simulation;

class Bullet {
public:
    WeaponType weaponType;
    int unitId;
    int playerId;

    Vec2Double leftTop;
    Vec2Double rightDown;

    Vec2Double position;
    Vec2Double velocity;
    Vec2Double frontPoint;

    short widthBorderSign;
    short heightBorderSign;

    int damage;
    double size;
    double halfSize;
    std::shared_ptr<ExplosionParams> explosionParams;
    Bullet();
    Bullet(WeaponType weaponType, int unitId, int playerId, Vec2Double position, Vec2Double velocity, int damage, double size, std::shared_ptr<ExplosionParams> explosionParams);
    static Bullet readFrom(InputStream& stream);
    void writeTo(OutputStream& stream) const;
    std::string toString() const;

    void explossion(Unit & unit, const Vec2Double & unitPosition, Simulation & simulation) const;

    bool equal(const Bullet &bullet, double eps) const;
    void move(const Vec2Double & vel, int microTicks, Properties * properties);
    vector<Vec2Double> getFrontPoints() const;
    int overlapWithRectMicroTicks(const Segment &rect, int microticks, Properties *properties) const;
    int overlapWithSegmentMicroTicks(const Segment &segment, int microticks, Properties *properties) const;
    void updateFrontPoint();


};

#endif
