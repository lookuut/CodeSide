//
// Created by lookuut on 06.12.19.
//

#ifndef AICUP2019_CONSTS_H
#define AICUP2019_CONSTS_H

class Consts {
public:
    static constexpr double eps = 1.0e-9;
    static constexpr int simulationTicks = 4;
    static constexpr int maxSimulationDeep = 7;
    static constexpr int microticks = 1;
    static constexpr double noLastAngleValue = -100.0;
    static constexpr int maxUnitCount = 4;
    static constexpr int weaponTargetType = 1;
    static constexpr int healthPackTargetType = 2;
    static constexpr int enemyTargetType = 1;
    static constexpr int simulationNodes = 3;
};
//      "ticks_per_second": 60,
//      "updates_per_tick": 100,wwwwwa
//      "ticks_per_second": 6000,
//      "updates_per_tick": 1,

//"seed" : 4234324345 bug with roof and 1.0e-9,

#endif //AICUP2019_CONSTS_H
