//
// Created by lookuut on 30.11.19.
//

#ifndef AICUP2019_GEOMETRY_H
#define AICUP2019_GEOMETRY_H

#include <math.h>
#include <optional>

using namespace std;

#include "../model/Vec2Float.hpp"

class Geometry{
public:

    static bool onSegment(const Vec2Float & p, const Vec2Float & q,  const Vec2Float & r)
    {
        if (q.x <= max(p.x, r.x) && q.x >= min(p.x, r.x) &&
            q.y <= max(p.y, r.y) && q.y >= min(p.y, r.y))
            return true;

        return false;
    }


    // To find orientation of ordered triplet (p, q, r).
    // The function returns following values
    // 0 --> p, q and r are colinear
    // 1 --> Clockwise
    // 2 --> Counterclockwise

    static int orientation(const Vec2Float & p, const Vec2Float & q, const Vec2Float & r)
    {
        // for details of below formula.
        int val = (q.y - p.y) * (r.x - q.x) -
                  (q.x - p.x) * (r.y - q.y);

        if (val == 0) return 0;  // colinear

        return (val > 0)? 1: 2; // clock or counterclock wise
    }

    // The main function that returns true if line segment 'p1q1'
    // and 'p2q2' intersect.
    static std::optional<Vec2Float>  doIntersect(const Vec2Float & fLineStart, const Vec2Float & fLineEnd, const Vec2Float & sLineStart, const Vec2Float & sLineEnd)
    {
        Vec2Float dir1 = fLineEnd - fLineStart;
        Vec2Float dir2 = sLineEnd - sLineStart;

        //считаем уравнения прямых проходящих через отрезки
        double a1 = -dir1.y;
        double b1 = +dir1.x;
        double d1 = -(a1 * fLineStart.x + b1 * fLineStart.y);

        double a2 = -dir2.y;
        double b2 = +dir2.x;
        double d2 = -(a2 * sLineStart.x + b2 * sLineStart.y);

        //подставляем концы отрезков, для выяснения в каких полуплоскотях они
        double seg1_line2_start = a2 * fLineStart.x + b2 * fLineStart.y + d2;
        double seg1_line2_end = a2 * fLineEnd.x + b2 * fLineEnd.y + d2;

        double seg2_line1_start = a1*sLineStart.x + b1*sLineStart.y + d1;
        double seg2_line1_end = a1*sLineEnd.x + b1*sLineEnd.y + d1;

        //если концы одного отрезка имеют один знак, значит он в одной полуплоскости и пересечения нет.
        if (seg1_line2_start * seg1_line2_end > 0 || seg2_line1_start * seg2_line1_end > 0)
            return nullopt;

        double u = seg1_line2_start / (seg1_line2_start - seg1_line2_end);
        return fLineStart + (dir1 * u);
    }
};
#endif //AICUP2019_GEOMETRY_H
