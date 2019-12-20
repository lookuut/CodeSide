//
// Created by lookuut on 13.12.19.
//

#include "Segment.h"

Segment::Segment() {}

Segment::Segment(const Vec2Double point1, const Vec2Double point2) : leftTop(point1), rightDown(point2) {

}


Vec2Double Segment::rectBackPoint(const Vec2Double &dir) const {
    if (dir.x >= 0 and dir.y >= 0) {
        return Vec2Double(leftTop.x, rightDown.y);
    } else if (dir.x >= 0 and dir.y < 0) {
        return leftTop;
    } else if (dir.x < 0 and dir.y < 0) {
        return Vec2Double(rightDown.x, leftTop.y);
    } else {
        return rightDown;
    }
}