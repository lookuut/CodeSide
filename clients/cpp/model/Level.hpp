#ifndef _MODEL_LEVEL_HPP_
#define _MODEL_LEVEL_HPP_

#include "../Stream.hpp"
#include <string>
#include <vector>
#include "Tile.hpp"
#include "Vec2Float.hpp"
#include <stdexcept>

using namespace std;

class Level {
public:

    vector<vector<Vec2Float>> walls;
    vector<std::vector<Tile>> tiles;

    Level();
    Level(std::vector<std::vector<Tile>> tiles);
    static Level readFrom(InputStream& stream);
    void writeTo(OutputStream& stream) const;
    std::string toString() const;
};

#endif
