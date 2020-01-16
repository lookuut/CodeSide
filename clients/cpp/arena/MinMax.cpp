//
// Created by lookuut on 13.12.19.
//

#include "MinMax.h"
#include "../model/Game.hpp"
#include "../utils/Geometry.h"
#include <math.h>
#include <iostream>
#include <iterator>
#include <vector>

size_t UnitStateNodeHasher::operator()(const UnitStateNode &node) const {
    return node.getNodeId() * node.getConstUnit().stateId();
}

const Unit& UnitStateNode::getConstUnit() const {
    return world.getUnit(unitId);
}

MinMax::MinMax(Game * game, Debug * debug):
        properties(&game->properties),
        level(&game->level),
        allyPlayerId(game->allyPlayerId),
        enemyPlayerId(game->enemyPlayerId),
        arena(game, debug),
        game(game),
        debug(debug)
{
    unitActions = vector<UnitAction>(game->properties.teamSize * 2);
    unitsStates = vector<vector<set<UnitStateNode, UnitStateComparator>>>(properties->teamSize, vector<set<UnitStateNode, UnitStateComparator>>());

    verActionVariances = {
            {//Jumping process (can cancel jump), rise - maxTime > 0
                    {true, false, -10},//Cancel and just go down
                    {true, false, 10},//Cancel and just go down

                    {false, true, 0},//Cancel and just go down
            },
            {//On ground
                    {false, false, -10},//Cancel and just go down
                    {false, false, 10},//Cancel and just go down
                    {true, false, 0},//Cancel and just go down
            },
            {//Cant jump and cant cancel
                    {false, false, 10},
                    {false, false, -10},
                    {true, false, 0}
            },
            {//On platform
                    {true, false, -10},
                    {true, false, 10},
                    {false, true, 0},
            },
    };

    currentUnitBestActions = vector<UnitAction>(properties->teamSize, UnitAction());

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

double MinMax::evaluation(const Unit &unit, int tick, Simulation & sima) {

    Vec2Double targetPos;

    double evaluateValue = 0;

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

            if (
                    (nearestEnemy->position.x < unit.position.x and nearestEnemy->position.x < healthPack.position.x) or
                    (nearestEnemy->position.x > unit.position.x and nearestEnemy->position.x > healthPack.position.x) or
                    abs(unit.position.y - nearestEnemy->position.y) > unit.size.y) {
                double distance = (healthPack.position - unit.position).sqrLen();

                if (nearestHealthPackDistance > distance) {
                    nearestHealthPackDistance = distance;
                    nearestHealthPack = &healthPack;
                }
            }
        }

        if (nearestHealthPack == NULL) {
            double distance = (nearestEnemy->position - unit.position).sqrLen();
            evaluateValue += 1.0 - 1.0 / (distance + 1) ;
        } else {
            targetPos = nearestHealthPack->position;

            double distance = (targetPos - unit.position).sqrLen();
            evaluateValue += 1.0 / (distance + 1);
        }
    } else {
        targetPos = nearestEnemy->position;

        double d = (targetPos - unit.position).sqrLen();

        double xDiff = abs(targetPos.x - unit.position.x);
        double yDiff = abs(targetPos.y - unit.position.y);

        bool canSuicide = nearestEnemy->mines > 0 and (xDiff < properties->mineExplosionParams.radius + unit.size.x
                                                           and ((targetPos.y < unit.position.y and yDiff < properties->mineExplosionParams.radius)
                                                           or (targetPos.y > unit.position.y and yDiff < properties->mineExplosionParams.radius + unit.size.y)));

        bool attack = ((nearestEnemy->weapon == nullptr or nearestEnemy->weapon.get()->fireTimer > Consts::enemyFireTimerMin)) and !canSuicide or d > Consts::sqrMinDistanceToEnemy;

        if (attack) {
            evaluateValue += 1.0 / (d + 1);
        } else {
            evaluateValue += 1.0 - 1.0 / d;
        }
    }

    evaluateValue += 2 * (int)(unit.weapon != nullptr) + (sima.allyPoints - sima.enemyPoints) + unit.health;
    evaluateValue += unit.mines / 10.0;

    //evaluateValue += prevPosDistance / (prevPosDistance + 1);
    //evaluateValue += (world.onGroundLadderTicks / (double)tick);

    return evaluateValue / (double)tick;
}

void MinMax::generateBestAction(const Game &game, Debug &debug) {
    choosenEnemyId = -1;
    arena.update();

    for (auto & unitAction : unitActions) {
        unitAction = UnitAction();
    }

    bool isFirst = true;
    for (int & unitId : this->game->aliveAllyUnits) {

        int allyUnitIndex = Game::allyUnitIndexById(unitId);
        Unit & unit = arena.units[Game::unitIndexById(unitId)];

        int actionType = unit.actionType();

        UnitAction bestAction;
        double maxEvalValue = INT32_MIN;
        short maxActionIndex = 0;

        int simulatedUnitId = this->game->aliveAllyUnits.front();

        if (!isFirst) {
            unitActions[Game::unitIndexById(simulatedUnitId)].update(verActionVariances[tree.actionType][tree.maxEvalValueActionIndex]);
        }

        for (int actionId = 0; actionId < Consts::simulationNodes; ++actionId) {

            unitActions[Game::unitIndexById(unitId)].update(verActionVariances[actionType][actionId]);

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

        this->currentUnitBestActions[allyUnitIndex] = bestAction;
        isFirst = false;
    }
}

UnitAction MinMax::getBestAction(const Unit & unit) {
    return currentUnitBestActions[Game::allyUnitIndexById(unit.id)];
}


double MinMax::simulation(Simulation arena, int deep, int simulationUnitId, Tree * tree, int simulatedUnitId) {
    Unit & unit = arena.units[Game::unitIndexById(simulationUnitId)];

    if (simulatedUnitId > 0) {
        Unit & simulatedUnit = arena.units[Game::unitIndexById(simulatedUnitId)];
    }

    arena.tick(unitActions, Consts::simTicks[deep - 1]);

    double evaluationValue = evaluation(unit, Consts::simTicksSum[deep - 1], arena);//0.045454545454545456

    if (deep >= this->simulationDeep) {
        return evaluationValue;
    }

    int actionType = unit.actionType();

    if (simulatedUnitId == 0) {
        tree->actionType = actionType;
    }

    if (simulatedUnitId > 0) {
        unitActions[Game::unitIndexById(simulatedUnitId)].update(verActionVariances[tree->actionType][tree->maxEvalValueActionIndex]);
    }

    double maxEvalValue = INT32_MIN;
    short maxEvalValueActionId = 0;

    for (int actionId = 0; actionId < Consts::simulationNodes; ++actionId) {

        unitActions[Game::unitIndexById(simulationUnitId)].update(verActionVariances[actionType][actionId]);
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


void MinMax::canShoot(const list<int>& allies, const list<int>& enemies, const vector<Unit> & units, const Unit &unit, UnitAction & action) {

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



/*

void MinMax::unitStateBootStrap(Unit &unit, int deep, const vector<UnitAction> &allyUnitActions) {

    vector<set<UnitStateNode, UnitStateComparator>> & unitStates = unitsStates[Game::allyUnitIndexById(unit.id)];

    UnitStateNode ssState;

    if (unitStates.size() < 50 or game->newBullet) {

        Simulation world(game, debug);
        world.update();

        UnitStateNode startNode = {
                .unitId = unit.id,
                .level = unit.jumpLevel,
                .nodeId = UnitStateNode::getUnitNodeId(unit),
                .action = UnitAction(),
                .evaluationValue = preEvalVal,
                .world = world,
                .parentNodeId = 0,
                .parentNodeEvaluationValue = .0
        };

        int startTick = unitStates.size() > 0 and unitStates[0].find(startNode) != unitStates[0].end() and !game->newBullet ? unitStates.size() - 1 : 0;

        if (startTick == 0) {
            unitStates.clear();

            unitStates.push_back(set<UnitStateNode, UnitStateComparator>());
            unitStates[0].insert(startNode);
        }

        vector<UnitAction> actions(properties->teamSize * 2, UnitAction());
        UnitAction & action = actions[Game::unitIndexById(unit.id)];

        for (int tick = startTick + 1; tick < startTick + deep; ++tick) {


            auto it = unitStates[tick - 1].end();

            if (unitStates[tick - 1].size() > 100) {
                it = unitStates[tick - 1].begin();
                std::advance(it, 100);
            }

            set<UnitStateNode, UnitStateComparator> queue(unitStates[tick - 1].begin(), it);

            unitStates.push_back(set<UnitStateNode, UnitStateComparator>());

            while (queue.size() > 0) {

                UnitStateNode currentNode = *queue.begin();

                queue.erase(currentNode);

                for (int vel = -1; vel <= 1; ++vel) {

                    action.velocity = properties->unitMaxHorizontalSpeed * vel;

                    for (int jumpState = -1; jumpState <= 1; ++jumpState) {
                        action.setJumpState(jumpState);
                        Simulation world(currentNode.world);

                        world.tick(actions, 1);

                        Unit & u = world.getUnit(unit.id);

                        double evalValue = evaluation(u, tick - startTick, world) + currentNode.evaluationValue;

                        UnitStateNode node = {
                                .unitId = unit.id,
                                .level = world.getUnit(unit.id).jumpLevel,
                                .nodeId = UnitStateNode::getUnitNodeId(u),
                                .action = action,
                                .evaluationValue = evalValue,
                                .world = world,
                                .parentNodeId = currentNode.getNodeId(),
                                .parentNodeEvaluationValue = currentNode.evaluationValue
                        };
                        debug->draw(CustomData::Rect( Vec2Double(u.position.x - u.widthHalf, u.position.y).toFloat(), u.size.toFloat(), ColorFloat(1.0, 1.0, .0, 1.0) ));
                        ssState.evaluationValue = currentNode.evaluationValue;
                        ssState.nodeId = currentNode.getNodeId();

                        if (unitStates[tick].find(node) != unitStates[tick].end()) {
                            continue;
                        }

                        unitStates[tick].insert(node);
                    }
                }
            }
        }
    }
}*/
