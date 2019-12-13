#include "Weapon.hpp"
#include "../Consts.h"
#include <math.h>

Weapon::Weapon() { }
Weapon::Weapon(
        WeaponType type,
        WeaponParams params,
        int magazine,
        bool wasShooting,
        double spread,
        double fireTimer,
        double lastAngle,
        int lastFireTick
        ) :
        type(type),
        params(params),
        magazine(magazine),
        wasShooting(wasShooting),
        spread(spread),
        fireTimer(fireTimer),
        lastAngle(lastAngle),
        lastFireTick(lastFireTick) {

}

Weapon Weapon::readFrom(InputStream& stream) {
    Weapon result;
    switch (stream.readInt()) {
    case 0:
        result.type = WeaponType::PISTOL;
        break;
    case 1:
        result.type = WeaponType::ASSAULT_RIFLE;
        break;
    case 2:
        result.type = WeaponType::ROCKET_LAUNCHER;
        break;
    default:
        throw std::runtime_error("Unexpected discriminant value");
    }
    result.params = WeaponParams::readFrom(stream);
    result.magazine = stream.readInt();
    result.wasShooting = stream.readBool();
    result.spread = stream.readDouble();
    if (stream.readBool()) {
        result.fireTimer = stream.readDouble();
    } else {
        result.fireTimer = .0;
    }
    if (stream.readBool()) {
        result.lastAngle = stream.readDouble();
    } else {
        result.lastAngle = Consts::noLastAngleValue;
    }
    if (stream.readBool()) {
        result.lastFireTick = stream.readInt();
    } else {
        result.lastFireTick = -1;
    }
    return result;
}
void Weapon::writeTo(OutputStream& stream) const {
    stream.write((int)(type));
    params.writeTo(stream);
    stream.write(magazine);
    stream.write(wasShooting);
    stream.write(spread);
    if (fireTimer > 0) {
        stream.write(true);
        stream.write(fireTimer);
    } else {
        stream.write(false);
    }

    if (lastAngle != Consts::noLastAngleValue) {
        stream.write(true);
        stream.write(lastAngle);
    } else {
        stream.write(false);
    }

    if (lastFireTick != -1) {
        stream.write(true);
        stream.write(lastFireTick);
    } else {
        stream.write(false);
    }
}
std::string Weapon::toString() const {
    return std::string("Weapon") + "(" +
        "TODO" + 
        params.toString() +
        std::to_string(magazine) +
        (wasShooting ? "true" : "false") + 
        std::to_string(spread) +
        "TODO" + 
        "TODO" + 
        "TODO" + 
        ")";
}

bool Weapon::equal(const Weapon &weapon, double eps) const {
    if (!(abs(weapon.spread - spread) < eps
        and
        weapon.magazine == magazine
        and
        (abs(weapon.fireTimer - fireTimer) < eps)
        and
        weapon.type == type
        and
        (abs(weapon.lastAngle - lastAngle) < eps)
        and
        (weapon.lastFireTick == lastFireTick))) {
        int i = 0;
    }

    return
    abs(weapon.spread - spread) < eps
    and
    weapon.magazine == magazine
    and
    (abs(weapon.fireTimer - fireTimer) < eps)
    and
    weapon.type == type
    and
    (abs(weapon.lastAngle - lastAngle) < eps)
    and
    (weapon.lastFireTick == lastFireTick);
}
