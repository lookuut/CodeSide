#ifndef _MODEL_WEAPON_HPP_
#define _MODEL_WEAPON_HPP_

#include "../Stream.hpp"
#include <string>
#include <stdexcept>
#include "WeaponType.hpp"
#include <stdexcept>
#include "WeaponParams.hpp"
#include <stdexcept>
#include "BulletParams.hpp"
#include <memory>
#include <stdexcept>
#include "ExplosionParams.hpp"
#include <memory>
#include <memory>
#include <memory>

class Weapon {
public:
    WeaponType type;
    WeaponParams params;
    int magazine;
    bool wasShooting;
    double spread;

    double fireTimer;
    double lastAngle;
    int lastFireTick;
    Weapon();
    Weapon(WeaponType typ, WeaponParams params, int magazine, bool wasShooting, double spread, double fireTimer, double lastAngle, int lastFireTick);
    static Weapon readFrom(InputStream& stream);
    void writeTo(OutputStream& stream) const;
    std::string toString() const;

    bool equal(const Weapon & weapon, double eps) const;
};

#endif
