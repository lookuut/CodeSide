#include "Debug.hpp"
#include "MyStrategy.hpp"
#include "TcpStream.hpp"
#include "model/PlayerMessageGame.hpp"
#include "model/ServerMessageGame.hpp"
#include <memory>
#include <string>
#include <unordered_map>

class Runner {
public:
  Runner(const std::string &host, int port, const std::string &token) {
    std::shared_ptr<TcpStream> tcpStream(new TcpStream(host, port));
    inputStream = getInputStream(tcpStream);
    outputStream = getOutputStream(tcpStream);
    outputStream->write(token);
    outputStream->flush();
  }
  void run() {

    Debug debug(outputStream);

    auto message = ServerMessageGame::init(*inputStream);
    const auto& playerView = message.playerView;

    if (!playerView) {
      return;
    }

    int enemyPlayerId;
    for (const Player & player : playerView.get()->game->players) {
        if (player.id != playerView->myId) {
            enemyPlayerId = player.id;
        }
    }

    MyStrategy myStrategy(&playerView.get()->game->properties, &playerView.get()->game->level, playerView.get()->myId, enemyPlayerId, playerView->game->units);

    std::unordered_map<int, UnitAction> actions;
    for (const Unit &unit : playerView->game->units) {
      if (unit.playerId == playerView->myId) {
        actions.emplace(std::make_pair(
                unit.id,
                myStrategy.getAction(unit, *playerView->game, debug)));
      }
    }
    PlayerMessageGame::ActionMessage(Versioned(actions)).writeTo(*outputStream);
    outputStream->flush();

    while (true) {
      auto message = ServerMessageGame::updateTick(*inputStream);
      const auto& playerView = message.playerView;
      if (!playerView) {
        break;
      }
      std::unordered_map<int, UnitAction> actions;
      for (const Unit &unit : playerView->game->units) {
        if (unit.playerId == playerView->myId) {
          actions.emplace(std::make_pair(
              unit.id,
              myStrategy.getAction(unit, *playerView->game, debug)));
        }
      }
      PlayerMessageGame::ActionMessage(Versioned(actions)).writeTo(*outputStream);
      outputStream->flush();
    }
  }

private:
  std::shared_ptr<InputStream> inputStream;
  std::shared_ptr<OutputStream> outputStream;
};

int main(int argc, char *argv[]) {
  std::string host = argc < 2 ? "127.0.0.1" : argv[1];
  int port = argc < 3 ? 31001 : atoi(argv[2]);
  std::string token = argc < 4 ? "0000000000000000" : argv[3];
  Runner(host, port, token).run();
  return 0;
}

/*
#include <iostream>
#include <time.h>

#include <chrono>
#include <ctime>


#include "../nlohmann/json.hpp"
#include "models/Game.h"
#include "mechanic/Strategy.h"
#include "mechanic/Input.h"

#define VIEWER
//#define REPEATER
//#define LOGGING
//#define LOCAL_RUNNER
//#define VISIO

#ifdef VIEWER
#include "viewer/OpenGLViewer.h"
#endif

#include <iostream>
#include <fstream>

using namespace std;

int main() {

    srand(1000) ;

    string input_string, input_type;

#ifdef LOGGING
    ofstream log;
    log.open ("paper.log");
#endif

    nlohmann::json command;

#ifdef LOCAL_RUNNER
    Input input(InputType::STDIN);
#else
    #ifdef REPEATER
        Input input(InputType::File);
    #else
        #ifdef VISIO
            Input input(InputType::Visio);
        #else
            Input input(InputType::LocalRunner);
        #endif
    #endif

#endif

    input_string = input.get_world_config();

#ifdef LOGGING
    log << input_string << endl;
#endif

    auto json = nlohmann::json::parse(input_string);
    auto world_params = json["params"];

    Strategy strategy(world_params);

#ifdef VIEWER
    OpenGLViewer viewer(Config::maxWidth, Config::maxHeight);
#endif
    long micros_sum = 0;
    while (true) {
        input_string = input.get_tick();

        chrono::system_clock::time_point start = chrono::system_clock::now();

        if (input_string == "") {
            break;
        }

#ifdef LOGGING
        log << input_string << endl;
#endif
        auto json = nlohmann::json::parse(input_string);

        if (json["type"] != "tick") {
            break;
        }

#ifdef REPEATER
        cout << json["params"]["tick_num"] << endl;
#endif
        nlohmann::json command;

        string str_command = strategy.tick(json);

        chrono::system_clock::time_point end = chrono::system_clock::now();
        long micros = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        micros_sum += micros;


        command["command"] = str_command;
        command["debug"] = str_command + ": Tick micros " + to_string(micros) + " sum " + to_string(micros_sum);

        cout << command.dump() << endl;

#ifdef VIEWER
        viewer.draw(*strategy.getWorld());
#endif
    }

    return 0;
}
*/
