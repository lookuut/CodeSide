#include "ServerMessageGame.hpp"

ServerMessageGame::ServerMessageGame() { }
ServerMessageGame::ServerMessageGame(std::shared_ptr<PlayerView> playerView) : playerView(playerView) { }

ServerMessageGame ServerMessageGame::init(InputStream &stream) {
    ServerMessageGame result;
    if (stream.readBool()) {
        result.playerView = std::shared_ptr<PlayerView>(new PlayerView());
        *result.playerView = PlayerView::init(stream);
    } else {
        result.playerView = std::shared_ptr<PlayerView>();
    }
    return result;
}

ServerMessageGame ServerMessageGame::updateTick(InputStream &stream) {
    ServerMessageGame result;
    if (stream.readBool()) {
        result.playerView = std::shared_ptr<PlayerView>(new PlayerView());
        *result.playerView = PlayerView::updateTick(stream);
    } else {
        result.playerView = std::shared_ptr<PlayerView>();
    }
    return result;
}

void ServerMessageGame::writeTo(OutputStream& stream) const {
    if (playerView) {
        stream.write(false);
    } else {
        stream.write(true);
        (*playerView).writeTo(stream);
    }
}
std::string ServerMessageGame::toString() const {
    return std::string("ServerMessageGame") + "(" +
        "TODO" + 
        ")";
}
