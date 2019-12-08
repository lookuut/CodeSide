#include "JumpState.hpp"

JumpState::JumpState() { }
JumpState::JumpState(bool canJump, double speed, double maxTime, bool canCancel) : canJump(canJump), speed(speed), maxTime(maxTime), canCancel(canCancel) { }
JumpState JumpState::readFrom(InputStream& stream) {
    JumpState result;
    result.canJump = stream.readBool();
    result.speed = stream.readDouble();
    result.maxTime = stream.readDouble();
    result.canCancel = stream.readBool();
    return result;
}
void JumpState::writeTo(OutputStream& stream) const {
    stream.write(canJump);
    stream.write(speed);
    stream.write(maxTime);
    stream.write(canCancel);
}
std::string JumpState::toString() const {
    return std::string("JumpState") + "(" +
        (canJump ? "true" : "false") + 
        std::to_string(speed) +
        std::to_string(maxTime) +
        (canCancel ? "true" : "false") + 
        ")";
}


bool JumpState::equal(const JumpState &state, double eps) const {
    return state.canCancel == state.canCancel and state.canJump == canJump and abs(state.speed - speed) <= 0 and abs(state.maxTime - maxTime) <= eps;
}