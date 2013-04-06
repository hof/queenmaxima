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

#include <stdlib.h>
#include <string.h> 

#include "bbtables.h" 
#include "fast.h"
#include "hash.h" 
#include "tables.h"
#include "egtb_lookup.h" 
#include "db_base.h" 

// define changed from stob to stob_init because 
// there is a function stob in bitboard. 

#define stob_init(sq) ((_int64)((_int64)1<<sq))

int findfirst (_int64 bb) 
{
    int result = 0;
    for (int i = 0; i < 64; i ++) {
	if (bb & (_int64) (1) << i) {
	    if (result > 0) {
		return 65;
	    }
	    result = i + 1;
	}
    }
    return result;
}

void InitAttackTables(void) {
    for (int ssq = a1; ssq <= h8; ssq ++) {
	for (int tsq = a1; tsq <= h8; tsq ++) {
	    t.pieceattacks [0] [ssq] [tsq] = false;
	    t.pieceattacks [PAWN] [ssq] [tsq] = (column (ssq) > 0 && ((ssq + 7) == tsq)) ||\
		(column (ssq) < 7 && ((ssq + 9) == tsq));
	    t.pieceattacks [KNIGHT] [ssq] [tsq] = ((knightmoves [ssq] & stob_init (tsq)) != 0); 
	    t.pieceattacks [BISHOP] [ssq] [tsq] = ((bishopmoves [ssq] & stob_init (tsq)) != 0);
	    t.pieceattacks [ROOK] [ssq] [tsq] = ((rookmoves [ssq] & stob_init (tsq)) != 0);
	    t.pieceattacks [QUEEN] [ssq] [tsq] = ((queenmoves [ssq] & stob_init (tsq)) != 0);
	    t.pieceattacks [KING] [ssq] [tsq] = ((kingmoves [ssq] & stob_init (tsq)) != 0);			
	    t.queen_kisses_king [ssq] [tsq] = findfirst (queenmoves [ssq] & kingmoves [tsq]);				
	}
    }
}

void InitNextStuff(void) {
    signed char nunmap[120] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, 0, 1, 2, 3, 4, 5, 6, 7, -1,
	-1, 8, 9, 10, 11, 12, 13, 14, 15, -1,
	-1, 16, 17, 18, 19, 20, 21, 22, 23, -1,
	-1, 24, 25, 26, 27, 28, 29, 30, 31, -1,
	-1, 32, 33, 34, 35, 36, 37, 38, 39, -1,
	-1, 40, 41, 42, 43, 44, 45, 46, 47, -1,
	-1, 48, 49, 50, 51, 52, 53, 54, 55, -1,
	-1, 56, 57, 58, 59, 60, 61, 62, 63, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
    char direc[8][8] = {
	{ 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 10, 9, 11, 0, 0, 0, 0, 0 }, //pawn
	{ 8, -8, 12, -12, 19, -19, 21, -21 }, //knight
	{ 9, 11, -9, -11, 0, 0, 0, 0 }, //bishop
	{ 1, 10, -1, -10, 0, 0, 0, 0 }, //rook
	{ 1, 10, -1, -10, 9, 11, -9, -11 }, //queen
	{ 1, 10, -1, -10, 9, 11, -9, -11 }, //king
	{ -10, -9, -11, 0, 0, 0, 0, 0 },    //bpawn 
    };

    char Stboard[64] = {
	ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK,
	PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN,
	ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK};

    char max_steps[8] = {0, 2, 1, 7, 7, 7, 1, 2};
    unsigned char ptyp, po, p0, d, s, di;
    char delta;
    char *ppos, *pdir;
    char dest[8][8];
    char steps[8];
    unsigned char sorted[8];
    for (ptyp = 0; ptyp < 8; ptyp++)
	for (po = 0; po < 64; po++)
	    for (p0 = 0; p0 < 64; p0++) {
		t.nextpos[ptyp][po][p0] = (char) po;
		t.nextdir[ptyp][po][p0] = (char) po;
	    }
    for (ptyp = 1; ptyp < 8; ptyp++)
	for (po = 21; po < 99; po++)
	    if (nunmap[po] >= 0) {
		ppos = t.nextpos[ptyp][nunmap[po]];
		pdir = t.nextdir[ptyp][nunmap[po]];
		for (d = 0; d < 8; d++) {
		    dest[d][0] = nunmap[po];
		    delta = direc[ptyp][d];
		    if (delta != 0) {
			p0 = po;
			for (s = 0; s < max_steps[ptyp]; s++) {
			    p0 = char(p0 + delta);
			    if ((nunmap[p0] < 0) || (((ptyp == PAWN) || (ptyp == BPAWN))
						     && ((s > 0) && ((d > 0) || (Stboard[nunmap[po]] != PAWN)))))
				break;
			    else
				dest[d][s] = nunmap[p0];
			}
		    } else
			s = 0;
		    steps[d] = s;
		    for (di = d; s > 0 && di > 0; di--)
			if (steps[sorted[di - 1]] == 0)	/* should be: < s */
			    sorted[di] = sorted[di - 1];
			else
			    break;
		    sorted[di] = d;
		}
		p0 = nunmap[po];
		if (ptyp == PAWN || ptyp == BPAWN) {
		    for (s = 0; s < steps[0]; s++) {
			ppos[p0] = (char) dest[0][s];
			p0 = dest[0][s];
		    }
		    p0 = nunmap[po];
		    for (d = 1; d < 3; d++) {
			pdir[p0] = (char) dest[d][0];
			p0 = dest[d][0];
		    }
		} else {
		    pdir[p0] = (char) dest[sorted[0]][0];
		    for (d = 0; d < 8; d++)
			for (s = 0; s < steps[sorted[d]]; s++) {
			    ppos[p0] = (char) dest[sorted[d]][s];
			    p0 = dest[sorted[d]][s];
			    if (d < 7)
				pdir[p0] = (char) dest[sorted[d + 1]][0];
			}
		}
	    }
}

void init_sqonline (void) 
{
    char sq;	
    for (unsigned char i = 0; i <= 63; i++) {
	for (unsigned char j = 0; j <= 63; j++) {
	    t.sqonline [i] [j] = i;
	    sq = t.nextpos [QUEEN] [i] [j];
	    if (sq != t.nextdir [QUEEN] [i] [j]) {
		t.sqonline [i] [j] = sq;			
	    }
	}
    }
}

void InitDirectionTables (void) //todo: 0x88
{
    int ssq_x,
	ssq_y,
	tsq_x,
	tsq_y,
	dx,
	dy;
    for (int ssq = 0; ssq < 64; ssq ++) {
	for (int tsq = 0; tsq < 64; tsq ++) {
	    ssq_x = column (ssq);
	    ssq_y = row (ssq);
	    tsq_x = column (tsq);
	    tsq_y = row (tsq);
	    dx = ssq_x - tsq_x;
	    dy = ssq_y - tsq_y;
	    if (dx < 0) {
		dx = - dx;
	    }
	    if (dy < 0) {
		dy = -dy;
	    }
	    if (dx == 0 && dy) {
		if (ssq < tsq) {
		    t.direction [ssq] [tsq] = (char) 8;
		    t.squares_to_edge [ssq] [tsq] = 7 - tsq_y; 
		} else {
		    t.direction [ssq] [tsq] = (char) - 8;
		    t.squares_to_edge [ssq] [tsq] = tsq_y;
		}
	    } else if (dy == 0 && dx) {
		if (ssq < tsq) {
		    t.direction [ssq] [tsq] = (char) 1;
		    t.squares_to_edge [ssq] [tsq] = 7 - tsq_x;
		} else {
		    t.direction [ssq] [tsq] = (char) - 1;
		    t.squares_to_edge [ssq] [tsq] = tsq_x;
		}      
	    } else if (dy == dx && dx) {
		if (ssq_y < tsq_y) {
		    if (ssq_x < tsq_x) {
			t.direction [ssq] [tsq] = (char) 9;
			t.squares_to_edge [ssq] [tsq] = MIN (7 - tsq_x, 7 - tsq_y);
		    } else {
			t.direction [ssq] [tsq] = (char) 7;
			t.squares_to_edge [ssq] [tsq] = MIN (tsq_x, 7 - tsq_y);
		    }
		} else {
		    if (ssq_x < tsq_x) {
			t.direction [ssq] [tsq] = (char) - 7;	
			t.squares_to_edge [ssq] [tsq] = MIN (7 - tsq_x, tsq_y);
		    } else {
			t.direction [ssq] [tsq] = (char) - 9;	
			t.squares_to_edge [ssq] [tsq] = MIN (tsq_x, tsq_y);
		    }					 
		}
	    } else {       
		t.direction [ssq] [tsq] = 0;
		t.squares_to_edge [ssq] [tsq] = 0;
	    }
	}
    }
}

void Master_Init(void) 
{
    g.last_known_nps = 40000;   	
    InitAttackTables ();       
    InitNextStuff ();       
    InitDirectionTables ();
    init_sqonline ();

    InitHash();     
    
    g.timer = g_timer_new();
    g_timer_start(g.timer); 		
    g.dbhandle = new db_base(); 
};









