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
#include "legality.h"
#include "hash.h"  
#include "w0.h"
#include "w0_search.h" 
#include "w0_evaluate.h" 
#include "parser.h"
#include "attack.h"
#include "minibook.h"
#include "tables.h"
#include "extensions.h"
#include "egtb_lookup.h"


void w0_sort_moves_wtm (TFastNode * node, int first, int last, int depth, int ply, int hashmove)
{
	int value,		
		move,
		val,
		tsq,
		piece,
		capval;
	for (int i = first; i < last; i ++) {				
		move = g.tmoves[i];				
		if (move == hashmove) {
			value = MATE;
		} else {
			value = g.tsort [move & 4095];
			val = piece_val [PIECE(move)];
			tsq = TARGETSQ (move);
			piece = PIECE (move);
			if (_CAPTURE (move)) {
				capval = piece_val [CAPTURED (move)];
				if (capval<val && ABB (node, tsq)) {
					value -= val;
 				} else {
					value += 10000*capval - val;
				} 
			} else {
			       	if (attacked_by_p (node, tsq)) {
					value -= 3*val;
				} else if (ABB (node, tsq)) {
					value -= val;
				}
				if (attacked_by_p (node, SOURCESQ(move))) {
					value += 3*val;
				}
			}
				 	
		}
		g.megasort[i] = value;
	}
}

int iterate (TFastNode *node) 
{
	int move, last = 0, elapsed = 0,
		value = INVALID;	

	_fast_init_iterate (node);

	if (g.stopsearch) { 
		return 0; 
	}

	if (g.checkbook) {
		move = move_from_minibook (node);
		if (move) {					
			g.rootmoves [0]. bookmove = true;
			g.rootmoves [0]. move = move;		
			g.pv [0] [0] = move;
			g.pv [0] [1] = 0;
			return 1;
		}
	}
		
	if (node->flags & _WTM) {
		node -> flags |= _ROOT_WTM;
		last = genrootmoves_w (node);	
		g.pv [0] [0] = g.rootmoves [0]. move;
		g.pv [0] [1] = 0;                   

		if (last < 2) {		
			return last;
		}		
		
		/* check if there are bookmoves */ 
		if (g.checkbook && select_bookmove(node,last)) { 			
				whisper_bookmove ();					
			return last; 
		} else { 
			g.checkbook = false; 
		}
		
		rootmove_sort (0, last);						
		value = 0;
		g.maxq = 50;
		for (int depth = 10; depth < _maxd; depth += FULL_PLY)
		{
			
			value = drive_wtm (node, value, depth, last);	       
		  	
			if (g.stopsearch /*|| MATE_VALUE (value)*/) { /* fixme */
                  break;
            }
			g.rootscore = value;
			elapsed = g.timer.elapsed();
			print_iteration (elapsed);
			g.iteration++;
			if (node->wpieces+node->bpieces>3 && elapsed * 2000 > g.stoptime) { 
				break;
			}
			rootmove_sort (1, last);
		}		
	} else { // blacks move
		node -> flags &= ~(_ROOT_WTM);
		last = genrootmoves_b (node);		
		g.pv [0] [0] = g.rootmoves [0]. move;
	        g.pv [0] [1] = 0;		

		if (last < 2) {		
			return last;
		}				

		/* check if there is a bookmove */ 
		if (g.checkbook && select_bookmove(node,last)) { 					
		  whisper_bookmove ();                       
			return last; 
		} else { 
			g.checkbook = false; 
		}
		
		rootmove_sort (0, last);					
		value = 0;
		g.maxq = 50;
		for (int depth = 10; depth < _maxd; depth += FULL_PLY)
		{						
			value = rootdrive_b (node, value, depth, last);
			if (g.stopsearch /*|| MATE_VALUE (value)*/ ) { /* fixme */
				break;
			}
			g.rootscore = value;
			elapsed = g.timer.elapsed();
			print_iteration (elapsed);			
			g.iteration++;	
			if (node->wpieces+node->bpieces>3 && elapsed * 2000 > g.stoptime) {
				break;
			}		
			rootmove_sort (1, last);
		}
	}
	return last;
}

int next_search_w (TFastNode * node, int move, int alpha, int beta, int ply, int depth, bool incheck, int truelegals, int legals)
{
	int value;
		
	depth = depth_wtm (node, ply, depth, move, incheck, truelegals);	
	g.pv [ply] [ply] = 0;
	if (legals) {
		if (depth < FULL_PLY) {
			if (! incheck) {				
				value = - quiescence_b (node, - alpha - 1, - alpha, ply, g.maxq);
				if (value > alpha && value < beta) {					
					value = - quiescence_b (node, - beta, - alpha, ply, g.maxq);
				}
			} else {	
				value = - quiescence_evade_b (node, - alpha - 1, - alpha, ply, g.maxq);
				if (value > alpha && value < beta) {
					value = - quiescence_evade_b (node, - beta, - alpha, ply, g.maxq);
				}
			}
		} else {
			if (! incheck) {
				value = - pvs_b (node, - alpha - 1, - alpha, ply, depth);
				if (value > alpha && value < beta) {
					value = - pvs_b (node, - beta, - alpha, ply, depth);
				}
			} else {				
				value = - pvs_evade_b (node, - alpha - 1, - alpha, ply, depth);
				if (value > alpha && value < beta) {
					value = - pvs_evade_b (node, - beta, - alpha, ply, depth);
				}
			}
		}
	} else {
		if (depth < FULL_PLY) {
			if (! incheck) {				
				value = - quiescence_b (node, - beta, - alpha, ply, g.maxq);
			} else {								
				value = - quiescence_evade_b (node, - beta, - alpha, ply, g.maxq);	
			}
		} else {
			if (! incheck) {				
				value = - pvs_b (node, - beta, - alpha, ply, depth);
			} else {							
				value = - pvs_evade_b (node, - beta, - alpha, ply, depth);	
			}		
		}
	}
	return value;
}

int MTDf_wtm (TFastNode * node, int value, int depth, int last)
{
	int 	lowerbound = -MATE, 
		upperbound = MATE,
		beta;
	do { 
		beta = (value==lowerbound) ? value+1 : value;
		value = pvs_root_w (node, beta-1, beta, depth, last);
		if (g.stopsearch) {
			return INVALID;
		}
		if (value<beta) {
			upperbound = value;
		} else {
			lowerbound = value;
		}	  
	} while (lowerbound < upperbound);
	return value;
}

int aspiration_wtm (TFastNode * node, int value, int depth, int last) 
{
	const int windows [3] = {window1, window2, MATE};
	int faillows = 0,
		failhighs = 0;
	int alpha, beta;
	alpha = value - windows [faillows];
	beta = value + windows [failhighs];	
	do {				
		value = pvs_root_w (node, alpha, beta, depth, last);
		if (g.stopsearch) {
			return INVALID;
		}
		if (value > alpha && value < beta) {
			return value;
		}		
		if (value <= alpha) {					       
		    // g_print ("< ");			
		    add_time ();		
		    alpha = value - windows [++faillows];
		} else {		       
		    // g_print ("> ");						
		    add_time ();			
		    beta = value + windows [++failhighs];
		}					
	} while (true);
	return INVALID;	
}

int drive_wtm (TFastNode * node, int value, int depth, int last)
{
	//return MTDf_wtm (node, value, depth, last);
	return aspiration_wtm (node, value, depth, last);
}

int pvs_root_w (TFastNode* node, int alpha, int beta, int depth, int last) 
{	 
	int best,		
		move,
		flags,
		fifty,
		nodes = g.fastnodes,
		value;	

#ifdef PRINT_SEARCH	
	g_print ("\nw0_pvs_root_w (a = %d b = %d depth = %d, nodes = %d)\n", alpha, beta, depth / FULL_PLY, g.fastnodes);
#endif



#ifndef PRINT_SEARCH 
	//g_print ("%d ", depth / FULL_PLY);
#endif	

        // search first sucsessor with open window (-beta, -alpha)
	
	move = g.rootmoves [0]. move;
	
	flags = node -> flags;
	fifty = node -> fifty;
	
	g.path [0] = move;	
	
	if (g.rootmoves [0]. draw) {
		g.pv[1][1]=0; 
		best = g.drawscore_wtm;
	} else if (g.rootmoves[0]. avoid) {
		g.pv[1][1]=0;
		best = avoidscore[MIN(3,g.rootmoves[0].avoid)];
	} else { // search for the score 
	
		_fast_dowmove (node, move);
		
		best = next_search_w (node, move, alpha, beta, 1, depth, checkcheck_w (node, move), 0, 0);
		
		_fast_undowmove (node, move, flags, fifty);		
		
		if (g.stopsearch) {
			return INVALID;
		}
	}
	
	g.rootmoves [0]. value = best;
	g.rootmoves [0]. nodes += g.fastnodes - nodes;
	g.rootmoves [0]. unknown = IS_UNKNOWN (best);
	
	if (MATED_VALUE (best)) {
		g.rootmoves [0]. matevalue = best;
	}
					
	root_new_best (node, 0, best);
	
	if (best > alpha) {		
		alpha = best;				
	}
		
	

	// search remaining sucsessors with closed window (-alpha-1, -alpha). 
	// research if value outside window.
	
	for (int i = 1; i < last; i++) {
		move = g.rootmoves [i]. move;
		g.path [0] = move;
		if (g.rootmoves [i]. draw) {
			g.pv[1][1]=0; 
			value = g.drawscore_wtm;
		} else if (g.rootmoves[i].avoid) {
			g.pv[1][1]=0;
			value = avoidscore[MIN(3,g.rootmoves[i].avoid)];
		} else { //search 
			
			nodes = g.fastnodes;
			
			_fast_dowmove (node, move);		


			value = next_search_w (node, move, alpha, beta, 1, depth, checkcheck_w (node, move), 0, 1);
				
			_fast_undowmove (node, move, flags, fifty);			
		

			
			if (g.stopsearch) { 
				return INVALID;
			}
		
	
			g.rootmoves [i]. value = value;
			g.rootmoves [i]. nodes += g.fastnodes - nodes;
			g.rootmoves [i]. unknown = IS_UNKNOWN (value);
			
			if (MATED_VALUE (value)) {
				g.rootmoves [i]. matevalue = value;
			}			
		} 
		
		
				
		if (value > best) {
			root_new_best (node, i, value); 
			if (value > alpha) { 				
				alpha = value;
			}
			best = value;									
		}
	}
	
	return best;  
}

int pvs_w (TFastNode* node, int alpha, int beta, int ply, int depth) 
{
	int best = - MATE,
		bestmove = 0, 
		move = 0,
		flags = node->flags,
		fifty = node -> fifty,
		value,
		hashmove,
		hflags,
		nodes,
		last,
		legals = 0,  		
		first = ply << 7;	 
       		
	g.fastnodes++;

	print_search_entry (node, _PS_PVS_W, ply, depth);

	//egtb lookup
	if (try_egtb (node, ply)) {
		value = EGTB_Lookup (node, ply);
		if (value != INVALID) {
			return value;
		}
	}

	if (ply >= _W0MAXPLY) {
		g.pv[ply][ply]=0;
		print_search (node, alpha, beta, 0, ply, beta, _PS_MAXPLY, _PS_PVS_W);
		return beta;		
	}
			
#ifdef DEBUG_INSPECT
	g_assert (depth >= 0);
	int error = _fast_inspectnode (node);
	if (error || attacked_by_pnbrqk (node, node->wkpos) || attacked_by_PNBRQK (node, node -> bkpos)) {
		g_print ("error %d detected in w0_pvs_w \n", error);
		print_path (ply);
		g_error ("DEBUG_INSEPECT error\n"); 	       
		return INVALID;
	}
#endif				

	if (trivial_draw_w (node, ply)) {
		g.pv[ply][ply]=0;
		print_search (node, alpha, beta, 0, ply, 0, _PS_TRIVIAL, _PS_PVS_W);
		return g.drawscore_wtm;
	}	
	
	if (Q_mates_k (node)) {		
		return MATE - ply - 1;
	}

	// hash.	
	if (!inhash (node)) {
		hashmove = 0;
	} else if (hashhit (node, depth, ply, value, hashmove, alpha, beta)) {				
		print_search (node, alpha, beta, hashmove, ply, value, _PS_HASH, _PS_PVS_W);
		return value;		
	}
	// nullmove
	g.nullmate [ply] = false;
	if (node->wpieces && g.path[ply-1]) {
		g.path [ply] = 0;	
		donullmove (node);
		//bepaal nullmovediepte
		int depth0;
		if ((depth <= 60) || ((depth <= 80) && (node->wpieces<2) && (node->bpieces<2))) {
			depth0 = depth-20;
		} else {
			depth0 = depth-30;
		}
		if (depth0<0) {
			depth0 = 0;
		}
		value = next_search_w (node, 0, alpha, beta, ply+1, depth0, 0, 0, 0);
		if (g.stopsearch) {
			undonullmove (node, flags, fifty);
			print_search (node, alpha, beta, 0, ply, INVALID, _PS_TIME, _PS_PVS_W);
			return INVALID;
		}
		undonullmove (node, flags, fifty);
		if (value >= beta) {
			return value;
		}
		if (MATED_VALUE (value)) {
			g.nullmate[ply] = true;
		}
	}
	
			
	nodes = g.fastnodes;
	hflags = upper_bound;
	
	g.reptable [g.repindex + ply] = node -> hashcode;
	
	// iid :
	if ((!hashmove) && depth>=30) {
		pvs_w (node, alpha, beta, ply+1, FMAX (0, depth-30));		
		if (inhash (node)) {
			hashhit (node, depth, ply, value, hashmove, alpha, beta);				
		}
	}
			        								
	// try moves from move generation	
	last = _fast_genmovesw (node, first);					
#ifdef DEBUG_INSPECT
	int debuglegals = count_legals_w (node, first, last);
#endif		
	w0_sort_moves_wtm (node, first, last, depth, ply, hashmove);	
	while (first < last) { 			
		move = megaselect (first, last);
		if (!legal_move_w (node, move)) {			
			continue;
		}		
		_fast_dowmove (node, move);		
		g.path [ply] = move; 				
		value = next_search_w (node, move, alpha, beta, ply + 1, depth, checkcheck_w (node, move), 0, legals);
		_fast_undowmove (node, move, flags, fifty);		
		if (g.stopsearch) { 
			print_search (node, alpha, beta, 0, ply, value, _PS_TIME, _PS_PVS_W);	
			return INVALID;
		}
		legals++;       	
		if (value > best) {
			if (value > alpha) {
				if (value >= beta) {
					store_cutoff (move, ply, depth);
					hash_store (node, depth, ply, value, move, g.fastnodes - nodes, lower_bound);
					print_search (node, alpha, beta, move, ply, value, _PS_CUT, _PS_PVS_W);
					return value;
				}
				alpha = value;
				hflags = exact;
				update_pv (node,move, ply);
			}
			best = value;
			bestmove = move;
		}				
	}

#ifdef DEBUG_INSPECT
	if (debuglegals != legals) {		 
		g_print ("hash = %d, nodes = %d, debuglegals = %d, legals = %d, hashmove = %d\n", (int) node -> hashcode,  g.fastnodes, debuglegals, legals, hashmove);
		print_path (ply);
		g_assert_not_reached ();
	}
#endif


	
	/* no legal moves --> stalemate */ 
	if (! legals) { 
		g.pv [ply] [ply] = 0; 
		print_search (node, alpha, beta, 0, ply, 0, _PS_TRIVIAL, _PS_PVS_W);
		return g.drawscore_wtm; 
	}
	
	hash_store (node, depth, ply, best, bestmove, g.fastnodes - nodes, hflags);	
	
	print_search (node, alpha, beta, bestmove, ply, best, _PS_NORMAL, _PS_PVS_W);
	return best; 
}

int pvs_evade_w (TFastNode* node, int alpha, int beta, int ply, int depth) 
{
	int best = - MATE,
		bestmove = 0,
		hflags,
		hashmove,
		nodes,
		move,
		flags = node -> flags,
		fifty = node -> fifty,
		value,
		last,		
		legals = 0,  
		first;	
#ifdef DEBUG_INSPECT
	int error = _fast_inspectnode (node);
	if (error ||  !attacked_by_pnbrqk (node, node->wkpos) || attacked_by_PNBRQK (node, node -> bkpos)) {
		g_print ("error %d detected in w0_pvs_evade_w\n", error);
		print_path (ply);
		g_error ("DEBUG_INSPECT error\n"); 	 
		return INVALID;	       
	}
#endif			
	g.fastnodes++;	
	print_search_entry (node, _PS_PVS_E_W, ply, depth);			

	//egtb lookup
	if (try_egtb (node, ply)) {
		value = EGTB_Lookup (node, ply);
		if (value != INVALID) {
			return value;
		}
	}


	if (ply >= _W0MAXPLY) {
		g.pv [ply] [ply] = 0;
		print_search (node, alpha, beta, 0, ply, beta, _PS_MAXPLY, _PS_PVS_E_W);		
		return beta;
	}					
	if (trivial_draw_w (node, ply)) {
		g.pv[ply][ply]=0;
		print_search (node, alpha, beta, 0, ply, 0, _PS_TRIVIAL, _PS_PVS_E_W);
	        return g.drawscore_wtm;
	}	
	if (!inhash (node)) {
		hashmove = 0;
	} else if (hashhit (node, depth, ply, value, hashmove, alpha, beta)) {
		
		print_search (node, alpha, beta, hashmove, ply, value, _PS_HASH, _PS_PVS_E_W);
		return value;		
	}
 	g.nullmate [ply] = false;	
	nodes = g.fastnodes;
	hflags = upper_bound;
	g.reptable [g.repindex + ply] = node -> hashcode;		

	// try moves from generation	
	
	first = ply << 7;  
	last = _fast_genmovesw (node, first);
#ifdef DEBUG_INSPECT
	int debuglegals = count_legals_w (node, first, last);
#endif	
	init_evasions_w (node, first, last, ply << 3, hashmove); // note: deze proc. verwijdert illegale zetten ...	
	while (first < last) {
		move = megaselect (first, last);
		_fast_dowmove (node, move);
		g.path [ply] = move;
		value = next_search_w (node, move, alpha, beta, ply + 1, depth, checkcheck_w (node, move), last-first, legals);
		_fast_undowmove (node, move, flags, fifty);		
		if (g.stopsearch) { 
			print_search (node, alpha, beta, 0, ply, value, _PS_TIME, _PS_PVS_E_W);
			return INVALID;
		}
		
		legals++;
		if (value > best) {
			if (value > alpha) {
				if (value >= beta) {
					store_cutoff (move, ply, depth);
					hash_store (node, depth, ply, value, move, g.fastnodes - nodes, lower_bound);
					print_search (node, alpha, beta, move, ply, value, _PS_CUT, _PS_PVS_E_W);
					return value;
				}
				hflags = exact;
				alpha = value;
				update_pv (node,move, ply);
			}
			bestmove = move;
			best = value;
		}
	}
#ifdef DEBUG_INSPECT
	g_assert (debuglegals == legals);
#endif
	if (! legals) { // MATE
		g.pv [ply] [ply] = 0; 
		print_search (node, alpha, beta, 0, ply, - MATE + ply, _PS_TRIVIAL, _PS_PVS_E_W);
		return - MATE + ply; 
	}	
	hash_store (node, depth, ply, best, bestmove, g.fastnodes - nodes, hflags);
	print_search (node, alpha, beta, hashmove, ply, best, _PS_NORMAL, _PS_PVS_E_W);
	return best; 
}














