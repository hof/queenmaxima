// QueenMaxima, a chess playing program. 
// Copyright (C) 1996-2013 Erik van het Hof and Hermen Reitsma 
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
// 

#ifndef mainH
#define mainH

#include <string>
#include <vector>
#include <map>

#include "fast.h"
#include "engine_impl.h"
#include "db_base.h"  

struct TMainForm { 
    
    // properties 
    std::map<std::string, std::string> properties; 

    // database handle
    db_base* dbhandle;

    // vars to detect if bugchess' search() has to be called
    int gameply;
    bool playoneven;

    int relayply; 
    
    // icc connection
    int       socket_connection;
    int       relay_connection; 
    bool      lost_connection;

    /* ---- stats about the current game */ 
    bool      continued_game; 

    /* ---- ICC related ---- */ 
    std::string    ICSCurrentLine; /* text we are currently receiving from the server */ 
    int       ICCstate;            /* internet connection parser state */ 
    std::vector<std::string> dgram_fields;  /* fields in an icc datagram */ 
    std::string current_field; // field currently being received 

    int       gamenumber;        /* game number of ics game */ 
    int       basetime;          /* base time of the current game */ 
    int       increment;         /* increment of the current game */
    int       whitetime;         /* time of white */ 
    int       blacktime;         /* time of black */   
    std::string    whitename;         /* name of white player */ 
    std::string    blackname;         /* name of black player */ 
    std::string    white_titles;      /* titles of white player */ 
    std::string    black_titles;      /* titles of black player */ 
    std::string    myname;            /* my name */ 
    int       wildnumber;        /* number of chess variant */ 
    bool      rated;             /* rated or unrated */ 
    int       whiterating;       /* rating of white */ 
    int       blackrating;       /* rating of black */ 
    std::string    rating_type;       /* rating_type like Blitz */ 
	
    bool      autoaccept; 
    bool      autoseek; 
}; 

extern TMainForm MainForm; 

void game_ended(int gameresult);

#endif 
