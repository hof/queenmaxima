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
//
// $Id: run_test.cpp,v 1.18 2002/08/28 09:23:48 hof Exp $
//  
// run a test suite from the database. 
//

#include "fast.h"
#include "engine_impl.h" 
#include "main.h"

int WAC[]   = {2, 55, 71, 100, 141, 163, 176, 180, 196, 210, 229, 235, 237, 241, 243, 247, 248, 265, 270, 999};
int BWTC[]  = {87, 999999};

void run_test (TFastNode* node, char *test, int tmax) 
{
    /*
	std::string fen; 
	std::string commands; 

	int seq = 1;
	int wac = 0;
	int  bwtc = 0;
	int nsum = 0;
	window1 = 750;
	window2 = 3000;
	if (tmax<50) {
		_maxd = tmax * 10;
		tmax = 20000000;
	}
	g.maxtime = tmax;
        g.stoptime = tmax;

        //int solved = 0;
	for (int i=0; i < (1 << _hashsize); i++) { 
		hashtable_0 [i]. key = 0;
	}
	if (test == "WAC") {
		seq = WAC[wac++];
	}
	if (test == "BWTC") {
		seq = BWTC[bwtc++];
	}

        while (MainForm.dbhandle->load_position(test, seq, fen, commands)) {
		fenton (node, fen.c_str());
		_fast_new_game ();
		g.stopsearch = false; 		
		g_timer_reset(g.timer);
		g_timer_start(g.timer); 		
		s_threadstate = 3;
		iterate (node);
		nsum += g.fastnodes;	
		s_threadstate = 0;
		
		// print some stuff 
		std::cout << std::endl; 
		print_move (g.rootmoves [0]. move);
		std::cout << " (ply=" << g.iteration << ", nodes=" << g.fastnodes << " ) " << commands;		
		print_pv();
		std::cout << std::endl; 

		if (test == "WAC") {
                	seq = WAC[wac++];
		} else if (test == "BWTC") {
			seq = BWTC[bwtc++];
		} else {
			seq ++;
		}				
	}
	std::cout << std::endl << "nodes: " << nsum << std::endl; 	

     */
}
