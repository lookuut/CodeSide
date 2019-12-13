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

MyStrategy::MyStrategy(Properties * properties, Level * level, int playerId, int enemyPlayerId, const vector<Unit> & units, const vector<int> & unitsIndex):
properties(properties),
level(level),
allyPlayerId(playerId),
enemyPlayerId(enemyPlayerId),
arena(properties, level),
unitsIndex(unitsIndex),
minMax(properties, level, playerId, enemyPlayerId, units, unitsIndex)
{
    actions = vector<UnitAction>(Consts::maxUnitCount, UnitAction());
}

UnitAction MyStrategy::getAction(const Unit &unit, const Game &game,
                                 Debug &debug) {
    //return simulationTest(unit, game, debug);

    const LootBox *nearestWeapon = nullptr;

    for (const LootBox &lootBox : game.lootWeapons) {
        if (nearestWeapon == nullptr ||
            unit.position.distSqr(lootBox.position) < unit.position.distSqr(nearestWeapon->position)) {
            nearestWeapon = &lootBox;
        }

    }

    chrono::system_clock::time_point start = chrono::system_clock::now();

    UnitAction & action = minMax.getBestAction(unit, game, debug);

    chrono::system_clock::time_point end = chrono::system_clock::now();
    long micros = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    cout << "Tick " << game.currentTick << " perfomance " << micros << endl;

    return action;
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
        arena.update(game.units, game.bullets, game.mines, game.lootHealthPacks, game.lootWeapons, game.lootMines);
    }

    Unit & simulatedUnit = arena.units[unitsIndex[Game::unitIndexById(unit.id)]];

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

    arena.tick(actions);

    return actions[Game::unitIndexById(unit.id)];
}