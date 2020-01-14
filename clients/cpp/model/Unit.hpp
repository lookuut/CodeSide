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
#include <unordered_set>

using namespace std;

class Unit {

private:
    void updateTilePos();

public:

    Properties * properties;
    Level * level;

    int playerId;
    int id;
    int health;
    double widthHalf;

    Vec2Double position;
    Vec2Double prevPosition;

    Vec2Float leftTop;
    Vec2Float rightDown;

    int onGroundLadderTicks = 0;

    int posTileX;
    int posTileY;

    int leftPosTileX;
    int rightPosTileX;

    int topTileY;
    int meanTileY;

    int minUpDeltaTileY;
    int lastTouchedJumpPad = -1;
    int lastTouchedJumpPadPart = -1;
    Vec2Double size;

    JumpState jumpState;

    bool walkedRight;
    bool stand;
    bool onGround;
    bool onLadder;

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

    Unit(const Unit & unit);

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
    bool isOnPlatform()const;
    bool isOnJumpPad();
    void heatRoofRoutine();

    void horizontalWallCollision(double velocity);
    void verticalWallCollision();
    void platformCollision();

    void applyJumpPad(double speed, double maxTime);
    void applyOnGround();
    void applyJump(double speed, double maxTime);
    void applyJumpCancel();
    void applyAction(const UnitAction & action, vector<Unit> & units);
    int collisionWithWallMicrotick(const UnitAction & action);

    void applyActionMicroticks(const UnitAction & action, vector<Unit> & units, int microticks);

    bool isInside(const Vec2Double &point) const;
    bool isOverlap(const Bullet & bullet) const;

    bool picUpkHealthPack(const LootBox &lootbox);
    bool pickUpWeapon(const LootBox &lootbox);
    bool picUpkMine(const LootBox &lootbox);

    bool isPickUpLootbox(const LootBox &lootbox) const;
    void weaponRoutine(double time, const Vec2Double & aim);
    void plantMine(vector<Mine> & mines);
    void unitHorCollide(Unit & unit);
    void unitVerCollide(Unit & unit);
    int crossBulletTick(Bullet & bullet, int microticks, const Vec2Double & unitVelocity, const Vec2Double & position);
    pair<double, double> setFrontSegments(const Vec2Double & dir, Vec2Double & frontPoint, const Vec2Double & position) const;

    int crossWithFrontPoint(const Vec2Double &frontUnitPoint,
                            const vector<Vec2Double> & bullets,
                            const Bullet & bullet,
                            const Vec2Double & unitVelocity,
                            const pair<double, double> & unitSegments,
                            int microticks);

    bool checkAim(const Unit & enemy, Vec2Double & aim, const list<int> & allies, const vector<Unit> & units) const;

    int actionType() const;
    bool shootable(const Unit &unit) const;


    pair<int, Vec2Double> chooseEnemy(const list<int> & enemyIds, const vector<Unit> & enemies, int choosenEnemyId) const;

    double enemyValue(const Unit & enemy, bool alreadyChoosed, const Vec2Double & aim) const;

    int stateIndex();
};


#endif
