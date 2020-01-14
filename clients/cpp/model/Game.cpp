#include "Game.hpp"
#include <set>

unique_ptr<Game> Game::game;

Game::Game() { }

Game::~Game() {
}

Game * Game::init(InputStream &stream, int allyPlayerId) {
    game = make_unique<Game>(Game());

    game->allyPlayerId = allyPlayerId;
    game->currentTick = stream.readInt();

    game->properties = Properties::readFrom(stream);
    game->level = Level::readFrom(stream);

    game->pp_width = game->level.width * Consts::ppFieldSize;
    game->pp_height = game->level.height * Consts::ppFieldSize;

    game->level.buildWalls();
    game->level.buildStandablePlaces();

    int playersCount = stream.readInt();

    game->players = std::map<int, Player>();

    for (size_t i = 0; i < playersCount; i++) {

        int playerId = stream.readInt();
        int score = stream.readInt();

        Player player(playerId, score);

        game->players.insert(make_pair(playerId, player));
        if (playerId != allyPlayerId) {
            game->enemyPlayerId = playerId;
        }
    }


    int unitSize = stream.readInt();

    Unit defaultUnit = Unit();

    defaultUnit.properties = &game->properties;
    defaultUnit.level = &game->level;

    game->unitAstarPath = vector<vector<AstarNode>>(unitSize / 2, vector<AstarNode>());
    game->units = vector<Unit>(unitSize, defaultUnit);
    game->aliveAllyUnits = list<int>();
    game->aliveEnemyUnits = list<int>();

    for (int i = 0; i < unitSize; i++) {
        int playerId = stream.readInt();
        int id = stream.readInt();


        game->units[Game::unitIndexById(id)].id = id;
        game->units[Game::unitIndexById(id)].playerId = playerId;
        game->units[Game::unitIndexById(id)].init(stream, &game->properties, &game->level);

        if (playerId == game->allyPlayerId) {
            game->aliveAllyUnits.push_back(id);
        } else {
            game->aliveEnemyUnits.push_back(id);
        }
        game->aliveUnits.push_back(id);
    }

    game->bullets = std::vector<Bullet>(stream.readInt());

    for (size_t i = 0; i < game->bullets.size(); i++) {
        game->bullets[i] = Bullet::readFrom(stream);
    }

    game->unitBullets = vector(game->properties.teamSize * 2, vector<Bullet*>());
    game->mines = std::vector<Mine>(stream.readInt());
    for (size_t i = 0; i < game->mines.size(); i++) {
        game->mines[i] = Mine::readFrom(stream);
    }

    game->updateLootBoxes(stream);

    return Game::game.get();
}

void Game::buildPotentionalField(const Vec2Double & pos, vector<vector<int>> & field) {
    list<pair<pair<int, int>, int>> queue = list<pair<pair<int, int>, int> >();
    vector<vector<bool>> visitedCells(pp_width, vector<bool>(pp_height, false));

    int lootX = (int)(pos.x * Consts::ppFieldSize);
    int lootY = (int)(pos.y * Consts::ppFieldSize);

    field[lootX][lootY] = 0;
    visitedCells[lootX][lootY] = true;

    queue.push_back(make_pair(make_pair(lootX, lootY) , 0));

    while (queue.size()) {

        auto & [point, d] = queue.front();
        auto & [x, y] = point;

        for (int ii = -1; ii <= 1; ii += 2) {
            int jj = 0;
            int levelX = (ii + x) / Consts::ppFieldSize;
            int levelY = (jj + y) / Consts::ppFieldSize;
            if (level.tiles[levelX][levelY] != Tile::WALL and d + 1 < field[levelX][levelY] and !visitedCells[ii + x][jj + y]) {
                field[ii + x][jj + y] = d + 1;
                queue.push_back(make_pair(make_pair(ii + x, jj + y), d + 1 ));
                visitedCells[ii + x][jj + y] = true;
            }
        }

        for (int jj = -1; jj <= 1; jj += 2) {
            int ii = 0;
            int levelX = (ii + x) / Consts::ppFieldSize;
            int levelY = (jj + y) / Consts::ppFieldSize;
            if (level.tiles[levelX][levelY] != Tile::WALL and d + 1 < field[levelX][levelY] and !visitedCells[ii + x][jj + y]) {
                field[ii + x][jj + y] = d + 1;
                queue.push_back(make_pair(make_pair(ii + x, jj + y), d + 1 ));
                visitedCells[ii + x][jj + y] = true;
            }
        }

        queue.pop_front();
    }
}


float Game::fCost(int x, int y, int x1, int y1) {
    return sqrt((x - x1) * (x - x1) + (y - y1) * (y - y1));
}

int Game::aStarPathInit(const Unit &u, const vector<LootBox> & loots, Debug & debug, vector<AstarNode> & allyUnitPath) {

    int allyUnitPathSize = allyUnitPath.size();

    this->unitAstarPath[Game::allyUnitIndexById(u.id)] = vector<AstarNode>();

    Unit unit(u);

    //debug.draw(CustomData::Rect(point.toFloat(), Vec2Float(1.0, 1.0), ColorFloat(1.0, .0, .0, 1.0)));
    set<AstarNode, SortedAstarNodeComparator> sortedQueue;
    unordered_set <AstarNode, AstarNodeHasher> queueNodes;

    unordered_map<int, unordered_map<int, AstarNode>> visitedNodes;

    for (int i = 0; i < Consts::predefinitionStates; ++i) {
        visitedNodes.insert(make_pair(i, unordered_map<int, AstarNode>()));
    }

    Vec2Double point = loots[Game::nearestLootBox(loots, u.position)].position;

    int goal_x = (int)(point.x * Consts::ppFieldSize);
    int goal_y = (int)(point.y * Consts::ppFieldSize);

    int start_x = (int)(unit.position.x * Consts::ppFieldSize);
    int start_y = (int)(unit.position.y * Consts::ppFieldSize);

    AstarNode startNode = {
            .x = start_x,
            .y = start_y,
            .vel = 0,
            .jump = false,
            .jumpDown = false,
            .timeCostFromStart = 0,
            .distanceCostToFinish = fCost(start_x, start_y, goal_x, goal_y),
            .unit = unit,
            .parentNodeIndex = 0,
            .parentNodeStateIndex = unit.stateIndex()
    };

    sortedQueue.insert(startNode);
    queueNodes.insert(startNode);

    UnitAction action;

    while(sortedQueue.size() > 0) {

        AstarNode currentNode = *sortedQueue.begin();

        int unitRightPosNodeX = (int)((currentNode.unit.position.x + currentNode.unit.widthHalf) * Consts::ppFieldSize);
        int unitLeftPosNodeX = (int)((currentNode.unit.position.x - currentNode.unit.widthHalf) * Consts::ppFieldSize);
        int unitMidPosNodeX = (int)((currentNode.unit.position.x) * Consts::ppFieldSize);

        Vec2Double point = loots[Game::nearestLootBox(loots, currentNode.unit.position)].position;

        int goal_x = (int)(point.x * Consts::ppFieldSize);
        int goal_y = (int)(point.y * Consts::ppFieldSize);

        if ((unitLeftPosNodeX == goal_x or unitRightPosNodeX == goal_x or unitMidPosNodeX == goal_x) and currentNode.y == goal_y) {

            list<AstarNode> unitTrace;

            while (!(currentNode.x == start_x and currentNode.y == start_y and currentNode.unit.stateIndex() == unit.stateIndex())) {
                unitTrace.push_back(currentNode);

                currentNode = visitedNodes[currentNode.parentNodeStateIndex][currentNode.parentNodeIndex];

                debug.draw(CustomData::Rect((currentNode.unit.position - Vec2Double(currentNode.unit.widthHalf, 0)).toFloat(), currentNode.unit.size.toFloat(), ColorFloat(1.0, 1.0, .0, 1.0)));
            }

            unitTrace.reverse();
            this->unitAstarPath[Game::allyUnitIndexById(u.id)] = vector<AstarNode>(unitTrace.begin(), unitTrace.end());

            return true;
        }

        sortedQueue.erase(sortedQueue.begin());
        queueNodes.erase(currentNode);

        visitedNodes[currentNode.unit.stateIndex()].insert(make_pair(currentNode.nodeIndex(), currentNode));

        for (int vel = -1; vel <= 1; vel += 2) {
            action.velocity = properties.unitMaxHorizontalSpeed * vel;

            for (int jumpState = -1; jumpState <= 1; ++jumpState) {
                Unit u(currentNode.unit);

                switch(jumpState) {
                    case -1:
                        action.jump = false;
                        action.jumpDown = false;
                        break;
                    case 0:
                        action.jump = true;
                        action.jumpDown = false;
                        break;
                    case 1:
                        action.jump = false;
                        action.jumpDown = true;
                        break;
                }

                vector<Unit> allyUnits;
                if (allyUnitPathSize > 0) {
                    allyUnits.push_back(allyUnitPathSize > currentNode.timeCostFromStart ? allyUnitPath[currentNode.timeCostFromStart].unit : allyUnitPath[allyUnitPathSize - 1].unit);
                }


                u.applyAction(action, allyUnits);

                int current_x = (int)(u.position.x * Consts::ppFieldSize);
                int current_y = (int)(u.position.y * Consts::ppFieldSize);

                int timeCost = currentNode.timeCostFromStart + 1;
                float hCost = fCost(goal_x, goal_y, current_x, current_y);

                AstarNode node = {
                        .x = current_x,
                        .y = current_y,
                        .vel = action.velocity,
                        .jump = action.jump,
                        .jumpDown = action.jumpDown,
                        .timeCostFromStart = timeCost,
                        .distanceCostToFinish = hCost,
                        .unit = u,
                        .parentNodeIndex = currentNode.nodeIndex(),
                        .parentNodeStateIndex = currentNode.unit.stateIndex()
                        };

                if (visitedNodes[u.stateIndex()].find(node.nodeIndex()) != visitedNodes[u.stateIndex()].end()) {
                    continue;
                }

                if (queueNodes.find(node) != queueNodes.end()) {
                    auto & n = *queueNodes.find(node);

                    if (n.timeCostFromStart > timeCost) {
                        sortedQueue.erase(n);

                        n.timeCostFromStart = timeCost;
                        n.parentNodeIndex = currentNode.nodeIndex();
                        n.parentNodeStateIndex = currentNode.unit.stateIndex();

                        sortedQueue.insert(n);
                    }

                } else {
                    queueNodes.insert(node);
                    sortedQueue.insert(node);
                }
            }
        }
    }
    return -1;
}


//@TODO optimize it
Game * Game::updateTick(InputStream &stream) {
    game->currentTick = stream.readInt();

    Properties::readFrom(stream);
    Level::readFrom(stream);

    int playerCount = stream.readInt();;

    for (size_t i = 0; i < playerCount; i++) {
        int playerId = stream.readInt();
        game->players[playerId].update(stream);
    }

    game->aliveAllyUnits = list<int>();
    game->aliveEnemyUnits = list<int>();
    game->aliveUnits = list<int>();

    int unitSize = stream.readInt();

    for (size_t i = 0; i < unitSize; i++) {
        int playerId = stream.readInt();//playerId
        int unitId = stream.readInt();

        Unit & unit = game->units[Game::unitIndexById(unitId)];
        unit.update(stream);

        if (playerId == game->allyPlayerId) {
            game->aliveAllyUnits.push_back(unitId);
        } else {
            game->aliveEnemyUnits.push_back(unitId);
        }
        game->aliveUnits.push_back(unitId);
    }

    game->bullets = std::vector<Bullet>(stream.readInt());

    for (size_t i = 0; i < game->bullets.size(); i++) {
        game->bullets[i] = Bullet::readFrom(stream);
        game->unitBullets[Game::unitIndexById(game->bullets[i].unitId)].push_back(&game->bullets[i]);
    }

    game->mines = std::vector<Mine>(stream.readInt());

    for (size_t i = 0; i < game->mines.size(); i++) {
        game->mines[i] = Mine::readFrom(stream);
    }

    game->updateLootBoxes(stream);

    return Game::game.get();
}



int Game::getNearestWeapon(const Vec2Double &unitPos, const vector<int> weaponIds) const {

    double minDistance = INT32_MAX;
    int nearestWeaponId = -1;

    for (int weaponId : weaponIds) {
        const LootBox & lootBox = lootWeapons[weaponId];

        double sqrDistance = (lootBox.position - unitPos).sqrLen();
        if (minDistance > sqrDistance) {
            minDistance = sqrDistance;
            nearestWeaponId = weaponId;
        }
    }

    return nearestWeaponId;
}

optional<int> Game::getNearestPistol(const Vec2Double &unitPos) const {
    if (lootWeaponPistolIds.size() == 0) {
        return nullopt;
    }

    return getNearestWeapon(unitPos, lootWeaponPistolIds);
}

optional<int> Game::getNearestAssult(const Vec2Double &unitPos) const {
    if (lootWeaponAssultIds.size() == 0) {
        return nullopt;
    }

    return getNearestWeapon(unitPos, lootWeaponAssultIds);
}


void Game::updateLootBoxes(InputStream &stream) {


    int lootBoxesSize = stream.readInt();
    std::vector<LootBox> lootBoxes = std::vector<LootBox>(lootBoxesSize);
    bool updatePPField = lootBoxesSize != (lootHealthPacks.size() + lootWeapons.size() + lootMines.size());

    lootHealthPacks = vector<LootBox>();
    lootWeapons = vector<LootBox>();
    lootMines = vector<LootBox>();

    lootWeaponIds = vector<int>();
    lootHealPacksIds = vector<int>();

    lootWeaponPistolIds = vector<int>();
    lootWeaponAssultIds = vector<int>();

    int weaponIterator = 0;
    int healthPackIterator = 0;

    for (size_t i = 0; i < lootBoxes.size(); i++) {
        lootBoxes[i] = LootBox::readFrom(stream);

        switch (lootBoxes[i].item.get()->getType()) {
            case ItemType ::ItemHealthPack:
                lootHealthPacks.push_back(lootBoxes[i]);
                lootHealPacksIds.push_back(healthPackIterator);

                ++healthPackIterator;

                break;
            case ItemType ::ItemWeapon:
                lootWeapons.push_back(lootBoxes[i]);
                lootWeaponIds.push_back(weaponIterator);

                if (dynamic_pointer_cast<Item::Weapon>(lootBoxes[i].item).get()->weaponType == WeaponType::PISTOL) {
                    lootWeaponPistolIds.push_back(weaponIterator);
                } else if (dynamic_pointer_cast<Item::Weapon>(lootBoxes[i].item).get()->weaponType == WeaponType::ASSAULT_RIFLE) {
                    lootWeaponAssultIds.push_back(weaponIterator);
                }
                ++weaponIterator;

                break;
            case ItemType ::ItemMine:
                lootMines.push_back(lootBoxes[i]);
                break;
        }
    }

}

list<int> & Game::getPlayerUnits(int playerId) {
    if (playerId == Game::game->allyPlayerId) {
        return Game::game->aliveAllyUnits;
    }
    return Game::game->aliveEnemyUnits;
}

void Game::writeTo(OutputStream& stream) const {
    stream.write(currentTick);
    properties.writeTo(stream);
    level.writeTo(stream);
    stream.write((int)(players.size()));

    for (const auto & [_, player] : players) {
        player.writeTo(stream);
    }

    stream.write((int)(units.size()));
    for (auto unitsElement : units) {
        unitsElement.writeTo(stream);
    }
    stream.write((int)(bullets.size()));
    for (const Bullet& bulletsElement : bullets) {
        bulletsElement.writeTo(stream);
    }
    stream.write((int)(mines.size()));
    for (const Mine& minesElement : mines) {
        minesElement.writeTo(stream);
    }
    stream.write((int)(lootHealthPacks.size() + lootMines.size() + lootWeapons.size()));

    for (const LootBox& lootBoxesElement : lootHealthPacks) {
        lootBoxesElement.writeTo(stream);
    }

    for (const LootBox& lootBoxesElement : lootMines) {
        lootBoxesElement.writeTo(stream);
    }

    for (const LootBox& lootBoxesElement : lootWeapons) {
        lootBoxesElement.writeTo(stream);
    }
}
std::string Game::toString() const {
    return std::string("Game") + "(" +
        std::to_string(currentTick) +
        properties.toString() +
        level.toString() +
        "TODO" + 
        "TODO" + 
        "TODO" + 
        "TODO" + 
        "TODO" + 
        ")";
}

Properties *Game::getProperties() {
    return &Game::game->properties;
}

Level* Game::getLevel() {
    return &Game::game->level;
}
