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

MyStrategy::MyStrategy(Properties * properties, Level * level, int playerId, int enemyPlayerId, const vector<Unit> & units):
properties(properties),
level(level),
allyPlayerId(playerId),
enemyPlayerId(enemyPlayerId),
arena(properties, level)
{
    unitIndex = 0;

    actions = vector<UnitAction>(5, UnitAction());
}

UnitAction MyStrategy::getAction(const Unit &unit, const Game &game,
                                 Debug &debug) {
    //return simulationTest(unit, game, debug);

    vector<Unit> units = {unit};
    arena.update(units, game.bullets, game.mines, game.lootHealthPacks, game.lootWeapons, game.lootMines);

    const LootBox *nearestWeapon = nullptr;

    for (const LootBox &lootBox : game.lootWeapons) {
        if (nearestWeapon == nullptr ||
            unit.position.distSqr(lootBox.position) < unit.position.distSqr(nearestWeapon->position)) {
            nearestWeapon = &lootBox;
        }

    }

    nearestWeaponPos = nearestWeapon->position;

    chrono::system_clock::time_point start = chrono::system_clock::now();

    UnitAction action = bestAction();
    shootLogic(action, unit, game, debug);

    chrono::system_clock::time_point end = chrono::system_clock::now();
    long micros = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    cout << "Tick " << game.currentTick << " perfomance " << micros << endl;

    return action;
}


UnitAction MyStrategy::simulationTest(const Unit &unit, const Game &game, Debug &debug) {

    cout.precision(17);
    vector<Unit> units = {unit};

    if (game.currentTick == 0) {
        arena.update(units, game.bullets, game.mines, game.lootHealthPacks, game.lootWeapons, game.lootMines);
    }

    if (arena.bullets.size() < game.bullets.size()) {
        arena.bullets = game.bullets;
    }

    cout << (arena.units.front().position.x - unit.position.x) << " " << (arena.units.front().position.y - unit.position.y) << endl;
    cout << arena.units.front().position.y << " " << unit.position.y << endl;


    if (!arena.units.front().equal(unit, Consts::eps)) {
        cout << "Not equal" << endl;
        cout << unit.toString() << endl;
        cout << arena.units.front().toString() << endl;
    }

    if (arena.bullets.size() != game.bullets.size()) {
        cout << "Bullets count not equal";
    }

    for (int i = arena.bullets.size() - 1; i >= 0; --i) {
        if (!arena.bullets[i].equal(game.bullets[i], Consts::eps)) {
            cout << arena.bullets[i].toString() << endl;
            cout << game.bullets[i].toString() << endl;
        }
    }


    if (arena.lootWeapons.size() != game.lootWeapons.size()) {
        cout << "Not equal";
    }

    if (arena.lootMines.size() != game.lootMines.size()) {
        cout << "Not equal";
    }

    if (arena.lootHealthPacks.size() != game.lootHealthPacks.size()) {
        cout << "Not equal";
    }


    actions[unit.id].shoot = false;

    actions[unit.id].aim = ZERO_VEC_2_DOUBLE;
    actions[unit.id].reload = false;
    actions[unit.id].swapWeapon = false;
    actions[unit.id].plantMine = false;

    actions[unit.id].jump = true;
    actions[unit.id].jumpDown = !actions[unit.id].jump;
    actions[unit.id].velocity = (game.currentTick % 17000 < 8000 ? -properties->unitMaxHorizontalSpeed : properties->unitMaxHorizontalSpeed);

    arena.tick(actions);

    return actions[unit.id];
}

UnitAction MyStrategy::bestAction() {

    int unitId = arena.units[unitIndex].id;
    bool canJump = arena.units[unitIndex].jumpState.canJump;
    bool canCancel = arena.units[unitIndex].jumpState.canCancel;

    actions[unitId].shoot = false;

    actions[unitId].jump = false;
    actions[unitId].jumpDown = !actions[unitId].jump;
    actions[unitId].aim = ZERO_VEC_2_DOUBLE;
    actions[unitId].reload = false;
    actions[unitId].swapWeapon = false;
    actions[unitId].plantMine = false;

    UnitAction bestAction = actions[unitId];

    double maxEvaluationValue = 0;

    if (canCancel) {
        actions[unitId].jump = false;
        actions[unitId].jumpDown = !actions[unitId].jump;
        actions[unitId].velocity = properties->unitMaxHorizontalSpeed;

        double evaluationValue = simulation(arena, 1, unitIndex);

        if (evaluationValue > maxEvaluationValue) {
            maxEvaluationValue = evaluationValue;
            bestAction.jump = false;
            bestAction.jumpDown = !bestAction.jump;
            bestAction.velocity = properties->unitMaxHorizontalSpeed;
        }

        actions[unitId].jump = false;
        actions[unitId].jumpDown = !actions[unitId].jump;
        actions[unitId].velocity = -properties->unitMaxHorizontalSpeed;

        evaluationValue = simulation(arena, 1, unitIndex);

        if (evaluationValue > maxEvaluationValue) {
            maxEvaluationValue = evaluationValue;
            bestAction.jump = false;
            bestAction.jumpDown = !bestAction.jump;
            bestAction.velocity = -properties->unitMaxHorizontalSpeed;
        }
    }

    if (canJump) {
        actions[unitId].jump = true;
        actions[unitId].jumpDown = !actions[unitId].jump;
        actions[unitId].velocity = properties->unitMaxHorizontalSpeed;

        double evaluationValue = simulation(arena, 1, unitIndex);

        if (evaluationValue > maxEvaluationValue) {
            maxEvaluationValue = evaluationValue;
            bestAction.jump = true;
            bestAction.jumpDown = !bestAction.jump;
            bestAction.velocity = properties->unitMaxHorizontalSpeed;
        }

        actions[unitId].jump = true;
        actions[unitId].jumpDown = !actions[unitId].jump;
        actions[unitId].velocity = -properties->unitMaxHorizontalSpeed;

        evaluationValue = simulation(arena, 1, unitIndex);

        if (evaluationValue > maxEvaluationValue) {
            maxEvaluationValue = evaluationValue;
            bestAction.jump = true;
            bestAction.jumpDown = !bestAction.jump;
            bestAction.velocity = -properties->unitMaxHorizontalSpeed;
        }
    }

    if (!canJump and !canCancel) {

        actions[unitId].jump = false;
        actions[unitId].jumpDown = !actions[unitId].jump;
        actions[unitId].velocity = properties->unitMaxHorizontalSpeed;

        double evaluationValue = simulation(arena, 1, unitIndex);

        if (evaluationValue > maxEvaluationValue) {
            maxEvaluationValue = evaluationValue;
            bestAction.jump = false;
            bestAction.jumpDown = !bestAction.jump;
            bestAction.velocity = properties->unitMaxHorizontalSpeed;
        }

        actions[unitId].jump = true;
        actions[unitId].jumpDown = !actions[unitId].jump;
        actions[unitId].velocity = properties->unitMaxHorizontalSpeed;

        evaluationValue = simulation(arena, 1, unitIndex);

        if (evaluationValue > maxEvaluationValue) {
            maxEvaluationValue = evaluationValue;
            bestAction.jump = true;
            bestAction.jumpDown = !bestAction.jump;
            bestAction.velocity = properties->unitMaxHorizontalSpeed;
        }

        actions[unitId].jump = false;
        actions[unitId].jumpDown = !actions[unitId].jump;
        actions[unitId].velocity = -properties->unitMaxHorizontalSpeed;

        evaluationValue = simulation(arena, 1, unitIndex);

        if (evaluationValue > maxEvaluationValue) {
            maxEvaluationValue = evaluationValue;
            bestAction.jump = false;
            bestAction.jumpDown = !bestAction.jump;
            bestAction.velocity = -properties->unitMaxHorizontalSpeed;
        }

        actions[unitId].jump = true;
        actions[unitId].jumpDown = !actions[unitId].jump;
        actions[unitId].velocity = -properties->unitMaxHorizontalSpeed;

        evaluationValue = simulation(arena, 1, unitIndex);

        if (evaluationValue > maxEvaluationValue) {
            maxEvaluationValue = evaluationValue;
            bestAction.jump = true;
            bestAction.jumpDown = !bestAction.jump;
            bestAction.velocity = -properties->unitMaxHorizontalSpeed;
        }
    }

    return bestAction;
}

double MyStrategy::simulation(Arena arena, int deep, int unitIndex) {

    int unitId = arena.units[unitIndex].id;

    for (int i = 0; i < Consts::simulationTicks; ++i) {
        arena.tick(actions);
    }

    if (deep >= Consts::maxSimulationDeep) {
        return evaluation(arena.units.front(), deep * Consts::simulationTicks);
    }

    bool canJump = arena.units[unitIndex].jumpState.canJump;
    bool canCancel = arena.units[unitIndex].jumpState.canCancel;

    double evaluationValue = evaluation(arena.units.front(), deep * Consts::simulationTicks);

    int simulationCount = 0;

    if (canJump) {
        actions[unitId].jump = true;
        actions[unitId].jumpDown = !actions[unitId].jump;
        actions[unitId].velocity = properties->unitMaxHorizontalSpeed;

        evaluationValue = max(evaluationValue, simulation(arena, deep + 1, unitIndex));

        actions[unitId].jump = true;
        actions[unitId].jumpDown = !actions[unitId].jump;
        actions[unitId].velocity = -properties->unitMaxHorizontalSpeed;

        evaluationValue = max(evaluationValue, simulation(arena, deep + 1, unitIndex));
        simulationCount += 2;
    }

    if (canCancel) {
        actions[unitId].jump = false;
        actions[unitId].jumpDown = !actions[unitId].jump;
        actions[unitId].velocity = properties->unitMaxHorizontalSpeed;

        evaluationValue = max(evaluationValue, simulation(arena, deep + 1, unitIndex));

        actions[unitId].jump = false;
        actions[unitId].jumpDown = !actions[unitId].jump;
        actions[unitId].velocity = -properties->unitMaxHorizontalSpeed;

        evaluationValue = max(evaluationValue, simulation(arena, deep + 1, unitIndex));
        simulationCount += 2;
    }


    if (!canJump and !canCancel) {
        actions[unitId].jump = false;
        actions[unitId].jumpDown = !actions[unitId].jump;
        actions[unitId].velocity = properties->unitMaxHorizontalSpeed;

        evaluationValue = max(evaluationValue, simulation(arena, deep + 1, unitIndex));


        actions[unitId].jump = true;
        actions[unitId].jumpDown = !actions[unitId].jump;
        actions[unitId].velocity = properties->unitMaxHorizontalSpeed;

        evaluationValue = max(evaluationValue, simulation(arena, deep + 1, unitIndex));

        actions[unitId].jump = false;
        actions[unitId].jumpDown = !actions[unitId].jump;
        actions[unitId].velocity = -properties->unitMaxHorizontalSpeed;

        evaluationValue = max(evaluationValue, simulation(arena, deep + 1, unitIndex));

        actions[unitId].jump = false;
        actions[unitId].jumpDown = !actions[unitId].jump;
        actions[unitId].velocity = -properties->unitMaxHorizontalSpeed;

        evaluationValue = max(evaluationValue, simulation(arena, deep + 1, unitIndex));
        simulationCount += 2;
    }

    return evaluationValue;
}


void MyStrategy::shootLogic(UnitAction &action, const Unit &unit, const Game &game, Debug &debug) {
    const Unit *nearestEnemy = nullptr;

    for (const Unit &other : game.units) {
        if (other.playerId != unit.playerId) {
            if (nearestEnemy == nullptr ||
                unit.position.distSqr(other.position) < unit.position.distSqr(nearestEnemy->position)) {
                nearestEnemy = &other;
            }
        }
    }


    Vec2Double aim = Vec2Double(0, 0);

    action.shoot = true;

    if (nearestEnemy != nullptr and unit.weapon != nullptr) {

        Vec2Float unitCenter = Vec2Float(unit.position + Vec2Double(0, game.properties.unitSize.y / 2.0));
        Vec2Float enemyCenter = Vec2Float(nearestEnemy->position + Vec2Double(0, game.properties.unitSize.y / 2.0));

        aim = Vec2Double(enemyCenter.x - unitCenter.x, enemyCenter.y - unitCenter.y);


        if (game.level.crossWall(unitCenter, enemyCenter)) {
            action.shoot = false;
        } else {
            Vec2Double lowLine = aim.rotate(unit.weapon.get()->spread);
            Vec2Double topLine = aim.rotate(-unit.weapon.get()->spread);

            auto lowLineCross = game.level.crossMiDistanceWall(unitCenter, unitCenter + lowLine);
            auto topLineCross = game.level.crossMiDistanceWall(unitCenter, unitCenter + topLine);

            action.shoot = (lowLineCross ? (lowLineCross.value() - enemyCenter).sqrLen() < (lowLineCross.value() - unitCenter).sqrLen() : true)
                    and
                    (topLineCross ? (topLineCross.value() - enemyCenter).sqrLen() < (topLineCross.value() - unitCenter).sqrLen() : true);
        }
    }

    action.aim = aim;
}

double MyStrategy::evaluation(const Unit &unit, int tick) {

    Unit * enemy = Game::getPlayerUnits(enemyPlayerId).front();

    double distance = (enemy->position - unit.position).len();
    double distanceFactor = 1.0 / distance;

    return (unit.health + (unit.health - enemy->health >= 0 and distance > 3 ? 1 : -1) * distanceFactor + 100 * (unit.weapon != nullptr)) / (double)tick;
}