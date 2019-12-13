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
#include "LootBox.hpp"
#include "Mine.hpp"


using namespace std;

class Unit {

private:
    void updateTilePos();

    Properties * properties;
    Level * level;

public:

    int playerId;
    int id;
    int health;
    double widthHalf;

    Vec2Double position;
    Vec2Double prevPosition;

    Vec2Float leftTop;
    Vec2Float rightDown;

    int posTileX;
    int posTileY;

    int leftPosTileX;
    int rightPosTileX;

    int topTileY;
    int meanTileY;

    int minUpDeltaTileY;

    Vec2Double size;

    JumpState jumpState;

    bool walkedRight;
    bool stand;
    bool onGround;
    bool onLadder;
    bool onPlatform = false;

    int mines;
    std::shared_ptr<Weapon> weapon;

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

    void init(InputStream &stream, Properties *properties, Level *level);
    void update(InputStream& stream);

    void writeTo(OutputStream& stream) const;
    std::string toString() const;

    void moveHor(double velocity);
    void updateMoveState();

    void jumping(double velocity, double time);
    void downing(double vel);

    bool isOnLadder();
    bool isOnGround();
    bool isOnWall();
    bool isInGround();
    bool isOnPlatform();
    bool isOnJumpPad();
    void heatRoofRoutine();

    void horizontalWallCollision(double velocity);
    void verticalWallCollision();
    void platformCollision();

    void applyJumpPad(double speed, double maxTime);
    void applyOnGround();
    void applyJump(double speed, double maxTime);
    void applyJumpCancel();


    bool isInside(const Vec2Double &point)const;
    bool isOverlap(const Bullet & bullet) const;

    bool picUpkHealthPack(const LootBox &lootbox);
    bool pickUpWeapon(const LootBox &lootbox);
    bool picUpkMine(const LootBox &lootbox);

    bool isPickUpLootbox(const LootBox &lootbox);
    void weaponRoutine(double time, const Vec2Double & aim);
    void plantMine(vector<Mine> & mines);
    void unitHorCollide(Unit & unit);
    void unitVerCollide(Unit & unit);
};


#endif
