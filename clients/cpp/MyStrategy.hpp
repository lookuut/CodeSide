#ifndef _MY_STRATEGY_HPP_
#define _MY_STRATEGY_HPP_

#include "Debug.hpp"
#include "model/CustomData.hpp"
#include "model/Game.hpp"
#include "model/Unit.hpp"
#include "model/UnitAction.hpp"
#include "arena/Arena.hpp"

class MyStrategy {
private:

    Arena arena;
    Properties properties;
    int unitIndex;
    int enemyPlayerId;
    int allyPlayerId;
public:
  MyStrategy();
  UnitAction getAction(const Unit &unit, const Game &game, Debug &debug);

  UnitAction simulationTest(const Unit &unit, const Game &game, Debug &debug);

  double simulation(Arena arena, int deep, int unitIndex);

  void shootLogic(UnitAction & action, const Unit &unit, const Game &game, Debug &debug);

  UnitAction bestAction();

  double evaluation(const Unit & unit, Game & game);
};

#endif