#include "Vec2Float.hpp"
#include "Vec2Double.hpp"

Vec2Float::Vec2Float() { }
Vec2Float::Vec2Float(float x, float y) : x(x), y(y) { }
Vec2Float::Vec2Float(const Vec2Double &vec) {
    this->x = vec.x;
    this->y = vec.y;
}
Vec2Float Vec2Float::readFrom(InputStream& stream) {
    Vec2Float result;
    result.x = stream.readFloat();
    result.y = stream.readFloat();
    return result;
}
void Vec2Float::writeTo(OutputStream& stream) const {
    stream.write(x);
    stream.write(y);
}
std::string Vec2Float::toString() const {
    return std::string("Vec2Float") + "(" +
        std::to_string(x) +
        std::to_string(y) +
        ")";
}


Vec2Float Vec2Float::operator+(const Vec2Float &v) const {
    return Vec2Float(x + v.x, y + v.y);
}


Vec2Float Vec2Float::operator-(const Vec2Float &v) const {
    return Vec2Float(x - v.x, y - v.y);
}

Vec2Float Vec2Float::operator*(double f) const {
    return Vec2Float(x * f, y * f);
}

Vec2Float Vec2Float::rotate(double angle) const{
    return Vec2Float(x * cos(angle) - y * sin(angle), y * cos(angle) + x * sin(angle));
}

Vec2Double Vec2Float::toDouble() const {
    return Vec2Double(x, y);
}