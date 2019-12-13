#ifndef _MODEL_VEC2_DOUBLE_HPP_
#define _MODEL_VEC2_DOUBLE_HPP_

#include "../Stream.hpp"
#include <string>
#include <math.h>
#include "Vec2Float.hpp"

class Vec2Double {
public:

    double x;
    double y;

    Vec2Double();

    Vec2Double(double x, double y);

    static Vec2Double readFrom(InputStream& stream);

    void writeTo(OutputStream& stream) const;

    double distSqr(const Vec2Double & point) const;

    std::string toString() const;

    Vec2Double operator+(const Vec2Double & vec) const;
    Vec2Double operator-(const Vec2Double & vec) const;

    void operator+=(const Vec2Double & vec);
    void operator-=(const Vec2Double & vec);
    bool operator==(const Vec2Double & vec) const;
    bool operator!=(const Vec2Double & vec) const;

    double cross(const Vec2Double & vec) const;
    double dot(const Vec2Double & vec) const;
    double angle(const Vec2Double & vec) const;

    void operator*=(double f);

    double len() const;
    double sqrLen() const;
    Vec2Double& normalize();
    Vec2Double rotate(double angle) const;
    Vec2Double getOpponentAngle(double size, bool isClockWise) const;

    Vec2Float toFloat();
};


#define ZERO_VEC_2_DOUBLE Vec2Double(0,0)

#endif
