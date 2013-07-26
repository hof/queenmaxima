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

void sort_qmoves_wtm (TFastNode * node, int first, int last, int ply)
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

int next_q_w (TFastNode * node, int alpha, int beta, int ply, int depth, bool incheck, int legals)
{
	int value;
	
	ply += 1;
	depth -= 1;
       
	if (legals) {		
		if (! incheck) {
			value = - quiescence_b (node, - alpha - 1, - alpha, ply, depth);

			if (value > alpha && value < beta) {
				value = - quiescence_b (node, - beta, - alpha, ply, depth);
			}
		} else {
			value = - quiescence_evade_b (node, - alpha - 1, - alpha, ply, depth);
			if (value > alpha && value < beta) {
				value = - quiescence_evade_b (node, - beta, - alpha, ply, depth);
			}			
		}			
	} else {		
		if (! incheck) {				
			value = - quiescence_b (node, - beta, - alpha, ply, depth);

		} else {			       		
			value = - quiescence_evade_b (node, - beta, - alpha, ply, depth);	
		}
	}
	return value;
}

bool q_move_w (TFastNode * node, int move, int ply)
{	
	int captured, pcval, capval;	 
									
	if ((! SPECIAL(move)) && _CAPTURE (move)) {
		captured = CAPTURED (move);
		capval = piece_val [captured];
		pcval = piece_val [PIECE(move)];			
		if (pcval <=  capval && capval>1) {
			return true;
		}
		if (minisearch_w (node, SOURCESQ (move), TARGETSQ (move)) > 0) {
			return true;
		}
		return false;
	} else if (SPECIAL (move)) {
		switch (SPECIALCASE (move)) {
		case _QUEEN_PROMOTION:
			return true;
		case _KNIGHT_PROMOTION:
			return t.pieceattacks [KNIGHT] [TARGETSQ (move)] [node -> bkpos];			
		case _EN_PASSANT:
			return true;
		default:
			return false;
		}
		
	}	
	return false;
}

int quiescence_w (TFastNode * node, int alpha, int beta, int ply, int depth) 
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
		g_print ("error %d detected in w0_quiescence_w\n", error);
		print_path (ply);
		g_error("DEBUG_INSPECT error\n"); 
		return INVALID;
	}
#endif
	g.fastnodes ++;
	
	print_search_entry (node, _PS_Q_W, ply, depth);

	//egtb lookup
	if (try_egtb (node, ply)) {
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
		print_search (node, alpha, beta, 0, ply, beta, _PS_MAXPLY, _PS_Q_W);
		return beta;
	}

	if (trivial_draw_w (node, ply)) {
		print_search (node, alpha, beta, 0, ply, 0, _PS_TRIVIAL, _PS_Q_W);	     
		return g.drawscore_wtm;
	}
	
	if (Q_mates_k (node)) {		
		return MATE - ply - 1;
	}

	best = white_score (node, ply, alpha, beta);
			
	if (depth < 0) {
		print_search (node, alpha, beta, 0, ply, best, _PS_MAXPLY, _PS_Q_W);		
		return best;
	}
			
	if (best > alpha) { 
		if (best >= beta) {
			print_search (node, alpha, beta, 0, ply, best, _PS_CUT, _PS_Q_W);
			return best;
		}
		alpha = best;
	}
	
	g.reptable [g.repindex + ply] = node -> hashcode;
		
	legals = 0;
	flags = node -> flags;
	fifty = node -> fifty;					
	first = ply << 7;
	last = genq_w (node, first);	
	sort_qmoves_wtm (node, first, last, ply);	
	while (first < last) {	
		move = megaselect (first, last);
		if (! legal_move_w (node, move)) {			
			continue;
		}
		if (! q_move_w (node, move, ply)) {			
			continue;
		}
		_fast_dowmove (node, move);		
		legals ++;		
		g.path [ply] = move;		
		value = next_q_w (node, alpha, beta, ply, depth, checkcheck_w (node, move), legals);		
		_fast_undowmove (node, move, flags, fifty);
		if (g.stopsearch) { 
			print_search (node, alpha, beta, 0, ply, INVALID, _PS_TIME, _PS_Q_W);
			return INVALID;
		}		
		if (value > best) {			
			if (value > alpha) {
				if (value >= beta) {
					print_search (node, alpha, beta, move, ply, value, _PS_CUT, _PS_Q_W);
					return value;
				}

				alpha = value;
			}
			bestmove = move;
			best = value;
		}		
	}		
	print_search (node, alpha, beta, bestmove, ply, best, _PS_NORMAL, _PS_Q_W);
	return best;	
}

int quiescence_evade_w (TFastNode * node, int alpha, int beta, int ply, int depth) 
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
	if (error ||  !attacked_by_pnbrqk (node, node->wkpos) || attacked_by_PNBRQK (node, node->bkpos)) {
		g_print ("error %d detected in w0_quiescence_evade_w\n", error);
		print_path (ply);
		g_error ("DEBUG_INSPECT error\n"); 	
		return INVALID;
	}
#endif
	g.fastnodes ++;
	print_search_entry (node, _PS_Q_E_W, ply, depth);	

	//egtb lookup
	if (try_egtb (node, ply)) {
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
		print_search (node, alpha, beta, 0, ply, beta, _PS_MAXPLY, _PS_Q_E_W);
		return beta;
	}
	if (trivial_draw_w (node, ply)) {
		print_search (node, alpha, beta, 0, ply, 0, _PS_TRIVIAL, _PS_Q_E_W);
		return g.drawscore_wtm;
	}
	g.reptable [g.repindex + ply] = node -> hashcode;
	legals = 0;
	flags = node -> flags;
	fifty = node -> fifty;
	best = - CHESS_INF;
	first = ply << 7;
	last = _fast_genmovesw (node, first);
#ifdef DEBUG_INSPECT
	int debuglegals = count_legals_w (node, first, last);
#endif	
	init_evasions_w (node, first, last, ply << 3, 0);
	while (first < last) {
		move = megaselect (first, last);		
		_fast_dowmove (node, move);		
		legals ++;		
		g.path [ply] = move;
		value = next_q_w (node, alpha, beta, ply, depth, checkcheck_w (node, move), legals);
		_fast_undowmove (node, move, flags, fifty);
		if (g.stopsearch) { 
			print_search (node, alpha, beta, 0, ply, INVALID, _PS_TIME, _PS_Q_E_W);
			return INVALID;
		}		
		if (value > best) {			
			if (value > alpha) {
				if (value >= beta) {
					print_search (node, alpha, beta, move, ply, value, _PS_CUT, _PS_Q_E_W);
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
		print_search (node, alpha, beta, 0, ply, MATE + ply, _PS_TRIVIAL, _PS_Q_E_W);
		return - MATE + ply;				
	}       
	print_search (node, alpha, beta, bestmove, ply, value, _PS_NORMAL, _PS_Q_E_W);
	return best;	
}












