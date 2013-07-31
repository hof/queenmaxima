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

#include "fast.h"
#include "w0.h" 
#include "legality.h"
#include "attack.h" 
#include "parser.h"
#include "book.h"
#include "tm_icc.h"
#include "engine_impl.h"
#include "main.h"

bool try_egtb (TFastNode * node, int ply)
{
	if (ply==1) {
		return true;
	}
	if (node->fifty) {
		return false;
	}
	if ((node->wpieces+node->bpieces) > 2) {
		return false;
	}
	int move = g.path[ply-1];
	if (_CAPTURE(move) || SPECIAL(move)) {
		return true;
	}
	return false;
}

void init_evasions_w (TFastNode * node, int first, int & last, int killerindex, int hashmove) 
{
	int zet,
		value,		
		stuk,
		flags = node -> flags,
		fifty = node -> fifty;
		
	while (first < last) {
		zet = g.tmoves [first];
		if (zet == hashmove) {
			value = MATE;
		} else {
			stuk = PIECE (zet);
			// verwijder uit lijst als de zet niet legaal is 
			if (! legal_move_w (node, zet)) {
				g.tmoves [first] = g.tmoves [-- last];
				continue;
			}
			_fast_dowmove (node, zet);
			if (attacked_by_pnbrqk (node, node -> wkpos)) {
				g.tmoves [first] = g.tmoves [-- last];
				_fast_undowmove (node, zet, flags, fifty);
				continue;
			}	       
			// bepaal sorteerwaarde van de zet voor de megasort-tabel				
			value = g.tsort [zet & 4095];						
			if (stuk == KING) {
				if (flags & (64 | 128)) {
					value -= 1;
				}
			} else if (! _CAPTURE (zet)) { // a blocking move.
				value += piece_val [QUEEN] - piece_val [stuk];			
				if (! attacked_by_PNBRQK (node, TARGETSQ (zet))) { 
					value -= piece_values [stuk];  
				}
			}
			_fast_undowmove (node, zet, flags, fifty);				
			if (_CAPTURE (zet)) {
				if (stuk == KING || profcap_w (node, zet)) {				
					value += piece_values [CAPTURED (zet)] - piece_val [stuk];
				} else {
					value += piece_values [CAPTURED (zet)] - piece_values [stuk];
				}
			}		
		}
		g.megasort [first ++] = value;
	}       
}

void init_evasions_b (TFastNode * node, int first, int & last, int killerindex, int hashmove) 
{
	int zet,
		value,		
		stuk,
		flags = node -> flags,
		fifty = node -> fifty;
		
	while (first < last) {
		zet = g.tmoves [first];
		if (zet == hashmove) {
			value = MATE;
		} else {
			stuk = PIECE (zet);
			// verwijder uit lijst als de zet niet legaal is 
			if (! legal_move_b (node, zet)) {
				g.tmoves [first] = g.tmoves [-- last];
				continue;
			}
			_fast_dobmove (node, zet);
			if (attacked_by_PNBRQK (node, node -> bkpos)) {
				g.tmoves [first] = g.tmoves [-- last];
				_fast_undobmove (node, zet, flags, fifty);
				continue;
			}	       
			// bepaal sorteerwaarde van de zet voor de megasort-tabel
			if (zet != hashmove) {		
				value = g.tsort [zet & 4095];		
				if (stuk == KING) {
					if (flags & (256 | 512)) {
						value -= 1;
					}
				} else if (! _CAPTURE (zet)) { // a blocking move.
					value += piece_val [QUEEN] - piece_val [stuk];			
					if (! attacked_by_pnbrqk (node, TARGETSQ (zet))) { 
						value -= piece_values [stuk];  
					}
				}
				_fast_undobmove (node, zet, flags, fifty);				
				if (_CAPTURE (zet)) {
					if (stuk == KING || profcap_b (node, zet)) {				
						value += piece_values [CAPTURED (zet)] - piece_val [stuk];
					} else {
						value += piece_values [CAPTURED (zet)] - piece_values [stuk];
					}
				}
			}
		}		
		g.megasort [first ++] = value;
	}       
}

bool trivial_draw_w (TFastNode * node, int ply) 
{
	if (node -> wpawns == 0 && node -> bpawns == 0 &&\
	    node -> wpieces <= 1 && node -> bpieces <= 1 &&\
	    (!node->wrooks) && (!node -> wqueens) && (!node->brooks) && (!node->bqueens)) {
		return true;
	}
	if (node -> fifty > 98) {
		return true;
	}
	if (draw_by_rep (node, ply)) {
		return true;
	}	
	return false;				
}

bool trivial_draw_b (TFastNode * node, int ply) 
{
	if (node -> wpawns == 0 && node -> bpawns == 0 &&\
	    node -> wpieces <= 1 && node -> bpieces <= 1 &&\
	    (!node -> wrooks) && (!node->wqueens) && (!node -> brooks) && (!node -> bqueens)) {
		return true;
	}
	return node -> fifty > 98 || draw_by_rep (node, ply);	
}

int select_qmove (int first, int & last) 
{
	if (last <= first + 1) { //last<2
		if (last <= first) {
			return 0; //last<1
		}
		last--;
		return g.tmoves [first];
	}
	
	int move = g.tmoves [first], 
		bestmove = move,
		value = piece_values [CAPTURED (move)] - PIECE (move),
		index = first,
		best = value;
	
	first++;
	do {
		move = g.tmoves [first];
		value = piece_values [CAPTURED (move)] - PIECE (move);		
		if (value > best) {
			index = first;
			best = value;
			bestmove = move;
		}
		first++;
	} while (first < last);
	
	last--;
	g.tmoves [index]= g.tmoves [last];
	return bestmove;
}

bool profcap_w (TFastNode * node, int move) 
{
	BOOST_ASSERT (_CAPTURE (move));
	if (! SPECIAL (move)) {
		if  (piece_val [CAPTURED (move)] < piece_val [PIECE (move)]) {
			return true;
		}
		if (minisearch_w (node, SOURCESQ (move), TARGETSQ (move)) > 0) {
			return true;
		}
		return false;
	} else {
		return ! _ENPASSANT (move);
	}
	return false;
}

bool profcap_b (TFastNode * node, int move) 
{
	BOOST_ASSERT (_CAPTURE (move));
	if (! SPECIAL (move)) {
		if  (piece_val [CAPTURED (move)] < piece_val [PIECE (move)]) {
			return true;
		}
		if (minisearch_b (node, SOURCESQ (move), TARGETSQ (move)) > 0) {
			return true;
		}
		return false;
	} else {
		return ! _ENPASSANT (move);
	}
	return false;
}

/**
 * Check the "learn" and "GM" books to see if the given position is in it.
 */
bool lookup_books (TFastNode* node, int& score, int& avoid, int tmax, int move)
{
	int     bookscore = 0,
			bookavoid = 0,
			npms = g.last_known_nps/1000, /* last known nodes per millisecond */
			min_score,
			wwin,
			draw,
			bwin,
			min_nodes;

	score = 0;
	avoid = 0;

	min_nodes = tmax*npms;
	min_score = 0; // tmax/5000;

	// try the learning book;
	if (database_lookup_learn (node->hashcode, bookscore, bookavoid, min_score, min_nodes, 0 /*wildnumber*/,
			MainForm.opponent_player_id)) {

		if (bookavoid==0 && bookscore<0) { //hack
			avoid = -bookscore/8;
		} else {
			avoid = bookavoid;
		}
		if (bookscore>0) {
			score = 10000000+bookscore;
		}
		if (s_threadstate == THREAD_THINKING) {
			std::cout << boost::format("learn: min_score=%d min_nodes=%d bookscore=%d bookavoid=%d move=") %
					(tmax/5000) % min_nodes % bookscore % bookavoid << " ";
			_print_SAN(move);
			std::cout << std::endl;
		}
		return true;
	}

	// try the GM book
	if (g.dbhandle->lookup_book (node -> hashcode, wwin, draw, bwin)) {

		/* calculate score when we have at least 2 wins */
		if (ROOT_WTM(node)) {
			if (wwin>2)
			score = wwin + draw - bwin;
		} else {
			if (bwin>2) {
				score = bwin + draw - wwin;
			}
		}

		/* don't mark the move as "book" when it has zero or negative score */
		if (score<=0) {
			return false;
		}

		if (rand() > rand()) {
			score <<= 1;
			if (rand() > rand()) {
				score <<= 1;
			}
		}
		return true;
	}
	return false;
}
 
int genrootmoves_w (TFastNode * node)
{
	int last = _fast_genmovesw (node, 0),
		index = 0,	       
		move,				
		fifty = node -> fifty,
		flags = node -> flags;

	bool inbook;
	int bookscore, bookavoid;
			
 	for (int i = 0; i < last; i ++) {
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
			g.rootmoves [index]. draw = trivial_draw_w (node, 1);
			g.rootmoves [index]. fifty = node->fifty;
			g.rootmoves [index]. unknown = false;
			g.rootmoves [index]. bookmove = false; 

			if (last > 1 && g.checkbook && !g.rootmoves[index].draw) {
				inbook = lookup_books (node, bookscore, bookavoid, g.tmax, move);

				if (bookavoid==0 && inbook) {
					g.rootmoves[index].bookmove = true;
					g.rootmoves[index].bookvalue = bookscore;
				}
				if (bookavoid && inbook) {
					std::cout << "bookavoid ";
					print_move(move);
					std::cout << std::endl;
					g.rootmoves[index].avoid = bookavoid; 
				}
			} 	
			index ++;

		}
		_fast_undowmove (node, move, flags, fifty);		
	}		
	return index;
}

int genrootmoves_b (TFastNode * node)
{
	int last = _fast_genmovesb (node, 0),
		index = 0,	       
		move,		
		fifty = node -> fifty,
		flags = node -> flags;

	bool inbook;
	int bookscore, bookavoid;
	
	for (int i = 0; i < last; i ++) {
		move = g.tmoves [i];
		if (move == ENCODESCB && (attacked_by_PNBRQK (node, e8) || attacked_by_PNBRQK (node, f8))) {
			continue;
		}
		if (move == ENCODELCB && (attacked_by_PNBRQK (node, e8) || attacked_by_PNBRQK (node, d8))) {
			continue;
		}
		_fast_dobmove (node, move);
		if (! attacked_by_PNBRQK (node, node -> bkpos)) {			
			g.rootmoves [index]. move = move;
			g.rootmoves [index]. avoid = 0;
			g.rootmoves [index]. bookvalue = 0;
			g.rootmoves [index]. matevalue = 0;
			g.rootmoves [index]. value = 0;
			g.rootmoves [index]. nodes = 0;		
			g.rootmoves [index]. draw = trivial_draw_b (node, 1);
			g.rootmoves [index]. fifty = node -> fifty;
			g.rootmoves [index]. unknown = false;
			g.rootmoves [index]. bookmove = false; 
			if (last > 1 && g.checkbook && !g.rootmoves[index].draw) {

				inbook = lookup_books (node, bookscore, bookavoid, g.tmax, move);
				if (bookavoid==0 && inbook) {
						g.rootmoves[index].bookmove = true;
						g.rootmoves[index].bookvalue = bookscore;
				}
				if (bookavoid && inbook) {
					std::cout << "bookavoid ";
					print_move(move);
					std::cout << std::endl;
					g.rootmoves[index].avoid = bookavoid;
				}
			}       	     
			index ++;
		}
		_fast_undobmove (node, move, flags, fifty);				
	}		
	return index;
}

void load_xray (TFastNode *node, 
		   int  target_sq,     /* square under investigation */ 
		   int  attacker_sq,   /* square of the original attacker */ 
		   int * xray_buffer, 
		   int & xray_count)
{
	/* check if there is an xray attacker on target_sq after the piece */ 
	/* on attacker_sq captures. */       
	if (node -> matrix [attacker_sq] == KNIGHT || node -> matrix [attacker_sq] == KING || 
	    node -> matrix [attacker_sq] == BKNIGHT || node -> matrix [attacker_sq] == BKING) { 
		return; 
	}	
	int offset = t.direction [target_sq] [attacker_sq],
		sq = attacker_sq,
		piece;
	while (t.squares_to_edge [target_sq] [sq]) {
		sq += offset;
		BOOST_ASSERT (sq >= 0 && sq <= 63);
		piece = node -> matrix [sq];
		if (piece) {
			if (piece < KING) {
				if (t.pieceattacks [piece] [sq] [target_sq]) {
					xray_buffer [xray_count ++] = sq;					
				}
			} else if (piece > BKNIGHT) {
				if (t.pieceattacks [piece - KING] [target_sq] [sq]) {
					xray_buffer [xray_count ++] = sq;
				}
			}
			return;
		}				
	}
}

bool load_attacker_w (TFastNode *node, 
			 int  square,       /* square to attack */ 
			 int  qmove_ssq,    /* ssq of first attacker (qmove piece) */
			 
			 int  *xray_buffer, /* table with squares of xrays */
			 int  &xray_count,  /* number of xrays in table */ 			
			 int  &new_sq,      /* new sq of attacker to square */  
			 int  &value,       /* value of the attacker. */ 
			 int  &state,       /* new state of the attacker search */		
			 int  &state_index) /* piece_index , to continue with */ 
{
	/* check if white pieces attack square square */ 
	int i,
		tsq = square,
		bestray,
		bestrayindex=0,
		ssq,
		raypiece;
	
	char offset;
	int piece_ssq; 

	while (state < 7) { 
		
		switch (state) { 

		case 1: 
			/* look for attacking pawns */ 
			for (i = state_index; i < node->wpawns; i++) {
				ssq = node->wpawnlist [i];
				if (t.pieceattacks [PAWN] [ssq] [tsq] && ssq != qmove_ssq) {				       
					/* set stuff and return */ 
					state_index = i+1; 
					value = 1; /* pawnalue */ 
					new_sq = ssq; 
					return true;  								
				}
			}
			state++;
			state_index = 0; 
			break; 

		case 2:
			/* look for attacking knights */		 
			for (i = state_index; i < node->wknights; i++) {
				ssq = node->wknightlist[i];
				if (t.pieceattacks [KNIGHT][ssq][tsq] && ssq != qmove_ssq) {				       
					/* set stuff and return */ 
					state_index = i+1;  
					value = 3; 
					new_sq = ssq; 
					return true;						       			
				}
			}

			state++; 
			state_index = 0; 
			break; 
  
		case 3:		
			/* attacked by white bishop? */   
			for (i = state_index; i < node->wbishops; i++) {
				ssq = node->wbishoplist[i];
				if (t.pieceattacks [BISHOP][ssq][tsq] && ssq != qmove_ssq) {
					piece_ssq = ssq; 
					offset = t.direction [ssq][tsq];
					do {
						ssq += offset;
						if (ssq == tsq) {
							state_index = i+1; 
							value = 3; 
							new_sq = piece_ssq; 
							return true;							
						}
					} while (!node->matrix [ssq]);
				}
			}

			state++; 
			state_index = 0; 
			break; 
  
		case 4:
			/* check WHITE BISHOPS in the xraytable */ 			
			for (i = 0; i < xray_count; i ++) {
				raypiece = node -> matrix [xray_buffer [i]];
 				if (raypiece == BISHOP) {
					value = piece_val [BISHOP];				
					new_sq = xray_buffer [i]; 
					/* xray uit de lijst halen */ 					
					xray_buffer [i] = xray_buffer [-- xray_count]; 
					return true; 
				}
			}
			
			/* attacked by white rook? */   
			for (i = state_index; i < node -> wrooks; i++) {
				ssq = node -> wrooklist [i];
				if (t.pieceattacks [ROOK] [ssq] [tsq] && ssq != qmove_ssq ) {
					piece_ssq = ssq; 
					offset = t.direction [ssq][tsq];
					do {
						ssq += offset;
						if (ssq == tsq) {
							state_index = i+1; 
							value = piece_val [ROOK]; 
							new_sq = piece_ssq; 
							return true;							
						}
					} while (!node->matrix [ssq]);
				}
			}
			
			state++;
			state_index = 0;
			break; 
		case 5:
			/* check bishop/r in the xraytable */
			if (xray_count) {
				bestray = piece_val [QUEEN];
				for (i = 0; i < xray_count; i++) {
					raypiece = node -> matrix [xray_buffer [i]];
					if (raypiece < QUEEN) {
						value = piece_val [raypiece];
						if (value < bestray) {
							bestray = value;
							new_sq = xray_buffer [i]; 
							/* xray uit de lijst halen */
							bestrayindex = i;				
						} 
					}
				}
				/* kan hier een bishop of een rook zijn */ 
				if (bestray < piece_val [QUEEN]) {
					new_sq = xray_buffer [bestrayindex];
					xray_buffer [bestrayindex] = xray_buffer [-- xray_count];
					value = bestray;
					return true;
				}
			}
									       
			/* attacked by white queen? */  
			for (i = state_index; i < node->wqueens; i++) {
				ssq = node -> wqueenlist [i];
				if (t.pieceattacks [QUEEN] [ssq] [tsq] && ssq != qmove_ssq) {
					offset = t.direction [ssq] [tsq];
					piece_ssq = ssq;
					do {					 
						ssq += offset;
						if (ssq == tsq) {							
							state_index = i+1; 
							value = piece_val [QUEEN]; 
							new_sq = piece_ssq; 
							return true;							
						}
					} while (! node->matrix [ssq]);
				}
			}
			
			state++; 
			state_index = 0; 
			break; 

		case 6: 

			/* check xrays */ 
			/* check for bishop/r/q in the xraytable */
			if (xray_count) {
				bestray = piece_val [KING];
				for (i = 0; i < xray_count; i++) { 
					raypiece = node -> matrix [xray_buffer [i]];
					if (raypiece < KING) {
						value = piece_val [raypiece];
						if (value < bestray) {
							bestray = value;
							new_sq = xray_buffer[i]; 
							/* xray uit de lijst halen */
							bestrayindex = i;				
						} 
					}
				}
				if (bestray < piece_val [KING]) {
					new_sq = xray_buffer [bestrayindex];
					xray_buffer [bestrayindex] = xray_buffer [-- xray_count];
					value = bestray;
					return true;
				}
			}
		
			/* attacked by white king? */
			if (t.pieceattacks [KING] [tsq] [node->wkpos]) {
				if (node -> wkpos != qmove_ssq) { 
					state++; 
					value = piece_val [KING]; 
					new_sq = node -> wkpos; 
					return true; 
				}
			}
			
			state++; 
			state_index = 0; 
			break; 
		default: 
			std::cerr << boost::format("w0_load_attacker_w: state = %d\n") % state;
			break; 
		}

	}

	return false; 
}


int load_attacker_b (TFastNode *node, 
			int  square,       /* square to attack */ 			 
			int  qmove_ssq,    /* ssq of qmove (first attacker) */ 

			int  *xray_buffer, /* table with squares of xrays */
			int  &xray_count,  /* number of xrays in table */ 			
			int  &new_sq,      /* new sq of attacker to square */  
			int  &value,       /* value of the attacker. */ 
			int  &state,       /* new state of the attacker search */		
			int  &state_index) /* piece_index , to continue with */ 
{
	int i,
		tsq = square,
		bestray,
		bestrayindex=0,
		ssq = -1,
		raypiece;
	char offset;
	int piece_ssq; 

	while (state < 7) { 

		switch (state) { 

	
		case 1: 
			/* attacked by black pawn? */  
			for (i = state_index; i < node -> bpawns; i++) {
				ssq = node -> bpawnlist [i];
				if (t.pieceattacks [PAWN] [tsq] [ssq] && ssq != qmove_ssq) { //note the swap of tsq and ssq 
					state_index = i+1; 
					value = 1; /* pawnalue */ 
					new_sq = ssq; 
					return true;  							
				}
			}				
			state++; 
			state_index = 0; 
			break; 

		case 2: 
						
			/* attacked by black knight? */ 	
			for (i = state_index; i < node -> bknights; i++) {
				ssq = node -> bknightlist [i];
				if (t.pieceattacks [KNIGHT] [ssq] [tsq] && ssq != qmove_ssq) {		
					state_index = i+1; 
					value = 3; 
					new_sq = ssq;
					return true; 					
				}
			}
			
			state++; 
			state_index = 0; 
			break; 

		case 3: 
			
			/* attacked by black bishop? */ 
			for (i = state_index; i < node -> bbishops; i++) {
				ssq = node->bbishoplist[i];
				if (t.pieceattacks [BISHOP][ssq][tsq] && ssq != qmove_ssq) {
					offset = t.direction [ssq][tsq];
					piece_ssq = ssq; 
					do {
						ssq += offset;
						if (ssq == tsq) {
							state_index = i+1; 
							value = 3; 
							new_sq = piece_ssq; 
							return true;		
						}
					} while (!node->matrix [ssq]);
				}
			}
			
			state++;
			state_index = 0; 
			break; 
			
		case 4: 
			/* check for BISHOP in the xraytable */ 
			if (xray_count) {
				for (i = 0; i < xray_count; i++) {
					raypiece = node -> matrix [xray_buffer [i]];
					if (raypiece == BBISHOP) {
						value = piece_val [BISHOP];	
						new_sq = xray_buffer[i]; 
						/* xray uit de lijst halen */ 					
						xray_buffer [i] = xray_buffer [-- xray_count];
						return true;
					} 
				}
			}	
			
			
			
			/* attacked by black rook? */ 
			for (i = state_index; i < node->brooks; i++) {
				ssq = node->brooklist[i];
				if (t.pieceattacks [ROOK][ssq][tsq] && ssq != qmove_ssq) {
					piece_ssq = ssq; 
					offset = t.direction [ssq][tsq];
					do {
						ssq += offset;
						if (ssq == tsq) {
	      						value = 5;
							state_index = i+1;
							new_sq = piece_ssq; 
							return true;
						}
					} while (!node->matrix [ssq]);
				}
			}
			
			state++; 
			state_index = 0; 
			break; 

		case 5: 
			if (xray_count) {
				bestray = piece_val [QUEEN];
				for (i = 0; i < xray_count; i++) {
					raypiece = node -> matrix [xray_buffer [i]];
					if (raypiece > BKNIGHT && raypiece < BQUEEN) {
						value = piece_val [raypiece];
						if (value < bestray) {
							bestray = value;
							new_sq = xray_buffer[i]; 
							/* xray uit de lijst halen */
							bestrayindex = i;				
						} 
					}
				}
				if (bestray < piece_val [QUEEN]) {
					new_sq = xray_buffer [bestrayindex];
					xray_buffer [bestrayindex] = xray_buffer [-- xray_count];
					value = bestray;
					return true;
				}
			}
			
			/* attacked by black queen? */ 
			for (i = state_index; i < node->bqueens; i++) {
				ssq = node->bqueenlist[i];
				if (t.pieceattacks [QUEEN][ssq][tsq] && ssq != qmove_ssq) {
					piece_ssq = ssq; 
					offset = t.direction [ssq][tsq];
					do {
						ssq += offset;
						if (ssq == tsq) {
							value = 9; 
							state_index = i+1; 
							new_sq = piece_ssq; 
							return true;
						}
					} while (!node->matrix [ssq]);
				}
			}
			
			state++; 
			state_index = 0; 
			break; 

		case 6: 

			/* check for pawns/kni/b/r in the xraytable */ 
			if (xray_count) {
				bestray = piece_val [KING];
				for (i = 0; i < xray_count; i++) {
					raypiece = node -> matrix [xray_buffer [i]];
					if (raypiece > BKNIGHT) {
						value = piece_val [raypiece];
						if (value < bestray) {
							bestray = value;
							new_sq = xray_buffer[i]; 
							/* xray uit de lijst halen */
							bestrayindex = i;				
						} 
					}
				}
				if (bestray < piece_val [KING]) {
					new_sq = xray_buffer [bestrayindex];
					xray_buffer [bestrayindex] = xray_buffer [-- xray_count];
					value = bestray;
					return true;
				}
			}
			
			/* attacked by black king? */ 
			if (t.pieceattacks [KING][tsq] [node -> bkpos]) {
				if (node -> bkpos != qmove_ssq) { 
					state++; 					
					new_sq = node -> bkpos; 
					value = 30; 
					return true;
				}
			}
			
			state++; 
			state_index = 0;  
			break; 
		default:
			std::cerr << boost::format("w0_load_attacker_b: state = %d\n") % state;
			break; 	
		}
	}	
	
	return false;
}

int minisearch_w (TFastNode *node, int ssq, int tsq)
{ 
	/* het init-gedeelte zorgt voor de 1e cap - dus van w */ 
	
	int xray_buffer [20];
	int xray_index = 0;
		       
	int piece_on_sq = piece_val [node->matrix [ssq]];  /* piece wat nu op tsq staat */  
	int saldo = piece_val [node->matrix [tsq]];                /* saldo van de caps */ 
	
	int prev_ssq = ssq;                   /* ssq of last capturing attacker/defender */  

	int qmove_ssq = ssq;               /* ssq van de originele (te onderzoeken) move */ 

	int state_w = 1; 
	int state_index_w = 0; 
	int state_b = 1; 
	int state_index_b = 0; 

	int value; 
	int bestray; 
	int bestrayindex=0; 
	int raypiece;

	do { 
		/* black doesn't like to recapture when it has a positive combo-result */ 
		/* at this point. */ 
		if (saldo < 0) { 
			return saldo; 
		}
		
		/* check if the precious capture enabled xray attackers/defenders */ 		 
		load_xray (node, tsq, prev_ssq, xray_buffer, xray_index); 		
		
		/* recapture with black */ 
		if (!load_attacker_b (node, tsq, qmove_ssq,
					xray_buffer, xray_index, 
					prev_ssq,value, state_b,
					state_index_b )) { 

			/* zwart heeft geen attacker meer. er kunnen nog xrays zijn. */ 
			/* check of er nog xrays zijn */ 
			if (xray_index > 0) { 			       
				bestray = piece_val [BKING];
				for (int i = 0; i < xray_index; i++) {
					raypiece = node -> matrix [xray_buffer [i]];
					if (raypiece > BKNIGHT) {
						value = piece_val [raypiece];
						if (value < bestray) {
							bestray = value;						
							/* xray uit de lijst halen */
							bestrayindex = i;				
						} 
					}
				}
				if (bestray < piece_val [BKING]) {
					prev_ssq = xray_buffer [bestrayindex];
					xray_buffer [bestrayindex] = xray_buffer  [-- xray_index];
					value = bestray;				       
				}
			} else { 			
				return saldo; 
			}
		} else { 
//			g_print("load_attacker_b sq = %d value = %d\n", prev_ssq_b, value); 
			
			saldo = saldo - piece_on_sq;
			piece_on_sq = value; 
		}
				
		/* white doesn't want to recapture if it has a positive combo result */ 				if (saldo > 0) { 
//			g_print("stopping: white doesnt like to cap, saldo>0\n"); 
			return saldo; 
		}
		
		/* check if the previous capture by white enabled xray attackers */ 		
		load_xray (node, tsq, prev_ssq, xray_buffer, xray_index); 	
		
		/* ok capture with white */ 
//		g_print("state_w = %d state_index_w = %d \n",state_w, state_index_w); 
		if (!load_attacker_w (node, tsq, qmove_ssq, 
					 xray_buffer, xray_index, prev_ssq,
					  value, state_w, state_index_w )) { 
			/* check of er nog xrays zijn */ 
			if (xray_index > 0) { 				
				
				/* xray_select */ 	
				int bestray = piece_val [KING];
				for (int i = 0; i < xray_index; i++) { 			
					raypiece = node -> matrix [xray_buffer [i]];
					if (raypiece < KING) {
						value = piece_val [raypiece];
						if (value < bestray) {
							bestray = value;
							bestrayindex = i;				
						} 
					}
				}
				if (bestray < piece_val [KING]) {
					prev_ssq = xray_buffer [bestrayindex];
					xray_buffer [bestrayindex] = xray_buffer [-- xray_index];
					value = bestray;				       
				}
			} else { 		
				return saldo; 
			}
		} else { 
//			g_print("load_attacker_w sq = %d value = %d\n", prev_ssq_w, value);
			saldo = saldo + piece_on_sq; 
			piece_on_sq = value;  
		}
	} while (true); 
}

int minisearch_b( TFastNode *node, int ssq, int tsq)
{ 
	/* het init-gedeelte zorgt voor de 1e cap - dus van w */ 

	int saldo = piece_val [node->matrix[tsq]];        /* saldo van de caps */ 	

	int xray_buffer [20];
	int xray_index = 0;
				
	int piece_on_sq = piece_val [node->matrix[ssq]];  /* piece wat nu op tsq staat */  	
	
	int prev_ssq = ssq;  /* last attacker */  
	int qmove_ssq = ssq; 

	int state_w = 1; 
	int state_index_w = 0; 
	int state_b = 1; 
	int state_index_b = 0; 

	int value; 
	int bestray; 
	int bestrayindex=0; 
	int raypiece;

	do { 

		/* white doesn't like to recapture when it has a positive combo-result */ 
		/* at this point. */ 
		if (saldo < 0) { 
			return saldo; 
		}
		
		/* check if the precious capture by black enabled xray attackers */ 		 
		load_xray (node, tsq, prev_ssq, xray_buffer, xray_index); 		
		
		/* recapture with black */ 
		if (!load_attacker_w(node, tsq, qmove_ssq, xray_buffer, 
					xray_index, prev_ssq, value, 
					state_w, state_index_w )) { 

			/* check of er nog xrays zijn */ 
			if (xray_index > 0) {  				
				bestray = piece_val [KING];
				for (int i = 0; i < xray_index; i++) { 			
					raypiece = node -> matrix [xray_buffer [i]];
					if (raypiece < KING) {
						value = piece_val [raypiece];
						if (value < bestray) {
							bestray = value;						
							/* xray uit de lijst halen */
							bestrayindex = i;				
						} 
					}
				}
				if (bestray < piece_val [KING]) {
					prev_ssq = xray_buffer [bestrayindex];
					xray_buffer [bestrayindex] = xray_buffer  [-- xray_index];
					value = bestray;				       
				}
			} else { 			
				return saldo; 
			}
		} else { 			
			saldo = saldo - piece_on_sq;
			piece_on_sq = value; 
		}
				
		/* black doesn't want to recapture if it has a positive combo result */ 
		if (saldo > 0) { 
			return saldo; 
		}
		
		/* check if the previous capture by white enabled xray attackers */ 	       
		load_xray (node, tsq, prev_ssq, xray_buffer, xray_index); 	       
		
		/* ok capture with black */ 
		if (!load_attacker_b(node, tsq, qmove_ssq, 
					xray_buffer, xray_index,
					prev_ssq, value, state_b,
					state_index_b )) { 
			/* check of er nog xrays zijn */ 
			if (xray_index > 0) { 				 
				
				/* xray_select */ 		
				int bestray = piece_val [BKING];
				for (int i = 0; i < xray_index; i++) {
					raypiece = node -> matrix [xray_buffer [i]];
					if (raypiece > BKNIGHT) {
						value = piece_val [raypiece];
						if (value < bestray) {
							bestray = value;
							bestrayindex = i;				
						} 
					}
				}
				if (bestray < piece_val [BKING]) {
					prev_ssq = xray_buffer [bestrayindex];
					xray_buffer [bestrayindex] = xray_buffer [-- xray_index];
					value = bestray;				       
				}
			} else { 		
				return saldo; 
			}
		} else { 
			saldo = saldo + piece_on_sq; 
			piece_on_sq = value;  
		}
	} while (true); 
}
 
int genq_w (TFastNode* node, int index) {
	int     i,
		ssq, 
		tsq,
		captured;
			
	// generate captures and promotions (queen and knight) with white pawns

	BOOST_ASSERT (index >= 0);
  
	for (i = 0; i < node->wpawns; i++) {
		ssq = node->wpawnlist [i];		
		if (ssq >= a7) {
			tsq = ssq + 8;
			if (! node -> matrix [tsq]) {
				g.tmoves [index ++] = ENCODESPECIAL (ssq, tsq, PAWN, 0, _QUEEN_PROMOTION);
				g.tmoves [index ++] = ENCODESPECIAL (ssq, tsq, PAWN, 0, _KNIGHT_PROMOTION);
			}
		}
		tsq = t.nextdir [PAWN] [ssq] [ssq];
		do {
			captured = node -> matrix [tsq];
			if (captured > KING) {
				if (tsq <= h7) {					
					g.tmoves [index++] = ENCODECAPTURE (ssq, tsq, PAWN, captured - KING);
				} else { //promotion  
					g.tmoves [index++] = ENCODESPECIAL (ssq, tsq, PAWN, captured - KING, _QUEEN_PROMOTION);
					g.tmoves [index++] = ENCODESPECIAL (ssq, tsq, PAWN, captured - KING, _KNIGHT_PROMOTION);
				}
			} else if (_EPSQ (node) && tsq == _EPSQ (node)) { //en passant				
				g.tmoves [index++] = ENCODEEP (ssq, tsq);
			}
			tsq = t.nextdir [PAWN][ssq][tsq];
		} while (ssq != tsq);
		
	}
  
	// generate captures with white knights

	for (i = 0; i < node->wknights; i++) {
		ssq = node->wknightlist[i];
		tsq = t.nextdir [KNIGHT][ssq][ssq];
		do {								
			captured = node -> matrix [tsq];
			if (captured > KING) {						
				g.tmoves [index ++] = ENCODECAPTURE (ssq, tsq, KNIGHT, captured - KING);
			}			
			tsq = t.nextdir [KNIGHT][ssq][tsq];
		} while (ssq != tsq);
	}
  
	// generate captures with white bishops
	
	for (i = 0; i < node->wbishops; i++) {
		ssq = node->wbishoplist[i];
		tsq = t.nextpos [BISHOP][ssq][ssq];
		do {
			captured = node->matrix [tsq];
			if (! captured) {				
				tsq = t.nextpos [BISHOP][ssq][tsq];	 
			} else {
				if (captured <= KING) {
					tsq = t.nextdir [BISHOP] [ssq] [tsq];
					continue;
				}				
				g.tmoves [index ++] = ENCODECAPTURE (ssq, tsq, BISHOP, captured - KING);
				tsq = t.nextdir [BISHOP] [ssq] [tsq];
			}	
		} while (ssq != tsq);
	}

	// generate captures with white rooks

	for (i = 0; i < node->wrooks; i++) {
		ssq = node->wrooklist[i];
		tsq = t.nextpos [ROOK][ssq][ssq];
		do {
			captured = node->matrix [tsq];
			if (! captured) {
				tsq = t.nextpos [ROOK] [ssq] [tsq];	 
			} else {
				if (captured <= KING) {
					tsq = t.nextdir [ROOK] [ssq] [tsq];
					continue;
				}
				g.tmoves [index ++] = ENCODECAPTURE (ssq, tsq, ROOK, captured - KING);
				tsq = t.nextdir [ROOK] [ssq] [tsq];			
			}	
		} while (ssq != tsq);
	}

	// generate captures with white queens

	for (i = 0; i < node->wqueens; i++) {
		ssq = node->wqueenlist[i];
		tsq = t.nextpos [QUEEN] [ssq] [ssq];
		do {
			captured = node->matrix [tsq];
			if (! captured) {
				tsq = t.nextpos [QUEEN][ssq][tsq];	 
			} else {
				if (captured > KING) {
					g.tmoves [index++] = ENCODECAPTURE (ssq, tsq, QUEEN, captured - KING);					
				}
				tsq = t.nextdir [QUEEN][ssq][tsq];
			}	
		} while (ssq != tsq);
	} 
	       
	// generate captures with the white king
 
	ssq = node->wkpos;
	tsq = t.nextdir [KING][ssq][ssq];
	do {
		captured = node->matrix [tsq];
		if (captured > KING) {
			g.tmoves[index++] = ENCODECAPTURE (ssq, tsq, KING, captured - KING);
		}
		tsq = t.nextdir [KING][ssq][tsq];
	} while (ssq != tsq);  

	BOOST_ASSERT (index <= ((MAXPLY + 2) << 7) + 128);

	return index;
}

int genq_b (TFastNode* node, int index) 
{
	int i,
		ssq, 
		tsq,
		captured;
  
  // generate captures and promotions (queen and knight) with black pawns

  BOOST_ASSERT (index >= 0);

  for (i = 0; i < node->bpawns; i++) {
	  ssq = node->bpawnlist [i];
	  
	  // promotions
	  
	  if (ssq <= h2) {
		  tsq = ssq - 8;
		  if (! node -> matrix [tsq]) {
			  g.tmoves [index ++] = ENCODESPECIAL (ssq, tsq, PAWN, 0, _QUEEN_PROMOTION);
			  g.tmoves [index ++] = ENCODESPECIAL (ssq, tsq, PAWN, 0, _KNIGHT_PROMOTION);
		  }
	  } 

	  // captures

	  tsq = t.nextdir [BPAWN] [ssq] [ssq];
	  do {
		  captured = node -> matrix [tsq];
		  if (captured && captured <= KING) {
			  if (tsq >= a2) {				  
				  g.tmoves [index++] = ENCODECAPTURE (ssq, tsq, PAWN, captured);
			  } else { //promotion  
				  g.tmoves [index++] = ENCODESPECIAL (ssq, tsq, PAWN, captured, _QUEEN_PROMOTION);	 
				  g.tmoves [index++] = ENCODESPECIAL (ssq, tsq, PAWN, captured, _KNIGHT_PROMOTION);
			  }
		  } else if (_EPSQ (node) && tsq == _EPSQ (node)) { //en passant
			  g.tmoves [index++] = ENCODEEP (ssq, tsq);
		  }
		  tsq = t.nextdir [BPAWN][ssq][tsq];
	  } while (ssq != tsq);
	 
  }
    
  // generate captures with black knights
  
  for (i = 0; i < node -> bknights; i++) {
	  ssq = node -> bknightlist [i];
	  tsq = t.nextdir [KNIGHT] [ssq] [ssq];
	  do {
		  captured = node -> matrix [tsq];
		  if (captured && captured <= KING) {			  
			  g.tmoves [index ++] = ENCODECAPTURE (ssq, tsq, KNIGHT, captured);
		  }		 
		  tsq = t.nextdir [KNIGHT] [ssq] [tsq];
	  } while (ssq != tsq);
  }
  
  // generate captures with black bishops
  
  for (i = 0; i < node->bbishops; i++) {
	  ssq = node->bbishoplist [i];
	  tsq = t.nextpos [BISHOP] [ssq] [ssq];
	  do {
		  captured = node -> matrix [tsq];
		  if (! captured) {
			  tsq = t.nextpos [BISHOP] [ssq] [tsq];	 
		  } else {
			  if (captured > KING) {
				  tsq = t.nextdir [BISHOP] [ssq] [tsq];
				  continue;
			  }			 
			  g.tmoves [index ++] = ENCODECAPTURE (ssq, tsq, BISHOP, captured);
			  tsq = t.nextdir [BISHOP] [ssq] [tsq];
		  }	
	  } while (ssq != tsq);
  }
  
  // generate captures with black rooks
  
  for (i = 0; i < node->brooks; i++) {
	  ssq = node->brooklist[i];
	  tsq = t.nextpos [ROOK][ssq][ssq];
	  do {
		  captured = node->matrix [tsq];
		   if (! captured) {
			   tsq = t.nextpos [ROOK] [ssq] [tsq];	 
		  } else {
			  if (captured > KING) {
				  tsq = t.nextdir [ROOK] [ssq] [tsq];
				  continue;
			  }
			  g.tmoves [index ++] = ENCODECAPTURE (ssq, tsq, ROOK, captured);
			  
			  tsq = t.nextdir [ROOK] [ssq] [tsq];
		  }		 
	  } while (ssq != tsq);
  }
  
  // generate captures with black queens
  
  for (i = 0; i < node->bqueens; i++) {
	  ssq = node->bqueenlist[i];
	  tsq = t.nextpos [QUEEN][ssq][ssq];
	  do {
		  captured = node->matrix [tsq];
		  if (!captured) {
			  tsq = t.nextpos [QUEEN] [ssq] [tsq];	 
		  } else {
			  if (captured <= KING) {
				  g.tmoves [index ++] = ENCODECAPTURE (ssq, tsq, QUEEN, captured);
			  }
			  tsq = t.nextdir [QUEEN] [ssq] [tsq];
		  }	
	  } while (ssq != tsq);
  } 
   
  // generate captures with the black king
  
  ssq = node->bkpos;
  tsq = t.nextdir [KING][ssq][ssq];
  do {
	  captured = node->matrix [tsq];
	  if (captured && captured <= KING) {
		  g.tmoves[index++] = ENCODECAPTURE (ssq, tsq, KING, captured);
	  }
	  tsq = t.nextdir [KING][ssq][tsq];
  } while (ssq != tsq);  
  
  BOOST_ASSERT (index <= (((MAXPLY + 2) << 7)) + 128);
  
  return index;
}
