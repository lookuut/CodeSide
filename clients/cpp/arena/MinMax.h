//
// Created by lookuut on 13.12.19.
//

#ifndef AICUP2019_MINMAX_H
#define AICUP2019_MINMAX_H


#include "Simulation.hpp"
#include "../model/Game.hpp"
#include "Target.h"


typedef struct Tree {

    double evaluationValue;
    short actionType;
    short maxEvalValueActionIndex;
    vector<Tree*> nodes;
} Tree;


class MinMax {

private:
    Simulation arena;

    int enemyPlayerId;
    int allyPlayerId;
    Level * level;
    Properties * properties;
    Game * game;
    Debug * debug;
    vector<UnitAction> unitActions;

    int simulationDeep = Consts::maxSimulationDeep;
    int simulationTicks = Consts::simulationTicks;

    vector<UnitAction> currentUnitBestActions;
    vector<vector<Action>> verActionVariances;

    Tree tree;

    int choosenEnemyId;

public:


    MinMax(
            Game * game,
            Debug * debug
    );

    UnitAction getBestAction(const Unit & unit);
    void generateBestAction(const Game & game, Debug & debug);

    double simulation(Simulation arena, int deep, int simulationUnitId, Tree * tree, int simulatedUnitId);
    void canShoot(const list<int>& allies, const list<int>& enemies, const vector<Unit> & units, const Unit &unit, UnitAction & action);

    double evaluation(const Unit & unit, int tick, Simulation & sima);
};


#endif //AICUP2019_MINMAX_H
