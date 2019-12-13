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

    int unitIndex;
    int enemyPlayerId;
    int allyPlayerId;
    vector<int> unitsIndex;
    Vec2Double nearestWeaponPos;

    Level * level;
    Properties * properties;

    vector<UnitAction> actions;
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

  double simulation(Arena arena, int deep, int unitIndex);

  void shootLogic(UnitAction & action, const Unit &unit, const Game &game, Debug &debug);

  UnitAction bestAction();

  double evaluation(const Unit & unit, int tick);
};

#endif