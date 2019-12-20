//
// Created by lookuut on 17.12.19.
//

#ifndef AICUP2019_TARGET_H
#define AICUP2019_TARGET_H


#include "../model/Vec2Double.hpp"

class Target {

public:

    Vec2Double pos;
    int type;

    Target(const Vec2Double & pos, int type) : pos(pos), type(type){

    }
};


#endif //AICUP2019_TARGET_H
