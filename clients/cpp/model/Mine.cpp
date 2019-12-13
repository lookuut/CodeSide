#include "Mine.hpp"
#include "../utils/Geometry.h"
#include "Unit.hpp"

Mine::Mine() { }

Mine::Mine(int playerId, Vec2Double position, Vec2Double size, MineState state, std::shared_ptr<double> timer, double triggerRadius, ExplosionParams explosionParams) : playerId(playerId), position(position), size(size), state(state), timer(timer), triggerRadius(triggerRadius), explosionParams(explosionParams) {
    leftTopAngle = Vec2Float(position.x - size.x / 2, position.y + size.y);
    rightDownAngle = Vec2Float(position.x + size.x / 2, position.y);
    activateLeftTopAngle = Vec2Float(position.x - triggerRadius - size.x / 2.0, position.y + triggerRadius + size.x);
    activateRightDownAngle = Vec2Float(position.x + triggerRadius + size.x / 2.0, position.y - triggerRadius);
}

Mine Mine::readFrom(InputStream& stream) {
    Mine result;
    result.playerId = stream.readInt();
    result.position = Vec2Double::readFrom(stream);
    result.size = Vec2Double::readFrom(stream);

    result.leftTopAngle = Vec2Float(result.position.x - result.size.x / 2, result.position.y + result.size.y);
    result.rightDownAngle = Vec2Float(result.position.x + result.size.x / 2, result.position.y);

    switch (stream.readInt()) {
    case 0:
        result.state = MineState::PREPARING;
        break;
    case 1:
        result.state = MineState::IDLE;
        break;
    case 2:
        result.state = MineState::TRIGGERED;
        break;
    case 3:
        result.state = MineState::EXPLODED;
        break;
    default:
        throw std::runtime_error("Unexpected discriminant value");
    }
    if (stream.readBool()) {
        result.timer = std::shared_ptr<double>(new double());
        *result.timer = stream.readDouble();
    } else {
        result.timer = std::shared_ptr<double>();
    }
    result.triggerRadius = stream.readDouble();
    result.explosionParams = ExplosionParams::readFrom(stream);
    return result;
}
void Mine::writeTo(OutputStream& stream) const {
    stream.write(playerId);
    position.writeTo(stream);
    size.writeTo(stream);
    stream.write((int)(state));
    if (timer) {
        stream.write(true);
        stream.write((*timer));
    } else {
        stream.write(false);
    }
    stream.write(triggerRadius);
    explosionParams.writeTo(stream);
}
std::string Mine::toString() const {
    return std::string("Mine") + "(" +
        std::to_string(playerId) +
        position.toString() +
        size.toString() +
        "TODO" + 
        "TODO" + 
        std::to_string(triggerRadius) +
        explosionParams.toString() +
        ")";
}


void Mine::explode(vector<Unit> &units, vector<Mine> & mines, int currentMineIndex) {

    Vec2Float explodeLeftTop = Vec2Float(position.x - explosionParams.radius, position.y + explosionParams.radius);
    Vec2Float explodeRightDown = Vec2Float(position.x + explosionParams.radius, position.y - explosionParams.radius);

    for (Unit & unit : units) {

        if (Geometry::isRectOverlap(explodeLeftTop, explodeRightDown, unit.leftTop, unit.rightDown)) {
            unit.health -= explosionParams.damage;
        }
    }

    mines.erase(mines.begin() + currentMineIndex);

    for (int i = mines.size() - 1; i >= 0; --i) {
        Vec2Float leftTop = Vec2Float(mines[i].position.x - mines[i].size.x / 2.0, mines[i].position.y + mines[i].size.y);
        Vec2Float rightDown = Vec2Float(mines[i].position.x + mines[i].size.x / 2.0, mines[i].position.y);

        if (Geometry::isRectOverlap(explodeLeftTop, explodeRightDown, leftTop, rightDown)) {
            mines[i].explode(units, mines, i);
        }
    }
}


bool Mine::equal(const Mine &mine, double eps) const {

    bool a1 = abs(mine.position.x - position.x) < eps;
    bool a2 = abs(mine.position.y - position.y) < eps;
    bool a3 = ((mine.timer == nullptr and timer == nullptr) or (mine.timer and timer and abs(*mine.timer.get() - *timer.get()) < eps));
    bool a4 = mine.state == state;

    if (!(a1 and a2 and a3 and a4)) {
        cout << "dawdad";
    }
    return
    abs(mine.position.x - position.x) < eps and abs(mine.position.y - position.y) < eps
    and
    abs(mine.size.x - size.x) < eps and abs(mine.size.y - size.y) < eps
    and
    abs(mine.leftTopAngle.x - leftTopAngle.x) < eps and abs(mine.leftTopAngle.y - leftTopAngle.y) < eps
    and
    abs(mine.rightDownAngle.x - rightDownAngle.x) < eps and abs(mine.rightDownAngle.y - rightDownAngle.y) < eps
    and
    ((mine.timer == nullptr and timer == nullptr) or (mine.timer != nullptr and timer != nullptr and abs(*mine.timer.get() - *timer.get()) < eps))
    and mine.state == state;
}