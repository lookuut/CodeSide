//
// Created by lookuut on 13.12.19.
//

#ifndef AICUP2019_MINMAX_H
#define AICUP2019_MINMAX_H


#include "Simulation.hpp"
#include "../model/Game.hpp"

class MinMax {

private:
    Simulation arena;

    int enemyPlayerId;
    int allyPlayerId;
    vector<int> unitsIndex;

    Level * level;
    Properties * properties;

    vector<UnitAction> actions;
    UnitAction bestAction;
public:

    MinMax(
            Properties * properties,
            Level * level,
            int playerId,
            int enemyPlayerId,
            const vector<Unit> & units,
            const vector<int> & unitsIndex
    );

    UnitAction & getBestAction(const Unit & unit, const Game & game, Debug & debug);
    double simulation(Simulation arena, int deep, int unitIndex);
    void shootLogic(UnitAction &action, const Unit &unit, const Game &game, Debug &debug);
    double evaluation(const Unit & unit, int tick);
};


#endif //AICUP2019_MINMAX_H
