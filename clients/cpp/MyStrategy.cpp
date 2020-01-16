#include "MyStrategy.hpp"
#include "utils/Geometry.h"
#include <iostream>
#include "Consts.h"
#include <math.h>

#include <chrono>
#include <ctime>

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}


using namespace std;

MyStrategy::MyStrategy(
        Game * game,
        Debug * debug
        ):
properties(&game->properties),
level(&game->level),
allyPlayerId(game->allyPlayerId),
enemyPlayerId(game->enemyPlayerId),
arena(game, debug),
minMax(game, debug),
game(game),
debug(debug),
enemies(game->enemies),
weapons(game->lootWeapons),
mines(game->lootMines),
healthPacks(game->lootHealthPacks)
{
    actions = vector<UnitAction>(Consts::maxUnitCount, UnitAction());

    unitTarget = vector<LootBox>(game->properties.teamSize, LootBox());
    unitTargetId = vector<int>(game->properties.teamSize, -1);
    unitTargetType = vector<int>(game->properties.teamSize, 0);
    suicideMines = vector<int>(game->properties.teamSize, -1);

    for (int unitId : game->aliveAllyUnits) {
        updateUnitPath(weapons, unitId, false);
    }


}

void MyStrategy::updateUnitPath(vector<LootBox> & targets, int unitId, bool isEnemy) {

    vector<AstarNode> unitPath;

    if (game->properties.teamSize == 2) {
        unitPath = game->unitAstarPath[Game::allyUnitIndexById(game->getNonEqualUnitId(unitId))];
    }

    int lootId = game->aStarPathInit(unitId, targets, *debug, unitPath, isEnemy);

    if (lootId < 0) {
        cout << "Cant find path for unit " << unitId << endl;
    } else {

        unitTargetId[Game::allyUnitIndexById(unitId)] = lootId;
        unitTarget[Game::allyUnitIndexById(unitId)] = targets[lootId];

        if (!isEnemy or targets.size() > 1) {
            targets.erase(targets.begin() + lootId);
        }
    }
}


bool MyStrategy::canUpdateUnitPath(int unitId) {
    if (game->newBullet) {
        return true;
    }

    if (game->unitAstarPath[Game::allyUnitIndexById(unitId)].size() <= 1) {
        return true;
    }

    Unit & unit = game->units[Game::unitIndexById(unitId)];
    const Unit & trace = game->unitAstarPath[Game::allyUnitIndexById(unitId)][0].getConstUnit();

    if (trace.position.distSqr(unit.position) > 0.001) {
        return true;
    }

    return false;
}

void MyStrategy::tick(Debug &debug) {

    chrono::system_clock::time_point start = chrono::system_clock::now();

    enemies = game->enemies;
    weapons = game->lootWeapons;
    mines = game->lootMines;
    healthPacks = game->lootHealthPacks;

    vector<int> noUpdateUnits;

    for (int unitId : game->aliveAllyUnits) {
        if (!canUpdateUnitPath(unitId) and unitTargetId[Game::allyUnitIndexById(unitId)] >= 0) {
            noUpdateUnits.push_back(unitId);
        }
    }

    for (int unitId : noUpdateUnits) {
        Unit & unit = game->units[Game::unitIndexById(unitId)];

        int target = unitTargetId[Game::allyUnitIndexById(unitId)];

        if (unit.weapon == nullptr) {
            weapons.erase(weapons.begin() + target);
        } else if (unit.health <= 50 and healthPacks.size() > 0) {
            healthPacks.erase(healthPacks.begin() + target);
        }
    }

    for (int unitId : game->aliveAllyUnits) {
        Unit & unit = game->units[Game::unitIndexById(unitId)];

        if (canUpdateUnitPath(unitId)) {
            if (unit.weapon == nullptr) {
                unitTargetType[Game::allyUnitIndexById(unitId)] = 0;
                updateUnitPath(weapons, unitId, false);
            } else if (unit.health <= 50 and healthPacks.size() > 0) {
                unitTargetType[Game::allyUnitIndexById(unitId)] = 2;
                updateUnitPath(healthPacks, unitId, false);
            } else {
                unitTargetType[Game::allyUnitIndexById(unitId)] = 3;
                updateUnitPath(enemies, unitId, true);
            }
        }
    }

    minMax.generateBestAction(*game, debug);

    chrono::system_clock::time_point end = chrono::system_clock::now();
    long micros = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    cout << "Tick " << game->currentTick << " perfomance " << micros << endl;
}

void MyStrategy::suicide(const Unit &unit, UnitAction & action) {

    if (suicideMines[Game::allyUnitIndexById(unit.id)] == 0) {
        action.velocity = 0;
        action.jump = false;
        action.jumpDown = false;
        action.aim = Vec2Double(0, -1);
        action.shoot = true;
        return;
    }

    if (suicideMines[Game::allyUnitIndexById(unit.id)] > 0) {
        action.plantMine = true;
        action.velocity = 0;
        action.jump = false;
        action.jumpDown = false;
        action.aim = Vec2Double(0, -1);
        suicideMines[Game::allyUnitIndexById(unit.id)]--;
        return;
    }


    if (unit.mines <= 0 or !unit.canPlantMine() or unit.weapon == nullptr) {
        return;
    }

    int ticks = ceil(unit.weapon.get()->fireTimer * properties->ticksPerSecond);

    if (ticks > 1) {
        return;
    }

    Vec2Float leftTop(unit.position.x - properties->mineExplosionParams.radius, unit.position.y + properties->mineExplosionParams.radius);
    Vec2Float rightDown(unit.position.x + properties->mineExplosionParams.radius, unit.position.y - properties->mineExplosionParams.radius);

    vector<int> explodedUnits;
    for (int unitId : game->aliveUnits) {
        Unit & unit = game->units[Game::unitIndexById(unitId)];

        if (Geometry::isRectOverlap(leftTop, rightDown, unit.leftTop, unit.rightDown)) {
            explodedUnits.push_back(unit.id);
        }
    }

    if (explodedUnits.size() == 1) {
        return;
    }

    int allyDamageTaken = 0;
    int enemyDamageTaken = 0;
    int allyScore = 0;
    int enemyScore = 0;

    suicideMines[Game::allyUnitIndexById(unit.id)] = -1;

    for (int i = 0; i < min(unit.mines, 2); ++i) {

        for (int unitId : explodedUnits) {
            Unit & unit = game->units[Game::unitIndexById(unitId)];

            if (unit.health <= 0) {
                continue;
            }

            int damageScore = min(properties->mineExplosionParams.damage, unit.health);

            unit.health -= properties->mineExplosionParams.damage;

            if (unit.playerId == game->allyPlayerId) {
                enemyScore += (unit.health <= 0 ? properties->killScore : 0);
                allyDamageTaken += damageScore;
            } else {
                allyScore += (unit.health <= 0 ? properties->killScore : 0);
                allyScore += damageScore;
                enemyDamageTaken += damageScore;
            }
        }

        if ((allyDamageTaken <= enemyDamageTaken and allyScore >= properties->killScore and allyScore > enemyScore and game->players[game->allyPlayerId].score + allyScore > game->players[game->enemyPlayerId].score + enemyScore)) {
            suicideMines[Game::allyUnitIndexById(unit.id)]++;

            action.plantMine = true;
            action.velocity = 0;
            action.jump = false;
            action.jumpDown = false;
            action.aim = Vec2Double(0, -1);
            action.shoot = false;
        }
    }
}

UnitAction MyStrategy::getAction(const Unit &unit, Debug & debug) {

    vector<AstarNode> & unitPath = game->unitAstarPath[Game::allyUnitIndexById(unit.id)];

    UnitAction action;

    if (unitPath.size() > 1 and unit.health <= unitPath.back().getConstUnit().health and !(unitTargetType[Game::allyUnitIndexById(unit.id)] == 3 and unit.shootable(unitTarget[Game::allyUnitIndexById(unit.id)]))) {
        action.velocity = unitPath[1].vel;
        action.jump = unitPath[1].jump;
        action.jumpDown = unitPath[1].jumpDown;
        unitPath.erase(unitPath.begin());
    } else {
        action = minMax.getBestAction(unit);
    }

    minMax.canShoot(game->aliveAllyUnits, game->aliveEnemyUnits, game->units, unit, action);

    if (unit.weapon != nullptr and unit.weapon.get()->type == WeaponType::ROCKET_LAUNCHER) {
        action.swapWeapon = true;
    }

    if (unit.weapon != nullptr and unit.weapon.get()->type != WeaponType::ASSAULT_RIFLE) {
        for (int weaponId : game->lootWeaponAssultIds) {
            const LootBox & lootBox = game->lootWeapons[weaponId];
            if (unit.isPickUpLootbox(lootBox)) {
                action.swapWeapon = true;
            }
        }
    }

    return action;
}
//return simulationTest(world, game, debug);

bool MyStrategy::simulationEqualTests(const Unit &simulatedUnit, const Unit &unit, const Game &game) const {
    int goThroughStarisBugTick = 24803;
    int pickUpLootBugTick1 = 63943;
    int pickUpLootBugTick2 = 71322;

    if (game.currentTick == goThroughStarisBugTick or game.currentTick == pickUpLootBugTick1 or game.currentTick == pickUpLootBugTick2) {
        return true;
    }

    if (!simulatedUnit.equal(unit, Consts::eps)) {
        cout << "Not equal" << endl;
        cout << unit.toString() << endl;
        cout << simulatedUnit.toString() << endl;
        return false;
    }

    if (((simulatedUnit.weapon == nullptr and unit.weapon != nullptr) or (simulatedUnit.weapon != nullptr and unit.weapon == nullptr))) {
        return false;
    }


    if (unit.weapon != nullptr and !simulatedUnit.weapon.get()->equal(*unit.weapon.get(), Consts::eps)) {
        return false;
    }

    for (int i = game.mines.size() - 1; i >= 0; --i) {
        if (!arena.mines[i].equal(game.mines[i], Consts::eps)) {
            return false;
        }
    }

    if (arena.lootWeapons.size() != game.lootWeapons.size()) {
        return false;
    }

    if (arena.lootMines.size() != game.lootMines.size()) {
        return false;
    }

    if (arena.lootHealthPacks.size() != game.lootHealthPacks.size()) {
        return false;
    }

    return true;
}


UnitAction MyStrategy::simulationTest(const Unit &unit, const Game &game, Debug &debug) {
    //Seed 34334423
    cout.precision(17);

    if (game.currentTick == 0) {
        //arena.update(game.allyUnits, game.bullets, game.mines, game.lootHealthPacks, game.lootWeapons, game.lootMines);
    }

    //Unit & simulatedUnit = arena.allyUnits[unitsIndex[Game::unitIndexById(world.id)]];
    Unit  simulatedUnit;

    if (!simulationEqualTests(simulatedUnit, unit, game)) {
        cout << "Simulation not equal";
    }

    actions[Game::unitIndexById(unit.id)].shoot = false;

    Vec2Double aim = Vec2Double(100, 0);
    aim.normalize();

    actions[Game::unitIndexById(unit.id)].aim = aim;
    actions[Game::unitIndexById(unit.id)].reload = false;
    actions[Game::unitIndexById(unit.id)].swapWeapon = false;
    actions[Game::unitIndexById(unit.id)].plantMine = true;

    actions[Game::unitIndexById(unit.id)].jump = false;
    actions[Game::unitIndexById(unit.id)].jumpDown = !actions[Game::unitIndexById(unit.id)].jump;

    if (game.currentTick < 50) {
        actions[Game::unitIndexById(unit.id)].velocity = -properties->unitMaxHorizontalSpeed;
    }

    if (game.currentTick > 3000) {
        actions[Game::unitIndexById(unit.id)].jump = true;
        actions[Game::unitIndexById(unit.id)].jumpDown = !actions[Game::unitIndexById(unit.id)].jump;
    }

    if (game.currentTick > 15000) {
        actions[Game::unitIndexById(unit.id)].jump = false;
        actions[Game::unitIndexById(unit.id)].jumpDown = !actions[Game::unitIndexById(unit.id)].jump;
        actions[Game::unitIndexById(unit.id)].velocity = properties->unitMaxHorizontalSpeed;
    }

    if (game.currentTick > 17000) {
        actions[Game::unitIndexById(unit.id)].jump = true;
        actions[Game::unitIndexById(unit.id)].jumpDown = !actions[Game::unitIndexById(unit.id)].jump;
        actions[Game::unitIndexById(unit.id)].velocity = properties->unitMaxHorizontalSpeed;
    }

    if (game.currentTick > 17638) {
        actions[Game::unitIndexById(unit.id)].jump = true;
        actions[Game::unitIndexById(unit.id)].jumpDown = !actions[Game::unitIndexById(unit.id)].jump;
        actions[Game::unitIndexById(unit.id)].velocity = 0;
    }

    if (game.currentTick > 20600) {
        actions[Game::unitIndexById(unit.id)].jump = false;
        actions[Game::unitIndexById(unit.id)].jumpDown = false;
        actions[Game::unitIndexById(unit.id)].velocity = -properties->unitMaxHorizontalSpeed;
    }

    if (unit.weapon) {
        actions[Game::unitIndexById(unit.id)].aim = Vec2Double(game.currentTick % 6 + 1, 3).normalize();
        actions[Game::unitIndexById(unit.id)].shoot = true;
        actions[Game::unitIndexById(unit.id)].velocity = -properties->unitMaxHorizontalSpeed;
    }

    if (game.mines.size() > 0) {
        actions[Game::unitIndexById(unit.id)].velocity = 0;
    }

    if (game.currentTick > 45000) {
        actions[Game::unitIndexById(unit.id)].velocity = properties->unitMaxHorizontalSpeed;
    }

    if (game.currentTick > 46000) {
        actions[Game::unitIndexById(unit.id)].velocity = 0;
    }

    if (game.currentTick > 55000) {
        actions[Game::unitIndexById(unit.id)].jump = true;
        actions[Game::unitIndexById(unit.id)].jumpDown = true;
        actions[Game::unitIndexById(unit.id)].velocity = properties->unitMaxHorizontalSpeed;
    }


    if (game.currentTick > 66054) {
        actions[Game::unitIndexById(unit.id)].jump = false;
        actions[Game::unitIndexById(unit.id)].jumpDown = false;
        actions[Game::unitIndexById(unit.id)].velocity = 0;
    }

    if (game.currentTick > 66554) {
        actions[Game::unitIndexById(unit.id)].jump = false;
        actions[Game::unitIndexById(unit.id)].jumpDown = false;
        actions[Game::unitIndexById(unit.id)].velocity = properties->unitMaxHorizontalSpeed;
    }

    if (game.currentTick > 70000) {
        actions[Game::unitIndexById(unit.id)].jump = true;
        actions[Game::unitIndexById(unit.id)].jumpDown = false;
        actions[Game::unitIndexById(unit.id)].velocity = properties->unitMaxHorizontalSpeed;
    }

    //arena.tick(actions, 1);

    return actions[Game::unitIndexById(unit.id)];
}

/*
    Unit testUnit(world);
    testUnit.position.x = 32.820000000005315;
    testUnit.position.y = 19.503333333336659;

    Vec2Double unitVelocity(-9.9999999999998579, -10.000000000000071);

    Bullet bullet(
            WeaponType::ASSAULT_RIFLE,
            1,
            1,
            Vec2Double(31.406203413355335, 19.635309864153797),
            Vec2Double(49.90902560021491, 3.0148239811126674),
            5,
            0.20000000000000001,
            nullptr);

    testUnit.crossBulletTick(bullet, 100, unitVelocity, testUnit.position);*/
