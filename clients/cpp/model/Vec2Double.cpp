#include "Vec2Double.hpp"

Vec2Double::Vec2Double() { }
Vec2Double::Vec2Double(double x, double y) : x(x), y(y) { }
Vec2Double Vec2Double::readFrom(InputStream& stream) {
    Vec2Double result;
    result.x = stream.readDouble();
    result.y = stream.readDouble();
    return result;
}
void Vec2Double::writeTo(OutputStream& stream) const {
    stream.write(x);
    stream.write(y);
}
std::string Vec2Double::toString() const {
    return std::string("Vec2Double") + "(" +
        std::to_string(x) +
        std::to_string(y) +
        ")";
}

double Vec2Double::distSqr(const Vec2Double &b) const {
    return (x - b.x) * (x - b.x) + (y - b.y) * (y - b.y);
}


Vec2Double Vec2Double::operator+(const Vec2Double &vec) const {
    return Vec2Double(x + vec.x, y + vec.y);
}

Vec2Double Vec2Double::operator-(const Vec2Double &vec) const {
    return Vec2Double(x - vec.x, y - vec.y);
}

void Vec2Double::operator+=(const Vec2Double &vec) {
    x += vec.x;
    y += vec.y;
}

void Vec2Double::operator-=(const Vec2Double &vec) {
    x -= vec.x;
    y -= vec.y;
}

bool Vec2Double::operator==(const Vec2Double &vec) {
    return x == vec.x and y == vec.y;
}

bool Vec2Double::operator!=(const Vec2Double &vec) {
    return !(*this == vec);
}