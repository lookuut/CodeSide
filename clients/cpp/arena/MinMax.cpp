//
// Created by lookuut on 13.12.19.
//

#include "MinMax.h"
#include "../model/Game.hpp"
#include "../utils/Geometry.h"
#include <math.h>

MinMax::MinMax(Game * game, Debug * debug):
        properties(&game->properties),
        level(&game->level),
        allyPlayerId(game->allyPlayerId),
        enemyPlayerId(game->enemyPlayerId),
        arena(game, debug),
        game(game),
        debug(debug)
{
    allyActions = vector<UnitAction>(game->properties.teamSize);

    verActionVariances = {
            {//Jumping process (can cancel jump), rise - maxTime > 0
                    {false, true, 0},//Cancel and just go down
                    {true, false, 10},//Cancel and just go down
                    {true, false, -10},//Cancel and just go down
            },
            {//On ground
                    {true, false, 0},//Cancel and just go down
                    {false, false, 10},//Cancel and just go down
                    {false, false, -10},//Cancel and just go down
            },
            {//Cant jump and cant cancel
                    {false, false, 10},
                    {false, false, -10},
                    {true, false, 0}
            },
    };

    unitAction = vector<vector<Action>>(properties->teamSize, vector<Action>());
    currentUnitBestActions = vector<vector<UnitAction>>(properties->teamSize, vector<UnitAction>());

    list<Tree*> toInitNodes;
    toInitNodes.push_back(&tree);

    for (int i = 0; i < Consts::maxSimulationDeep; ++i) {
        int simulations = pow(Consts::simulationNodes, i);
        for (int j = 0; j < simulations; ++j) {
            Tree * tree = toInitNodes.front();
            toInitNodes.pop_front();

            tree->nodes = vector<Tree*>(Consts::simulationNodes, NULL);

            for (int node = 0; node < Consts::simulationNodes; ++node) {
                tree->nodes[node] = new Tree();
                toInitNodes.push_back(tree->nodes[node]);
            }
        }
    }

}

double MinMax::evaluation(const Unit &unit, int tick) {

    Vec2Double targetPos;


    double evaluateValue = 0;

    if (unit.weapon == nullptr) {
        LootBox * nearestWeapon = NULL;

        double nearestWeaponDistance = INT32_MAX;

        for (LootBox & weapon : arena.lootWeapons) {
            double distance = (weapon.position - unit.position).sqrLen();

            if (nearestWeaponDistance > distance) {
                nearestWeaponDistance = distance;
                nearestWeapon = &weapon;
            }
        }
        targetPos = nearestWeapon->position;

        double distance = (targetPos - unit.position).sqrLen();
        evaluateValue += 1.0 / distance;
    } else {
        Unit * nearestEnemy = NULL;

        double nearestEnemyDistance = INT32_MAX;
        for (int enemyUnitId : game->aliveEnemyUnits) {
            Unit * enemy = &game->units[Game::unitIndexById(enemyUnitId)];

            double distance = (enemy->position - unit.position).sqrLen();

            if (nearestEnemyDistance > distance) {
                nearestEnemyDistance = distance;
                nearestEnemy = enemy;
            }
        }

        if (unit.health < nearestEnemy->health and arena.lootHealthPacks.size() > 0) {
            LootBox * nearestHealthPack = NULL;

            double nearestHealthPackDistance = INT32_MAX;

            for (LootBox & healthPack : arena.lootHealthPacks) {
                double distance = (healthPack.position - unit.position).sqrLen();

                if (nearestHealthPackDistance > distance) {
                    nearestHealthPackDistance = distance;
                    nearestHealthPack = &healthPack;
                }
            }
            targetPos = nearestHealthPack->position;

            double distance = (targetPos - unit.position).sqrLen();
            evaluateValue += 1.0 / distance;
        } else {
            targetPos = nearestEnemy->position;

            bool attack = (unit.weapon.get()->params.fireRate >= unit.weapon.get()->fireTimer);
            double d = (targetPos - unit.position).sqrLen();
            evaluateValue += attack / d + !attack * (1.0 - 1.0 / d);
        }
    }

    Unit & prevUnitState = game->units[Game::unitIndexById(unit.id)];

    evaluateValue += 2 * (int)(unit.weapon != nullptr) + unit.health + (unit.health <= 0) * -properties->killScore;

    evaluateValue += unit.mines / 10.0;
    evaluateValue += unit.position.y / (level->height * level->height);
    //evaluateValue += (unit.weapon == nullptr ? 1 / (distance  + 1) : (unit.position.y / level->height));

    //evaluateValue += prevPosDistance / (prevPosDistance + 1);
    //evaluateValue += (unit.onGroundLadderTicks / (double)tick);

    return evaluateValue / (double)tick;
}

void MinMax::generateBestAction(const Game &game, Debug &debug) {
    arena.update();

    vector<vector<UnitAction>> curActions(properties->teamSize, vector<UnitAction>());
    currentUnitBestActions = vector<vector<UnitAction>>(properties->teamSize, vector<UnitAction>());

    for (auto & unitAction : allyActions) {
        unitAction = UnitAction();
    }

    bool isFirst = true;
    for (int & unitId : this->game->aliveAllyUnits) {

        int allyUnitIndex = Game::allyUnitIndexById(unitId);
        Unit & unit = arena.units[Game::unitIndexById(unitId)];

        int actionType = unit.jumpState.maxTime > 0 ? 0 : (unit.onGround ? 1 : 2);

        UnitAction bestAction;
        double maxEvalValue = INT32_MIN;
        short maxActionIndex = 0;

        int simulatedUnitId = this->game->aliveAllyUnits.front();

        if (!isFirst) {
            allyActions[Game::allyUnitIndexById(simulatedUnitId)].update(verActionVariances[tree.actionType][tree.maxEvalValueActionIndex]);
        }

        for (int actionId = 0; actionId < Consts::simulationNodes; ++actionId) {

            allyActions[allyUnitIndex].update(verActionVariances[actionType][actionId]);

            double evalValue = simulation(arena, 1, unitId, tree.nodes[actionId], (isFirst ? 0 : this->game->aliveAllyUnits.front()));

            if (evalValue > maxEvalValue) {
                maxEvalValue = evalValue;
                bestAction.update(verActionVariances[actionType][actionId]);
                maxActionIndex = actionId;
            }
        }

        tree.maxEvalValueActionIndex = maxActionIndex;
        tree.evaluationValue = maxEvalValue;
        tree.actionType = actionType;

        this->currentUnitBestActions[allyUnitIndex] = vector<UnitAction>(1, bestAction);
        isFirst = false;
    }
}

UnitAction MinMax::getBestAction(const Unit & unit) {
    vector<UnitAction> & bestActions = currentUnitBestActions[Game::allyUnitIndexById(unit.id)];
    UnitAction action = bestActions.front();

    shootLogic(action, unit);

    bestActions.erase(bestActions.begin());

    return action;
}


double MinMax::simulation(Simulation arena, int deep, int simulationUnitId, Tree * tree, int simulatedUnitId) {

    arena.tick(allyActions, this->simulationTicks);

    int allyUnitIndex = Game::allyUnitIndexById(simulationUnitId);

    Unit & unit = arena.units[Game::unitIndexById(simulationUnitId)];

    double evaluationValue = evaluation(unit, deep * Consts::simulationTicks);

    if (deep >= this->simulationDeep) {
        return evaluationValue;
    }

    int actionType = unit.jumpState.maxTime > 0 ? 0 : (unit.onGround ? 1 : 2);

    if (simulatedUnitId == 0) {
        tree->actionType = actionType;
    }

    if (simulatedUnitId > 0) {
        allyActions[Game::allyUnitIndexById(simulatedUnitId)].update(verActionVariances[tree->actionType][tree->maxEvalValueActionIndex]);
    }

    double maxEvalValue = INT32_MIN;
    short maxEvalValueActionId = 0;

    for (int actionId = 0; actionId < Consts::simulationNodes; ++actionId) {

        allyActions[allyUnitIndex].update(verActionVariances[actionType][actionId]);
        double evalValue = simulation(arena, deep + 1, simulationUnitId, tree->nodes[actionId], simulatedUnitId);

        if (maxEvalValue < evalValue) {
            maxEvalValue = evalValue;
            maxEvalValueActionId = actionId;
        }
    }

    if (simulatedUnitId == 0) {
        tree->maxEvalValueActionIndex = maxEvalValueActionId;
        tree->evaluationValue = maxEvalValue + evaluationValue;
    }

    return evaluationValue + maxEvalValue;
}


bool MinMax::refreshBestSimulation() {

    if (unitMaxEvaluationValue.size() == 0) {
        return true;
    }

    int size = unitBestActions.front().size();

    for (int step = 0; step < size; ++step) {

        for (const auto & unitId : game->aliveAllyUnits) {
            int unitIndex = Game::allyUnitIndexById(unitId);
            if (unitBestActions[unitIndex].size() > 0) {
                allyActions[unitIndex] = unitBestActions[unitIndex][step];
            }
        }
        arena.tick(allyActions, Consts::simulationTicks);

        Unit & unit = arena.units[Game::unitIndexById(game->aliveAllyUnits.front())];
        debug->draw(CustomData::Rect(Vec2Float(unit.leftTop.x, unit.rightDown.y), unit.size.toFloat(), ColorFloat(1.0, 1.0, .0, 1.0)));
    }

    bool recalculation = false;
    for (int & unitId : game->aliveAllyUnits) {
        Unit & unit = arena.units[Game::unitIndexById(unitId)];
        double evaluationValue = evaluation(unit, Consts::maxSimulationDeep * Consts::simulationTicks);

        if (evaluationValue + 1 < unitMaxEvaluationValue[Game::allyUnitIndexById(unit.id)]) {
            recalculation = true;
            break;
        }
    }

    if (recalculation) {
        arena.update();
    }

    return recalculation;
}


void MinMax::shootLogic(UnitAction &action, const Unit &unit) {

    const list<int> & enemies = game->aliveEnemyUnits;
    const list<int> & allies = game->aliveAllyUnits;

    Vec2Double bestAim(0,0);
    double minDistance = -1;

    for (int enemyUnitId : game->aliveEnemyUnits) {
        Unit * enemy = &game->units[Game::unitIndexById(enemyUnitId)];
        Vec2Double aim(0, 0);

        action.shoot = true;

        if (unit.weapon != nullptr) {

            Vec2Float unitCenter = Vec2Float(unit.position + Vec2Double(0, game->properties.unitSize.y / 2.0));
            Vec2Float enemyCenter = Vec2Float(enemy->position + Vec2Double(0, game->properties.unitSize.y / 2.0));

            aim = Vec2Double(enemyCenter.x - unitCenter.x, enemyCenter.y - unitCenter.y);

            if (game->level.crossWall(unitCenter, enemyCenter)) {
                action.shoot = false;
            } else {

                Vec2Double lowLineFromBulletCenter = aim.rotate(unit.weapon.get()->spread);
                Vec2Double topLineFromBulletCenter = aim.rotate(-unit.weapon.get()->spread);

                Vec2Double bulletLowLineDelta = lowLineFromBulletCenter.getOpponentAngle(unit.weapon.get()->params.bullet.size / 2.0, false);
                Vec2Double bulletTopLineDelta = topLineFromBulletCenter.getOpponentAngle(unit.weapon.get()->params.bullet.size / 2.0, true);

                Vec2Double lowLine = bulletLowLineDelta + lowLineFromBulletCenter;
                Vec2Double topLine = bulletTopLineDelta + topLineFromBulletCenter;

                auto lowLineCross = game->level.crossMiDistanceWall(unitCenter + bulletLowLineDelta, unitCenter + lowLine);
                auto topLineCross = game->level.crossMiDistanceWall(unitCenter + bulletTopLineDelta, unitCenter + topLine);

                action.shoot = (lowLineCross ? (lowLineCross.value() - enemyCenter).sqrLen() < (lowLineCross.value() - unitCenter).sqrLen() : true)
                               and
                               (topLineCross ? (topLineCross.value() - enemyCenter).sqrLen() < (topLineCross.value() - unitCenter).sqrLen() : true);


                if (action.shoot) {
                    for (int allyUnitId : allies) {
                        Unit * allyUnit = &game->units[Game::unitIndexById(allyUnitId)];
                        if (allyUnit->id != unit.id) {


                            vector<vector<Vec2Double>> allyBorders = {
                                        {
                                            Vec2Double(allyUnit->position.x - allyUnit->widthHalf, allyUnit->position.y), Vec2Double(allyUnit->position.x + allyUnit->widthHalf, allyUnit->position.y)
                                        },
                                        {
                                            Vec2Double(allyUnit->position.x - allyUnit->widthHalf, allyUnit->position.y), Vec2Double(allyUnit->position.x - allyUnit->widthHalf, allyUnit->position.y + allyUnit->size.y)
                                        },
                                        {
                                            Vec2Double(allyUnit->position.x + allyUnit->widthHalf, allyUnit->position.y), Vec2Double(allyUnit->position.x + allyUnit->widthHalf, allyUnit->position.y + allyUnit->size.y)
                                        },
                                        {
                                            Vec2Double(allyUnit->position.x - allyUnit->widthHalf, allyUnit->position.y + allyUnit->size.y), Vec2Double(allyUnit->position.x + allyUnit->widthHalf, allyUnit->position.y + allyUnit->size.y)
                                        }
                            };


                            for (const vector<Vec2Double> & border : allyBorders) {
                                if (Geometry::doIntersect(border[0], border[1], unitCenter, enemyCenter)) {
                                    action.shoot = false;
                                    break;
                                } else if (Geometry::doIntersect(border[0], border[1], (unitCenter + bulletLowLineDelta),  (unitCenter + lowLine))) {
                                    action.shoot = false;
                                    break;
                                } else if (Geometry::doIntersect(border[0], border[1], (unitCenter + bulletTopLineDelta),  (unitCenter + topLine))) {
                                    action.shoot = false;
                                    break;
                                }
                            }
                        }
                    }
                }
            }

            if (action.shoot) {
                if (minDistance < 0 or minDistance > aim.sqrLen()) {
                    bestAim = aim;
                    minDistance = aim.sqrLen();
                }
            }
        }
    }

    action.shoot = minDistance > 0;
    action.aim = bestAim;

    if (!action.shoot) {
        action.reload = true;
    }
}