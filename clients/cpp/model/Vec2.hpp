//
// Created by lookuut on 11.12.19.
//

#ifndef AICUP2019_VEC2_HPP
#define AICUP2019_VEC2_HPP

#include "../Stream.hpp"
#include <string>
#include <math.h>
#include <bits/shared_ptr.h>


using namespace std;

class Vec2 {
public:

    virtual void writeTo(OutputStream& stream) const =0;

    virtual double distSqr(const Vec2 & point) const =0;

    virtual std::string toString() const =0;

    virtual std::shared_ptr<Vec2> operator+(const Vec2 & vec) const =0;
    virtual std::shared_ptr<Vec2> operator-(const Vec2 & vec) const =0;

    virtual void operator+=(const Vec2 & vec) =0;
    virtual void operator-=(const Vec2 & vec) =0;
    virtual bool operator==(const Vec2 & vec) =0;
    virtual bool operator!=(const Vec2 & vec) =0;

    virtual void operator*=(double f) =0;

    virtual double len() const =0;
    virtual double sqrLen() const =0;
    virtual shared_ptr<Vec2> normalize() =0;
    virtual shared_ptr<Vec2> rotate(double angle) const =0;
    virtual shared_ptr<Vec2> toFloat() =0;
};
#endif //AICUP2019_VEC2_HPP
