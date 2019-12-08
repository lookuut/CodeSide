#ifndef _MODEL_JUMP_STATE_HPP_
#define _MODEL_JUMP_STATE_HPP_

#include "../Stream.hpp"
#include <string>
#include <math.h>

class JumpState {
public:
    bool canJump;
    double speed;
    double maxTime;
    bool canCancel;
    JumpState();
    JumpState(bool canJump, double speed, double maxTime, bool canCancel);
    static JumpState readFrom(InputStream& stream);
    void writeTo(OutputStream& stream) const;
    std::string toString() const;

    bool equal(const JumpState & state, double eps) const;
};

#endif
