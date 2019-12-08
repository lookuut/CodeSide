#ifndef _MODEL_UNIT_HPP_
#define _MODEL_UNIT_HPP_

#include "../Stream.hpp"
#include <string>
#include <stdexcept>
#include "Vec2Double.hpp"
#include "Vec2Double.hpp"
#include "JumpState.hpp"
#include <memory>
#include "Weapon.hpp"
#include "WeaponType.hpp"
#include "WeaponParams.hpp"
#include "BulletParams.hpp"
#include "ExplosionParams.hpp"
#include "UnitAction.hpp"
#include "Tile.hpp"
#include "Level.hpp"
#include "Properties.hpp"
#include <list>
#include "Bullet.hpp"


using namespace std;

class Unit {

private:
    void updateTilePos();

public:

    UnitAction action;

    int playerId;
    int id;
    int health;

    Level level;
    Vec2Double position;

    int posTileX;
    int posTileY;

    int centerPosTileY;

    int leftPosTileX;
    int rightPosTileX;

    int topTileY;
    int meanTileY;

    int minDownDeltaTileY;

    Vec2Double size;

    JumpState jumpState;

    bool walkedRight;
    bool stand;
    bool onGround;
    bool onLadder;
    int mines;
    std::shared_ptr<Weapon> weapon;
    list<UnitAction> actions;

    Unit();
    Unit(
            int playerId,
            int id,
            int health,
            Vec2Double position,
            Vec2Double size,
            JumpState jumpState,
            bool walkedRight,
            bool stand,
            bool onGround,
            bool onLadder,
            int mines,
            std::shared_ptr<Weapon> weapon
            );


    bool equal(const Unit &unit, double eps) const;

    static Unit readFrom(InputStream& stream, const Level &level);
    void writeTo(OutputStream& stream) const;
    std::string toString() const;

    void moveHor(double velocity);
    void updateMoveState();

    void jumping(double velocity, double time);
    void laddering(double vel);
    void downing(double vel);

    bool isOnLadder();
    bool isOnGround();
    bool isOnWall();
    bool isInGround();
    bool isOnPlatform();
    bool isOnJumpPad();
    bool isHeatRoof();

    void horizontalWallCollision(double velocity);
    void verticalWallCollision();

    void applyJumpPad(double speed, double maxTime);
    void applyOnGround();
    void applyJump(double speed, double maxTime);
    void applyJumpCancel();


    bool isInside(const Vec2Double &point)const;
    bool isOverlap(const Bullet & bullet) const;

    UnitAction & getAction();
    void setAction(UnitAction & action);
};

#endif
