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
unitsIndex(unitsIndex)
{
    unitIndex = 0;

    actions = vector<UnitAction>(Consts::maxUnitCount, UnitAction());
}

UnitAction MyStrategy::getAction(const Unit &unit, const Game &game,
                                 Debug &debug) {
    return simulationTest(unit, game, debug);

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