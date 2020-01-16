#ifndef _MODEL_UNIT_ACTION_HPP_
#define _MODEL_UNIT_ACTION_HPP_

#include "../Stream.hpp"
#include <string>
#include <stdexcept>
#include "Vec2Double.hpp"

typedef struct Action{
    bool jump;
    bool jumpDown;
    double velocity;
} Action;

class UnitAction {
public:
    double velocity = 0;
    bool jump = false;
    bool jumpDown = false;
    Vec2Double aim = ZERO_VEC_2_DOUBLE;
    bool shoot = false;
    bool reload = false;
    bool swapWeapon = false;
    bool plantMine = false;
    UnitAction();
    UnitAction(double velocity, bool jump, bool jumpDown, Vec2Double aim, bool shoot, bool reload, bool swapWeapon, bool plantMine);

    void update(const Action & action);
    static UnitAction readFrom(InputStream& stream);
    void writeTo(OutputStream& stream) const;
    std::string toString() const;

    void setJumpState(int jumpState) {
        switch (jumpState) {
            case -1:
                jump = false;
                jumpDown = false;
                break;
            case 0:
                jump = true;
                jumpDown = false;
                break;
            case 1:
                jump = false;
                jumpDown = true;
                break;
        }
    }
};

#endif
