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

bool Vec2Double::operator==(const Vec2Double &vec) const{
    return x == vec.x and y == vec.y;
}

bool Vec2Double::operator!=(const Vec2Double &vec) const{
    return !(*this == vec);
}

void Vec2Double::operator*=(double f) {
    x *=f;
    y *=f;
}

double Vec2Double::len() const {
    return sqrt(x * x + y * y);
}

double Vec2Double::sqrLen() const {
    return x * x + y * y;
}

Vec2Double& Vec2Double::normalize(){
    double l = len();

    x = x / l;
    y = y / l;
    return *this;
}

Vec2Double Vec2Double::rotate(double angle) const{
    return Vec2Double(x * cos(angle) - y * sin(angle), y * cos(angle) + x * sin(angle));
}

double Vec2Double::cross(const Vec2Double &vec) const {
    return x * vec.y - y * vec.x;
}

double Vec2Double::dot(const Vec2Double &vec) const {
    return x * vec.x - y * vec.y;
}

double Vec2Double::angle(const Vec2Double &vec) const {
    return atan2(this->cross(vec), this->dot(vec));
}

Vec2Double Vec2Double::getOpponentAngle(double size, bool isClockWise) const {

    if (x >= 0 and y >= 0) {
        return Vec2Double((isClockWise ? size: -size), (isClockWise ? -size: size));
    } else if (x >= 0 and y < 0) {
        return Vec2Double((isClockWise ? -size: size), (isClockWise ? -size: size));
    } else if (x < 0 and y < 0) {
        return Vec2Double((isClockWise ? -size: size), (isClockWise ? size: -size));
    } else {
        return Vec2Double((isClockWise ? size: -size), (isClockWise ? size: -size));
    }
}

Vec2Float Vec2Double::toFloat() {
    return Vec2Float((float)x, (float)y);
}


