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
    unitActions = vector<UnitAction>(game->properties.teamSize * 2);

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

    if (unit.weapon == nullptr or unit.weapon.get()->type != WeaponType::PISTOL) {
        if (auto nearestPistolId = game->getNearestPistol(unit.position)) {
            //evaluateValue += 1.0 / (double)(game->ppVal(PPFieldType::PPWeapon, nearestPistolId.value(), unit.position)  + 1);
        }
    }

    if (unit.weapon == nullptr) {
        int nearestWeaponId = game->getNearestWeapon(unit.position, game->lootWeaponIds);
        //evaluateValue += 1.0 / (double)(game->ppVal(PPFieldType::PPWeapon, nearestWeaponId, unit.position)  + 1);
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

            bool attack = (unit.weapon.get()->params.fireRate > unit.weapon.get()->fireTimer)
                          and (nearestEnemy->weapon == nullptr or nearestEnemy->weapon.get()->fireTimer > Consts::enemyFireTimerMin) or d > Consts::sqrMinDistanceToEnemy;

            if (attack) {
                evaluateValue += 1.0 / (d + 1);
            } else {
                evaluateValue += 1.0 - 1.0 / d;
            }
        }
    }

    Unit & prevUnitState = game->units[Game::unitIndexById(unit.id)];

    evaluateValue += 2 * (int)(unit.weapon != nullptr) + (sima.allyPoints - sima.enemyPoints) + unit.health;

    evaluateValue += unit.mines / 10.0;
    evaluateValue += unit.position.y / (level->height * level->height);

    //evaluateValue += prevPosDistance / (prevPosDistance + 1);
    //evaluateValue += (unit.onGroundLadderTicks / (double)tick);

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
    UnitAction & action = currentUnitBestActions[Game::allyUnitIndexById(unit.id)];

    canShoot(game->aliveAllyUnits, game->aliveEnemyUnits, game->units, unit, action);

    if (unit.weapon != nullptr and unit.weapon.get()->type == WeaponType::ROCKET_LAUNCHER) {
        action.swapWeapon = true;
    }

    if (unit.weapon != nullptr and unit.weapon.get()->type != WeaponType::PISTOL) {
        for (int weaponId : game->lootWeaponPistolIds) {
            const LootBox & lootBox = game->lootWeapons[weaponId];
            if (unit.isPickUpLootbox(lootBox)) {
                action.swapWeapon = true;
            }
        }
    }

    return action;
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

    if (unit.weapon == nullptr) {
        return;
    }


    auto [enemyId, aim] = unit.chooseEnemy(enemies, units, choosenEnemyId);
    const Unit & enemy = game->units[Game::unitIndexById(enemyId)];

    aim.normalize();

    action.aim = aim;
    bool shoot = false;

    if (unit.checkAim(enemy, aim, allies, units)) {
        shoot = true;
    }

    action.shoot = shoot;
}
