//
// Created by lookuut on 13.12.19.
//

#ifndef AICUP2019_SEGMENT_H
#define AICUP2019_SEGMENT_H


#include "../model/Vec2Double.hpp"
#include <vector>

using namespace std;

class Segment {

public:
    Vec2Double leftTop;
    Vec2Double rightDown;
    Segment();
    Segment(const Vec2Double point1, const Vec2Double point2);


    Vec2Double rectBackPoint(const Vec2Double & dir) const;
};


#endif //AICUP2019_SEGMENT_H
