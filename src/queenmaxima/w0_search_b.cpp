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
#include "attack.h"
#include "extensions.h"
#include "egtb_lookup.h"

void sort_moves_btm (TFastNode * node, int first, int last, int ply, int hashmove)
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
                                if (capval<val && ABW (node, tsq)) {
                                        value -= val;
                                } else {
                                        value += 10000*capval - val;
                                }
                        } else if (value < 50) {
                                if (attacked_by_P (node, tsq)) {
                                        value -= 3*val;
                                } else if (ABW (node, tsq)) {
					value -= val;
				}
				if (attacked_by_P (node, SOURCESQ (move))) {
					value += 3*val;
				}
                        }

                }
                g.megasort[i] = value;
        }
}

int next_search_b (TFastNode * node, int move, int alpha, int beta, int ply, int depth, bool incheck, int truelegals, int legals)
{
	int value;
	
	depth = depth_btm (node, ply, depth, move, incheck, truelegals); 	
	g.pv [ply] [ply] = 0;
	if (legals) {
		if (depth < FULL_PLY) {						
			if (! incheck) {				
				value = - quiescence_w (node, -alpha-1, -alpha, ply, g.maxq);
				if (value > alpha && value < beta) {
					value = - quiescence_w (node, -beta, -alpha, ply, g.maxq);
				}
			} else {
				value = - quiescence_evade_w (node, -alpha-1, -alpha, ply, g.maxq);
				if (value > alpha && value < beta) {
					value = - quiescence_evade_w (node, -beta, -alpha, ply, g.maxq);
				}
			}
		} else {
			if (! incheck) {
				value = - pvs_w (node, - alpha - 1, - alpha, ply, depth);
				if (value > alpha && value < beta) {
					value = - pvs_w (node, - beta, - alpha, ply, depth);
				}
			} else {			
				value = - pvs_evade_w (node, - alpha - 1, - alpha, ply, depth);
				if (value > alpha && value < beta) {
					value = - pvs_evade_w (node, - beta, - alpha, ply, depth);
				}					
			}
		}
	} else {
		if (depth < FULL_PLY) {
			
			
			if (! incheck) {				
				value = - quiescence_w (node, - beta, - alpha, ply, g.maxq);
			} else {									
				value = - quiescence_evade_w (node, - beta, - alpha, ply, g.maxq);	
			}
		} else {
			if (! incheck) {				
				value = - pvs_w (node, - beta, - alpha, ply, depth);
			} else {							
				value = - pvs_evade_w (node, - beta, - alpha, ply, depth);	
			}		
		}
	}
	return value;
}


int rootdrive_b (TFastNode * node, int value, int depth, int last) 
{
	const int windows [3] = {window1, window2, MATE};
	int faillows = 0,
		failhighs = 0;
	int alpha = value - windows [faillows],
		beta = value + windows [failhighs];	
	
	do {				
		value = pvs_root_b (node, alpha, beta, depth, last);
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

int pvs_root_b (TFastNode* node, int alpha, int beta, int depth, int last) 
{	 
	int best,		
		move, 
		flags,
		fifty,
		nodes = g.fastnodes,
		value;	

#ifdef PRINT_SEARCH 		
	g_print ("\nw0_pvs_root_b (depth = %d, nodes = %d)\n", depth / FULL_PLY, g.fastnodes);
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
		best = g.drawscore_btm;
	} else if (g.rootmoves[0].avoid) {
		g.pv[1][1]=0;
		best = avoidscore [MIN (3, g.rootmoves[0].avoid)];
	} else { 	
		_fast_dobmove (node, move);
		
		best = next_search_b (node, move, alpha, beta, 1, depth, checkcheck_b (node, move), 0,  0);
		
		_fast_undobmove (node, move, flags, fifty);
		
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
		
	

	// search remaining sucsessors with closed window (-alpha - 1, -alpha). 
	// research if value outside window.
	
	for (int i = 1; i < last; i++) {
		
		move = g.rootmoves [i]. move;
	
		g.path [0] = move;
				
		if (g.rootmoves [i]. draw) {
			g.pv[1][1]=0; 
			value = g.drawscore_btm;
		} else if (g.rootmoves[i].avoid) {
			g.pv[1][1]=0;
			value = avoidscore[MIN(3, g.rootmoves[i].avoid)];
		} else { /* if (! g.rootmoves [i]. matevalue) */ 
			
			nodes = g.fastnodes;
			
			_fast_dobmove (node, move);			
			
			value = next_search_b (node, move, alpha, beta, 1, depth, checkcheck_b (node, move), 0, 1);
										
			_fast_undobmove (node, move, flags, fifty);
						
			if (g.stopsearch) { 
				return INVALID;
			}
			
			g.rootmoves [i]. value = value;
			g.rootmoves [i]. nodes += g.fastnodes - nodes;
			g.rootmoves [i]. unknown = IS_UNKNOWN (value);

			if (MATED_VALUE (value)) {
				g.rootmoves [i]. matevalue = value;
			}
												
		} /*  else {
			value = g.rootmoves [i]. matevalue;
			} */ 
		
		
						
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

int pvs_b (TFastNode* node, int alpha, int beta, int ply, int depth) 
{
	int best = - MATE,
		bestmove = 0,
		move = 0, 
		flags = node -> flags,
		fifty = node -> fifty,
		value,
		hashmove,
		hflags,
		nodes,
		last,		
		legals = 0,  		
		first = ply << 7;	

	


	
	g.fastnodes++;	

	print_search_entry (node, _PS_PVS_B, ply, depth);
	
	//egtb lookup
	if (try_egtb (node, ply)) {
		value = EGTB_Lookup (node, ply);
		if (value != INVALID) {
			return value;
		}
	}


	if (ply >= _W0MAXPLY) {
		g.pv[ply][ply]=0; 
		print_search (node, alpha, beta, move, ply, beta, _PS_MAXPLY, _PS_PVS_B);	
		return beta;	
	}
		

#ifdef DEBUG_INSPECT
	g_assert (depth >= 0);
	int error = _fast_inspectnode (node);
	if (error || attacked_by_pnbrqk (node, node->wkpos) || attacked_by_PNBRQK (node, node -> bkpos)) {
		g_print ("error %d detected in w0_pvs_b \n", error);
		// g_print (g.path (ply));
		g_error ("DEBUG_INSPECT error\n"); 	
		return INVALID;
	}
#endif				
	
	


	if (trivial_draw_b (node, ply)) {
		g.pv[ply][ply]=0; 
		print_search (node, alpha, beta, 0, ply, 0, _PS_TRIVIAL, _PS_PVS_B);
		return g.drawscore_btm ;
	}
      
	if (q_mates_K (node)) {		
		return MATE - ply - 1;
	}
										
        // hash.
	if (!inhash (node)) {
		hashmove = 0;
	} else if (hashhit (node, depth, ply, value, hashmove, alpha, beta)) {
		
		print_search (node, alpha, beta, hashmove, ply, value, _PS_HASH, _PS_PVS_B);
		return value;		
	}	

	// nullmove
	g.nullmate[ply] = false;		
	if (node->bpieces && g.path[ply-1]) {
		g.path [ply] = 0;		
		donullmove (node);	
		int depth0;
		if ((depth <= 60) || ((depth <= 80) && (node->wpieces<2) && (node->bpieces<2))) {
			depth0 = depth-20;
		} else {
			depth0 = depth-30;
		}
		if (depth0<0) {
			depth0 = 0;
		}
		value = next_search_b (node, 0, alpha, beta, ply+1, depth0, 0, 0, 0);
		undonullmove (node, flags, fifty);
		if (g.stopsearch) {
			print_search (node, alpha, beta, 0, ply, INVALID, _PS_TIME, _PS_PVS_B);
			return INVALID;
		}
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
		pvs_b (node, alpha, beta, ply+1, FMAX (0, depth-30));
		if (inhash (node)) {			
			hashhit (node, depth, ply, value, hashmove, alpha, beta);			
		}
	}
       	       						
	// try moves from move generation	
	
	last = _fast_genmovesb (node, first);
					
	#ifdef DEBUG_INSPECT
	int debuglegals = count_legals_b (node, first, last);
	#endif
		
	sort_moves_btm (node, first, last, ply, hashmove);
	
	while (first < last) { 
		move = megaselect (first, last);
					
		if (!legal_move_b (node, move)) {			
			continue;
		}
					
		_fast_dobmove (node, move);
					
		g.path [ply] = move; 

		value = next_search_b (node, move, alpha, beta, ply + 1, depth, checkcheck_b (node, move), 0, legals);
	
		_fast_undobmove (node, move, flags, fifty);
					
		if (g.stopsearch) { 
			print_search (node, alpha, beta, 0, ply, value, _PS_TIME, _PS_PVS_B);	
			return INVALID;
		}
		
		
		legals++;
		
		if (value > best) {
			if (value > alpha) {
				if (value >= beta) {
					store_cutoff (move, ply, depth);
					hash_store (node, depth, ply, value, move, g.fastnodes - nodes, lower_bound);
					print_search (node, alpha, beta, move, ply, value, _PS_CUT, _PS_PVS_B);
				        return value;
				}
				alpha = value;
				hflags = exact;
				update_pv (node, move, ply);
			}
			best = value;
			bestmove = move;
		}
				
	}
	
	/* no legal moves --> stalemate */ 
	if (!legals) { 
		g.pv[ply][ply]=0; 
		print_search (node, alpha, beta, 0, ply, 0, _PS_TRIVIAL, _PS_PVS_B);
		return g.drawscore_btm; 
	}
	
	hash_store (node, depth, ply, best, bestmove, g.fastnodes - nodes, hflags);	
	
	print_search (node, alpha, beta, bestmove, ply, best, _PS_NORMAL, _PS_PVS_B);
#ifdef DEBUG_INSPECT
	if (debuglegals != legals) {		 
		g_print ("hash = %d, nodes = %d, debuglegals = %d, legals = %d, hashmove = %d\n", (int) node -> hashcode,  g.fastnodes, debuglegals, legals, hashmove);
		print_path (ply);
		g_assert_not_reached ();
	}
#endif
	return best; 
}

int pvs_evade_b (TFastNode* node, int alpha, int beta, int ply, int depth) 
{
	int best = - MATE,
		bestmove = 0,
		hflags,
		hashmove,
		nodes,
		move = 0,
		flags = node->flags,
		fifty = node -> fifty,
		value,
		last,
		legals = 0,  
		first;	
#ifdef DEBUG_INSPECT
	int error = _fast_inspectnode (node);
	if (error ||  !attacked_by_PNBRQK (node, node->bkpos) || attacked_by_pnbrqk (node, node -> wkpos)) {
		g_print ("error %d detected in w0_pvs_evade_b\n", error);
		print_path (ply);
		g_error ("DEBUG_INSPECT error\n"); 	       
		return INVALID;	       
	}
#endif		
	g.fastnodes++;	
	print_search_entry (node, _PS_PVS_E_B, ply, depth);	

	//egtb lookup
	if (try_egtb (node, ply)) {
		value = EGTB_Lookup (node, ply);
		if (value != INVALID) {
			return value;
		}
	}


	if (ply >= _W0MAXPLY) {
		print_search (node, alpha, beta, 0, ply, beta, _PS_MAXPLY, _PS_PVS_E_B);
		g.pv[ply][ply]=0;
		return beta;
       }
	if (trivial_draw_b (node, ply)) {
		g.pv[ply][ply]=0;		
		print_search (node, alpha, beta, 0, ply, 0,_PS_TRIVIAL, _PS_PVS_E_B);
		return g.drawscore_btm;
	}

	// hash 					
	if (!inhash (node)) {
		hashmove = 0;
	} else if (hashhit (node, depth, ply, value, hashmove, alpha, beta)) {		
		print_search (node, alpha, beta, hashmove, ply, value, _PS_HASH, _PS_PVS_E_B);
		return value;		
	}
	
	g.nullmate [ply] = false;
	nodes = g.fastnodes;
	hflags = upper_bound;
	g.reptable [g.repindex + ply] = node -> hashcode;
							
	// try moves from generation
	
	first = ply << 7;  
	last = _fast_genmovesb (node, first);
#ifdef DEBUG_INSPECT
	int debuglegals = count_legals_b (node, first, last);
#endif	
	init_evasions_b (node, first, last, ply << 3, hashmove); // note: deze proc. verwijdert illegale zetten ...
	
	while (first < last) {
		move = megaselect (first, last);				
		_fast_dobmove (node, move);
		g.path [ply] = move; 
		value = next_search_b (node, move, alpha, beta, ply + 1, depth, checkcheck_b (node, move), last-first, legals);
		_fast_undobmove (node, move, flags, fifty);
		if (g.stopsearch) { 
			return INVALID;
		}
		
		legals++;
		if (value > best) {
			if (value > alpha) {
				if (value >= beta) {
					store_cutoff (move, ply, depth);
					hash_store (node, depth, ply, value, move, g.fastnodes - nodes, lower_bound);
					print_search (node, alpha, beta, move, ply, value, _PS_CUT, _PS_PVS_E_B);
					return value;
				}
				hflags = exact;
				alpha = value;
				update_pv (node, move, ply);
			}
			bestmove = move;
			best = value;
		}				
	}
#ifdef DEBUG_INSPECT
	g_assert (debuglegals == legals);
#endif	
	if (!legals) {  
		g.pv[ply][ply] = 0; /* terminator */ 
		print_search (node, alpha, beta, 0, ply, - MATE + ply, _PS_TRIVIAL, _PS_PVS_E_B);
		return - MATE + ply; 
	}			
	hash_store (node, depth, ply, best, bestmove, g.fastnodes - nodes, hflags);
	print_search (node, alpha, beta, bestmove, ply, best, _PS_NORMAL, _PS_PVS_E_B);
	return best; 
}



