//
// Created by lookuut on 20.07.19.
//

#ifndef PAPER_HELPER_H
#define PAPER_HELPER_H


#include "../models/Point.h"

class Helper{
public:

    static bool in_polygon(const Point & point, const vector<int> & xp, const vector<int> & yp) {
        int c = 0;

        for (unsigned int i = 0; i < xp.size(); i++) {

            int y_val = (i == 0 ? yp[yp.size() - 1] : yp[i - 1]);
            int x_val = (i == 0 ? xp[xp.size() - 1] : xp[i - 1]);

            if (((yp[i] <= point.y and point.y < y_val) or (y_val <= point.y and point.y < yp[i]))
                    and
                    (point.x > (x_val - xp[i]) * (point.y - yp[i]) / (y_val - yp[i]) + xp[i])
                ) {
                c = 1 - c;
            }
        }

        return c == 1;
    }


    static bool is_intersect(const Point & p1, const Point & p2, int width = Config::width) {
        return abs(p1.x - p2.x) < width and abs(p1.y - p2.y) < width;
    }

    static string direction(const Direction & direction) {
        switch (direction) {
            case Direction ::down:
                return "down";
            case Direction ::up:
                return "up";
            case Direction ::left:
                return "left";
            case Direction ::right:
                return "right";
            case Direction ::stop:
                throw std::invalid_argument("Invalid direction");
        }
    }

    static Direction relative_direction(const Direction & direction, int relative_dir) {


        if (relative_dir == 0 && direction != Direction::stop) {
            return direction;
        }

        switch (direction) {
            case Direction ::down:

                if (relative_dir == -1) {
                    return Direction ::right;
                } else {
                    return Direction ::left;
                }
            case Direction ::up:
                if (relative_dir == -1) {
                    return Direction ::left;
                } else {
                    return Direction ::right;
                }
            case Direction ::left:
                if (relative_dir == -1) {
                    return Direction ::down;
                } else {
                    return Direction ::up;
                }
            case Direction ::right:
                if (relative_dir == -1) {
                    return Direction ::up;
                } else {
                    return Direction ::down;
                }
            case Direction ::stop:
                switch(relative_dir){
                    case -1:
                        return Direction ::left;
                    case 0:
                        return Direction ::down;
                    case 1:
                        return Direction ::right;
                }

        }
    }
};

#endif //PAPER_HELPER_H
