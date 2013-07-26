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

#ifndef ENGINE_IMPL_H
#define ENGINE_IMPL_H

#include <cstring>
#include "defines.h" 
#include "fast.h"
#include "db_pg.h"

struct Tengine_records { 
	bool      maxima_thinking; 
	_int64    hashcode_after_move;  /* hashcode after move is played */
	int       move; 
	int       thinktime; /* in ms */ 
	int       nodes; 	
    int       score; 
};

extern TFastNode engine_rootnode;
extern Tengine_records   engine_records[512];
extern int engine_rootply;

// init 
void init_engine(); 
void set_depth_callback(void (*depth_callback)(char *bestmove, char depth, long nodes, int time, int score));
void set_think_callback(void (*think_callback)(char *bestmove, char depth, long nodes, int time, int score)); 
void set_book_callback (void (*book_callback) (char *bestmove, char *booknamem, int score)); 
void set_claimdraw_callback (void (*claimdraw_callback)(char *reason)); 
void set_fen(const char* fen);
void set_dbhandle(db_pg* dbptr);

// searching 		
void stop_thinking(); 
void search_time(int max_time_in_ms); 
void search_depth(int max_depth);  

// game management 
void game_start(); 
void game_move_forward(int move);
void game_search(int ui_ply, int ui_lastmove, int whitetime, int blacktime, int increment);
void game_end(); 


#endif
