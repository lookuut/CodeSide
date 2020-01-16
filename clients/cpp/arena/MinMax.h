//
// Created by lookuut on 13.12.19.
//

#ifndef AICUP2019_MINMAX_H
#define AICUP2019_MINMAX_H


#include "Simulation.hpp"
#include "../model/Game.hpp"
#include "Target.h"
#include <set>

typedef struct UnitStateNode{
    int unitId;

    int level;

    int nodeId;

    UnitAction action;

    double evaluationValue;

    mutable Simulation world;

    mutable int parentNodeId;
    mutable double parentNodeEvaluationValue;

    bool operator==(const UnitStateNode & node) const{
        return node.getNodeId() == getNodeId();
    }

    int getNodeId() const {
        return nodeId;
    }

    static int getUnitNodeId(const Unit & unit) {
        return (unit.position.x + unit.position.y * unit.level->width * Consts::ppFieldSize) * Consts::ppFieldSize * unit.jumpLevel;
    }

    Unit & getUnit();

    const Unit & getConstUnit() const;

} UnitStateNode;


struct UnitStateNodeHasher {
    size_t operator() (const UnitStateNode &node) const;
};

struct UnitStateComparator {
    bool operator() (const UnitStateNode & node1,const UnitStateNode & node2) {

        if (node1.evaluationValue > node2.evaluationValue) {
            return true;
        }

        if (node1.evaluationValue == node2.evaluationValue and node1.getNodeId() < node2.getNodeId()) {
            return true;
        }

        return false;
    }
};

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

    vector<vector<set<UnitStateNode, UnitStateComparator>>> unitsStates;

    double preEvalVal = 0;
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

    void unitStateBootStrap(Unit & unit, int deep, const vector<UnitAction> & allyUnitActions);
};


#endif //AICUP2019_MINMAX_H
