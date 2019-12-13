//
// Created by lookuut on 13.12.19.
//

#include "MinMax.h"
#include "../model/Game.hpp"

MinMax::MinMax(Properties *properties, Level *level, int playerId, int enemyPlayerId, const vector<Unit> &units,
               const vector<int> &unitsIndex):
        properties(properties),
        level(level),
        allyPlayerId(playerId),
        enemyPlayerId(enemyPlayerId),
        arena(properties, level),
        unitsIndex(unitsIndex)
{
    actions = vector<UnitAction>(Consts::maxUnitCount, UnitAction());
}

double MinMax::evaluation(const Unit &unit, int tick) {
    Unit * enemy = Game::getPlayerUnits(enemyPlayerId).front();

    double distance = (enemy->position - unit.position).len();
    double distanceFactor = 1.0 / distance;

    return (unit.health + (unit.health - enemy->health >= 0 and distance > 3 ? 1 : -1) * distanceFactor + 100 * (unit.weapon != nullptr)) / (double)tick;
}

UnitAction& MinMax::getBestAction(const Unit & unit, const Game & game, Debug & debug) {

    arena.update(game.units, game.bullets, game.mines, game.lootHealthPacks, game.lootWeapons, game.lootMines);

    int unitId = unit.id;
    int unitIndex = unitsIndex[Game::unitIndexById(unit.id)];

    bool canJump = arena.units[unitIndex].jumpState.canJump;
    bool canCancel = arena.units[unitIndex].jumpState.canCancel;

    actions[Game::unitIndexById(unitId)].shoot = false;

    actions[Game::unitIndexById(unitId)].jump = false;
    actions[Game::unitIndexById(unitId)].jumpDown = !actions[Game::unitIndexById(unitId)].jump;
    actions[Game::unitIndexById(unitId)].aim = ZERO_VEC_2_DOUBLE;
    actions[Game::unitIndexById(unitId)].reload = false;
    actions[Game::unitIndexById(unitId)].swapWeapon = false;
    actions[Game::unitIndexById(unitId)].plantMine = false;

    bestAction = actions[Game::unitIndexById(unitId)];

    double maxEvaluationValue = 0;

    if (canCancel) {
        actions[Game::unitIndexById(unitId)].jump = false;
        actions[Game::unitIndexById(unitId)].jumpDown = !actions[Game::unitIndexById(unitId)].jump;
        actions[Game::unitIndexById(unitId)].velocity = properties->unitMaxHorizontalSpeed;

        double evaluationValue = simulation(arena, 1, unitIndex);

        if (evaluationValue > maxEvaluationValue) {
            maxEvaluationValue = evaluationValue;
            bestAction.jump = false;
            bestAction.jumpDown = !bestAction.jump;
            bestAction.velocity = properties->unitMaxHorizontalSpeed;
        }

        actions[Game::unitIndexById(unitId)].jump = false;
        actions[Game::unitIndexById(unitId)].jumpDown = !actions[Game::unitIndexById(unitId)].jump;
        actions[Game::unitIndexById(unitId)].velocity = -properties->unitMaxHorizontalSpeed;

        evaluationValue = simulation(arena, 1, unitIndex);

        if (evaluationValue > maxEvaluationValue) {
            maxEvaluationValue = evaluationValue;
            bestAction.jump = false;
            bestAction.jumpDown = !bestAction.jump;
            bestAction.velocity = -properties->unitMaxHorizontalSpeed;
        }
    }

    if (canJump) {
        actions[Game::unitIndexById(unitId)].jump = true;
        actions[Game::unitIndexById(unitId)].jumpDown = !actions[Game::unitIndexById(unitId)].jump;
        actions[Game::unitIndexById(unitId)].velocity = properties->unitMaxHorizontalSpeed;

        double evaluationValue = simulation(arena, 1, unitIndex);

        if (evaluationValue > maxEvaluationValue) {
            maxEvaluationValue = evaluationValue;
            bestAction.jump = true;
            bestAction.jumpDown = !bestAction.jump;
            bestAction.velocity = properties->unitMaxHorizontalSpeed;
        }

        actions[Game::unitIndexById(unitId)].jump = true;
        actions[Game::unitIndexById(unitId)].jumpDown = !actions[Game::unitIndexById(unitId)].jump;
        actions[Game::unitIndexById(unitId)].velocity = -properties->unitMaxHorizontalSpeed;

        evaluationValue = simulation(arena, 1, unitIndex);

        if (evaluationValue > maxEvaluationValue) {
            maxEvaluationValue = evaluationValue;
            bestAction.jump = true;
            bestAction.jumpDown = !bestAction.jump;
            bestAction.velocity = -properties->unitMaxHorizontalSpeed;
        }
    }

    if (!canJump and !canCancel) {

        actions[Game::unitIndexById(unitId)].jump = false;
        actions[Game::unitIndexById(unitId)].jumpDown = !actions[Game::unitIndexById(unitId)].jump;
        actions[Game::unitIndexById(unitId)].velocity = properties->unitMaxHorizontalSpeed;

        double evaluationValue = simulation(arena, 1, unitIndex);

        if (evaluationValue > maxEvaluationValue) {
            maxEvaluationValue = evaluationValue;
            bestAction.jump = false;
            bestAction.jumpDown = !bestAction.jump;
            bestAction.velocity = properties->unitMaxHorizontalSpeed;
        }

        actions[Game::unitIndexById(unitId)].jump = true;
        actions[Game::unitIndexById(unitId)].jumpDown = !actions[Game::unitIndexById(unitId)].jump;
        actions[Game::unitIndexById(unitId)].velocity = properties->unitMaxHorizontalSpeed;

        evaluationValue = simulation(arena, 1, unitIndex);

        if (evaluationValue > maxEvaluationValue) {
            maxEvaluationValue = evaluationValue;
            bestAction.jump = true;
            bestAction.jumpDown = !bestAction.jump;
            bestAction.velocity = properties->unitMaxHorizontalSpeed;
        }

        actions[Game::unitIndexById(unitId)].jump = false;
        actions[Game::unitIndexById(unitId)].jumpDown = !actions[Game::unitIndexById(unitId)].jump;
        actions[Game::unitIndexById(unitId)].velocity = -properties->unitMaxHorizontalSpeed;

        evaluationValue = simulation(arena, 1, unitIndex);

        if (evaluationValue > maxEvaluationValue) {
            maxEvaluationValue = evaluationValue;
            bestAction.jump = false;
            bestAction.jumpDown = !bestAction.jump;
            bestAction.velocity = -properties->unitMaxHorizontalSpeed;
        }

        actions[Game::unitIndexById(unitId)].jump = true;
        actions[Game::unitIndexById(unitId)].jumpDown = !actions[Game::unitIndexById(unitId)].jump;
        actions[Game::unitIndexById(unitId)].velocity = -properties->unitMaxHorizontalSpeed;

        evaluationValue = simulation(arena, 1, unitIndex);

        if (evaluationValue > maxEvaluationValue) {
            maxEvaluationValue = evaluationValue;
            bestAction.jump = true;
            bestAction.jumpDown = !bestAction.jump;
            bestAction.velocity = -properties->unitMaxHorizontalSpeed;
        }
    }

    shootLogic(bestAction, unit, game, debug);

    return bestAction;
}


double MinMax::simulation(Simulation arena, int deep, int unitIndex) {

    int unitId = arena.units[unitIndex].id;

    for (int i = 0; i < Consts::simulationTicks; ++i) {
        arena.tick(actions);
    }

    if (deep >= Consts::maxSimulationDeep) {
        return evaluation(arena.units[unitIndex], deep * Consts::simulationTicks);
    }

    bool canJump = arena.units[unitIndex].jumpState.canJump;
    bool canCancel = arena.units[unitIndex].jumpState.canCancel;

    double evaluationValue = evaluation(arena.units[unitIndex], deep * Consts::simulationTicks);

    int simulationCount = 0;

    if (canJump) {
        actions[Game::unitIndexById(unitId)].jump = true;
        actions[Game::unitIndexById(unitId)].jumpDown = !actions[Game::unitIndexById(unitId)].jump;
        actions[Game::unitIndexById(unitId)].velocity = properties->unitMaxHorizontalSpeed;

        evaluationValue = max(evaluationValue, simulation(arena, deep + 1, unitIndex));

        actions[Game::unitIndexById(unitId)].jump = true;
        actions[Game::unitIndexById(unitId)].jumpDown = !actions[Game::unitIndexById(unitId)].jump;
        actions[Game::unitIndexById(unitId)].velocity = -properties->unitMaxHorizontalSpeed;

        evaluationValue = max(evaluationValue, simulation(arena, deep + 1, unitIndex));
        simulationCount += 2;
    }

    if (canCancel) {
        actions[Game::unitIndexById(unitId)].jump = false;
        actions[Game::unitIndexById(unitId)].jumpDown = !actions[Game::unitIndexById(unitId)].jump;
        actions[Game::unitIndexById(unitId)].velocity = properties->unitMaxHorizontalSpeed;

        evaluationValue = max(evaluationValue, simulation(arena, deep + 1, unitIndex));

        actions[Game::unitIndexById(unitId)].jump = false;
        actions[Game::unitIndexById(unitId)].jumpDown = !actions[Game::unitIndexById(unitId)].jump;
        actions[Game::unitIndexById(unitId)].velocity = -properties->unitMaxHorizontalSpeed;

        evaluationValue = max(evaluationValue, simulation(arena, deep + 1, unitIndex));
        simulationCount += 2;
    }


    if (!canJump and !canCancel) {
        actions[Game::unitIndexById(unitId)].jump = false;
        actions[Game::unitIndexById(unitId)].jumpDown = !actions[Game::unitIndexById(unitId)].jump;
        actions[Game::unitIndexById(unitId)].velocity = properties->unitMaxHorizontalSpeed;

        evaluationValue = max(evaluationValue, simulation(arena, deep + 1, unitIndex));


        actions[Game::unitIndexById(unitId)].jump = true;
        actions[Game::unitIndexById(unitId)].jumpDown = !actions[Game::unitIndexById(unitId)].jump;
        actions[Game::unitIndexById(unitId)].velocity = properties->unitMaxHorizontalSpeed;

        evaluationValue = max(evaluationValue, simulation(arena, deep + 1, unitIndex));

        actions[Game::unitIndexById(unitId)].jump = false;
        actions[Game::unitIndexById(unitId)].jumpDown = !actions[Game::unitIndexById(unitId)].jump;
        actions[Game::unitIndexById(unitId)].velocity = -properties->unitMaxHorizontalSpeed;

        evaluationValue = max(evaluationValue, simulation(arena, deep + 1, unitIndex));

        actions[Game::unitIndexById(unitId)].jump = false;
        actions[Game::unitIndexById(unitId)].jumpDown = !actions[Game::unitIndexById(unitId)].jump;
        actions[Game::unitIndexById(unitId)].velocity = -properties->unitMaxHorizontalSpeed;

        evaluationValue = max(evaluationValue, simulation(arena, deep + 1, unitIndex));
        simulationCount += 2;
    }

    return evaluationValue;
}


void MinMax::shootLogic(UnitAction &action, const Unit &unit, const Game &game, Debug &debug) {
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

            Vec2Double lowLineFromBulletCenter = aim.rotate(unit.weapon.get()->spread);
            Vec2Double topLineFromBulletCenter = aim.rotate(-unit.weapon.get()->spread);

            Vec2Double bulletLowLineDelta = lowLineFromBulletCenter.getOpponentAngle(unit.weapon.get()->params.bullet.size, false);
            Vec2Double bulletTopLineDelta = topLineFromBulletCenter.getOpponentAngle(unit.weapon.get()->params.bullet.size, true);

            Vec2Double lowLine = bulletLowLineDelta + lowLineFromBulletCenter;
            Vec2Double topLine = bulletTopLineDelta + topLineFromBulletCenter;

            auto lowLineCross = game.level.crossMiDistanceWall(unitCenter + bulletLowLineDelta, unitCenter + lowLine);
            auto topLineCross = game.level.crossMiDistanceWall(unitCenter + bulletTopLineDelta, unitCenter + topLine);

            debug.draw(CustomData::Line( (unitCenter + bulletLowLineDelta),  (unitCenter + lowLine), 0.2, ColorFloat(1.0, 0.0, .0, 1.0)  ));
            debug.draw(CustomData::Line( (unitCenter + bulletTopLineDelta),  (unitCenter + topLine), 0.2, ColorFloat(1.0, 0.0, .0, 1.0)  ));

            action.shoot = (lowLineCross ? (lowLineCross.value() - enemyCenter).sqrLen() < (lowLineCross.value() - unitCenter).sqrLen() : true)
                           and
                           (topLineCross ? (topLineCross.value() - enemyCenter).sqrLen() < (topLineCross.value() - unitCenter).sqrLen() : true);
        }
    }

    action.aim = aim;
}
