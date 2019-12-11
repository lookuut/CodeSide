#include "Game.hpp"

unique_ptr<Game> Game::game;

Game::Game() { }

Game::Game(
        int currentTick,
        Properties properties,
        Level level,
        std::vector<Player> players,
        std::vector<Unit> units,
        std::vector<Bullet> bullets,
        std::vector<Mine> mines
        ) : currentTick(currentTick), properties(properties), level(level), players(players), units(units), bullets(bullets), mines(mines){ }


Game * Game::init(InputStream &stream) {
    game = make_unique<Game>(Game());
    
    game->currentTick = stream.readInt();

    game->properties = Properties::readFrom(stream);
    game->level = Level::readFrom(stream);

    game->level.width = game->level.tiles.size();
    game->level.height = game->level.tiles[0].size();

    game->level.buildWalls();
    game->level.buildStandablePlaces();

    game->players = std::vector<Player>(stream.readInt());
    game->playerUnits = map<int, vector<Unit*>>();

    for (size_t i = 0; i < game->players.size(); i++) {
        game->players[i] = Player::readFrom(stream);
        game->playerUnits[game->players[i].id] = vector<Unit*>();
    }
    game->units = std::vector<Unit>(stream.readInt());

    for (size_t i = 0; i < game->units.size(); i++) {
        game->units[i] = Unit::readFrom(stream, &game->properties, &game->level);
        game->playerUnits[game->units[i].playerId].push_back(&game->units[i]);
    }
    game->bullets = std::vector<Bullet>(stream.readInt());
    for (size_t i = 0; i < game->bullets.size(); i++) {
        game->bullets[i] = Bullet::readFrom(stream);
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


//@TODO optimize it
Game * Game::updateTick(InputStream &stream) {
    game->currentTick = stream.readInt();

    Properties::readFrom(stream);
    Level::readFrom(stream);

    game->players = std::vector<Player>(stream.readInt());
    game->playerUnits = map<int, vector<Unit*>>();

    for (size_t i = 0; i < game->players.size(); i++) {
        game->players[i] = Player::readFrom(stream);
        game->playerUnits[game->players[i].id] = vector<Unit*>();
    }
    game->units = std::vector<Unit>(stream.readInt());

    for (size_t i = 0; i < game->units.size(); i++) {
        game->units[i] = Unit::readFrom(stream, &game->properties, &game->level);
        game->playerUnits[game->units[i].playerId].push_back(&game->units[i]);
    }
    game->bullets = std::vector<Bullet>(stream.readInt());
    for (size_t i = 0; i < game->bullets.size(); i++) {
        game->bullets[i] = Bullet::readFrom(stream);
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

vector<Unit*>& Game::getPlayerUnits(int playerId) {
    return Game::game->playerUnits[playerId];
}

void Game::writeTo(OutputStream& stream) const {
    stream.write(currentTick);
    properties.writeTo(stream);
    level.writeTo(stream);
    stream.write((int)(players.size()));
    for (const Player& playersElement : players) {
        playersElement.writeTo(stream);
    }
    stream.write((int)(units.size()));
    for (const Unit& unitsElement : units) {
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