#ifndef _MODEL_VEC2_FLOAT_HPP_
#define _MODEL_VEC2_FLOAT_HPP_

#include "../Stream.hpp"
#include <string>


class Vec2Double;
class Vec2Float {
public:
    float x;
    float y;
    Vec2Float();
    Vec2Float(float x, float y);
    Vec2Float(const Vec2Double & vec);
    static Vec2Float readFrom(InputStream& stream);
    void writeTo(OutputStream& stream) const;
    std::string toString() const;

    Vec2Float operator-(const Vec2Float & v) const;
    Vec2Float operator+(const Vec2Float & v) const;
    Vec2Float operator*(double f) const;
    Vec2Float rotate(double angle) const;
    float sqrLen() const {
        return (x * x + y * y);
    }
};

#endif
