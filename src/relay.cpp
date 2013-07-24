
#include <iostream>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include "main.h" 
#include "parser.h"

void connect_to_relay(const char *host, const int port) 
{ 
    struct hostent *hp;
    struct sockaddr_in server; 

    hp = gethostbyname(host); 
    if (hp==NULL) {  
	std::cerr << "maxima failed to resolve hostname (relay)\n"; 
	exit(1); 
	return; 
    }
 
    memset( (char *)&server, 0, sizeof (server));
    memcpy( (char *)&server.sin_addr, hp->h_addr, hp->h_length);
    server.sin_family = hp->h_addrtype;
    server.sin_port = htons((unsigned short)port);

    MainForm.relay_connection = socket(hp->h_addrtype,SOCK_STREAM,0);  
    if (MainForm.relay_connection<0) { 
	std::cerr << "could not create relay socket\n"; 
    }

    int ecode=0; 
    ecode = connect(MainForm.relay_connection, (sockaddr *)&server, sizeof (server)); 
    if (ecode<0) {
	std::cerr << "connect error: " << ecode << "\n";
	return; 
    }

    MainForm.lost_connection = false; 
}

void relay_game(std::string whitename, std::string blackname, std::string whitetitles, std::string blacktitles, 
        int whiterating, int blackrating, int basetime, int increment, int wildnumber) { 
    
    std::ostringstream msg;
    msg << "game" << "\t" << whitename << "\t" << blackname << "\t" << whitetitles << "\t" 
            << blacktitles << "\t" << whiterating << "\t" << blackrating << "\t" << 
            basetime << "\t" << increment << "\t" << wildnumber << "\n"; 
    
    send(MainForm.relay_connection, msg.str().c_str(), strlen(msg.str().c_str()), 0); 
}

void relay_move(int move, int ply) {
    
    std::ostringstream msg;
    msg << "move" << "\t" << move << "\t";
    _fast_SAN(msg, move); 
    msg << "\t" << ply << "\n"; 
        
    send(MainForm.relay_connection, msg.str().c_str(), strlen(msg.str().c_str()), 0); 
}

void relay_time(std::string side, std::string time) { 
    
    std::ostringstream msg;
    msg << "time" << "\t" << side << "\t" << time << "\n"; 
        
    send(MainForm.relay_connection, msg.str().c_str(), strlen(msg.str().c_str()), 0);     
}

void relay_result(std::string code, std::string description) { 
    
    std::ostringstream msg;
    msg << "result" << "\t" << code << "\t" << description << "\n"; 
        
    send(MainForm.relay_connection, msg.str().c_str(), strlen(msg.str().c_str()), 0);     
}

