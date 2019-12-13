#ifndef _MY_STRATEGY_HPP_
#define _MY_STRATEGY_HPP_

#include "Debug.hpp"
#include "model/CustomData.hpp"
#include "model/Game.hpp"
#include "model/Unit.hpp"
#include "model/UnitAction.hpp"
#include "arena/Simulation.hpp"
#include "arena/MinMax.h"

class MyStrategy {
private:

    Simulation arena;

    int enemyPlayerId;
    int allyPlayerId;
    vector<int> unitsIndex;

    Level * level;
    Properties * properties;

    vector<UnitAction> actions;

    MinMax minMax;

public:

  MyStrategy(
          Properties * properties,
          Level * level,
          int playerId,
          int enemyPlayerId,
          const vector<Unit> & units,
          const vector<int> & unitsIndex
          );

  UnitAction getAction(const Unit &unit, const Game &game, Debug &debug);

  UnitAction simulationTest(const Unit &unit, const Game &game, Debug &debug);
  bool simulationEqualTests(const Unit & simulatedUnit, const Unit & unit, const Game & game) const;
};

#endif