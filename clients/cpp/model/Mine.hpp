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
    std::shared_ptr<double> timer;
    double triggerRadius;
    ExplosionParams explosionParams;
    Mine();
    Mine(int playerId, Vec2Double position, Vec2Double size, MineState state, std::shared_ptr<double> timer, double triggerRadius, ExplosionParams explosionParams);
    static Mine readFrom(InputStream& stream);
    void writeTo(OutputStream& stream) const;
    std::string toString() const;

    void explode(vector<Unit> & units, vector<Mine> & mines, int currentMineIndex);

    bool equal(const Mine & mine, double eps) const;
};

#endif
