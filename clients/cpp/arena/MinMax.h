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
    vector<UnitAction> allyActions;

    int simulationDeep = Consts::maxSimulationDeep;
    int simulationTicks = Consts::simulationTicks;

    vector<vector<UnitAction>> unitBestActions;
    vector<double> unitMaxEvaluationValue;
    vector<vector<UnitAction>> currentUnitBestActions;
    vector<vector<Action>> unitAction;
    vector<vector<Action>> verActionVariances;
    Unit bestSimulatedUnitState;
    Tree tree;


public:


    MinMax(
            Game * game,
            Debug * debug
    );

    UnitAction getBestAction(const Unit & unit);
    void generateBestAction(const Game & game, Debug & debug);

    double simulation(Simulation arena, int deep, int simulationUnitId, Tree * tree, int simulatedUnitId);
    void shootLogic(UnitAction &action, const Unit &unit);
    double evaluation(const Unit & unit, int tick);

    void defineTarget(Unit & unit);

    bool refreshBestSimulation();
};


#endif //AICUP2019_MINMAX_H
