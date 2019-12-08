#ifndef _MODEL_BULLET_HPP_
#define _MODEL_BULLET_HPP_

#include "../Stream.hpp"
#include <string>
#include <stdexcept>
#include "WeaponType.hpp"
#include "Vec2Double.hpp"
#include <memory>
#include "ExplosionParams.hpp"
#include <math.h>
#include <vector>
using namespace std;

class Unit;

class Bullet {
public:
    WeaponType weaponType;
    int unitId;
    int playerId;
    Vec2Double position;
    Vec2Double velocity;
    int damage;
    double size;
    std::shared_ptr<ExplosionParams> explosionParams;
    Bullet();
    Bullet(WeaponType weaponType, int unitId, int playerId, Vec2Double position, Vec2Double velocity, int damage, double size, std::shared_ptr<ExplosionParams> explosionParams);
    static Bullet readFrom(InputStream& stream);
    void writeTo(OutputStream& stream) const;
    std::string toString() const;

    void explossion(vector<Unit> &units) const;

    bool equal(const Bullet &bullet, double eps) const;
    void move(const Vec2Double & vel);
};

#endif
