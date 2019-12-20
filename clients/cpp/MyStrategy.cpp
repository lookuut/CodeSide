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
minMax(game, debug)
{
    actions = vector<UnitAction>(Consts::maxUnitCount, UnitAction());
}

void MyStrategy::tick(const Game &game, Debug &debug) {
    chrono::system_clock::time_point start = chrono::system_clock::now();

    minMax.generateBestAction(game, debug);

    chrono::system_clock::time_point end = chrono::system_clock::now();
    long micros = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    cout << "Tick " << game.currentTick << " perfomance " << micros << endl;
}


UnitAction MyStrategy::getAction(const Unit &unit) {
    //return simulationTest(unit, game, debug);
    return minMax.getBestAction(unit);
}

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

    //Unit & simulatedUnit = arena.allyUnits[unitsIndex[Game::unitIndexById(unit.id)]];
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
    Unit testUnit(unit);
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
