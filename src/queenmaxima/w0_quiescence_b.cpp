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
#include "fast.h"
#include "legality.h"  
#include "w0.h"
#include "w0_search.h" 
#include "w0_evaluate.h" 
#include "attack.h"
#include "egtb_lookup.h"

void sort_qmoves_btm (TFastNode * node, int first, int last, int ply)
{
	int value, 
		move;
	for (int i = first; i < last; i ++) {
		move = g.tmoves [i];
		value = 0;
		if (_CAPTURE (move)) {
			value += piece_values [CAPTURED (move)] - piece_val [PIECE (move)];
		}		
		g.megasort [i] = value;
	}
}

int next_q_b (TFastNode * node, int alpha, int beta, int ply, int depth, bool incheck, int legals)
{
	int value;
	
	ply += 1;
	depth -= 1;
       
	if (legals) {		
		if (! incheck) {
			value = - quiescence_w (node, - alpha - 1, - alpha, ply, depth);
			if (value > alpha && value < beta) {
				value = - quiescence_w (node, - beta, - alpha, ply, depth);
			}
		} else {
			value = - quiescence_evade_w (node, - alpha - 1, - alpha, ply, depth);
			if (value > alpha && value < beta) {
				value = - quiescence_evade_w (node, - beta, - alpha, ply, depth);
			}			
		}			
	} else {		
		if (! incheck) {				
			value = - quiescence_w (node, - beta, - alpha, ply, depth);
		} else {			       		
			value = - quiescence_evade_w (node, - beta, - alpha, ply, depth);	
		}
	}
	return value;
}

bool q_move_b (TFastNode * node, int move, int ply)
{
	int captured, pcval, capval;				 
									
	if ((! SPECIAL(move)) && _CAPTURE (move)) {		
		captured = CAPTURED (move);
		pcval = piece_val [PIECE (move)];
		capval = piece_val [captured];		
		if (pcval <=  capval && pcval>1) {
			return true;
		}
		if (minisearch_b (node, SOURCESQ (move), TARGETSQ (move)) > 0) {
			return true;
		}
		return false;
	} else if (SPECIAL (move)) {
		switch (SPECIALCASE (move)) {
		case _QUEEN_PROMOTION:
			return true;
		case _KNIGHT_PROMOTION:
			return t.pieceattacks [KNIGHT] [TARGETSQ (move)] [node -> wkpos];			
		case _EN_PASSANT:
			return true;
		default:
			return false;
		}
		
	}		
	return false;
}



int quiescence_b (TFastNode * node, int alpha, int beta, int ply, int depth) 
{
	int first,
		last,
		legals,
		value,
		best,		
		move,
		bestmove = 0,		
		flags,
		fifty;		
	
#ifdef DEBUG_INSPECT	
	int error = _fast_inspectnode (node);
	if (error || attacked_by_pnbrqk (node, node->wkpos) || attacked_by_PNBRQK (node, node->bkpos)) {
		g_print ("error %d detected in w0_quiescence_b\n", error);
		print_path (ply);	
		g_error ("DEBUG_INSPECT error\n"); 
		return INVALID;
	}
#endif
	
	g.fastnodes ++;	

	print_search_entry (node, _PS_Q_B, ply, depth);

	if (try_egtb (node, ply)) { //egtb lookup
		value = EGTB_Lookup (node, ply);
		if (value != INVALID) {
			return value;
		}
	}



	if (ply > g.maxply) {
		g.maxply = ply;
		memcpy (g.maxpath, g.path, ply * 4);
	}	

	if (ply >= _W0MAXPLY) {
		print_search (node, alpha, beta, 0, ply, beta, _PS_MAXPLY, _PS_Q_B);
		return beta;
	}

	if (trivial_draw_b (node, ply)) {
		print_search (node, alpha, beta, 0, ply, 0, _PS_TIME, _PS_Q_B);
		return g.drawscore_btm;
	}



	if (q_mates_K (node)) {		
		return MATE - ply - 1;
	}

	best = black_score (node, ply, alpha, beta);
	
	if (depth < 0) {		
		print_search (node, alpha, beta, 0, ply, best, _PS_MAXPLY, _PS_Q_B);
		return best;
	}


			
	if (best > alpha) { 
		if (best >= beta) {
			print_search (node, alpha, beta, 0, ply, best, _PS_CUT, _PS_Q_B);
			return best;
		}
		alpha = best;
	}
	


	
	g.reptable [g.repindex + ply] = node -> hashcode;
	
	legals = 0;
	flags = node -> flags;
	fifty = node -> fifty;
			
	first = ply << 7;
	last = genq_b (node, first);	
	sort_qmoves_btm (node, first, last, ply);	
	while (first < last) {
		move = megaselect (first, last);
		if (! legal_move_b (node, move)) {			
			continue;
		}		
		if (! q_move_b (node, move, ply)) {			
			continue;
		}
		_fast_dobmove (node, move);		
		legals ++;
		
		g.path [ply] = move;
		value = next_q_b (node, alpha, beta, ply, depth, checkcheck_b (node, move), legals);
		
		_fast_undobmove (node, move, flags, fifty);
		if (g.stopsearch) { 
			print_search (node, alpha, beta, 0, ply, INVALID, _PS_TIME, _PS_Q_B);
			return INVALID;
		}		
		if (value > best) {			
			if (value > alpha) {
				if (value >= beta) {					
					print_search (node, alpha, beta, move, ply, value, _PS_CUT, _PS_Q_B);
					return value;
				}				
				alpha = value;
			}
			bestmove = move;
			best = value;
		}		
	}	
	print_search (node, alpha, beta, bestmove, ply, best, _PS_NORMAL, _PS_Q_B);
	return best;		
}

int quiescence_evade_b (TFastNode * node, int alpha, int beta, int ply, int depth) 
{
	int first,
		last,
		legals,
		value,
		best,		
		move,
		bestmove,		
		flags,
		fifty; 

#ifdef DEBUG_INSPECT	
	int error = _fast_inspectnode (node);
	if (error ||  !attacked_by_PNBRQK (node, node->bkpos) || attacked_by_pnbrqk (node, node->wkpos)) {
		g_print ("error %d detected in w0_quiescence_evade_b\n", error);
		print_path (ply);	
		g_error ("DEBUG INSPECT error\n"); 
		return INVALID;
	}
#endif
	g.fastnodes ++;	

	print_search_entry (node, _PS_Q_E_B, ply, depth);

	if (try_egtb (node, ply)) { //egtb lookup
		value = EGTB_Lookup (node, ply);
		if (value != INVALID) {
			return value;
		}
	}

	if (ply > g.maxply) {
		g.maxply = ply;
		memcpy (g.maxpath, g.path, ply * 4);
	}		

	if (ply >= _W0MAXPLY) {
		print_search (node, alpha, beta, 0, ply, beta, _PS_MAXPLY, _PS_Q_E_B);
		return beta;
	}	

	if (trivial_draw_b (node, ply)) {
		print_search (node, alpha, beta, 0, ply, 0, _PS_TRIVIAL, _PS_Q_E_B);
		return g.drawscore_btm;
	}	
						
	g.reptable [g.repindex + ply] = node -> hashcode;
	legals = 0;
	flags = node -> flags;
	fifty = node -> fifty;
	best = - INFINITY;		
	first = ply << 7;		
	last = _fast_genmovesb (node, first);

#ifdef DEBUG_INSPECT
	int debuglegals = count_legals_b (node, first, last);
#endif	

	init_evasions_b (node, first, last, ply << 3, 0); 
				
	while (first < last) {
		move = megaselect (first, last);	
		_fast_dobmove (node, move);		
		legals ++;		
		g.path [ply] = move;

		value = next_q_b (node, alpha, beta, ply, depth, checkcheck_b (node, move), legals);
		
		_fast_undobmove (node, move, flags, fifty);
		if (g.stopsearch) { 
			print_search (node, alpha, beta, 0, ply, INVALID, _PS_TIME, _PS_Q_E_B);
			return INVALID;
		}		
		if (value > best) {			
			if (value > alpha) {
				if (value >= beta) {
					print_search (node, alpha, beta, move, ply, value, _PS_CUT, _PS_Q_E_B);
					return value;
				}				
				alpha = value;
			}
			bestmove = move;
			best = value;
		}
	}

#ifdef DEBUG_INSPECT
	g_assert (legals == debuglegals);
#endif

	if (! legals) {		
		print_search (node, alpha, beta, 0, ply, - MATE + ply, _PS_TRIVIAL, _PS_Q_E_B);
		return - MATE + ply;		
	}	

	print_search (node, alpha, beta, bestmove, ply, best, _PS_NORMAL, _PS_Q_E_B);
	return best;		
}














