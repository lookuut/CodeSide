#ifndef _MODEL_MINE_HPP_
#define _MODEL_MINE_HPP_

#include "../Stream.hpp"
#include <string>
#include <stdexcept>
#include "Vec2Double.hpp"
#include "MineState.hpp"
#include <memory>
#include "ExplosionParams.hpp"
#include <vector>
#include <map>

using namespace std;

class Unit;

class Mine {
public:
    int playerId;
    Vec2Double position;
    Vec2Double size;

    Vec2Float leftTopAngle;
    Vec2Float rightDownAngle;

    Vec2Float activateLeftTopAngle;
    Vec2Float activateRightDownAngle;

    MineState state;
    double timer;
    double triggerRadius;
    ExplosionParams explosionParams;
    Mine();
    Mine(int playerId, Vec2Double position, Vec2Double size, MineState state, double timer, double triggerRadius, ExplosionParams explosionParams);
    Mine(const Mine & mine);
    static Mine readFrom(InputStream& stream);
    void writeTo(OutputStream& stream) const;
    std::string toString() const;

    void explode(vector<Unit> & units, vector<Mine> & mines, int currentMineIndex, map<int, bool> & deletedMines);

    bool equal(const Mine & mine, double eps) const;
};

#endif
