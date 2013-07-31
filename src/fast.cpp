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

#include "fast.h"
#include "hashcodes.h" 
#include "legality.h" 
#include "parser.h" 
#include "attack.h"
#include "w0.h"

using namespace std;
using namespace boost;

Vars g; 
Tables t; 

int _maxd = 500;
int window1= 750;   // 1st aspiration search window 
int window2 = 3000; // 2nd aspiration search window
const int piece_val [] = { 0, 1, 3, 3, 5, 9, 30, 1, 3, 3, 5, 9, 30 };			  
const int piece_values [] = { 0, PAWN_VALUE, KNIGHT_VALUE, BISHOP_VALUE, ROOK_VALUE, QUEEN_VALUE, 0, 
			  PAWN_VALUE, KNIGHT_VALUE, BISHOP_VALUE, ROOK_VALUE, QUEEN_VALUE, 0 };
const int avoidscore[4] = { 0, -2500, -5000, -MATE/2 };
const bool _edge [] = {
		1, 1, 1, 1, 1, 1, 1, 1,
		1, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 1,
		1, 1, 1, 1, 1, 1, 1, 1       		
};

bool select_bookmove(TFastNode *node, int mc)
{ 
	int value, best = - CHESS_INF, bestindex = 0;
	for (int i=0; i < mc; i++) { 
		if (g.rootmoves[i].bookmove) { 
			value = g.rootmoves[i].bookvalue;
			if (value > best) {
				bestindex = i;
				best = value;
			}
		}
	}
	if (best > 0) {		
		g.pv[1][1] = 0; 
		root_new_best(node, bestindex, best); 
		return true; 	
	}	
	return false; 
}

void store_cutoff (int move, int ply, int depth)  
{
 	g.tsort [move & 4095] += MAX (1, depth/10);
}

bool open_file (TFastNode * node, int sq)
{
	sq = column (sq) + 8;
	do {
		if (node -> matrix [sq] == PAWN || node -> matrix [sq] == BPAWN) {
			return false;
		}
		sq += 8;
	} while (sq < h7);
	return true;
}

bool white_octopus (TFastNode * node, int sq) 
{
	if (sq == e6) {
		if (attacked_by_p (node, e6)) {
			return false;
		}
		if (attacked_by_P (node, e6)) {
			return true;
		}
	}
	if (sq == d6) {
		if (attacked_by_p (node, d6)) {
			return false;
		}
		if (attacked_by_P (node, d6)) {
			return true;
		}
	}
	return false;
}

bool white_small_octopus (TFastNode * node, int sq)
{
        if (sq == c6) {
                if (attacked_by_p (node, c6)) {
                        return false;
                }
                if (attacked_by_P (node, c6)) {
                        return true;
                }
        }
        if (sq == f6) {
                if (attacked_by_p (node, c6)) {
                        return false;
                }
                if (attacked_by_P (node, f6)) {
                        return true;
                }
        }
        return false;
}


bool black_octopus (TFastNode * node, int sq) 
{
	if (sq == e3) {
		if (attacked_by_P (node, e3)) {
			return false;
		}
		if (attacked_by_p (node, e3)) {
			return true;
		}
	}
	if (sq == d3) {
		if (attacked_by_P (node, d3)) {
			return false;
		}
		if (attacked_by_p (node, d3)) {
			return true;
		}
	}
	return false;
}

bool black_small_octopus (TFastNode * node, int sq)
{
        if (sq == c3) {
                if (attacked_by_P (node, c3)) {
                        return false;
                }
                if (attacked_by_p (node, c3)) {
                        return true;
                }
        }
        if (sq == f3) {
                if (attacked_by_P (node, f3)) {
                        return false;
                }
                if (attacked_by_p (node, f3)) {
                        return true;
                }
        }
        return false;
}

bool passed_wpawn (TFastNode * node, int sq) 
{
	int sq2 = a7 + column (sq);
	while (sq < sq2) {		 				
		if (node -> matrix [sq2] == BPAWN || node -> matrix [sq2] == PAWN) {
			return false;
		}
		sq2 -= 8;
		if (attacked_by_p (node, sq2)) {
			return false;
		}		
	}
	return true;
}

bool passed_bpawn (TFastNode * node, int sq) 
{		
	int sq2 = a2 + column (sq);
	while (sq > sq2) {		 				
		if (node -> matrix [sq2] == PAWN || node -> matrix [sq2] == BPAWN) {
			return false;
		}
		sq2 += 8;
		if (attacked_by_P (node, sq2)) {
			return false;
		}		
	}
	return true;
}

int get_move_from_db (TFastNode * node) 
{
	int best = - CHESS_INF,
		value,
		move,
		bestmove = 0, 
		last,
		score,
		avoid,
		flags = node -> flags, 
		fifty = node -> fifty;
	if (node -> flags & _WTM) {
		last = _fast_genmovesw (node, 0);
		for (int i = 0; i < last; i ++) {
			move = g.tmoves [i];
			if (legal_move_w (node, move)) {
				_fast_dowmove (node, move);
				if (lookup_books (node, score, avoid, 1, move)) {
					if (avoid==0) {
						value = score;
						if (value > best) {
							best = value;
							bestmove = move;
						}
					}
				}
				_fast_undowmove (node, move, flags, fifty);
			}
		}
	} else { 
		last = _fast_genmovesb (node, 0);
		for (int i = 0; i < last; i ++) {			
			move = g.tmoves [i];
			if (legal_move_b (node, move)) {
				_fast_dobmove (node, move);
				if (lookup_books (node, score, avoid, 1, move)) {
					if (avoid==0) {
						value = score; 
						if (value > best) {
							best = value;
							bestmove = move;
						}
					}
				}
				_fast_undobmove (node, move, flags, fifty);
			}
		}
	}
	return bestmove;
}

void _fast_new_game()
{
	g.winning = false;
	g.checkbook = true; 
	memset (g.tsort, 0, sizeof (g.tsort));
	g.repindex = 0;	
}

bool quiet_w (TFastNode * node)
{	
	int i; 
	for (i = 0; i < node -> wknights; i ++) {
		if (attacked_by_p (node, node -> wknightlist [i])) {
			return false;
		}
	}	
	for (i = 0; i < node -> wbishops; i ++) {
		if (attacked_by_p (node, node -> wbishoplist [i])) {
			return false;
		}
	}	
	for (i = 0; i < node -> wrooks; i ++) {
		if (attacked_by_pnb (node, node -> wrooklist [i])) {
			return false;
		}
	}	
	for (i = 0; i < node -> wqueens; i ++) {
		if (attacked_by_pnbr (node, node -> wqueenlist [i])) {
			return false;
		}
	}	
	return true;
}

bool quiet_b (TFastNode * node)
{	
	int i; 
	for (i = 0; i < node -> bknights; i ++) {
		if (attacked_by_P (node, node -> bknightlist [i])) {
			return false;
		}
	}
	for (i = 0; i < node -> bbishops; i ++) {
		if (attacked_by_P (node, node -> bbishoplist [i])) {
			return false;
		}
	}
	for (i = 0; i < node -> brooks; i ++) {
		if (attacked_by_PNB (node, node -> brooklist [i])) {
			return false;
		}
	}
	for (i = 0; i < node -> bqueens; i ++) {
		if (attacked_by_PNBR (node, node -> bqueenlist [i])) {
			return false;
		}	
	}
	return true;
}

void print_cutoffs () 
{
	int i; 
	int tot_cut_w = g.cutoffs_w [1],
		tot_cut_b = g.cutoffs_b [1];
	
	BOOST_ASSERT (g.cutoffs_w [0] == 0);
	BOOST_ASSERT (g.cutoffs_b [0] == 0);
	
	long double per; 
	
	for (i = 2; i < 100; i ++) {		
		tot_cut_w += g.cutoffs_w [i];
		tot_cut_b += g.cutoffs_b [i];
	}
	per = (100.0 * g.cutoffs_w [1]) / (tot_cut_w + 1.0);
	cout << format("cutoffs_w (%d): %d (%d%%)") % tot_cut_w % g.cutoffs_w [1] % (int)per;
	for (i = 2; i <= 7; i ++) {
		per = (100.0 * g.cutoffs_w [i]) / (tot_cut_w + 1.0);
		cout << format(", %d (%d%%)") % g.cutoffs_w [i] % (int) per;
	}
	cout << "\n";
	per = (100.0 * g.cutoffs_b [1]) / (tot_cut_b + 1.0);
	cout << format("cutoffs_b (%d): %d (%d%%)") % tot_cut_b % g.cutoffs_b [1] % (int) per;
	for (i = 2; i <= 7; i ++) {
		per = (100.0 * g.cutoffs_b [i]) / (tot_cut_b + 1.0);
		cout << format(", %d (%d%%)") % g.cutoffs_b [i] % (int) per;
	}
	cout << "\n";
}

int count_legals_w (TFastNode * node, int first, int last) 
{
	int index = first,
		move,
		legals = 0,
		flags = node -> flags,
		fifty = node -> fifty;
	while (index < last) {
		move = g.tmoves [index ++];
		_fast_dowmove (node, move);
		if (! attacked_by_pnbrqk (node, node->wkpos)) {
			if ((move == ENCODESCW) && (attacked_by_pnbrqk (node, e1) || attacked_by_pnbrqk (node, f1))) {
				legals --;
			} 
			if ((move == ENCODELCW) && (attacked_by_pnbrqk (node, e1) || attacked_by_pnbrqk (node, d1))) {
				legals --;
			}
			legals ++;
		}						
		_fast_undowmove (node, move, flags, fifty);
	}
	return legals;
}

int count_legals_b (TFastNode * node, int first, int last) 
{
	int index = first,
		move,
		legals = 0,
		flags = node -> flags,
		fifty = node -> fifty;
	while (index < last) {
		move = g.tmoves [index ++];
		_fast_dobmove (node, move);
		if (! attacked_by_PNBRQK (node, node->bkpos)) {
			if ((move == ENCODESCB) && (attacked_by_PNBRQK (node, e8) || attacked_by_PNBRQK (node, f8))) {
				legals --;
			} 
			if ((move == ENCODELCB) && (attacked_by_PNBRQK (node, e8) || attacked_by_PNBRQK (node, d8))) {
				legals --;
			}
			legals ++;
		}						
		_fast_undobmove (node, move, flags, fifty);
	}
	return legals;
}

#ifdef DEBUG_INSPECT
void update_pv (TFastNode *node,int move, int ply) 
{		
	g.pv [ply] [ply] = move;
	int i = ply+1; 	
	while (g.pv [ply+1][i]) {
		g.pv [ply][i] = g.pv [ply+1][i++];
				
		BOOST_ASSERT( i <= MAXPLY );

	}
	g.pv[ply][i]=0;

#ifdef PRINT_SEARCH
	std::cout << "update_pv: move="; 
	print_move(move); 
	std::cout << " ply=" << ply << std::endl; 
	std::cout << "update_pv: pv=";
	int j = ply; 
	while (g.pv[ply][j]) {
		print_move(g.pv[ply][j]);
		std::cout << " "; 		 
		j++; 
	}
	std::cout << std::endl;

#endif 

	/* ok let's checkit */ 
	TFastNode tempnode; 
	memcpy(&tempnode, node, sizeof(TFastNode));	

	i=ply;
	while (g.pv[ply][i]) {
		int ecode = _fast_inspectnode(&tempnode); 
		if (ecode) { 
			
			std::cout << "update_pv: inspect error " << ecode << " ply=" << ply << " index=" << i << std::endl;
			std::ostringstream str;
			_fast_ntofen(str,node);
			std::cout << "fen=" << str << std::endl; 
			std::cout << "error_pv = "; 				
			for (int j = ply; j <= i; j++) { 
				print_move(g.pv[ply][j]);
				std::cout << " "; 
			}
			std::cout << std::endl; 
			std::cerr << "last move is illegal\n"; 
			abort(); 

		}
		if (tempnode.flags & _WTM) { 
			if (!legal_move_w (&tempnode, g.pv[ply][i])) {
				std::cout << "update_pv: inspect error " << ecode << " index=" << i << "\n"; 
				std::cout << "error_pv = ";
				for (int j = ply; j <= i; j++) { 
					print_move(g.pv[ply][j]);
					std::cout << " "; 
				}
				std::cout << "\n"; 
				std::cerr << "last move is illegal !legalmovev || _fastmoveokw\n";
				abort(); 
				
			}
			_fast_dowmove(&tempnode, g.pv[ply][i]);
		} else { 
			if (!legal_move_b (&tempnode, g.pv[ply][i])) {
				std::cout << "update_pv: inspect error " << ecode << " index=" << i << "\n"; 
				std::cout << "error_pv = ";
				for (int j = ply; j <= i; j++) { 
					print_move(g.pv[ply][j]);
					std::cout << " "; 
				}
				std::cout << "\n"; 
				std::cerr << "last move is illegal !legalmoveb || _fastmoveokb\n";
				abort(); 				
			}
			_fast_dobmove(&tempnode, g.pv[ply][i]);
		}
		i++; 
	}


	/* original pv recording macro

           #define UPDATE_PV(move,ply,length) {\
                           pv[ply][ply] = move;\
                           for (int i = ply+1; i<length; i++)\
                           pv[ply][i] = pv[ply+1][i];\
                           pv_length[ply] = length;\
                    }
	*/ 
}
#endif

#ifndef DEBUG_INSPECT
void update_pv (TFastNode *node, int move, int ply)
{
 	g.pv [ply][ply] = move;
        int i = ply+1;
        while (g.pv [ply+1][i]) {
            int j = i;
            g.pv [ply][i] = g.pv [ply+1][j];
            i++; 
        }
        g.pv[ply][i]=0;
}
#endif														    

bool claimdraw (TFastNode * node, int ply)
{
	// draw by fifty move rule?
	if (node->fifty >= 100) {
		return true;
	}
	// draw by repetition?
	if (node->fifty>=2) {
		int reps = 0;
		int fifty = node->fifty-2;
		int index = g.repindex+ply-2;
		while (fifty >= 0) 
		{
			if (node->hashcode == g.reptable [index]) {				
				if (++reps>1) {
					return true;
				}
			}
			index -= 2;
			fifty -= 2;			
		}				
	}
	return false;	
}

bool draw_by_rep (TFastNode * node, int ply) 
{
	// check for draw by repetition
	if (node->fifty >= 2) {	
		int fifty = node->fifty-2;
		int index = g.repindex+ply-2;
		while (fifty >= 0) 
		{
			if (node -> hashcode == g.reptable [index]) {				
				return true;
			}
			index -= 2;
			fifty -= 2;			
		}				
	}
	return false;
}

int _fast_distance (int sq1, int sq2)
{
	int dx, 
		dy;
	dx = abs (row (sq1) - row (sq2));
	dy = abs (column (sq1) - column (sq2));
	return dx + dy;	
}

bool Q_mates_k (TFastNode * node)
{			
	int matesq, queensq, escapesq, piece;	
	if (node -> wqueens == 1) {
		queensq = node -> wqueenlist [0];					
		
		// check if the queen has a contact-mate in one
		matesq = t.queen_kisses_king [queensq] [node -> bkpos];
		if (matesq == 0) {
			return false;			
		}
		node -> matrix [queensq] = 0;		
		if (-- matesq < 64) {
			piece = node->matrix[matesq];
			if (piece && piece<KING) {
				node->matrix [queensq] = QUEEN;
				return false;
			} 
			if (nooccs (node, queensq, matesq) == false) {		
				node -> matrix [queensq] = QUEEN;
				return false;
			}
			if (!legal_move_w (node, ENCODEMOVE (queensq, matesq, QUEEN))) {
				node -> matrix [queensq] = QUEEN;
				return false;
			}
			if (attacked_by_pnbrq (node, matesq)) {
				node -> matrix [queensq] = QUEEN;
				return false;
			}
			if (attacked_by_PNBRK (node, matesq) == false) {
				node -> matrix [queensq] = QUEEN;
				return false;
			}			
			// now see if the king can escape by walking away
			node -> wqueenlist [0] = matesq;
			escapesq = t.nextpos [KING] [node -> bkpos] [node -> bkpos];
			do {
				piece = node -> matrix [escapesq];
				if (piece < KING && (! attacked_by_PNBRQK (node, escapesq))) {
					break;
				}				
				escapesq = t.nextpos [KING] [node -> bkpos] [escapesq];
			} while (escapesq != node -> bkpos);			
			node -> wqueenlist [0] = queensq;			
			if (escapesq == node -> bkpos) { // king can not escape.. it is mate
				node -> matrix [queensq] = QUEEN;				
				return true;			
			}
		} else {
			matesq = t.nextpos [KING] [node -> bkpos] [node -> bkpos];					
			do {
				piece = node->matrix [matesq];
				if (piece && piece<KING) {
					matesq = t.nextpos [KING] [node->bkpos] [matesq];
					continue;
				}
				if (t.pieceattacks [QUEEN] [queensq] [matesq] == false) {
					matesq = t.nextpos [KING] [node -> bkpos] [matesq];
					continue;
				}
				if (nooccs (node, queensq, matesq) == false) {				
					matesq = t.nextpos [KING] [node -> bkpos] [matesq];
					continue;
				}
				if (!legal_move_w (node, ENCODEMOVE (queensq, matesq, QUEEN))) {
                        		matesq = t.nextpos [KING] [node->bkpos] [matesq];
					continue;
				}

				if (attacked_by_pnbrq (node, matesq)) {
					matesq = t.nextpos [KING] [node -> bkpos] [matesq];
					continue;
				}
				if (attacked_by_PNBRK (node, matesq) == false) {
					matesq = t.nextpos [KING] [node -> bkpos] [matesq];
					continue;
				}			
				// now see if the king can escape by walking away
				node -> wqueenlist [0] = matesq;
				escapesq = t.nextpos [KING] [node -> bkpos] [node -> bkpos];
				do {
					piece = node -> matrix [escapesq];
					if (piece < KING && (! attacked_by_PNBRQK (node, escapesq))) {
						break;
					}				
					escapesq = t.nextpos [KING] [node -> bkpos] [escapesq];
				} while (escapesq != node -> bkpos);			
				node -> wqueenlist [0] = queensq;
				matesq = t.nextpos [KING] [node -> bkpos] [matesq];
				if (escapesq == node -> bkpos) { // king can not escape.. it is mate
					node -> matrix [queensq] = QUEEN;					
					return true;			
				}
			} while (matesq != node -> bkpos);
		}
		node -> matrix [queensq] = QUEEN;		
	}	
	return false;
}

bool q_mates_K (TFastNode * node)
{	
	int matesq, queensq, escapesq, piece;	
	if (node -> bqueens == 1) {
		queensq = node -> bqueenlist [0];
		matesq = t.queen_kisses_king [queensq] [node -> wkpos];
		if (matesq == 0) {
			return false;			
		}
		node -> matrix [queensq] = 0;		
		if (-- matesq < 64) {
			piece = node->matrix [matesq];
			if (piece>KING) {
				node->matrix [queensq] = BQUEEN;
				return false;
			}
			if (nooccs (node, queensq, matesq) == false) {
				node -> matrix [queensq] = BQUEEN;
				return false;
			}
			if (!legal_move_b (node, ENCODEMOVE (queensq, matesq, QUEEN))) {
                                node -> matrix [queensq] = BQUEEN;
                                return false;
                        }
			if (attacked_by_PNBRQ (node, matesq)) {
				node -> matrix [queensq] = BQUEEN;
				return false;
			}
			if (attacked_by_pnbrk (node, matesq) == false) {
				node -> matrix [queensq] = BQUEEN;
				return false;
			}			
			// now see if the king can escape by walking away
			node -> bqueenlist [0] = matesq;
			escapesq = t.nextpos [KING] [node -> wkpos] [node -> wkpos];
		do {
				piece = node -> matrix [escapesq];
				if ((piece == 0 || piece > KING) && (! attacked_by_pnbrqk (node, escapesq))) {
					break;
				}				
				escapesq = t.nextpos [KING] [node -> wkpos] [escapesq];
			} while (escapesq != node -> wkpos);			
			node -> bqueenlist [0] = queensq;			
			if (escapesq == node -> wkpos) { // king can not escape.. it is mate
				node -> matrix [queensq] = BQUEEN;				
				return true;			
			}
		} else {		
			matesq = t.nextpos [KING] [node -> wkpos] [node -> wkpos];		
			do { 
				piece = node->matrix [matesq];
				if (piece>KING) {
					matesq = t.nextpos [KING] [node->wkpos] [matesq];
					continue;
				}
				if (t.pieceattacks [QUEEN] [queensq] [matesq] == false) { 
					matesq = t.nextpos [KING] [node -> wkpos] [matesq];
					continue;
				}
				if (nooccs (node, queensq, matesq) == false) {
					matesq = t.nextpos [KING] [node -> wkpos] [matesq];
					continue;
				}
				if (!legal_move_b (node, ENCODEMOVE (queensq, matesq, QUEEN))) {
                                        matesq = t.nextpos [KING] [node->wkpos] [matesq];
                                        continue;
                                }
				if (attacked_by_PNBRQ (node, matesq)) {
					matesq = t.nextpos [KING] [node -> wkpos] [matesq];
					continue;
				}
				if (attacked_by_pnbrk (node, matesq) == false) {
					matesq = t.nextpos [KING] [node -> wkpos] [matesq];
					continue;
				}			
				// now see if the king can escape by walking away
				node -> bqueenlist [0] = matesq;
				escapesq = t.nextpos [KING] [node -> wkpos] [node -> wkpos];
				do {
					piece = node -> matrix [escapesq];
					if ((piece == 0 || piece > KING) && (! attacked_by_pnbrqk (node, escapesq))) {
						break;
					}				
				escapesq = t.nextpos [KING] [node -> wkpos] [escapesq];
				} while (escapesq != node -> wkpos);			
				node -> bqueenlist [0] = queensq;
				matesq = t.nextpos [KING] [node -> wkpos] [matesq];
				if (escapesq == node -> wkpos) { // king can not escape.. it is mate
					node -> matrix [queensq] = BQUEEN;					
					return true;			
				}
			} while (matesq != node -> wkpos);
		}
		node -> matrix [queensq] = BQUEEN;		
	}	
	return false;	
}

void found_mate_w (TFastNode* node) {
	if (g.winning) {
		g.stopsearch = true; 
		return;
	}	

	g.winning = true;
	g.stopsearch = true;	
	return;
}

void found_mate_b (TFastNode* node) {
	if (g.winning) {
		g.stopsearch = true; 
		return;
	}	
	g.winning = true;
	g.stopsearch = true;
	return;
}



void _fast_AddPiece(TFastNode* node, char piece, unsigned char sq) {
	node -> matrix [sq] = piece;
	node -> hashcode ^= hashnumbers [piece - 1] [sq];
	switch (piece) {
	case PAWN:    
		node -> pawncode ^= hashnumbers [PAWN - 1] [sq];
		node -> index [sq] = node -> wpawns; 
		node -> wpawnlist [node->wpawns ++] = sq;
		//node -> material += PAWN_VALUE;
		break;
	case KNIGHT:  
		node -> index [sq] = node -> wknights; 
		node -> wknightlist [node -> wknights ++] = sq; 
		node -> wpieces ++; 
		//node -> material += KNIGHT_VALUE;
		break;
	case BISHOP:  
		node -> index [sq] = node -> wbishops; 
		node -> wbishoplist [node -> wbishops ++] = sq; 
		node -> wpieces ++; 
		//node -> material += BISHOP_VALUE;
		break;
	case ROOK:    
		node -> index [sq] = node -> wrooks; 
		node->wrooklist [node -> wrooks ++] = sq; 
		node -> wpieces ++; 
		//node -> material += ROOK_VALUE;
		break;
	case QUEEN:   
		node -> index [sq] = node -> wqueens; 
		node->wqueenlist [node -> wqueens ++] = sq; 
		node -> wpieces ++; 
		//node -> material += QUEEN_VALUE;
		break;
	case KING:    
		node -> wkpos = sq; break;
	case BPAWN:    
		node -> pawncode ^= hashnumbers [BPAWN - 1] [sq];
		node -> index [sq] = node -> bpawns; 
		node -> bpawnlist [node -> bpawns ++] = sq;
		//node -> material -= PAWN_VALUE;
		break;
	case BKNIGHT:  
		node -> index [sq] = node -> bknights; 
		node -> bknightlist [node -> bknights ++] = sq; 
		node -> bpieces ++; 
		//node -> material -= KNIGHT_VALUE;
		break;
	case BBISHOP:  
		node -> index [sq] = node -> bbishops; 
		node -> bbishoplist [node -> bbishops ++] = sq; 
		node -> bpieces ++; 
		//node -> material -= BISHOP_VALUE;
		break;
	case BROOK:    
		node -> index [sq] = node -> brooks; 
		node -> brooklist [node -> brooks ++] = sq; 
		node -> bpieces ++; 
		//node -> material -= ROOK_VALUE;
		break;
	case BQUEEN:   
		node -> index [sq] = node -> bqueens; 
		node -> bqueenlist [node -> bqueens ++] = sq; 
		node -> bpieces++; 
		//node -> material -= QUEEN_VALUE;
		break;
	case BKING:    
		node -> bkpos = sq;
	}
}


void _fast_clearnode(TFastNode* node) {
	memset (node, 0, sizeof (TFastNode));
}

void _fast_matrix2node(TFastNode* node, unsigned char matrix[64], int flags, int fifty) {
	_fast_clearnode(node);
	for (unsigned char sq=0; sq<64; sq++) {
		if (matrix[sq])
			_fast_AddPiece(node,matrix[sq],sq);
	}
	node -> flags = flags;
	if (_SCW (node)) {
		HashSCW (node);
	}
	if _SCB(node) HashSCB(node);
	if _LCW(node) HashLCW(node);
	if _LCB(node) HashLCB(node);
	if _EPSQ(node) node->hashcode ^= ephash[_EPSQ(node)];
	if (flags & _WTM) node->hashcode |= 1;
	else node->hashcode &= _LL(0xFFFFFFFFFFFFFFFE);
	node->fifty = fifty; 
}

void print__move (int move) {
	_print_LAN (move); 
}

int _fast_resolve_pv(int *dest)
{
	/* just copy the pv */ 
	int i=0; 
	while (g.pv[0][i]) {  
		dest[i] = g.pv[0][i];
 		i++; 
	}	
	dest[i] = 0; 
	return i; 

} 

void print_pv () 
{	
	
	int pv[512]; 
	int pv_len = _fast_resolve_pv(pv);

	for (int i=0; i<pv_len; i++) {
		_print_LAN (pv[i]);
		std::cout << " "; 
	}
}

int print_pv_addstr (std::ostringstream& str)
{
        int pv[512];
        int pv_len = _fast_resolve_pv(pv);

        for (int i=0; i<pv_len; i++) {
                _fast_LAN (str, pv[i]);
                str << " "; 		
        }
	return pv_len; 
}


void print_path (int ply) 
{     
	for (int i = 0; i < ply; i++) {
		if (i>0) {
			cout <<  ", ";
		}
		_print_LAN (g.path [i]); 	          
	}
}

void add_time () {
	if (g.stoptime < g.maxtime) {
		g.stoptime = g.maxtime;		
	} 	
}

#ifdef PRINT_SEARCH 

void print_search_entry (TFastNode *node, int type, int ply, int depth)
{
	for (int i=1; i<=ply; i++) { 
		cout << "  ";
	}

	cout << format("-->%10x ") % (int)node->hashcode;

	switch (type) {
	case _PS_PVS_W:
		cout << "PVS_W ";
		break;
	case _PS_PVS_B:
		cout << "PVS_B ";
		break;
	case _PS_PVS_E_W:
		cout << "EVA_W ";
		break;
	case _PS_PVS_E_B:
		cout << "EVA_B ";
		break;
	case _PS_Q_W:
		cout << "QUI_W ";
		break;
	case _PS_Q_B:
		cout << "QUI_B ";
		break;
	case _PS_Q_E_W:
		cout << "QEV_W ";
		break;
	case _PS_Q_E_B:
		cout << "QEV_B ";
		break;
	}

	cout << format("ply=%d depth=%d entry_move=") % ply % depth;
	print_move(g.path[ply-1]);
	cout << "\n";

}

void print_search (TFastNode* node, int alpha, int beta, int move, int ply, int score, int type, int proc) 
{     
	for (int i=1; i<=ply; i++) { 
		g_print("  "); 
	}

	cout << format("<--%10x ")  % (int)node->hashcode;

	switch (proc) {
	case _PS_PVS_W:
		cout << "PVS_W ";
		break;
	case _PS_PVS_B:
		cout << "PVS_B ";
		break;
	case _PS_PVS_E_W:
		cout << "EVA_W ";
		break;
	case _PS_PVS_E_B:
		cout << "EVA_B ";
		break;
	case _PS_Q_W:
		cout << "QUI_W ";
		break;
	case _PS_Q_B:
		cout << "QUI_B ";
		break;
	case _PS_Q_E_W:
		cout << "QEV_W ";
		break;
	case _PS_Q_E_B:
		cout << "QEV_B ";
		break;
	}
	// print_score (_result_value);
	cout << format(" i=%d, n=%d, p=%d, v=%d, a=%d, b=%d, path: " % g.iteration %
			g.fastnodes % ply % score % alpha % beta);
	print_path (ply);
	if (move) {
		cout << ", ";
		print_move (move);
	}
	cout << " ";
	if ((int)node->hashcode == 0x2abac3b) {
		cout << "gotcha!\n";
	}
	switch (type) {
	case _PS_HASH:
		cout << "hash\n";
		return;
	case _PS_CUT:
		cout << "cut\n";
		return;
	case _PS_TRIVIAL:
		cout << "trivial\n";
		return;
	case _PS_NORMAL:
		cout << "normal\n";
		return;
	case _PS_TIME:
		cout << "time\n";
		return;
	case _PS_MAXPLY:
		cout << "maxply\n";
		return; 
	case _PS_NULLCUT:
		cout << "nullcut\n";
		return;	
	case _PS_EVAL:
		cout << "eval\n";
		return; 
	case _PS_UNKNOWN: 
		cout << "score_unknown\n";
		break; 
	default:
		cout << "ERROR\n";
		return;
	}
}

#endif

void root_new_best (TFastNode *node, int index, int value) {
	TRootMove swap;	
	update_pv (node, g.rootmoves [index]. move, 0);
	
	if ((!index) && (g.result_value == value)) {
		return;
	}	
	
	if (index) {
		swap = g.rootmoves [0];
		g.rootmoves [0] = g.rootmoves [index];
		g.rootmoves [index] = swap;				
	}
		
	if (g.result_value != value) {					
		g.result_value = value;	       
	}	
			
// #ifdef PRINT_SEARCH
        if (g.iteration > 4) {            
            cout << format("  --> %d d=%d (%d) ") % value % g.iteration % g.fastnodes;
            print_pv();
            cout << "                                           \r";
        }
// #endif

}

bool rootmove_isgreater (int a, int b) 
{
	TRootMove move_a = g.rootmoves [a];
	TRootMove move_b = g.rootmoves [b];

	if (move_a. avoid != move_b. avoid) {
		return move_a. avoid < move_b. avoid;
	}
	if (move_a. unknown != move_b. unknown) {
		return move_b. unknown;
	}
	if (move_a. value != move_b. value) {
		return move_a. value > move_b. value;
	}
	if (move_a. nodes != move_b. nodes) {
		return move_a. nodes > move_b. nodes;
	}
	if (move_a. bookvalue != move_b. bookvalue) {
		return move_a. bookvalue > move_b. bookvalue;
	}       	
	return false;
}

void rootmove_sort (int first, int last) // bubble sort root moves
{
  
	TRootMove move;
	for (int i = first; i < last - 1; i ++) {
		for (int j = first; j < last - 1 - i; j ++) {
			if (rootmove_isgreater (j + 1, j)) {
				move = g.rootmoves [j];
				g.rootmoves [j] = g.rootmoves [j + 1];
				g.rootmoves [j + 1] = move;
			}
		}
	}	
}

void display_time (_int64 time) 
{
	int hours,
		mins,
		secs;
	hours = time / MSECS_PER_HOUR;
	time -= hours * MSECS_PER_HOUR;
	mins = time / MSECS_PER_MIN;
	time -= mins * MSECS_PER_MIN;
	secs = time / MSECS_PER_SEC;
	time -= secs * MSECS_PER_SEC;

	if (!secs && !mins && !hours) { 
		cout << format("%dms") % (int)time;
		return;
	}

	if (hours) {
		cout << format("%d:") %  hours;
	} else {
		//g_print ("0:");
	}
	if (mins) {
		if (hours && mins < 10) {
			cout << "0";
		}
		cout << format("%d:") %  mins;
	} else {
		if (hours) {
			cout << "00:";
		} else {
			cout << "0:";
		}
	}
	if (secs) {
		if (secs < 10) {
			cout << "0";
		}
		cout << format("%d") % secs;
	} else {
		cout << "00";
	}	
}

void _fast_init_iterate (TFastNode * node) 
{			
	for (int i = 0; i < 4096; i ++) {
		g.tsort [i] >>= 1;
	}
	node->maximum_mobility_score >>= 1;
	g.iteration = 1;
	g.maxply = 0;
	g.result_value = - CHESS_INF;
	g.fastnodes = 0;	
	g.repindex = node -> fifty;
	g.rootscore = 0;
	g.crisis = false; 
}

int _fast_gennoncapsw (TFastNode* node, int index) 
{
	int i,
	ssq,
	tsq;
  
	// generate non captures with white pawns
	for (i = 0; i < node->wpawns; i++) {
		ssq = node->wpawnlist [i];
		tsq = ssq + 8;
		if (!node->matrix [tsq]) {
			if (ssq < a7) {
				g.tmoves [index++] = ENCODEMOVE (ssq, tsq, PAWN);
				if (ssq < a3) { // double pawn push
					tsq += 8;
					if (!node->matrix [tsq]) {
						g.tmoves [index++] = ENCODEMOVE (ssq, tsq, PAWN);
					}
				}
			} else { // promotion
				g.tmoves [index++] = ENCODESPECIAL (ssq, tsq, PAWN, 0, _QUEEN_PROMOTION);
				g.tmoves [index++] = ENCODESPECIAL (ssq, tsq, PAWN, 0, _ROOK_PROMOTION);
				g.tmoves [index++] = ENCODESPECIAL (ssq, tsq, PAWN, 0, _BISHOP_PROMOTION);
				g.tmoves [index++] = ENCODESPECIAL (ssq, tsq, PAWN, 0, _KNIGHT_PROMOTION);
			}
		}
	}
  
	// generate non captures with knights
	for (i = 0; i < node->wknights; i++) {
		ssq = node->wknightlist [i];
		tsq = t.nextdir [KNIGHT][ssq][ssq];
		do {
			if (!node->matrix [tsq]) {
				g.tmoves [index++] = ENCODEMOVE (ssq, tsq, KNIGHT);
			}
			tsq = t.nextdir [KNIGHT][ssq][tsq];
		} while (ssq != tsq);
	}

	// generate non captures with bishops
	for (i = 0; i < node->wbishops; i++) {
		ssq = node->wbishoplist[i];
		tsq = t.nextpos [BISHOP][ssq][ssq];
		do {
			if (!node->matrix [tsq]) {
				g.tmoves [index++] = ENCODEMOVE (ssq, tsq, BISHOP);
				tsq = t.nextpos [BISHOP][ssq][tsq];
			} else {
				tsq = t.nextdir [BISHOP][ssq][tsq];
			}
		} while (ssq != tsq);
	}

	// generate non captures with rooks
	for (i = 0; i < node->wrooks; i++) {
		ssq = node->wrooklist[i];
		tsq = t.nextpos [ROOK][ssq][ssq];
		do {
			if (!node->matrix [tsq]) {
				g.tmoves [index++] = ENCODEMOVE (ssq, tsq, ROOK);
				tsq = t.nextpos [ROOK][ssq][tsq];
			} else {
				tsq = t.nextdir [ROOK][ssq][tsq];
			}
		} while (ssq != tsq);
	}

	// generate non captures with queens
	for (i = 0; i < node->wqueens; i++) {
		ssq = node->wqueenlist[i];
		tsq = t.nextpos [QUEEN][ssq][ssq];
		do {
			if (!node->matrix [tsq]) {
				g.tmoves [index++] = ENCODEMOVE (ssq, tsq, QUEEN);
				tsq = t.nextpos [QUEEN][ssq][tsq];
			} else {
				tsq = t.nextdir [QUEEN][ssq][tsq];
			}
		} while (ssq != tsq);
	}

	// generate non captures with king
	ssq = node->wkpos;
	tsq = t.nextpos [KING][ssq][ssq];
	do {
		if (!node->matrix [tsq]) {
			g.tmoves [index++] = ENCODEMOVE (ssq, tsq, KING);
		}
		tsq = t.nextpos [KING][ssq][tsq];
	} while (ssq != tsq);

	// castle moves
	if (_CASTLEW (node)) {
		if (_SCW(node) && !node->matrix [f1] && !node->matrix [g1]) {
			g.tmoves[index++] = ENCODESCW;
		}
		if (_LCW(node) && !node->matrix [d1] && !node->matrix [c1] && !node->matrix [b1]) {
			g.tmoves[index++] = ENCODELCW;
		}	
	}

	return index;
}

int _fast_gennoncapsb (TFastNode* node, int index)
{ 
	int i, ssq, tsq;
  
	// generate non captures with black pawns  
	for (i = 0; i < node->bpawns; i++) {
		ssq = node->bpawnlist [i];
		tsq = ssq - 8;
		if (!node->matrix [tsq]) {
			if (ssq > h2) {
				g.tmoves [index++] = ENCODEMOVE (ssq, tsq, PAWN);
				if (ssq > h6) { // double pawn push
					tsq -= 8;
					if (!node->matrix [tsq]) {
						g.tmoves [index++] = ENCODEMOVE (ssq, tsq, PAWN);
					}
				}
			} else { // promotion
				g.tmoves [index++] = ENCODESPECIAL (ssq, tsq, PAWN, 0, _QUEEN_PROMOTION);
				g.tmoves [index++] = ENCODESPECIAL (ssq, tsq, PAWN, 0, _ROOK_PROMOTION);
				g.tmoves [index++] = ENCODESPECIAL (ssq, tsq, PAWN, 0, _BISHOP_PROMOTION);
				g.tmoves [index++] = ENCODESPECIAL (ssq, tsq, PAWN, 0, _KNIGHT_PROMOTION);
			}
		}
	}
  
	// generate non captures with knights
	for (i = 0; i < node->bknights; i++) {
		ssq = node->bknightlist [i];
		tsq = t.nextdir [KNIGHT][ssq][ssq];
		do {
			if (!node->matrix [tsq]) {
				g.tmoves [index++] = ENCODEMOVE (ssq, tsq, KNIGHT);
			}
			tsq = t.nextdir [KNIGHT][ssq][tsq];
		} while (ssq != tsq);
	}

	// generate non captures with bishops
	for (i = 0; i < node->bbishops; i++) {
		ssq = node->bbishoplist[i];
		tsq = t.nextpos [BISHOP][ssq][ssq];
		do {
			if (!node->matrix [tsq]) {
				g.tmoves [index++] = ENCODEMOVE (ssq, tsq, BISHOP);
				tsq = t.nextpos [BISHOP][ssq][tsq];
			} else {
				tsq = t.nextdir [BISHOP][ssq][tsq];
			}
		} while (ssq != tsq);
	}

	// generate non captures with rooks
	for (i = 0; i < node->brooks; i++) {
		ssq = node->brooklist[i];
		tsq = t.nextpos [ROOK][ssq][ssq];
		do {
			if (!node->matrix [tsq]) {
				g.tmoves [index++] = ENCODEMOVE (ssq, tsq, ROOK);
				tsq = t.nextpos [ROOK][ssq][tsq];
			} else {
				tsq = t.nextdir [ROOK][ssq][tsq];
			}
		} while (ssq != tsq);
	}

	// generate non captures with queens
	for (i = 0; i < node->bqueens; i++) {
		ssq = node->bqueenlist[i];
		tsq = t.nextpos [QUEEN][ssq][ssq];
		do {
			if (!node->matrix [tsq]) {
				g.tmoves [index++] = ENCODEMOVE (ssq, tsq, QUEEN);
				tsq = t.nextpos [QUEEN][ssq][tsq];
			} else {
				tsq = t.nextdir [QUEEN][ssq][tsq];
			}
		} while (ssq != tsq);
	}

	// generate non captures with king
	ssq = node->bkpos;
	tsq = t.nextpos [KING][ssq][ssq];
	do {
		if (!node->matrix [tsq]) {
			g.tmoves [index++] = ENCODEMOVE (ssq, tsq, KING);
		}
		tsq = t.nextpos [KING][ssq][tsq];
	} while (ssq != tsq);

	// castle moves
	if (_CASTLEB (node)) {
		if (_SCB(node) && !node->matrix [f8] && !node->matrix [g8]) {
			g.tmoves[index++] = ENCODESCB;
		}
		if (_LCB(node) && !node->matrix [d8] && !node->matrix [c8] && !node->matrix [b8]) {
			g.tmoves[index++] = ENCODELCB;
		}
	}

	return index;
}

int _fast_gencapsw (TFastNode* node, int index) 
{
	int     i, ssq, tsq, captured;

	// generate captures with white pawns
	for (i = 0; i < node->wpawns; i++) {
		ssq = node->wpawnlist [i];
		tsq = t.nextdir [PAWN][ssq][ssq];
		do {
			captured = node->matrix [tsq];
			if (captured>KING) {
				if (tsq <= h7) {
					g.tmoves [index++] = ENCODECAPTURE (ssq, tsq, PAWN, captured - KING);
				} else { //promotion  
					g.tmoves [index++] = ENCODESPECIAL (ssq, tsq, PAWN, captured - KING, _QUEEN_PROMOTION);
                                        g.tmoves [index++] = ENCODESPECIAL (ssq, tsq, PAWN, captured - KING, _ROOK_PROMOTION);
                                        g.tmoves [index++] = ENCODESPECIAL (ssq, tsq, PAWN, captured - KING, _BISHOP_PROMOTION);
                                        g.tmoves [index++] = ENCODESPECIAL (ssq, tsq, PAWN, captured - KING, _KNIGHT_PROMOTION);
				}
			} else if (_EPSQ (node) && (tsq == _EPSQ (node))) { //en passant
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
			captured = node->matrix [tsq];
			if (captured > KING) {
				g.tmoves[index++] = ENCODECAPTURE (ssq, tsq, KNIGHT, captured - KING);
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
			if (!captured) {
				tsq = t.nextpos [BISHOP][ssq][tsq];	 
			} else {
				if (captured > KING) {
					g.tmoves [index++] = ENCODECAPTURE (ssq, tsq, BISHOP, captured - KING);
				}
				tsq = t.nextdir [BISHOP][ssq][tsq];
			}	
		} while (ssq != tsq);
	}

	// generate captures with white rooks
	for (i = 0; i < node->wrooks; i++) {
		ssq = node->wrooklist[i];
		tsq = t.nextpos [ROOK][ssq][ssq];
		do {
			captured = node->matrix [tsq];
			if (!captured) {
				tsq = t.nextpos [ROOK][ssq][tsq];	 
			} else {
				if (captured > KING) {
					g.tmoves [index++] = ENCODECAPTURE (ssq, tsq, ROOK, captured - KING);
				}
				tsq = t.nextdir [ROOK][ssq][tsq];
			}	
		} while (ssq != tsq);
	}

	// generate captures with white queens
	for (i = 0; i < node->wqueens; i++) {
		ssq = node->wqueenlist[i];
		tsq = t.nextpos [QUEEN][ssq][ssq];
		do {
			captured = node->matrix [tsq];
			if (!captured) {
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

int _fast_gencapsb (TFastNode* node, int index) 
{
	int i, ssq, tsq, captured;
  
	// generate captures with black pawns
	for (i = 0; i < node->bpawns; i++) {
		ssq = node->bpawnlist [i];
		tsq = t.nextdir [BPAWN][ssq][ssq];
		do {
			captured = node->matrix [tsq];
			if (captured && captured <= KING) {
				if (tsq >= a2) {
					g.tmoves [index++] = ENCODECAPTURE (ssq, tsq, PAWN, captured);
				} else { //promotion  
					g.tmoves [index++] = ENCODESPECIAL (ssq, tsq, PAWN, captured, _QUEEN_PROMOTION);
                                        g.tmoves [index++] = ENCODESPECIAL (ssq, tsq, PAWN, captured, _ROOK_PROMOTION);
                                        g.tmoves [index++] = ENCODESPECIAL (ssq, tsq, PAWN, captured, _BISHOP_PROMOTION); 
                                        g.tmoves [index++] = ENCODESPECIAL (ssq, tsq, PAWN, captured, _KNIGHT_PROMOTION);
				}
			} else if (_EPSQ (node) && (tsq == _EPSQ (node))) { //en passant
				g.tmoves [index++] = ENCODEEP (ssq, tsq);
			} 
			tsq = t.nextdir [BPAWN][ssq][tsq];
		} while (ssq != tsq);
	}
    
	// generate captures with black knights
	for (i = 0; i < node->bknights; i++) {
		ssq = node->bknightlist[i];
		tsq = t.nextdir [KNIGHT][ssq][ssq];
		do {
			captured = node->matrix [tsq];
			if (captured && captured <= KING) {
				g.tmoves[index++] = ENCODECAPTURE (ssq, tsq, KNIGHT, captured);
			}
			tsq = t.nextdir [KNIGHT][ssq][tsq];
		} while (ssq != tsq);
	}
  
	// generate captures with black bishops
	for (i = 0; i < node->bbishops; i++) {
		ssq = node->bbishoplist[i];
		tsq = t.nextpos [BISHOP][ssq][ssq];
		do {
			captured = node->matrix [tsq];
			if (!captured) {
				tsq = t.nextpos [BISHOP][ssq][tsq];	 
			} else {
				if (captured <= KING) {
					g.tmoves [index++] = ENCODECAPTURE (ssq, tsq, BISHOP, captured);
				}
				tsq = t.nextdir [BISHOP][ssq][tsq];
			}	
		} while (ssq != tsq);
	}

	// generate captures with black rooks
	for (i = 0; i < node->brooks; i++) {
		ssq = node->brooklist[i];
		tsq = t.nextpos [ROOK][ssq][ssq];
		do {
			captured = node->matrix [tsq];
			if (!captured) {
				tsq = t.nextpos [ROOK][ssq][tsq];	 
			} else {
				if (captured <= KING) {
					g.tmoves [index++] = ENCODECAPTURE (ssq, tsq, ROOK, captured);
				}
				tsq = t.nextdir [ROOK][ssq][tsq];
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
				tsq = t.nextpos [QUEEN][ssq][tsq];	 
			} else {
				if (captured <= KING) {
					g.tmoves [index++] = ENCODECAPTURE (ssq, tsq, QUEEN, captured);
				}
				tsq = t.nextdir [QUEEN][ssq][tsq];
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

	return index;
}

int _fast_genmovesw(TFastNode* node,int index) 
{
	int ssq, tsq, i, captured;
 
	//wpawn
	for (i=0; i<node->wpawns; i++) {
		ssq = node->wpawnlist[i];
		tsq = ssq+8;
		captured = node->matrix[tsq];
		if (!captured) {
			if (tsq<=h7) {
				g.tmoves[index++] = ENCODEMOVE(ssq,tsq,PAWN);
				if (ssq<a3) {
					tsq += 8;
					if (!node->matrix[tsq]) { 
						g.tmoves[index++] = ENCODEMOVE(ssq,tsq,PAWN);
					}

				}
			} else { //promotion
				g.tmoves[index++]=ENCODESPECIAL(ssq,tsq,PAWN,0,_QUEEN_PROMOTION);
				g.tmoves[index++]=ENCODESPECIAL(ssq,tsq,PAWN,0,_ROOK_PROMOTION);
				g.tmoves[index++]=ENCODESPECIAL(ssq,tsq,PAWN,0,_BISHOP_PROMOTION);
				g.tmoves[index++]=ENCODESPECIAL(ssq,tsq,PAWN,0,_KNIGHT_PROMOTION);
			}
		}
		tsq = t.nextdir[PAWN][ssq][ssq];
		do {
			captured = node->matrix[tsq];
			if (captured>KING) {
				if (tsq<=h7) { 
					g.tmoves[index++]=ENCODECAPTURE(ssq,tsq,PAWN,captured-KING);
				} else { //promotion
					g.tmoves[index++]=ENCODESPECIAL(ssq,tsq,PAWN,captured-KING,_QUEEN_PROMOTION);
					g.tmoves[index++]=ENCODESPECIAL(ssq,tsq,PAWN,captured-KING,_ROOK_PROMOTION);
					g.tmoves[index++]=ENCODESPECIAL(ssq,tsq,PAWN,captured-KING,_BISHOP_PROMOTION);
					g.tmoves[index++]=ENCODESPECIAL(ssq,tsq,PAWN,captured-KING,_KNIGHT_PROMOTION);
				}
			} else if (_EPSQ(node) && tsq==_EPSQ(node)) { 
				g.tmoves[index++]=ENCODEEP(ssq,tsq);
			}
			tsq = t.nextdir[PAWN][ssq][tsq];
		} while (ssq != tsq);
	}

	//wknight
	for (i=0; i<node->wknights; i++) {
		ssq = node->wknightlist[i];
		tsq = t.nextpos[KNIGHT][ssq][ssq];
		do {
			captured = node->matrix[tsq];
			if (!captured) { 
				g.tmoves[index++] = ENCODEMOVE(ssq,tsq,KNIGHT);
			} else if (captured>KING) { 
				g.tmoves[index++] = ENCODECAPTURE(ssq,tsq,KNIGHT,captured-KING);
			}
			tsq = t.nextpos[KNIGHT][ssq][tsq];
		} while (ssq != tsq);
	}

	//wbishop
	for (i=0; i<node->wbishops; i++) {
		ssq = node->wbishoplist[i];
		tsq = t.nextpos[BISHOP][ssq][ssq];
		do {
			captured = node->matrix[tsq];
			if (!captured) {
				g.tmoves[index++] = ENCODEMOVE(ssq,tsq,BISHOP);
				tsq = t.nextpos[BISHOP][ssq][tsq];
			} else {
				if (captured>KING) { 
					g.tmoves[index++] = ENCODECAPTURE(ssq,tsq,BISHOP,captured-KING);
				}
				tsq = t.nextdir[BISHOP][ssq][tsq];
			}
		} while (ssq != tsq);
	}

	//wrook
	for (i=0; i<node->wrooks; i++) {
		ssq = node->wrooklist[i];
		tsq = t.nextpos[ROOK][ssq][ssq];
		do {
			captured = node->matrix[tsq];
			if (!captured) {
				g.tmoves[index++] = ENCODEMOVE(ssq,tsq,ROOK);
				tsq = t.nextpos[ROOK][ssq][tsq];
			} else {
				if (captured>KING) { 
					g.tmoves[index++] = ENCODECAPTURE(ssq,tsq,ROOK,captured-KING);
				}
				tsq = t.nextdir[ROOK][ssq][tsq];
			}
		} while (ssq != tsq);
	}

	//wqueen
	for (i=0; i<node->wqueens; i++) {
		ssq = node->wqueenlist[i];
		tsq = t.nextpos[QUEEN][ssq][ssq];
		do {
			captured = node->matrix[tsq];
			if (!captured) {
				g.tmoves[index++] = ENCODEMOVE(ssq,tsq,QUEEN);
				tsq = t.nextpos[QUEEN][ssq][tsq];
			} else {
				if (captured>KING) { 
					g.tmoves[index++] = ENCODECAPTURE(ssq,tsq,QUEEN,captured-KING);
				}
				tsq = t.nextdir[QUEEN][ssq][tsq];
			}
		} while (ssq != tsq);
	}
 
	// wking
	ssq = node->wkpos;
	tsq = t.nextpos[KING][ssq][ssq];
	do {
		captured = node->matrix[tsq];
		if (!captured) { 
			g.tmoves[index++] = ENCODEMOVE(ssq,tsq,KING);
		} else if (captured>KING) { 
			g.tmoves[index++] = ENCODECAPTURE(ssq,tsq,KING,captured-KING);
		}
		tsq = t.nextpos[KING][ssq][tsq];
	} while (ssq != tsq);
	if _CASTLEW(node) {
		if (_SCW(node) && !node->matrix[f1] && !node->matrix[g1]) { 
			g.tmoves[index++] = ENCODESCW;
		}
		if (_LCW(node) && !node->matrix[d1] && !node->matrix[c1] && !node->matrix[b1]) { 
			g.tmoves[index++] = ENCODELCW;
		}
	}

	return index;
}

int _fast_genmovesb(TFastNode* node,int index) 
{
	int ssq, tsq, i, captured;

	//bpawn
	for (i=0; i<node->bpawns; i++) {
		ssq = node->bpawnlist[i];
		tsq = ssq-8;
		captured = node->matrix[tsq];
		if (!captured) {
			if (tsq>=a2) {
				g.tmoves[index++] = ENCODEMOVE(ssq,tsq,PAWN);
				if (ssq>h6) {
					tsq -= 8;
					if (!node->matrix[tsq]) { 
						g.tmoves[index++] = ENCODEMOVE(ssq,tsq,PAWN);
					}
				}
			} else { //promotion
				g.tmoves[index++]=ENCODESPECIAL(ssq,tsq,PAWN,0,_QUEEN_PROMOTION);
				g.tmoves[index++]=ENCODESPECIAL(ssq,tsq,PAWN,0,_ROOK_PROMOTION);
				g.tmoves[index++]=ENCODESPECIAL(ssq,tsq,PAWN,0,_BISHOP_PROMOTION);
				g.tmoves[index++]=ENCODESPECIAL(ssq,tsq,PAWN,0,_KNIGHT_PROMOTION);
			}
		}
		tsq = t.nextdir[BPAWN][ssq][ssq];
		do {
			captured = node->matrix[tsq];
			if (captured && captured<=KING) {
				if (tsq>=a2) { 
					g.tmoves[index++]=ENCODECAPTURE(ssq,tsq,PAWN,captured);
				} else { //promotion
					g.tmoves[index++]=ENCODESPECIAL(ssq,tsq,PAWN,captured,_QUEEN_PROMOTION);
					g.tmoves[index++]=ENCODESPECIAL(ssq,tsq,PAWN,captured,_ROOK_PROMOTION);
					g.tmoves[index++]=ENCODESPECIAL(ssq,tsq,PAWN,captured,_BISHOP_PROMOTION);
					g.tmoves[index++]=ENCODESPECIAL(ssq,tsq,PAWN,captured,_KNIGHT_PROMOTION);
				}
			} else if (_EPSQ(node) && tsq==_EPSQ(node)) { 
				g.tmoves[index++]=ENCODEEP(ssq,tsq);
			}
			tsq = t.nextdir[BPAWN][ssq][tsq];
		} while (ssq != tsq);
	}

	//bknight
	for (i=0; i<node->bknights; i++) {
		ssq = node->bknightlist[i];
		tsq = t.nextpos[KNIGHT][ssq][ssq];
		do {
			captured = node->matrix[tsq];
			if (!captured) { 
				g.tmoves[index++] = ENCODEMOVE(ssq,tsq,KNIGHT);
			} else if (captured<=KING) { 
				g.tmoves[index++] = ENCODECAPTURE(ssq,tsq,KNIGHT,captured);
			}
			tsq = t.nextpos[KNIGHT][ssq][tsq];
		} while (ssq != tsq);
	}

	//bbishop
	for (i=0; i<node->bbishops; i++) {
		ssq = node->bbishoplist[i];
		tsq = t.nextpos[BISHOP][ssq][ssq];
		do {
			captured = node->matrix[tsq];
			if (!captured) {
				g.tmoves[index++] = ENCODEMOVE(ssq,tsq,BISHOP);
				tsq = t.nextpos[BISHOP][ssq][tsq];
			} else {
				if (captured<=KING) {
					g.tmoves[index++] = ENCODECAPTURE(ssq,tsq,BISHOP,captured);
				}
				tsq = t.nextdir[BISHOP][ssq][tsq];
			}
		} while (ssq != tsq);
	}

	//brook
	for (i=0; i<node->brooks; i++) {
		ssq = node->brooklist[i];
		tsq = t.nextpos[ROOK][ssq][ssq];
		do {
			captured = node->matrix[tsq];
			if (!captured) {
				g.tmoves[index++] = ENCODEMOVE(ssq,tsq,ROOK);
				tsq = t.nextpos[ROOK][ssq][tsq];
			} else {
				if (captured<=KING) {
					g.tmoves[index++] = ENCODECAPTURE(ssq,tsq,ROOK,captured);
				}
				tsq = t.nextdir[ROOK][ssq][tsq];
			}
		} while (ssq != tsq);
	}

	//bqueen
	for (i=0; i<node->bqueens; i++) {
		ssq = node->bqueenlist[i];
		tsq = t.nextpos[QUEEN][ssq][ssq];
		do {
			captured = node->matrix[tsq];
			if (!captured) {
				g.tmoves[index++] = ENCODEMOVE(ssq,tsq,QUEEN);
				tsq = t.nextpos[QUEEN][ssq][tsq];
			} else {
				if (captured<=KING) {
					g.tmoves[index++] = ENCODECAPTURE(ssq,tsq,QUEEN,captured);
				}
				tsq = t.nextdir[QUEEN][ssq][tsq];
			}
		} while (ssq != tsq);
	}

	//bking
	ssq = node->bkpos;
	tsq = t.nextpos[KING][ssq][ssq];
	do {
		captured = node->matrix[tsq];
		if (!captured) { 
			g.tmoves[index++] = ENCODEMOVE(ssq,tsq,KING);
		} else if (captured<=KING) { 
			g.tmoves[index++] = ENCODECAPTURE(ssq,tsq,KING,captured);
		}
		tsq = t.nextpos[KING][ssq][tsq];
	} while (ssq != tsq);
	if _CASTLEB(node) {
		if (_SCB(node) && !node->matrix[f8] && !node->matrix[g8]) { 
			g.tmoves[index++] = ENCODESCB;
		}
		if (_LCB(node) && !node->matrix[d8] && !node->matrix[c8] && !node->matrix[b8]) {
			g.tmoves[index++] = ENCODELCB;
		}
	}

	return index;
}

void donullmove (TFastNode* node)
{
	if (_EPSQ (node)) {
		node -> hashcode ^= ephash [_EPSQ (node)];
		node -> flags &= ~ 63;
	}
	node -> hashcode ^= 1;
	node -> flags ^= _WTM;
	node -> flags |= NONULL;
	node -> fifty = 0;
	
}

void undonullmove (TFastNode * node, int oldflags, int oldfifty) 
{
	node -> flags = oldflags;
	node -> fifty = oldfifty;

	if (_EPSQ (node)) {
		node -> hashcode ^= ephash [_EPSQ (node)];
	}
	node -> hashcode ^= 1;
}


void _fast_dowmove(TFastNode* node, int move) 
{
	unsigned char ssq = SOURCESQ(move),
		tsq = TARGETSQ(move),
                i = node -> index [ssq];
        char piece = PIECE(move);
		
	node -> flags ^= _WTM;
	node -> hashcode ^= 1;
	if (_EPSQ(node)) {
		node -> hashcode ^= ephash [_EPSQ (node)];
		node -> flags &= ~63;
	}
	node -> matrix [ssq] = 0;
	node -> matrix [tsq] = piece;
	switch (piece) {
	case PAWN:    		
		node -> fifty = 0;
                if (!SPECIAL(move)) {
			node->hashcode^=hashnumbers[PAWN-1][ssq]^hashnumbers[PAWN-1][tsq];
			node -> pawncode ^= hashnumbers [PAWN - 1] [ssq] ^ hashnumbers [PAWN - 1] [tsq];
			node->wpawnlist[i]=tsq;
			if (((char) ssq + 16) == tsq) {
				node->flags |= ssq+8;
				node->hashcode ^= ephash[_EPSQ(node)];
			}
                } else switch SPECIALCASE(move) {
                 case _EN_PASSANT:
			 node->wpawnlist[i]=tsq;
			 node->index[tsq]=node->index[ssq];
			 node->hashcode^=hashnumbers[PAWN-1][ssq]^hashnumbers[PAWN-1][tsq];
			 node -> pawncode ^= hashnumbers [PAWN - 1] [ssq] ^ hashnumbers [PAWN - 1] [tsq];
			 node->hashcode^=hashnumbers[BPAWN-1][(char)tsq-8];
			 node -> pawncode ^= hashnumbers [BPAWN - 1] [(char) tsq - 8];
			 node->matrix[(char)tsq-8]=0;
			 i = node->index[(char)tsq-8];
			 node->bpawnlist[i]=node->bpawnlist[--node->bpawns];
			 node->index[node->bpawnlist[i]]=i;
			 //node -> material += PAWN_VALUE;
			 return;
                 case _QUEEN_PROMOTION:
			 node->wpieces ++;
			 node->hashcode ^= hashnumbers [PAWN -1] [ssq] ^ hashnumbers [QUEEN - 1] [tsq];
			 node -> pawncode ^= hashnumbers [PAWN - 1] [ssq];
			 node->matrix [tsq] = QUEEN;
			 node->wpawnlist [i] = node->wpawnlist [-- node->wpawns];
			 node->index [node->wpawnlist [i]] = i;
			 node->index [ssq] = node->wqueens; //yes, Ssq! prepares node->index[tsq]=node->index[ssq]
			 node->wqueenlist [node->wqueens ++] = tsq;
			 //node -> material += QUEEN_VALUE - PAWN_VALUE;
			 break;
                 case _ROOK_PROMOTION:
		   node->wpieces++;
                  node->hashcode^=hashnumbers[PAWN-1][ssq]^hashnumbers[ROOK-1][tsq];
		  node -> pawncode ^= hashnumbers [PAWN - 1] [ssq];
                  node->matrix[tsq]=ROOK;
                  node->wpawnlist[i]=node->wpawnlist[--node->wpawns];
                  node->index[node->wpawnlist[i]]=i; 
		  node->index[ssq]=node->wrooks;
                  node->wrooklist[node->wrooks++]=tsq;
		  //node -> material += ROOK_VALUE - PAWN_VALUE;
                  break;
                 case _BISHOP_PROMOTION:
		   node->wpieces++;
                  node->hashcode^=hashnumbers[PAWN-1][ssq]^hashnumbers[BISHOP-1][tsq]; 
		  node -> pawncode ^= hashnumbers [PAWN - 1] [ssq];           
                  node->wpawnlist[i]=node->wpawnlist[--node->wpawns];
                  node->index[node->wpawnlist[i]]=i;
		  node->matrix[tsq]=BISHOP;
		  node->index[ssq]=node->wbishops;
                  node->wbishoplist[node->wbishops++]=tsq;
		  //node -> material += BISHOP_VALUE - PAWN_VALUE;
                  break;
                 case _KNIGHT_PROMOTION:
		   node->wpieces++;
                  node->hashcode^=hashnumbers[PAWN-1][ssq]^hashnumbers[KNIGHT-1][tsq];
		  node -> pawncode ^= hashnumbers [PAWN - 1] [ssq];
                  node->matrix[tsq]=KNIGHT;   
                  node->wpawnlist[i]=node->wpawnlist[--node->wpawns];
                  node->index [node->wpawnlist[i]]=i;
		  node->index [ssq] = node->wknights; 
                  node->wknightlist[node->wknights++]=tsq;
		  //node -> material += KNIGHT_VALUE - PAWN_VALUE;
                  break;
                }
                break;
  case KNIGHT:   node->fifty++;
                 node->hashcode^=hashnumbers[KNIGHT-1][ssq]^hashnumbers[KNIGHT-1][tsq];
                 node->wknightlist[i]=tsq;
                 break;
  case BISHOP:   node->fifty++;
                 node->hashcode^=hashnumbers[BISHOP-1][ssq]^hashnumbers[BISHOP-1][tsq];
                 node->wbishoplist[i]=tsq;
                 break;
  case ROOK:    node->fifty++;
                node->hashcode^=hashnumbers[ROOK-1][ssq]^hashnumbers[ROOK-1][tsq];
                node->wrooklist[i]=tsq;
                if _CASTLEW(node) {
                 if ((ssq==h1) && _SCW(node)) {
                  node->flags &= ~64;
                  HashSCW(node);
                 }
                 if ((ssq==a1) && _LCW(node)) {
                  node->flags &= ~128;
                  HashLCW(node);
                 }
                }
                 break;
  case QUEEN:    node->fifty++;
                 node->hashcode^=hashnumbers[QUEEN-1][ssq]^hashnumbers[QUEEN-1][tsq];
                 node->wqueenlist[i]=tsq;
                 break;
  case KING:     node->fifty++;
                 node->hashcode^=hashnumbers[KING-1][ssq]^hashnumbers[KING-1][tsq];
                 node->wkpos=tsq;
                 if _CASTLEW(node) {
                  if _SCW(node) HashSCW(node);
                  if _LCW(node) HashLCW(node);
                  node->flags &= _LL(4294967103);
                  if SPECIAL(move) switch SPECIALCASE(move) {
                   case _SHORT_CASTLE:
                    node->matrix[h1]=0;
                    node->matrix[f1]=ROOK;
                    node->index[f1]=node->index[h1];
                    node->wrooklist[node->index[f1]]=f1;
                    node->hashcode ^= hashnumbers[ROOK-1][h1]^hashnumbers[ROOK-1][f1];
                    return;
                   case _LONG_CASTLE:
                    node->matrix[a1]=0;
                    node->matrix[d1]=ROOK;
                    node->index[d1]=node->index[a1];
                    node->wrooklist[node->index[d1]]=d1;
                    node->hashcode ^= hashnumbers[ROOK-1][a1]^hashnumbers[ROOK-1][d1];
                    return;
                  }
                 }
 }
 if (_CAPTURE(move)) {
	 node->fifty=0;
  i = node->index[tsq];
  switch (CAPTURED(move)) {
   case PAWN:
    node->hashcode ^= hashnumbers[BPAWN-1][tsq];
    node -> pawncode ^= hashnumbers [BPAWN - 1] [tsq];
    node->bpawnlist[i]=node->bpawnlist[--node->bpawns];
    node->index[node->bpawnlist[i]]=i;
    //node -> material += PAWN_VALUE;
    break;
   case KNIGHT:
    node->hashcode ^= hashnumbers[BKNIGHT-1][tsq];
    node->bknightlist[i]=node->bknightlist[--node->bknights];
    node->index[node->bknightlist[i]]=i;
    node->bpieces--;
    //node -> material += KNIGHT_VALUE;
    break;
   case BISHOP:
    node->hashcode ^= hashnumbers[BBISHOP-1][tsq];
    node->bbishoplist[i]=node->bbishoplist[--node->bbishops];
    node->index[node->bbishoplist[i]]=i;
    node->bpieces--;
    //node -> material += BISHOP_VALUE;
    break;
   case ROOK:
    node->hashcode ^= hashnumbers[BROOK-1][tsq];
    node->brooklist[i]=node->brooklist[--node->brooks];
    node->index[node->brooklist[i]]=i;
    node->bpieces--;
    if ((tsq == h8) && _SCB(node)) {
     node->flags &= ~256;
     HashSCB(node);
    } else if ((tsq == a8) && _LCB(node)) {
     node->flags &= ~512;
     HashLCB(node);
    }
    //node -> material += ROOK_VALUE;
    break;
   case QUEEN:
    node->hashcode ^= hashnumbers[BQUEEN-1][tsq];
    node->bqueenlist[i]=node->bqueenlist[--node->bqueens];
    node->index[node->bqueenlist[i]]=i;
    node->bpieces--;
    //node -> material += QUEEN_VALUE;
  }
 }
 node->index[tsq] = node->index[ssq];
}

void _fast_dobmove(TFastNode* node, int move) {
	unsigned char ssq = SOURCESQ(move);
	unsigned char tsq = TARGETSQ(move);
	char piece = PIECE(move);
	node->flags ^= _WTM;
	node->hashcode ^= 1;
	if _EPSQ(node) {
		node->hashcode ^= ephash[_EPSQ(node)];
		node->flags &= ~63;;
	}
	unsigned char i = node->index[ssq];
	node->matrix[ssq]=0;
	node->matrix[tsq]=char(piece+KING);
	switch (piece) {
	case PAWN:
                node->fifty=0;
                if (!SPECIAL(move)) {
			node->hashcode^=hashnumbers[BPAWN-1][ssq]^hashnumbers[BPAWN-1][tsq];
			node -> pawncode ^= hashnumbers [BPAWN - 1] [ssq] ^ hashnumbers [BPAWN - 1] [tsq];
			node->bpawnlist[i]=tsq;
			if (((char) ssq - 16) == tsq) {
				node->flags |= ssq-8;
				node->hashcode ^= ephash[_EPSQ(node)];
			}
                } else switch SPECIALCASE(move) {
                 case _EN_PASSANT:
                  node->bpawnlist[i]=tsq;
                  node->matrix[(char)tsq+8]=0;
                  node->index[tsq]=node->index[ssq];
                  node->hashcode^=hashnumbers[BPAWN-1][ssq]^hashnumbers[BPAWN-1][tsq];
		  node -> pawncode ^= hashnumbers [BPAWN - 1] [ssq] ^ hashnumbers [BPAWN - 1] [tsq];
                  node->hashcode ^= hashnumbers[PAWN-1][(char)tsq+8];
		  node -> pawncode ^= hashnumbers [PAWN - 1] [(char) tsq + 8];
                  i = node->index[(char)tsq+8];
                  node->wpawnlist[i]=node->wpawnlist[--node->wpawns];
                  node->index[node->wpawnlist[i]]=i;
		  //node -> material -= PAWN_VALUE;
                  return;
                 case _QUEEN_PROMOTION:
		   node->bpieces++;
                  node->hashcode^=hashnumbers [BPAWN -1] [ssq] ^hashnumbers [BQUEEN -1] [tsq];
		  node -> pawncode ^= hashnumbers [BPAWN - 1] [ssq];
                  node->matrix[tsq]=BQUEEN;
                  node->bpawnlist[i]=node->bpawnlist[--node->bpawns];
                  node->index[node->bpawnlist[i]]=i; 
		  node->index[ssq]=node->bqueens;
                  node->bqueenlist[node->bqueens++]=tsq;
		  //node -> material -= QUEEN_VALUE - PAWN_VALUE;
                  break;
                 case _ROOK_PROMOTION:
			 node->bpieces++;
			 node->hashcode ^= hashnumbers [BPAWN - 1] [ssq] ^ hashnumbers [BROOK - 1] [tsq];
			 node -> pawncode ^= hashnumbers [BPAWN - 1] [ssq];
			 node->matrix[tsq]=BROOK;                  
			 node->bpawnlist[i]=node->bpawnlist[--node->bpawns];
			 node->index[node->bpawnlist[i]]=i; 
			 node->index[ssq]=node->brooks;
			 node->brooklist[node->brooks++]=tsq;
			 //node -> material -= ROOK_VALUE - PAWN_VALUE;
			 break;
                 case _BISHOP_PROMOTION:
		   node->bpieces++;
                  node->hashcode^=hashnumbers[BPAWN-1][ssq]^hashnumbers[BBISHOP-1][tsq];
		  node -> pawncode ^= hashnumbers [BPAWN - 1] [ssq];
                  node->matrix[tsq]=BBISHOP;
		  node->bpawnlist[i]=node->bpawnlist[--node->bpawns];
                  node->index[node->bpawnlist[i]]=i; 
		  node->index[ssq]=node->bbishops;
                  node->bbishoplist[node->bbishops++]=tsq;
		  //node -> material -= BISHOP_VALUE - PAWN_VALUE;
                  break;
                 case _KNIGHT_PROMOTION:
		   node->bpieces++;
                  node->hashcode^=hashnumbers[BPAWN-1][ssq]^hashnumbers[BKNIGHT-1][tsq];
		  node -> pawncode ^= hashnumbers [BPAWN - 1] [ssq];
                  node->matrix[tsq]=BKNIGHT;                  
                  node->bpawnlist[i]=node->bpawnlist[--node->bpawns];
                  node->index[node->bpawnlist[i]]=i;
		  node->index[ssq]=node->bknights;
                  node->bknightlist[node->bknights++]=tsq;
		  //node -> material -= KNIGHT_VALUE - PAWN_VALUE;
                  break;
                }
                break;
  case KNIGHT:   node->fifty++;
                 node->hashcode^=hashnumbers[BKNIGHT-1][ssq]^hashnumbers[BKNIGHT-1][tsq];
                 node->bknightlist[i]=tsq;
                 break;
  case BISHOP:    node->fifty++;
                 node->hashcode^=hashnumbers[BBISHOP-1][ssq]^hashnumbers[BBISHOP-1][tsq];
                 node->bbishoplist[i]=tsq;
                 break;
  case ROOK:    node->fifty++;
                node->hashcode^=hashnumbers[BROOK-1][ssq]^hashnumbers[BROOK-1][tsq];
                node->brooklist[i]=tsq;
                if _CASTLEB(node) {
                 if ((ssq==h8) && _SCB(node)) {
                  node->flags &= ~256;
                  HashSCB(node);
                 }
                 if ((ssq==a8) && _LCB(node)) {
                  node->flags &= ~512;
                  HashLCB(node);
                 }
                }
                 break;
  case QUEEN:    node->fifty++;
                 node->hashcode^=hashnumbers[BQUEEN-1][ssq]^hashnumbers[BQUEEN-1][tsq];
                 node->bqueenlist[i]=tsq;
                 break;
  case KING:     node->fifty++;
                 node->hashcode^=hashnumbers[BKING-1][ssq]^hashnumbers[BKING-1][tsq];
                node->bkpos=tsq;
                if _CASTLEB(node) {
                 if _SCB(node) HashSCB(node);
                 if _LCB(node) HashLCB(node);
                 node->flags &= ~(256|512);
                 if SPECIAL(move) switch SPECIALCASE(move) {
                  case _SHORT_CASTLE:
                   node->matrix[h8]=0;
                   node->matrix[f8]=BROOK;
                   node->index[f8]=node->index[h8];
                   node->brooklist[node->index[f8]]=f8;
                   node->hashcode ^= hashnumbers[BROOK-1][h8]^hashnumbers[BROOK-1][f8];
                   return;
                  case _LONG_CASTLE:
                   node->matrix[a8]=0;
                   node->matrix[d8]=BROOK;
                   node->index[d8]=node->index[a8];
                   node->brooklist[node->index[d8]]=d8;
                   node->hashcode ^= hashnumbers[BROOK-1][a8]^hashnumbers[BROOK-1][d8];
                   return;
                 }
                }
 }
 if (_CAPTURE(move)) {
	 node->fifty=0;
	 i = node->index[tsq];
	 switch (CAPTURED(move)) {
	 case PAWN:
		 node -> hashcode ^= hashnumbers [PAWN - 1] [tsq];
		 node -> pawncode ^= hashnumbers [PAWN - 1] [tsq];
		 node->wpawnlist[i]=node->wpawnlist[--node->wpawns];
		 node->index[node->wpawnlist[i]]=i;
		 //node -> material -= PAWN_VALUE;
		 break;
	 case KNIGHT:
		 node->wpieces--;
		 node->hashcode ^= hashnumbers [KNIGHT-1] [tsq];
		 node->wknightlist[i]=node->wknightlist[--node->wknights];
		 node->index[node->wknightlist[i]]=i;
		 //node -> material -= KNIGHT_VALUE;
		 break;
	 case BISHOP:
		 node->wpieces--;
		 node->hashcode ^= hashnumbers[BISHOP-1][tsq];
		 node->wbishoplist[i]=node->wbishoplist[--node->wbishops];
		 node->index[node->wbishoplist[i]]=i;
		 //node -> material -= BISHOP_VALUE;
		 break;
	 case ROOK:
		 node->wpieces--;
		 node->hashcode ^= hashnumbers[ROOK-1][tsq];
		 node->wrooklist[i]=node->wrooklist[--node->wrooks];
		 node->index[node->wrooklist[i]]=i;
		 if ((tsq == h1) && _SCW(node)) {
			 node->flags &= ~64;
			 HashSCW(node);
		 } else if ((tsq == a1) && _LCW(node)) {
			 node->flags &= ~128;
			 HashLCW(node);
		 }
		 //node -> material -= ROOK_VALUE;
		 break;
	 case QUEEN:
		 node->wpieces--;
		 node->hashcode ^= hashnumbers[QUEEN-1][tsq];
		 node->wqueenlist[i]=node->wqueenlist[--node->wqueens];
		 node->index[node->wqueenlist[i]]=i;
		 //node -> material -= QUEEN_VALUE;
	 }
 }
 node->index[tsq] = node->index[ssq];
}

void _fast_undowmove(TFastNode* node, int move, int oldflags, int oldfifty) {
	unsigned char ssq = SOURCESQ (move),
		tsq = TARGETSQ (move);
        char piece = PIECE(move);

	node -> flags = oldflags;
	node -> fifty = oldfifty;
	node -> hashcode ^= 1;
	if (_EPSQ (node)) { 
		node -> hashcode ^= ephash [_EPSQ (node)];
	}
	node -> matrix [ssq] = piece;
	node -> matrix [tsq] = 0;
	
	unsigned char i = node->index[tsq];
	node -> index [ssq] = i;
	switch (piece) {
  case PAWN:    if (!SPECIAL(move)) {
                 node->wpawnlist[i]=ssq;
                 node->hashcode^=hashnumbers[PAWN-1][ssq]^hashnumbers[PAWN-1][tsq];
		 node -> pawncode ^= hashnumbers [PAWN - 1] [ssq] ^ hashnumbers [PAWN - 1] [tsq];
                 if (((char) ssq + 16) == tsq) node->hashcode ^= ephash[ssq+8];
                } else switch SPECIALCASE(move) {
                 case _EN_PASSANT:
                  node->wpawnlist[i]=ssq;
                  node->index[(char)tsq-8]=node->bpawns;
                  node->hashcode^=hashnumbers[PAWN-1][ssq]^hashnumbers[PAWN-1][tsq];
		  node -> pawncode ^= hashnumbers [PAWN - 1] [ssq] ^ hashnumbers [PAWN - 1] [tsq];
                  node->hashcode ^= hashnumbers[BPAWN-1][(char)tsq-8];
		  node -> pawncode ^= hashnumbers [BPAWN - 1] [(char) tsq - 8];
                  node->bpawnlist[node->bpawns++]=char(tsq-8);
                  node->matrix[(unsigned char)(tsq-8)]=BPAWN;
		  //node -> material -= PAWN_VALUE;
                  return;
                 case _QUEEN_PROMOTION:
			 node->wpieces--;
			 node->hashcode^=hashnumbers[PAWN-1][ssq]^hashnumbers[QUEEN-1][tsq];	
			 node -> pawncode ^= hashnumbers [PAWN - 1] [ssq];
			 node->wqueenlist[i]=node->wqueenlist[--node->wqueens];
			 node->index[node->wqueenlist[i]]=i; 
			 node->index[ssq]=node->wpawns; 
			 node->wpawnlist[node->wpawns++]=ssq;
			 //node -> material -= QUEEN_VALUE - PAWN_VALUE;
			 break;
                 case _ROOK_PROMOTION:
			 node->wpieces--;
			 node->hashcode^=hashnumbers[PAWN-1][ssq]^hashnumbers[ROOK-1][tsq]; 
			 node -> pawncode ^= hashnumbers [PAWN - 1] [ssq];
			 node->wrooklist[i]=node->wrooklist[--node->wrooks];
			 node->index[node->wrooklist[i]]=i; 
			 node->index[ssq]=node->wpawns;
			 node->wpawnlist[node->wpawns++]=ssq;
			 //node -> material -= ROOK_VALUE - PAWN_VALUE;
			 break;
                 case _BISHOP_PROMOTION:
			 node->wpieces--;
			 node->hashcode^=hashnumbers[PAWN-1][ssq]^hashnumbers[BISHOP-1][tsq]; 
			 node -> pawncode ^= hashnumbers [PAWN - 1] [ssq];
			 node->wbishoplist[i]=node->wbishoplist[--node->wbishops];
			 node->index[node->wbishoplist[i]]=i; 
			 node->index[ssq]=node->wpawns;
			 node->wpawnlist[node->wpawns++]=ssq;
			 //node -> material -= BISHOP_VALUE - PAWN_VALUE;
			 break;
                 case _KNIGHT_PROMOTION:
			 node->wpieces--;
			 node->hashcode^=hashnumbers[PAWN-1][ssq]^hashnumbers[KNIGHT-1][tsq];
			 node -> pawncode ^= hashnumbers [PAWN - 1] [ssq];
			 node->wknightlist[i]=node->wknightlist[--node->wknights];
			 node->index[node->wknightlist[i]]=i; 
			 node->index[ssq]=node->wpawns;
			 node->wpawnlist[node->wpawns++]=ssq;
			 //node -> material -= KNIGHT_VALUE - PAWN_VALUE;
                }
                break;
  case KNIGHT:  node->hashcode^=hashnumbers[KNIGHT-1][ssq]^hashnumbers[KNIGHT-1][tsq];
                node->wknightlist[i]=ssq;
                break;
  case BISHOP:  node->hashcode^=hashnumbers[BISHOP-1][ssq]^hashnumbers[BISHOP-1][tsq];
                node->wbishoplist[i]=ssq;
                 break;
  case ROOK:    node->hashcode^=hashnumbers[ROOK-1][ssq]^hashnumbers[ROOK-1][tsq];
                node->wrooklist[i]=ssq;
                if _CASTLEW(node) {
                 if (ssq==h1 && _SCW(node)) {
                  HashSCW(node);
                 }
                 if (ssq==a1 && _LCW(node)) {
                  HashLCW(node);
                 }
                }
                break;
  case QUEEN:   node->hashcode^=hashnumbers[QUEEN-1][ssq]^hashnumbers[QUEEN-1][tsq];
                node->wqueenlist[i]=ssq;
                break;
  case KING:    node->hashcode^=hashnumbers[KING-1][ssq]^hashnumbers[KING-1][tsq];
                node->wkpos=ssq;
                if _CASTLEW(node) {
                 if _SCW(node) HashSCW(node);
                 if _LCW(node) HashLCW(node);
                 if SPECIAL(move) switch SPECIALCASE(move) {
                  case _SHORT_CASTLE:
                   node->matrix[h1]=ROOK;
                   node->matrix[f1]=0;
                   node->index[h1]=node->index[f1];
                   node->wrooklist[node->index[h1]]=h1;
                   node->hashcode ^= hashnumbers[ROOK-1][f1]^hashnumbers[ROOK-1][h1];
                   return;
                  case _LONG_CASTLE:
                   node->matrix[a1]=ROOK;
                   node->matrix[d1]=0;
                   node->index[a1]=node->index[d1];
                   node->wrooklist[node->index[a1]]=a1;
                   node->hashcode ^= hashnumbers[ROOK-1][d1]^hashnumbers[ROOK-1][a1];
                   return;
                 }
                }

 }
 if (_CAPTURE(move)) switch (CAPTURED(move)) {
  case PAWN:
   node->matrix[tsq]=BPAWN;
   node->hashcode ^= hashnumbers[BPAWN-1][tsq];
   node -> pawncode ^= hashnumbers [BPAWN - 1] [tsq];
   node->index[tsq]=node->bpawns;
   node->bpawnlist[node->bpawns++]=tsq;
   //node -> material -= PAWN_VALUE;
   break;
  case KNIGHT:
    node->bpieces++;
   node->matrix[tsq]=BKNIGHT;
   node->hashcode ^= hashnumbers[BKNIGHT-1][tsq];
   node->index[tsq]=node->bknights;
   node->bknightlist[node->bknights++]=tsq;
   //node -> material -= KNIGHT_VALUE;
   break;
  case BISHOP:
    node->bpieces++;
   node->matrix[tsq]=BBISHOP;
   node->hashcode ^= hashnumbers[BBISHOP-1][tsq];
   node->index[tsq]=node->bbishops;
   node->bbishoplist[node->bbishops++]=tsq;
   //node -> material -= BISHOP_VALUE;
   break;
  case ROOK:
    node->bpieces++;
   node->matrix[tsq]=BROOK;
   node->hashcode ^= hashnumbers[BROOK-1][tsq];
   node->index[tsq]=node->brooks;
   node->brooklist[node->brooks++]=tsq;
   if ((tsq == h8) && _SCB(node)) HashSCB(node);
   else if ((tsq == a8) && _LCB(node)) HashLCB(node);
   //node -> material -= ROOK_VALUE;
   break;
  case QUEEN:
    node->bpieces++;
   node->matrix[tsq]=BQUEEN;
   node->hashcode ^= hashnumbers[BQUEEN-1][tsq];
   node->index[tsq]=node->bqueens;
   node->bqueenlist[node->bqueens++]=tsq;
   //node -> material -= QUEEN_VALUE;
 }
}

void _fast_undobmove(TFastNode* node, int move, int oldflags, int oldfifty) {
	unsigned char ssq = SOURCESQ(move);
	unsigned char tsq = TARGETSQ(move);
	char piece = PIECE(move);
	node->hashcode ^= 1;
	node->flags = oldflags;
	node -> fifty = oldfifty;
	if _EPSQ(node) node->hashcode ^= ephash[_EPSQ(node)];
	node->matrix[ssq]=char(piece+KING);
	node->matrix[tsq]=0;
	unsigned char i = node->index[tsq];
	node->index[ssq]=i;
	switch (piece) {
	case PAWN:    
		if (!SPECIAL(move)) {
			node -> hashcode ^= hashnumbers [BPAWN - 1] [ssq] ^ hashnumbers [BPAWN - 1] [tsq];
			node -> pawncode ^= hashnumbers [BPAWN - 1] [ssq] ^ hashnumbers [BPAWN - 1] [tsq];
			node->bpawnlist[i]=ssq;
			if (((char) ssq - 16) == tsq) node->hashcode ^= ephash[ssq-8];
		} else switch SPECIALCASE(move) {
		case _EN_PASSANT:
			node->hashcode^=hashnumbers[BPAWN-1][ssq]^hashnumbers[BPAWN-1][tsq];
			node -> pawncode ^= hashnumbers [BPAWN - 1] [ssq] ^ hashnumbers [BPAWN - 1] [tsq];
			node->bpawnlist[i]=ssq;
			node->index[(char)tsq+8]=node->wpawns;
			node->hashcode ^= hashnumbers[PAWN-1][(char)tsq+8];
			node -> pawncode ^= hashnumbers [PAWN - 1] [(char) tsq + 8];
			node->wpawnlist[node->wpawns++]=char(tsq+8);
			node->matrix[(unsigned char)(tsq+8)]=PAWN;
			//node -> material += PAWN_VALUE;
			return;
                 case _QUEEN_PROMOTION:
			 node->bpieces--;
			 node->hashcode^=hashnumbers[BPAWN-1][ssq]^hashnumbers[BQUEEN-1][tsq]; 
			 node -> pawncode ^= hashnumbers [BPAWN - 1] [ssq];
			 node->bqueenlist[i]=node->bqueenlist[--node->bqueens];
			 node->index[node->bqueenlist[i]]=i;
			 node->index[ssq]=node->bpawns;
			 node->bpawnlist[node->bpawns++]=ssq;
			 //node -> material += QUEEN_VALUE - PAWN_VALUE;
			 break;
                 case _ROOK_PROMOTION:
			 node->bpieces--;
			 node->hashcode^=hashnumbers[BPAWN-1][ssq]^hashnumbers[BROOK-1][tsq];
			 node -> pawncode ^= hashnumbers [BPAWN - 1] [ssq];
			 node->brooklist[i]=node->brooklist[--node->brooks];
			 node->index[node->brooklist[i]]=i;
			 node->index[ssq]=node->bpawns;
			 node->bpawnlist[node->bpawns++]=ssq;
			 //node -> material += ROOK_VALUE - PAWN_VALUE;
			 break;
                 case _BISHOP_PROMOTION:
			 node->bpieces--;
			 node->hashcode^=hashnumbers[BPAWN-1][ssq]^hashnumbers[BBISHOP-1][tsq];
			 node -> pawncode ^= hashnumbers [BPAWN - 1] [ssq];
			 node->bbishoplist[i]=node->bbishoplist[--node->bbishops];
			 node->index[node->bbishoplist[i]]=i; 
			 node->index[ssq]=node->bpawns;
			 node->bpawnlist[node->bpawns++]=ssq;
			 //node -> material += BISHOP_VALUE - PAWN_VALUE;
			 break;
                 case _KNIGHT_PROMOTION:
			 node->bpieces--;
			 node->hashcode^=hashnumbers[BPAWN-1][ssq]^hashnumbers[BKNIGHT-1][tsq];  
			 node -> pawncode ^= hashnumbers [BPAWN - 1] [ssq];
			 node->bknightlist[i]=node->bknightlist[--node->bknights];
			 node->index[node->bknightlist[i]]=i; 
			 node->index[ssq]=node->bpawns;
			 node->bpawnlist[node->bpawns++]=ssq;
			 //node -> material += KNIGHT_VALUE - PAWN_VALUE;
                }
                break;
  case KNIGHT:  node->hashcode^=hashnumbers[BKNIGHT-1][ssq]^hashnumbers[BKNIGHT-1][tsq];
                node->bknightlist[i]=ssq;
                break;
  case BISHOP:  node->hashcode^=hashnumbers[BBISHOP-1][ssq]^hashnumbers[BBISHOP-1][tsq];
                node->bbishoplist[i]=ssq;
                 break;
  case ROOK:    node->hashcode^=hashnumbers[BROOK-1][ssq]^hashnumbers[BROOK-1][tsq];
                node->brooklist[i]=ssq;
                if _CASTLEB(node) {
                 if ((ssq==h8) && _SCB(node)) {
                  HashSCB(node);
                 }
                 if ((ssq==a8) && _LCB(node)) {
                  HashLCB(node);
                 }
                }
                break;
  case QUEEN:   node->hashcode^=hashnumbers[BQUEEN-1][ssq]^hashnumbers[BQUEEN-1][tsq];
                node->bqueenlist[i]=ssq;
                break;
  case KING:    node->hashcode^=hashnumbers[BKING-1][ssq]^hashnumbers[BKING-1][tsq];
                node->bkpos=ssq;
                if _CASTLEB(node) {
                 if _SCB(node) HashSCB(node);
                 if _LCB(node) HashLCB(node);
                 if SPECIAL(move) switch SPECIALCASE(move) {
                  case _SHORT_CASTLE:
                   node->matrix[h8]=BROOK;
                   node->matrix[f8]=0;
                   node->index[h8]=node->index[f8];
                   node->brooklist[node->index[h8]]=h8;
                   node->index[e8]=node->index[g8];
                   node->hashcode ^= hashnumbers[BROOK-1][h8]^hashnumbers[BROOK-1][f8];
                   return;
                  case _LONG_CASTLE:
                   node->matrix[a8]=BROOK;
                   node->matrix[d8]=0;
                   node->index[a8]=node->index[d8];
                   node->brooklist[node->index[a8]]=a8;
                   node->index[e8]=node->index[c8];
                   node->hashcode ^= hashnumbers[BROOK-1][a8]^hashnumbers[BROOK-1][d8];
                   return;
                 }
                }

 }
 if (_CAPTURE(move)) switch (CAPTURED(move)) {
  case PAWN:   
   node->matrix[tsq]=PAWN;
   node->hashcode ^= hashnumbers[PAWN-1][tsq];
   node -> pawncode ^= hashnumbers [PAWN - 1] [tsq];
   node->index[tsq]=node->wpawns;
   node->wpawnlist[node->wpawns++]=tsq;
   //node -> material += PAWN_VALUE;
   break;
  case KNIGHT:
    node->wpieces++;
   node->matrix[tsq]=KNIGHT;
   node->hashcode ^= hashnumbers[KNIGHT-1][tsq];
   node->index[tsq]=node->wknights;
   node->wknightlist[node->wknights++]=tsq;
   //node -> material += KNIGHT_VALUE;
   break;
  case BISHOP:
    node->wpieces++;
   node->matrix[tsq]=BISHOP;
   node->hashcode ^= hashnumbers[BISHOP-1][tsq];
   node->index[tsq]=node->wbishops;
   node->wbishoplist[node->wbishops++]=tsq;
   //node -> material += BISHOP_VALUE;
   break;
  case ROOK:
    node->wpieces++;
   node->matrix[tsq]=ROOK;
   node->hashcode ^= hashnumbers[ROOK-1][tsq];
   node->index[tsq]=node->wrooks;
   node->wrooklist[node->wrooks++]=tsq;
   if ((tsq == h1) && _SCW(node)) HashSCW(node);
   else if ((tsq == a1) && _LCW(node)) HashLCW(node);
   //node -> material += ROOK_VALUE;
   break;
  case QUEEN:
    node->wpieces++;
   node->matrix[tsq]=QUEEN;
   node->hashcode ^= hashnumbers[QUEEN-1][tsq];
   node->index[tsq]=node->wqueens;
   node->wqueenlist[node->wqueens++]=tsq;
   //node -> material += QUEEN_VALUE;
 }
}

int megaselect (int first, int & last) 
{
	int move,
		value,		
		bestindex = first,			       
		bestvalue = g.megasort [first];
	
	for (int i = first + 1; i < last; i ++) {				
		value = g.megasort [i];		
		if (value > bestvalue) {
			bestvalue = value;
			bestindex = i;
		}
	}	
	move = g.tmoves [bestindex]; 
	g.tmoves [bestindex] = g.tmoves [-- last]; 
	g.megasort [bestindex] = g.megasort [last];
	return move;
}

int _fast_selectmove(int first, int &last) 
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
		value = g.tsort [move & 4095],
		index = first,
		best = value;
	
	first++;
	do {
		move = g.tmoves [first];
		value = g.tsort [move & 4095];
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

bool nooccs (TFastNode* node, int ssq, int tsq)  // returns "no occupied squares between ssq and tsq"
{
	int offset = t.direction [ssq][tsq];
	while (offset) {
		ssq += offset;
		if (ssq == tsq) { 
			return true;
		}
		if (node->matrix [ssq]) { 
			return false;
		}
	}
	return true;
}
