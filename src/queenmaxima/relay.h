
#ifndef RELAY_H 
#define RELAY_H 

void connect_to_relay(const char *host, const int port);

void relay_game(std::string whitename, std::string blackname, std::string whitetitles, std::string blacktitles, 
        int whiterating, int blackrating, int basetime, int increment, int wildnumber); 

void relay_move(int move, int ply); 

void relay_time(std::string side, std::string time); 

void relay_result(std::string code, std::string description); 

#endif 
