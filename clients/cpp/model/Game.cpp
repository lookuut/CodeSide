#include "Game.hpp"

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

    game->level.width = game->level.tiles.size();
    game->level.height = game->level.tiles[0].size();

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
    std::vector<LootBox> lootBoxes = std::vector<LootBox>(stream.readInt());

    game->lootHealthPacks = vector<LootBox>();
    game->lootWeapons = vector<LootBox>();
    game->lootMines = vector<LootBox>();

    for (size_t i = 0; i < lootBoxes.size(); i++) {
        lootBoxes[i] = LootBox::readFrom(stream);

        switch (lootBoxes[i].item.get()->getType()) {
            case ItemType ::ItemHealthPack:
                game->lootHealthPacks.push_back(lootBoxes[i]);
                break;
            case ItemType ::ItemWeapon:
                game->lootWeapons.push_back(lootBoxes[i]);
                break;
            case ItemType ::ItemMine:
                game->lootMines.push_back(lootBoxes[i]);
                break;
        }
    }

    return Game::game.get();
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

    std::vector<LootBox> lootBoxes = std::vector<LootBox>(stream.readInt());

    game->lootHealthPacks = vector<LootBox>();
    game->lootWeapons = vector<LootBox>();
    game->lootMines = vector<LootBox>();

    for (size_t i = 0; i < lootBoxes.size(); i++) {
        lootBoxes[i] = LootBox::readFrom(stream);

        switch (lootBoxes[i].item.get()->getType()) {
            case ItemType ::ItemHealthPack:
                game->lootHealthPacks.push_back(lootBoxes[i]);
                break;
            case ItemType ::ItemWeapon:
                game->lootWeapons.push_back(lootBoxes[i]);
                break;
            case ItemType ::ItemMine:
                game->lootMines.push_back(lootBoxes[i]);
                break;
        }
    }

    return Game::game.get();
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