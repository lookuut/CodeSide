//
// Created by lookuut on 29.11.19.
//

#include "Arena.hpp"

Arena::Arena() {}


Arena::Arena(
        Properties &properties,
        Level &level,
        std::vector<Player> &players,
        std::vector<Unit> &units,
        std::vector<Bullet> &bullets,
        std::vector<Mine> &mines,
        std::vector<LootBox> &lootBoxes
) : currentTick(currentTick), properties(properties), level(level), players(players), units(units), bullets(bullets),
    mines(mines), lootBoxes(lootBoxes) {

}


void Arena::tick() {

    /*players.begin()->
    for (auto & unit : units) {
        unit.
    }*/

}
