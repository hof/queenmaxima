#include <fstream>
#include "uci.h"
#include "engine_impl.h"
#include "main.h"

bool uci(std::string cmd) {
    bool result = true;
    TInputParser parser(cmd);

    std::ofstream myfile;
    myfile.open("uci.log", std::ios::app);
    myfile << "> " << cmd << std::endl;
    myfile.close();

    std::string token;
    if (parser >> token) { // operator>>() skips any whitespace
        if (token == "quit" || token == "exit" || token == "bye") {
            result = false;
        } else if (token == "stop") {
            result = handleStop();
        } else if (token == "go") {
            result = handleGo(parser);
        } else if (token == "uci") {
            result = handleUci();
        } else if (token == "ucinewgame") {
            result = handleNewGame();
        } else if (token == "isready") {
            result = handleIsReady();
        } else if (token == "position") {
            result = handlePosition(parser);
        } else if (token == "forward") {
            result = handleForward(parser);
        } else if (token == "setoption") {
            result = handleSetOption(parser);
        } else if (token == "ponderhit") {
            result = handlePonderHit();
        } else {
            output("No such option: " + cmd);
        }
    }
    return result;
}

bool handleIsReady() {
    output("readyok");
    return true;
}

bool handleUci() {
    output("id name QueenMaxima");
    output("id author Hof and Hajewiet");
    output("option name OwnBook type check default true");
    output("option name UCI_AnalyseMode type check default false");
    output("option name UCI_Opponent type string");
    output("uciok");
    return true;
}

bool handleNewGame() {
    _fast_new_game();
    set_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    return true;
}

bool handleStop() {
    stop_thinking();
    return true;
}

bool handlePonderHit() {
    //todo: just continue searching, but now set a time limit 
    return true;
}

bool handleGo(TInputParser &parser) {
    MainForm.whitetime = 3000;
    MainForm.blacktime = 3000;
    MainForm.increment = 0;
    /*
     * Todo: start a game search without pondering
     
    game_search(0, 0, MainForm.whitetime, MainForm.blacktime, MainForm.increment);
    */
    return true;
}

bool handlePosition(TInputParser &parser) {
    //todo
    return true;
}

bool handleForward(TInputParser &parser) {
    //todo
    return true;
}

bool handleSetOption(TInputParser &parser) {
    //todo
    return true;
}





