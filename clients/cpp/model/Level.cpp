#include "Level.hpp"
#include "../utils/Geometry.h"

Level::Level() { }

Level::Level(std::vector<std::vector<Tile>> tiles) : tiles(tiles) { }

Level Level::readFrom(InputStream& stream) {
    static Level result;

    result.tiles = std::vector<std::vector<Tile>>(stream.readInt());

    for (size_t i = 0; i < result.tiles.size(); i++) {
        result.tiles[i] = std::vector<Tile>(stream.readInt());
        for (size_t j = 0; j < result.tiles[i].size(); j++) {
            switch (stream.readInt()) {
            case 0:
                result.tiles[i][j] = Tile::EMPTY;
                break;
            case 1:
                result.tiles[i][j] = Tile::WALL;
                break;
            case 2:
                result.tiles[i][j] = Tile::PLATFORM;
                break;
            case 3:
                result.tiles[i][j] = Tile::LADDER;
                break;
            case 4:
                result.tiles[i][j] = Tile::JUMP_PAD;
                break;
            default:
                throw std::runtime_error("Unexpected discriminant value");
            }
        }
    }

    if (result.walls.empty()) {

        int width = result.tiles.size();
        int height = result.tiles[0].size();

        vector<int> visited = vector<int>(width * height, 0);

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {

                if (result.tiles[x][y] == Tile::WALL and visited[y * width + x] == 0) {
                    vector<Vec2Float> wall;

                    if (x + 1 < width and result.tiles[x + 1][y] == Tile::WALL) {
                        int startX = x;

                        while (x < width and result.tiles[x][y] == Tile::WALL) {
                            visited[y * width + x] = 1;
                            ++x;
                        }

                        wall.push_back(Vec2Float(startX, y));
                        wall.push_back(Vec2Float(startX, y + 1));
                        wall.push_back(Vec2Float(x, y + 1));
                        wall.push_back(Vec2Float(x, y));

                    } else if (y + 1 < height and result.tiles[x][y + 1] == Tile::WALL) {
                        int startHor = y;
                        int hor = y;

                        while (hor < height and result.tiles[x][hor] == Tile::WALL) {
                            visited[hor * width + x] = 1;
                            ++hor;
                        }

                        wall.push_back(Vec2Float(x, startHor));
                        wall.push_back(Vec2Float(x + 1, startHor));
                        wall.push_back(Vec2Float(x + 1, hor));
                        wall.push_back(Vec2Float(x, hor));
                    }
                    result.walls.push_back(wall);
                }
            }
        }
    }

    return result;
}
void Level::writeTo(OutputStream& stream) const {
    stream.write((int)(tiles.size()));
    for (const std::vector<Tile>& tilesElement : tiles) {
        stream.write((int)(tilesElement.size()));
        for (const Tile& tilesElementElement : tilesElement) {
            stream.write((int)(tilesElementElement));
        }
    }
}
std::string Level::toString() const {
    return std::string("Level") + "(" +
        "TODO" + 
        ")";
}


optional<Vec2Float> Level::crossWall(const Vec2Float &p1, const Vec2Float &p2) const {
    for (auto const & wall : walls) {
        int wallLines = wall.size();

        for (int i = 1; i < wallLines; ++i) {
            if (auto crossPoint = Geometry::doIntersect(wall[i - 1], wall[i], p1, p2)) {
                return crossPoint;
            }
        }

        if (auto crossPoint = Geometry::doIntersect(wall[wallLines - 1], wall[0], p1, p2)) {
            return crossPoint;
        }
    }

    return nullopt;
}