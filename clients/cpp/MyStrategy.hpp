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

    int enemyPlayerId = 0;
    int allyPlayerId = 0;

    Level * level = NULL;
    Properties * properties = NULL;

    vector<UnitAction> actions;

    MinMax minMax;

public:

  MyStrategy(
          Game * game,
          Debug * debug
          );


  void tick(const Game &game, Debug &debug);
  UnitAction getAction(const Unit &unit);

  UnitAction simulationTest(const Unit &unit, const Game &game, Debug &debug);
  bool simulationEqualTests(const Unit & simulatedUnit, const Unit & unit, const Game & game) const;
};

#endif