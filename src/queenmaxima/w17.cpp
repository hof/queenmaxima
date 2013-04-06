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

#include <string.h> 
#include <stdlib.h> 

#include "fast.h" 
#include "w17.h" 
#include "db_base.h" 
#include "hash.h"
#include "attack.h"
#include "book.h" 
#include "tm_icc.h"

/* global tables */ 
int sort_w [4096];
int sort_b [4096];
int sort_cap_w [4096];
int sort_cap_b [4096];
int sort_eva_w [4096];
int sort_eva_b [4096];

/* tree statistics */ 
int state_count[8]; 

void w17_init_iterate(TFastNode* node) 
{
  memset (sort_w, 0, 4096 * 4);
  memset (sort_b, 0, 4096 * 4);
  memset (sort_cap_w, 0, 4096 * 4);
  memset (sort_cap_b, 0, 4096 * 4);	
  memset (state_count, 0, 8 * 4); 	
}

void w17_new_game()
{
  g.checkbook = true; 
  memset (sort_w, 0, 4096 * 4);
  memset (sort_b, 0, 4096 * 4);
  memset (sort_cap_w, 0, 4096 * 4);
  memset (sort_cap_b, 0, 4096 * 4);	
}


bool w17_lookup_books_wtm (TFastNode* node, int& score, int& avoid, int tmax) 
{
  int value = - INFINITY,
    wwin = 0, 
    bwin = 0,
    draw = 0,
    flags = 0,
    dbavoid =0;

  avoid = 0;

  if (g.dbhandle->w17_database_lookup_book (node -> hashcode, wwin, draw, bwin, flags)) {
    bwin += draw;
    if (flags & 63) {
      value = MATE - (flags & 63);			
      if (flags & 64) {				
	score = - value;
	avoid = 1; 
	return true;
      }
    } else {
      if (!bwin) {
	value = 100;
      } else {
	value = (wwin / bwin);				
      }				
    }		
    dbavoid = flags >> 7;
    if (dbavoid > 7) { // agen
      g.dbhandle->w17_database_unage(node->hashcode); 
      dbavoid = flags >> 10;
    }
    if (dbavoid) {
      avoid = dbavoid; 
      score = - INFINITY;
      return true; 
    }
    if (! value) {
      avoid = 1; 
      score = - INFINITY;
      return true; 
    }
    score = value; 
    return true; 
  }
  return value;
}

bool w17_lookup_books_btm (TFastNode* node, int& score, int& avoid, int tmax)
{
  int value = - INFINITY,
    wwin = 0, 
    bwin = 0,
    draw = 0,
    flags = 0,
    dbavoid = 0;
  
  avoid=0;
  
  if (g.dbhandle->w17_database_lookup_book (node -> hashcode, wwin, draw, bwin, flags)) {
    wwin += draw;		
    if (flags & 63) {			
      value = MATE - (flags & 63);		
      if (flags & 64) {
	avoid = 1; 
	score = - value;
	return true;
      }
    } else {
      if (!wwin) {
	value = 100;
      } else {
	value = (bwin / wwin);
      }					
    }
    dbavoid = flags >> 7;
    if (dbavoid > 7) { // agen
      g.dbhandle->w17_database_unage(node->hashcode); 
      dbavoid = flags >> 10;
    }
    if (dbavoid) {
      avoid = dbavoid; 
      score = - INFINITY;
      return true; 
    }
    if (!value) {
      avoid = 1; 
      score = - INFINITY;
      return true; 
    }
    score = value; 
    return true; 
  } 
  return false; 
}

bool w17_trivial_draw_w (TFastNode* node, int ply) 
{	
	return node -> fifty > 98 || draw_by_rep (node, ply);
}

bool w17_trivial_draw_b (TFastNode* node, int ply) 
{
	return node -> fifty > 98 || draw_by_rep (node, ply);
}


int w17_genrootmoves_w (TFastNode* node) 
{
	int last = _fast_gencapsw (node, 0),
		index = 0,	       
		move,		
		value=-INFINITY,
		fifty = node -> fifty,
		flags = node -> flags,
		best = - INFINITY,
		i;
	bool inbook;
	int bookscore, bookavoid;
	
	for (i = 0; i < last; i ++) {
		move = g.tmoves [i];
		_fast_dowmove (node, move);
		if (! attacked_by_pnbrqk (node, node -> wkpos)) {			
			g.rootmoves [index]. move = move;
			g.rootmoves [index]. avoid = 0;
			g.rootmoves [index]. bookvalue = 0;
			g.rootmoves [index]. matevalue = 0;
			g.rootmoves [index]. value = 0;
			g.rootmoves [index]. nodes = 0;		
			g.rootmoves [index]. draw = w17_trivial_draw_w (node, 1);
			g.rootmoves [index]. fifty = node -> fifty;
			g.rootmoves [index]. unknown = false;
			g.rootmoves [index]. exact_score = false; 
			g.rootmoves [index]. bookmove = false;
                        g.rootmoves [index]. forced_move = false;
			
			if (last > 1 && g.checkbook && !g.rootmoves[index].draw) {
				inbook = w17_lookup_books_wtm (node, bookscore, bookavoid, g.tmax);
				if (bookavoid==0 && inbook) {

					g.rootmoves[index].bookmove = true;
					g.rootmoves[index].bookvalue = bookscore;
				}
				if (bookavoid && inbook) {
					g.rootmoves[index].avoid = bookavoid; 
				}
			} 	
			
			index ++;
		}
		_fast_undowmove (node, move, flags, fifty);
		if (value > best) {
			best = value;			
			g.pv[1][1]=0;
			root_new_best (node, index-1, value);
		}			
	}
 
	if (index) {				
		return index;
	}
	
	last = _fast_gennoncapsw (node, 0);
	
	for (i = 0; i < last; i ++) {
		move = g.tmoves [i];		
		if (move == ENCODESCW && (attacked_by_pnbrqk (node, e1) || attacked_by_pnbrqk (node, f1))) {
			continue;
		}
		if (move == ENCODELCW && (attacked_by_pnbrqk (node, e1) || attacked_by_pnbrqk (node, d1))) {
			continue;
		}
		_fast_dowmove (node, move);
		if (! attacked_by_pnbrqk (node, node -> wkpos)) {			
			g.rootmoves [index]. move = move;
			g.rootmoves [index]. avoid = 0;
			g.rootmoves [index]. bookvalue = 0;
			g.rootmoves [index]. matevalue = 0;
			g.rootmoves [index]. value = 0;
			g.rootmoves [index]. nodes = 0;
			g.rootmoves [index]. draw = w17_trivial_draw_w (node, 1);
			g.rootmoves [index]. fifty = node -> fifty;
			g.rootmoves [index]. unknown = false;
			g.rootmoves [index]. exact_score = false; 
			g.rootmoves [index]. bookmove = false; 
                        g.rootmoves [index]. forced_move = false;
			
			if (last > 1 && g.checkbook && !g.rootmoves[index].draw) {
				inbook = w17_lookup_books_wtm (node, bookscore, bookavoid, g.tmax);
				if (bookavoid==0 && inbook) {

					g.rootmoves[index].bookmove = true;
					g.rootmoves[index].bookvalue = bookscore;
				}
				if (bookavoid && inbook) {
					g.rootmoves[index].avoid = bookavoid; 
				}
			} 	

			index ++;
		}
		_fast_undowmove (node, move, flags, fifty);
		if (value > best) {
			best = value;
			g.pv[1][1]=0;
			root_new_best (node, index-1, value);
		}
	}	
	return index;
}


int w17_genrootmoves_b (TFastNode* node) 
{
	int last = _fast_gencapsb (node, 0),
		index = 0,	      
		fifty = node -> fifty,
		move,		
		value,
		flags = node -> flags,
		best = - INFINITY,
		i;

	bool inbook;
	int bookscore, bookavoid;

	for (i = 0; i < last; i ++) {
		move = g.tmoves [i];
		_fast_dobmove (node, move);
		value = -INFINITY; 
		if (! attacked_by_PNBRQK (node, node -> bkpos)) {
			g.rootmoves [index]. move = move;
			g.rootmoves [index]. avoid = 0;
			g.rootmoves [index]. bookvalue = 0;
			g.rootmoves [index]. matevalue = 0;	
			g.rootmoves [index]. value = 0;    
			g.rootmoves [index]. nodes = 0;			
			g.rootmoves [index]. draw = w17_trivial_draw_b (node, 1);
			g.rootmoves [index]. unknown = false;
			g.rootmoves [index]. fifty = node -> fifty;
			g.rootmoves [index]. exact_score = false; 
			g.rootmoves [index]. bookmove = false; 
                        g.rootmoves [index]. forced_move = false;

			if (last > 1 && g.checkbook && !g.rootmoves[index].draw) {
				inbook = w17_lookup_books_btm (node, bookscore, bookavoid, g.tmax);
                                if (bookavoid==0 && inbook) {
                                        g.rootmoves[index].bookmove = true;
                                        g.rootmoves[index].bookvalue = bookscore;
                                }
				if (bookavoid && inbook) {
					g.rootmoves[index].avoid = bookavoid;
				}
			}       	     
		
			index ++;

		}
		_fast_undobmove (node, move, flags, fifty);
		if (value > best) {
			best = value;
			g.pv[1][1]=0;
			root_new_best (node, index-1, value);
		}		
	}
	
	if (index) {
		return index;
	}
		
	last = _fast_gennoncapsb (node, 0);
	
	for (i = 0; i < last; i ++) {
		move = g.tmoves [i];		
		if (move == ENCODESCB && (attacked_by_PNBRQK (node, e8) || attacked_by_PNBRQK (node, f8))) {
			continue;
		}
		if (move == ENCODELCB && (attacked_by_PNBRQK (node, e8) || attacked_by_pnbrqk (node, d8))) {
			continue;
		}
		_fast_dobmove (node, move);
		value = -INFINITY; 
		if (! attacked_by_PNBRQK (node, node -> bkpos)) {			
			g.rootmoves [index]. move = move;
			g.rootmoves [index]. avoid = 0;
			g.rootmoves [index]. bookvalue = 0;
			g.rootmoves [index]. matevalue = 0;
			g.rootmoves [index]. value = 0;
			g.rootmoves [index]. nodes = 0;
			g.rootmoves [index]. draw = w17_trivial_draw_b (node, 1);
			g.rootmoves [index]. fifty = node -> fifty;
			g.rootmoves [index]. unknown = false;
			g.rootmoves [index]. exact_score = false; 
			g.rootmoves [index]. bookmove = false;
                        g.rootmoves [index]. forced_move = false; 

			if (last > 1 && g.checkbook && !g.rootmoves[index].draw) {
				inbook = w17_lookup_books_btm (node, bookscore, bookavoid, g.tmax);
                                if (bookavoid==0 && inbook) {
                                        g.rootmoves[index].bookmove = true;
                                        g.rootmoves[index].bookvalue = bookscore;
                                }
				if (bookavoid && inbook) {
					g.rootmoves[index].avoid = bookavoid;
				}
			}       	     
			index ++;
		}
		_fast_undobmove (node, move, flags, fifty);
		if (value > best) {
			best = value;
			g.pv[1][1]=0; 
			root_new_best (node, index-1, value);
		}		
	}	
	return index;	
}    

int w17_select_capture_w (TFastNode* node, int first, int & last) 
{
	int move = g.tmoves [first],	
		index = first,		
		value,
		best = sort_cap_w [move & 4095],
		tsq;
    
	if (attacked_by_pnbrqk (node, TARGETSQ (move))) {
		best += 5;
	}
	for (int i = first + 1; i < last; i ++) {	
		move = g.tmoves [i];
		tsq = TARGETSQ (move);
		value = sort_cap_w [move & 4095];
		if (attacked_by_pnbrqk (node, tsq)) {
			value += 5;
		}
		if (value > best) {
			best = value;
			index = i;
		}
	}	
	move = g.tmoves [index];
	g.tmoves [index] = g.tmoves [-- last];
	return move;
}

int w17_select_capture_b (TFastNode* node, int first, int & last) 
{
	int move = g.tmoves [first],	
		index = first,		
		value,
		best = sort_cap_b [move & 4095],
		tsq;

	if (attacked_by_PNBRQK (node, TARGETSQ (move))) {
		best += 5;
	}
	for (int i = first + 1; i < last; i ++) {	
		move = g.tmoves [i];
		tsq = TARGETSQ (move);
		value = sort_cap_b [move & 4095];
		if (attacked_by_PNBRQK (node, tsq)) {
			value += 5;
		}
		if (value > best) {
			best = value;
			index = i;
		}
	}	
	move = g.tmoves [index];
	g.tmoves [index] = g.tmoves [-- last];
	return move;
}

/* ---- some move sortings to try ---- */ 

//int w17_select_noncap_w_keep (TFastNode* node, int first, int &last, int eval_state)
//{
//	/* try to keep pieces by moving to non attacked fields first */ 
//	int move = g.tmoves [first],
//		i, value, 
//		best = _w17_sort_w [move & 4095],
//		index = first; 
	
       

int w17_select_noncap_w_dump (TFastNode* node, int first, int &last, int eval_state)
{
  /* try to get rid of stuff - in history order */ 
  /* init */ 
  int move = g.tmoves [first],
    i,  
    value,
    best = sort_w [move & 4095],
    index = first;
  bool attacked = attacked_by_pnbrqk (node, TARGETSQ (move)); 

  /* search best move */ 
  for (i = first + 1; i < last; i ++) {
    move = g.tmoves [i];
    value = sort_w [move & 4095];		
    bool dumpable = attacked_by_pnbrqk (node, TARGETSQ (move)); 

    if (dumpable && !attacked) { 
      index = i; 
      attacked = true; 
      best = value; 
    } else if (dumpable && attacked) {
      if (value > best) {
	index = i;
	best = value;
      }
    } else if (!dumpable && !attacked) { 
      if (value < best) { 
	index = i; 
	best = value; 
      }
    }
  }

  /* swap move */ 
  move = g.tmoves [index];
  g.tmoves [index] = g.tmoves [-- last];
  return move;			

}

int w17_select_noncap_b_dump (TFastNode* node, int first, int &last, int eval_state)
{
  /* try to get rid of stuff - in history order */ 
  /* init */ 
  int move = g.tmoves [first],
    i, 
    value,
    best = sort_b [move & 4095],
    index = first;
  bool attacked = attacked_by_PNBRQK (node, TARGETSQ( move)); 

  /* search best move */ 
  for (i = first + 1; i < last; i++) { 
    move = g.tmoves [i];
    value = sort_b [move & 4095];
    bool dumpable = attacked_by_PNBRQK (node, TARGETSQ( move)); 

    if (dumpable && !attacked) { 
      index = i; 
      attacked = true; 
      best = value; 
    } else if (dumpable && attacked) { 
      if (value > best) {
	index = i; 
	best = value;
      }
    } else if (!dumpable && !attacked) { 
      if (value > best) { 
	index = i; 
	best = value; 
      }
    }
  }
  
  /* swap */ 
  move = g.tmoves [index];
  g.tmoves [index] = g.tmoves [-- last];
  return move;			

}

int w17_select_noncap_w_bigpiecesfirst (TFastNode* node, int first, int &last, int eval_state)
{
  /* idea: try to dump Q first, followed by R B N. */ 

  /* init */ 
  int move = g.tmoves [first],
    i,  
    value, piece, 
    best = sort_w [move & 4095],
    bestpiece = PIECE(move),
    index=first; 

  if (bestpiece==KING) { 
    bestpiece = 0; 
  }

  /* search best move */ 
  for (i = first + 1; i < last; i ++) {
    move = g.tmoves [i];
    value = sort_w [move & 4095];		
    piece = PIECE(move); 
    if (piece<KING && piece>bestpiece) {
      index = i; 
      bestpiece = piece; 
      best = value; 
    } else if (piece<KING && piece==bestpiece) { 
      if (value > best) {
	index = i;
	best = value;
      }
    }
  }

  /* swap move */ 
  move = g.tmoves [index];
  g.tmoves [index] = g.tmoves [-- last];
  return move;			

}

int w17_select_noncap_b_bigpiecesfirst (TFastNode* node, int first, int &last, int eval_state)
{
  /* idea: try to dump Q first, followed by R B N. */ 
  /* init */ 
  int move = g.tmoves [first],
    i,  
    value, piece, 
    best = sort_w [move & 4095],
    bestpiece = PIECE(move),
    index=first; 

  if (bestpiece==KING) { 
    bestpiece = 0; 
  }

  /* search best move */ 
  for (i = first + 1; i < last; i ++) {
    move = g.tmoves [i];
    value = sort_w [move & 4095];		
    piece = PIECE(move); 
    if (piece<KING && piece>bestpiece) { 
      index=i; 
      bestpiece=piece; 
      best=value; 
    } else if (piece<KING && piece==bestpiece) { 
      if (value > best) {
	index = i;
	best = value;
      }
    }
  }

  /* swap move */ 
  move = g.tmoves [index];
  g.tmoves [index] = g.tmoves [-- last];
  return move;			
}


int w17_select_noncap_w_history (TFastNode* node, int first, int &last, int eval_state)
{
  /* init */ 
  int move = g.tmoves [first],
    i,  
    value,
    best = sort_w [move & 4095],
    index = first;

  /* search best move */ 
  for (i = first + 1; i < last; i ++) {
    move = g.tmoves [i];
    value = sort_w [move & 4095];		
    if (value > best) {
      index = i;
      best = value;
    }
  }

  /* swap move */ 
  move = g.tmoves [index];
  g.tmoves [index] = g.tmoves [-- last];
  return move;			

}

int w17_select_noncap_b_history (TFastNode* node, int first, int &last, int eval_state)
{
  /* init */ 
  int move = g.tmoves [first],
    i, 
    value,
    best = sort_b [move & 4095],
    index = first;

  /* search best move */ 
  for (i = first + 1; i < last; i++) { 
    move = g.tmoves [i];
    value = sort_b [move & 4095];
    if (value > best) {
      index = i; 
      best = value;
    }
  }
  
  /* swap */ 
  move = g.tmoves [index];
  g.tmoves [index] = g.tmoves [-- last];
  return move;			

}

/* ------------------------------------------------------------------------ */ 

int w17_select_noncapture_w (TFastNode* node, int first, int & last, int eval_state) 
{
	return w17_select_noncap_w_history(node,first,last,eval_state);
}

int w17_select_noncapture_b (TFastNode* node, int first, int & last, int eval_state) 
{
	return w17_select_noncap_b_history(node,first,last,eval_state); 
}

/* ------------------------------------------------------------------------ */
/* history: 1.a3 d6     __fastnodes==501844 */ 
/* bigpieces: 1.a3 d6   __fastnodes==504391 */ 
/* dump 1.a3 d6         __fastnodes==530543 */ 
/* kant die zoekt voor mate --> dump | 
   andere kant --> history: __fastnodes==486907 */ 

/* evaluate score tables */ 

//static int w17_PawnTable [8] [8] = 
//{
//	{   0, -10,-100,-200,-300,-400,-500,-600},
//	{  10,   0, -10, -90,-190,-290,-390,-490},
//	{ 100,  10,   0, -10, -80,-180,-280,-380},
//	{ 200,  90,  10,   0, -10, -70,-170,-270},
//	{ 300, 190,  80,  10,   0, -10, -60,-160},
//	{ 400, 290, 180,  70,  10,   0, -10, -50},
//	{ 500, 390, 280, 170,  60,  10,   0, -10},
//	{ 600, 490, 380, 270, 160,  50,  10,   0}
//};

static int w17_PawnTable [8] [8] = 
{
	{   0,-100,-200,-300,-400,-500,-600,-700},
	{ 100,   0, -99,-199,-299,-399,-499,-599},
	{ 200,  99,   0, -98,-198,-298,-398,-498},
	{ 300, 199,  98,   0, -97,-197,-297,-397},
	{ 400, 299, 198,  97,   0, -96,-196,-296},
	{ 500, 399, 298, 197,  96,   0, -95,-195},
	{ 600, 499, 398, 297, 196,  95,   0, -94},
	{ 700, 599, 498, 397, 296, 195,  94,   0}
};

static int w17_PieceTable[8][8] = {
     {   0,  -250,  -440,  -630,  -820, -1010, -1200, -1390},
     { 250,     0,  -240,  -430,  -620,  -810, -1000, -1190},
     { 440,   240,     0,  -230,  -420,  -610,  -800,  -990},
     { 630,   430,   230,     0,  -220,  -410,  -600,  -790},
     { 820,   620,   420,   220,     0,  -210,  -400,  -590},
     {1010,   810,   610,   410,   210,     0,  -200,  -390},
     {1200,  1000,   800,   600,   400,   200,     0,  -190},
     {1390,  1190,   990,   790,   590,   390,   190,     0}};


char _w17_pawn_table_w [64] = 
{
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1,
	2, 2, 2, 2, 2, 2, 2, 2,
	3, 3, 3, 3, 3, 3, 3, 3,
	4, 4, 4, 4, 4, 4, 4, 4,
	5, 5, 5, 5, 5, 5, 5, 5,
	0, 0, 0, 0, 0, 0, 0, 0		
};

char _w17_pawn_table_b [64] = 
{
	0, 0, 0, 0, 0, 0, 0, 0,	
	5, 5, 5, 5, 5, 5, 5, 5,
	4, 4, 4, 4, 4, 4, 4, 4,
	3, 3, 3, 3, 3, 3, 3, 3,
	2, 2, 2, 2, 2, 2, 2, 2,
	1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0		
};

int w17_evaluate_pawns_w (TFastNode * node) 
{
	//fixme: check pionnenhash
	int sq,
		value = 0;
	for (int i = 0; i < node -> wpawns; i ++) {
		sq = node -> wpawnlist [i];
		value += _w17_pawn_table_w [sq];
	}

	//fixme: opslaan in pionnenhash
	return value;
}

int w17_evaluate_pawns_b (TFastNode * node) 
{
	//fixme: check pionnenhash
	int sq,
		value = 0;
	for (int i = 0; i < node -> bpawns; i ++) {
		sq = node -> bpawnlist [i];
		value += _w17_pawn_table_b [sq];
	}
	
	//fixme: opslaan in pionnenhash
	return value;
}

void w17_print_state_stats() 
{ 
  g_print("PIECES_PAWNS: %d\n", state_count[W17_PIECES_PAWNS]); 
  g_print("PIECES      : %d\n", state_count[W17_PIECES]); 
  g_print("WNOPAWNS    : %d\n", state_count[W17_WNOPAWNS]); 
  g_print("BNOPAWNS    : %d\n", state_count[W17_BNOPAWNS]); 
  g_print("NOPIECES    : %d\n", state_count[W17_NOPIECES]); 
  g_print("NOWPIECES   : %d\n", state_count[W17_NOWPIECES]); 
  g_print("NOBPIECES   : %d\n", state_count[W17_NOBPIECES]); 
}

int w17_eval_state(TFastNode *node)
{

  if (node -> wpieces && node -> bpieces && node -> wpawns && node -> bpawns) { 
    state_count[W17_PIECES_PAWNS]++; 
    return W17_PIECES_PAWNS;
  }

  if (node->wpawns == 0 && node->bpawns == 0) {
    state_count[W17_PIECES]++; 
    return W17_PIECES; 
  }

  if (node->wpawns == 0) { 
    /* wit heeft geen pionnen meer, zwart nog wel... */ 
    state_count[W17_WNOPAWNS]++; 
    return W17_WNOPAWNS; 
  }

  if (node->bpawns == 0) {
    /* zwart heeft geen pionnen meer, wit nog wel... */ 
    state_count[W17_BNOPAWNS]++; 
    return W17_BNOPAWNS; 
  }

  if (node->wpieces == 0 && node->bpieces == 0) {
    state_count[W17_NOPIECES]++; 
    return W17_NOPIECES; 
  }
		
  if (node->wpieces == 0) { 
    state_count[W17_NOWPIECES]++; 
    return W17_NOWPIECES; 
  }

  if (node->bpieces == 0) { 
    state_count[W17_NOBPIECES]++; 
    return W17_NOBPIECES;
  }

  g_assert_not_reached(); 
  return 0; 
}

int w17_evaluate_w(TFastNode *node, int ply, int eval_state)
{
  int score;
  int wp,bp,i;        
 	
  switch(eval_state) { 
  case W17_PIECES_PAWNS:	
    score = 0; 	
   
    /* heuristic: lopers op diagonalen is nogal slecht */ 
    //
    // for (int i=0; i<node->wbishops; i++) {
    //       if (diagonaal[node->wnishop[i]]) { 
    //           score -= 600; 
    //       }
    // }
    // 
    // for (int i=0; i<node->bbishops; i++) {
    //       if (diagonaal[node->bbishop[i]]) {
    //           score += 600; 
    //       }
    // }
    
    /* heuristic: king in the middle is not good. */ 
    if ((row(node->wkpos) > 2) && (node->wpieces > 4)) { 
      score -= 100; 
     }
	  
    if ((row(node->bkpos) < 7) && (node->bpieces > 4)) { 
      score += 100; 
     }
	  
    /* general scoring ... */ 
    wp = (node->wpieces > 8) ? 7 : node->wpieces - 1; 
    bp = (node->bpieces > 8) ? 7 : node->bpieces - 1; 		
	  
    score += w17_PawnTable[node->wpawns-1][node->bpawns-1]; 
    score += w17_PieceTable[wp][bp]; 
    score += w17_evaluate_pawns_w (node);
    score -= w17_evaluate_pawns_b (node);
	  
    //printf("eval_w: wp=%d bp=%d wpawn=%d bpawn=%d score=%d\n",wp,bp,node->wpawns,node->bpawns,score); 
    g_assert (score > - INFINITY && score < INFINITY);	
	  
    return score; 
  case W17_PIECES:
    return (node->bpieces - node->wpieces) * 100; 
  case W17_WNOPAWNS: 
    /* wit heeft geen pionnen meer, zwart nog wel... */ 
    return 20000 - (node->wpieces * 100); 
    
  case W17_BNOPAWNS:
    /* zwart heeft geen pionnen meer, wit nog wel... */ 
    return  -20000 + (node->bpieces * 100); 
    
  case W17_NOPIECES:
    return (node->wpawns - node->bpawns)*100; 
	
  case W17_NOBPIECES:
    /* zwart geen stukken meer */ 
    score = 10000; /* level score */ 
    score -= (node->bpawns + node->wpawns)*100; /* geen pionnen */ 
    score += node->wpieces * 100; /* wel stukken */ 
  
    /* endgame heuristic - blockable pawns */ 
    if (node->bpawns==1 && node->wpawns==1) {
      if ((column(node->wpawnlist[0]) != column(node->bpawnlist[0])) && 
    	  (node->wbishops | node->wknights) ) {
    score += 5000;
      }
    }
	  
    /* endgame heuristic - white pawns promote */ 
    for (i=0; i<node->wpawns; i++) {
      score += row(node->wpawnlist[i])*10; 
    }
    
    /* endgame heuristic - drive black king to our last pawn */
    if (node -> wpawns == 1) {
      score -= _fast_distance (node -> bkpos, node -> wpawnlist [0]);
    }
    
    return score; 
  case W17_NOWPIECES:

    /* wit geen stukken meer */ 
    score = -10000; 
    score += (node->wpawns+node->bpawns)*100; 
    score -= node->bpieces*100; 
    
    /* endgame heuristic - blockable pawns */ 
    if (node->bpawns==1 && node->wpawns==1) { 
      if ((column(node->wpawnlist[0]) != column(node->bpawnlist[0])) && 
	  (node->bbishops | node->bknights)) {
	score -= 5000; 
      }
    }
    
    /* endgame heuristic - black pawns promote */ 
    for (i=0; i<node->bpawns; i++) {
      score -= 8-row(node->bpawnlist[i])*10;
    }
    
    /* endgame heuristic - drive black king to our last pawn */
    if (node -> bpawns == 1) {
      score += _fast_distance (node -> wkpos, node -> bpawnlist [0]);
    } 

    g_assert (score > - INFINITY && score < INFINITY);
    return score; 
  }
  
  g_assert_not_reached ();
  return INVALID;		
}

int w17_evaluate_b(TFastNode *node, int ply, int eval_state)
{
	return - (w17_evaluate_w(node,ply,eval_state));
}

