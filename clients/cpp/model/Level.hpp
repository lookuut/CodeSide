#ifndef _MODEL_LEVEL_HPP_
#define _MODEL_LEVEL_HPP_

#include "../Stream.hpp"
#include <string>
#include <vector>
#include "Vec2Float.hpp"
#include <stdexcept>
#include <optional>
#include <map>
#include "Tile.hpp"
#include "../Debug.hpp"

using namespace std;

class Level {
public:
    int width;
    int height;

    vector<vector<Vec2Float>> walls;
    vector<Vec2Float> standablePlaces;
    vector<std::vector<Tile>> tiles;

    Level();
    Level(std::vector<std::vector<Tile>> tiles);

    static Level readFrom(InputStream& stream);

    void writeTo(OutputStream& stream) const;
    std::string toString() const;
    optional<Vec2Float> crossWall(const Vec2Float & p1, const Vec2Float & p2) const;
    optional<Vec2Float> crossMiDistanceWall(const Vec2Float & p1, const Vec2Float & p2) const;


    void buildWalls();
    void buildStandablePlaces();
};

#endif
