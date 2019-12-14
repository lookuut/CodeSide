//
// Created by lookuut on 13.12.19.
//

#ifndef AICUP2019_SEGMENT_H
#define AICUP2019_SEGMENT_H


#include "../model/Vec2Double.hpp"

class Segment {

public:
    Vec2Double point1;
    Vec2Double point2;

    Segment(const Vec2Double point1, const Vec2Double point2);
};


#endif //AICUP2019_SEGMENT_H
