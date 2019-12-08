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

MyStrategy::MyStrategy() {}

double distSqr(Vec2Double a, Vec2Double b) {
    return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
}

UnitAction MyStrategy::getAction(const Unit &unit, const Game &game,
                                 Debug &debug) {
    //return simulationTest(unit, game, debug);
    chrono::system_clock::time_point start = chrono::system_clock::now();

    vector<Unit> units = {unit};

    if (game.currentTick == 0) {
        arena = Arena(game.properties, game.level, game.players, units, game.bullets, game.mines, game.lootBoxes);
        allyPlayerId = unit.playerId;
        enemyPlayerId = (allyPlayerId != game.players[0].id ? game.players[0].id : game.players[1].id);
        properties = game.properties;
        unitIndex = 0;
    } else {
        arena.bullets = game.bullets;
        arena.units = units;
        arena.lootBoxes = game.lootBoxes;
        arena.mines = game.mines;
    }


    UnitAction action = bestAction();
    shootLogic(action, unit, game, debug);

    chrono::system_clock::time_point end = chrono::system_clock::now();
    long micros = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    cout << "Tick " << game.currentTick << "perfomance" << micros << endl;

    return action;
}


UnitAction MyStrategy::simulationTest(const Unit &unit, const Game &game, Debug &debug) {

    cout.precision(17);
    vector<Unit> units = {unit};

    if (game.currentTick == 0) {
        arena = Arena(game.properties, game.level, game.players, units, game.bullets, game.mines, game.lootBoxes);
        properties = game.properties;
    }

    if (arena.bullets.size() < game.bullets.size()) {
        arena.bullets = game.bullets;
    }

    cout << (arena.units.front().position.x - unit.position.x) << " " << (arena.units.front().position.y - unit.position.y) << endl;

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

    UnitAction action;
    action.shoot = false;

    action.aim = ZERO_VEC_2_DOUBLE;
    action.reload = false;
    action.swapWeapon = false;
    action.plantMine = false;

    action.jump = true;
    action.jumpDown = !action.jump;
    action.velocity = -properties.unitMaxHorizontalSpeed;
    arena.units.front().setAction(action);

    arena.tick();

    for (const Bullet & bullet : arena.bullets) {
        Vec2Float p = Vec2Float(bullet.position.x - bullet.size / 2.0, bullet.position.y - bullet.size / 2.0);
        Vec2Float s = Vec2Float(bullet.size, bullet.size);
        debug.draw(CustomData::Rect(p, s, ColorFloat(0.0,1.0,0.0, 1.0)));
    }

    return action;
}

UnitAction MyStrategy::bestAction() {

    bool canJump = arena.units[unitIndex].jumpState.canJump;
    bool canCancel = arena.units[unitIndex].jumpState.canCancel;

    UnitAction action;
    action.shoot = false;

    action.jump = false;
    action.jumpDown = !action.jump;
    action.aim = ZERO_VEC_2_DOUBLE;
    action.reload = false;
    action.swapWeapon = false;
    action.plantMine = false;

    UnitAction bestAction = action;

    double maxEvaluationValue = 0;

    if (canCancel) {
        action.jump = false;
        action.jumpDown = !action.jump;

        action.velocity = properties.unitMaxHorizontalSpeed;
        arena.units[unitIndex].setAction(action);

        double evaluationValue = simulation(arena, 1, unitIndex);

        if (evaluationValue > maxEvaluationValue) {
            maxEvaluationValue = evaluationValue;
            bestAction = action;
        }
        action.velocity = -properties.unitMaxHorizontalSpeed;
        arena.units[unitIndex].setAction(action);

        evaluationValue = simulation(arena, 1, unitIndex);

        if (evaluationValue > maxEvaluationValue) {
            maxEvaluationValue = evaluationValue;
            bestAction = action;
        }
    }

    if (canJump) {
        action.jump = true;
        action.jumpDown = !action.jump;
        action.velocity = properties.unitMaxHorizontalSpeed;
        arena.units[unitIndex].setAction(action);

        double evaluationValue = simulation(arena, 1, unitIndex);

        if (evaluationValue > maxEvaluationValue) {
            maxEvaluationValue = evaluationValue;
            bestAction = action;
        }

        action.velocity = -properties.unitMaxHorizontalSpeed;
        arena.units[unitIndex].setAction(action);

        evaluationValue = simulation(arena, 1, unitIndex);

        if (evaluationValue > maxEvaluationValue) {
            maxEvaluationValue = evaluationValue;
            bestAction = action;
        }
    }

    if (!canJump and !canCancel) {
        action.jump = false;
        action.jumpDown = !action.jump;

        action.velocity = properties.unitMaxHorizontalSpeed;
        arena.units[unitIndex].setAction(action);

        double evaluationValue = simulation(arena, 1, unitIndex);

        if (evaluationValue > maxEvaluationValue) {
            maxEvaluationValue = evaluationValue;
            bestAction = action;
        }

        action.velocity = -properties.unitMaxHorizontalSpeed;
        arena.units[unitIndex].setAction(action);

        evaluationValue = simulation(arena, 1, unitIndex);

        if (evaluationValue > maxEvaluationValue) {
            maxEvaluationValue = evaluationValue;
            bestAction = action;
        }
    }

    cout << "Best action " << bestAction.toString() << endl;

    return bestAction;
}

double MyStrategy::simulation(Arena arena, int deep, int unitIndex) {


    for (int i = 0; i < Consts::simulationTicks; ++i) {
        arena.tick();
    }

    if (deep >= Consts::maxSimulationDeep) {
        return evaluation(arena.units.front(), Game::getInstance());
    }

    bool canJump = arena.units[unitIndex].jumpState.canJump;
    bool canCancel = arena.units[unitIndex].jumpState.canCancel;

    UnitAction action;
    action.shoot = false;

    action.jump = false;
    action.jumpDown = !action.jump;
    action.aim = ZERO_VEC_2_DOUBLE;
    action.reload = false;
    action.swapWeapon = false;
    action.plantMine = false;

    double evaluationValue = 0;
    int simulationCount = 0;

    if (canJump) {
        action.jump = true;
        action.jumpDown = !action.jump;
        action.velocity = properties.unitMaxHorizontalSpeed;
        arena.units[unitIndex].setAction(action);

        evaluationValue = max(evaluationValue, simulation(arena, deep + 1, unitIndex));

        action.velocity = -properties.unitMaxHorizontalSpeed;
        arena.units[unitIndex].setAction(action);

        evaluationValue = max(evaluationValue, simulation(arena, deep + 1, unitIndex));
        simulationCount += 2;
    }

    if (canCancel) {
        action.jump = false;
        action.jumpDown = !action.jump;

        action.velocity = properties.unitMaxHorizontalSpeed;
        arena.units[unitIndex].setAction(action);

        evaluationValue = max(evaluationValue, simulation(arena, deep + 1, unitIndex));

        action.velocity = -properties.unitMaxHorizontalSpeed;
        arena.units[unitIndex].setAction(action);

        evaluationValue = max(evaluationValue, simulation(arena, deep + 1, unitIndex));
        simulationCount += 2;
    }


    if (!canJump and !canCancel) {
        action.jump = false;
        action.jumpDown = !action.jump;

        action.velocity = properties.unitMaxHorizontalSpeed;
        arena.units[unitIndex].setAction(action);

        evaluationValue = max(evaluationValue, simulation(arena, deep + 1, unitIndex));

        action.velocity = -properties.unitMaxHorizontalSpeed;
        arena.units[unitIndex].setAction(action);

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

    const LootBox *nearestWeapon = nullptr;

    for (const LootBox &lootBox : game.lootBoxes) {
        if (std::dynamic_pointer_cast<Item::Weapon>(lootBox.item)) {
            if (nearestWeapon == nullptr ||
                unit.position.distSqr(lootBox.position) < unit.position.distSqr(nearestWeapon->position)) {
                nearestWeapon = &lootBox;
            }
        }
    }

    if (unit.weapon == nullptr && nearestWeapon != nullptr) {
        action.velocity = sgn(nearestWeapon->position.x - unit.position.x) * game.properties.unitMaxHorizontalSpeed;
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

            auto lowLineCross = game.level.crossWall(unitCenter, unitCenter + lowLine);
            auto topLineCross = game.level.crossWall(unitCenter, unitCenter + topLine);

            if (lowLineCross) {
                action.shoot = (lowLineCross.value() - enemyCenter).sqrLen() < (lowLineCross.value() - unitCenter).sqrLen();
            }

            if (topLineCross) {
                action.shoot = (topLineCross.value() - enemyCenter).sqrLen() < (topLineCross.value() - unitCenter).sqrLen();
            }

        }
    }

    action.aim = aim;
}

double MyStrategy::evaluation(const Unit &unit, Game &game) {
    Unit * enemy = game.playerUnits[enemyPlayerId].front();

    double distanceFactor = 1.0 / (enemy->position - unit.position).len();

    return unit.health + (unit.health - enemy->health >= 0 ? 1 : -1) * distanceFactor;
}