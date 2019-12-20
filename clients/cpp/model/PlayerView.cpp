#include "PlayerView.hpp"

PlayerView::PlayerView() { }
PlayerView::PlayerView(int myId, Game * game) : myId(myId), game(game) { }

PlayerView PlayerView::init(InputStream &stream) {
    PlayerView result;

    result.myId = stream.readInt();
    result.game = Game::init(stream, result.myId);

    return result;
}

PlayerView PlayerView::updateTick(InputStream &stream) {
    PlayerView result;

    result.myId = stream.readInt();
    result.game = Game::updateTick(stream);

    return result;
}

void PlayerView::writeTo(OutputStream& stream) const {
    stream.write(myId);
    game->writeTo(stream);
}
std::string PlayerView::toString() const {
    return std::string("PlayerView") + "(" +
        std::to_string(myId) +
        game->toString() +
        ")";
}
