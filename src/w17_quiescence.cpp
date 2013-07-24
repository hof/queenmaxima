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
#include "hash.h" 
#include "legality.h"
#include "w17_quiescence.h"
#include "w17.h"  
#include "attack.h"

int w17_unknown_w (TFastNode * node)
{
	if (node -> flags & _ROOT_WTM) {
		return UNKNOWN;
	}
	return - UNKNOWN;
}

int w17_unknown_b (TFastNode * node)
{
	if (node -> flags & _ROOT_WTM) {
		return - UNKNOWN;
	}
	return UNKNOWN;
}

bool w17_quiescence_check_move_w (TFastNode* node, int move, int ply) 
{	
	/* looszet? */ 
	if (attacked_by_pnbrqk (node, TARGETSQ (move)) && ! (attacked_by_PNBRQK (node, TARGETSQ (move)))) {
		return true;
	}
	/* of hebben we meerdere caps. */ 
	int first = ply << 7,
		last = _fast_gencapsw (node, first);
	return last > first + 1;
	
}

bool w17_quiescence_check_move_b (TFastNode* node, int move, int ply) 
{		
	if (attacked_by_PNBRQK (node, TARGETSQ (move)) && ! (attacked_by_pnbrqk (node, TARGETSQ (move)))) {
		return true;
	}
	int first = ply << 7,
		last = _fast_gencapsb (node, first);
	return last > first + 1;	
}

int w17_qd_w (TFastNode * node, int alpha, int beta, int ply) 
{
	return 0;
}

int w17_qd_b (TFastNode * node, int alpha, int beta, int ply) 
{
	return 0;
}

int w17_quiescence_w (TFastNode* node, int alpha, int beta, int ply, int depth) 
{
	int first,
		last,
		legals,
		value,
		best,		
		move,
		flags,
		fifty = node -> fifty,
		hashmove,
		nodes,
		hflags;	
	
#ifdef DEBUG_INSPECT	
	int error = _fast_inspectnode (node);
	if (error || attacked_by_PNBRQK (node, node -> bkpos) || attacked_by_pnbrqk (node, node->wkpos)) {
		g_print ("error %d detected in w17_quiescence_w \n", error);
		print_path (ply);
		g_error ("DEBUG_INSPECT error\n"); 	 
		return INVALID;
	}	
#endif	
	
	g.fastnodes ++;	
        g.pv[ply][ply] = 0;

	if (ply > g.maxply) {
		g.maxply = ply;
		memcpy (g.maxpath, g.path, ply * 4);
	}	

	if (ply >= _W17MAXPLY) {
		print_search (node, alpha, beta, 0, ply, beta, _PS_MAXPLY, _PS_Q_W);
                return beta;
	}
	
	if (! (node -> wpawns) && ! (node -> wpieces)) {
		print_search (node, alpha, beta, 0, ply, beta, _PS_TRIVIAL, _PS_Q_W);
		return MATE - ply;
	}

	if (w17_trivial_draw_w (node, ply)) {
		print_search (node, alpha, beta, 0, ply, 0, _PS_TRIVIAL, _PS_Q_W);
		return 0;
	}
	
	if (depth <= 0) {
		print_search (node, alpha, beta, 0, ply, UNKNOWN, _PS_UNKNOWN, _PS_Q_W);
		return w17_unknown_w (node);
	}

	// hash.
	if (!inhash (node)) {
		hashmove = 0;
	} else if (hashhit (node, 0, ply, value, hashmove, alpha, beta)) {
		print_search (node, alpha, beta, hashmove, ply, value, _PS_HASH, _PS_Q_W);
		return value;				
	}
	
	nodes = g.fastnodes;
	hflags = upper_bound;
	
	legals = 0;
	flags = node -> flags;
	best = - INFINITY;
	first = ply << 7;
	
	g.reptable [g.repindex + ply] = node -> hashcode;

	if (! hashmove || _CAPTURE (hashmove)) {
		
		// captures	
		if (hashmove) {
			move = hashmove;
			legals ++;
			g.path [ply] = move;                        

			_fast_dowmove (node, move);

			if (! checkcheck_w (node, move)) {
				value = - w17_quiescence_b (node, - beta, - alpha, ply + 1, depth - 1);
			} else {
				value = - w17_quiescence_evade_b (node, - beta, - alpha, ply + 1, depth - 1);
			}

			_fast_undowmove (node, move, flags, fifty);

			if (g.stopsearch) { 
				print_search (node, alpha, beta, 0, ply, INVALID, _PS_TIME, _PS_Q_W); 
				return INVALID;
			}		

			if (value > best) {			
				if (value > alpha) {
					if (value >= beta) {
						sort_cap_w [move & 4095] ++;
						hash_store (node, 0, ply, value, move, g.fastnodes - nodes, lower_bound);
						print_search (node, alpha, beta, move, ply, value, _PS_CUT, _PS_Q_W); 
						return value;
					}
					hflags = exact;
					alpha = value;

					/* qpv_update */
                                        update_pv(node, move, ply);
				}				
				best = value;
			}
		}

		last = _fast_gencapsw (node, first);

		if (hashmove) {
			for (int i = first; i < last; i ++) {
				if (g.tmoves [i] == hashmove) {
					g.tmoves [i] = g.tmoves [-- last];
					break;
				}
			}
		}		

		while (last > first)  {	
			move = w17_select_capture_w (node, first, last);

			if (! legal_move_w (node, move)) {			
				continue;
			}		
			legals ++;		
			g.path [ply] = move;
			_fast_dowmove (node, move);				
			
			if (!checkcheck_w (node, move)) {
				value = - w17_quiescence_b (node, - beta, - alpha, ply + 1, depth - 1);
			} else {						
				value = - w17_quiescence_evade_b (node, - beta, - alpha, ply + 1, depth - 1);
			}

			_fast_undowmove (node, move, flags, fifty);
			
			if (g.stopsearch) { 
				print_search (node, alpha, beta, 0, ply, INVALID, _PS_CUT, _PS_Q_W); 
				return INVALID;
			}		
			
			if (value > best) {						
				if (value > alpha) {
					if (value >= beta) {						
					        sort_cap_w [move & 4095] ++;
						hash_store (node, 0, ply, value, move, g.fastnodes - nodes, lower_bound);
						print_search (node, alpha, beta, move, ply, value, _PS_CUT, _PS_Q_W); 
						return value;
					}
					hflags = exact;
					alpha = value;

					/* qpv_update */
                                        update_pv(node, move, ply); 
				}
				hashmove = move;
				best = value;
			}		
		}				
		
		if (legals) {
			hash_store (node, 0, ply, best, hashmove, g.fastnodes - nodes, hflags);
			print_search (node, alpha, beta, hashmove, ply, best, _PS_NORMAL, _PS_Q_W); 
			return best;
		}
	}
	
	// de positie is stil voor wit (er zijn geen gedwongen zetten)
	
	// int eval_state = eval_state(node); 
	value = w17_evaluate_w (node, ply, w17_eval_state(node));
	
	if (flags & B_ONLY_MATE_SEARCH) {
		return value;
	}

	if (value > alpha) { 
		if (value >= beta) {
			print_search (node, alpha, beta, 0, ply, value, _PS_CUT, _PS_Q_W);
			return value;
		}
		alpha = value;
	}
	
	best = alpha;
				
	node -> flags |= W_ONLY_MATE_SEARCH;
	flags = node -> flags;
		
	while (hashmove) {
		move = hashmove;
		legals ++;		

		g.path [ply] = move;

		_fast_dowmove (node, move);		

		if (! checkcheck_w (node, move)) {			
			value = - w17_quiescence_b (node, - beta, - alpha, ply + 1, depth - 1);
		} else {
			if (!w17_quiescence_check_move_w (node, move, ply + 1)) {
				_fast_undowmove (node, move, flags, fifty);
				break;
			}
			value = - w17_quiescence_evade_b (node, - beta, - alpha, ply + 1, depth - 1); 
		}
		_fast_undowmove (node, move, flags, fifty);
		
		if (g.stopsearch) {
			print_search (node, alpha, beta, 0, ply, INVALID, _PS_TIME, _PS_Q_W); 
			return INVALID;
		}
		
		if (value > best) {
			if (value > alpha) {
				if (value >= beta) {
					sort_w [move & 4095] ++;
					hash_store (node, 0, ply, value, move, g.fastnodes - nodes, lower_bound);
					print_search (node, alpha, beta, move, ply, value, _PS_CUT, _PS_Q_W); 
					return value;
				}
				hflags = exact;
				alpha = value;

				/* qpv_update */
                                update_pv(node, move, ply); 
			}			
			best = value;
		}
		break;
	}

	last = _fast_gennoncapsw (node, first);
	
	if (hashmove) {
		for (int i = first; i < last; i ++) {
			if (g.tmoves [i] == hashmove) {
				g.tmoves [i] = g.tmoves [-- last];
				break;
			}
		}
	}

	while (last > first) {		
		move = w17_select_noncapture_w (node, first, last, w17_eval_state(node));

		g_assert (move != hashmove);

		if (! legal_move_w (node, move)) {
			continue;
		}	
	
		legals ++;

		if (!hashmove) {
			hashmove = move;
		}

		g.path [ply] = move;

		_fast_dowmove (node, move);		

		if (! checkcheck_w (node, move)) {			
			value = - w17_quiescence_b (node, - beta, - alpha, ply + 1, depth - 1);
		} else {
			if (!w17_quiescence_check_move_w (node, move, ply + 1)) {
				_fast_undowmove (node, move, flags, fifty);
				continue;
			}
			value = - w17_quiescence_evade_b (node, - beta, - alpha, ply + 1, depth - 1); 
		}
		_fast_undowmove (node, move, flags, fifty);
		
		if (g.stopsearch) {
			print_search (node, alpha, beta, 0, ply, INVALID, _PS_TIME, _PS_Q_W); 
			return INVALID;
		}

		if (value > best) {
			if (value > alpha) {
				if (value >= beta) {
					sort_w [move & 4095] ++;
					hash_store (node, 0, ply, value, move, g.fastnodes - nodes, lower_bound);
					print_search (node, alpha, beta, move, ply, value, _PS_CUT, _PS_Q_W); 
					return value;
				}
				hflags = exact;
				alpha = value;

				/* qpv_update */
                                update_pv(node, move, ply); 
			}
			hashmove = move;
			best = value;
		}
	}
	
	if (! legals) {
		print_search (node, alpha, beta, 0, ply, MATE-ply, _PS_TRIVIAL, _PS_Q_W); 
		return MATE - ply;
	}

	hash_store (node, 0, ply, best, hashmove, g.fastnodes - nodes, hflags);
	print_search (node, alpha, beta, hashmove, ply, best, _PS_NORMAL, _PS_Q_W);  
	return best;
} 

int w17_quiescence_b (TFastNode* node, int alpha, int beta, int ply, int depth) {
	int first,
		last,
		legals,
		value,
		best,		
		move,
		hashmove,
		hflags,		
		nodes,
		flags,
		fifty = node -> fifty;
	
#ifdef DEBUG_INSPECT	
	int error = _fast_inspectnode (node);
	if (error || attacked_by_PNBRQK (node, node -> bkpos) || attacked_by_pnbrqk (node, node->wkpos)) {
		g_print ("error %d detected in w17_quiescence_b \n", error);
		print_path (ply);
		g_error ("DEBUG_INSPECT error\n"); 
		return INVALID;
	}	
#endif	
	
	g.fastnodes ++;	
        g.pv[ply][ply] = 0;

	if (ply > g.maxply) {
		g.maxply = ply;
		memcpy (g.maxpath, g.path, ply * 4);
	}
	
	if (ply >= 50) {
		print_search (node, alpha, beta, 0, ply, beta, _PS_MAXPLY, _PS_Q_B); 
                return beta;
	}
	
	if (! (node -> bpawns) && ! (node -> bpieces)) {
		print_search (node, alpha, beta, 0, ply, beta, _PS_TRIVIAL, _PS_Q_B); 
                return MATE - ply;
	}

	if (w17_trivial_draw_b (node, ply)) {
		print_search (node, alpha, beta, 0, ply, 0, _PS_TRIVIAL, _PS_Q_B);
		return 0;
	}

	if (depth <= 0) {
		print_search (node, alpha, beta, 0, ply, UNKNOWN, _PS_UNKNOWN, _PS_Q_B);
		return w17_unknown_b (node);
	}
	
	// hash.
	if (! inhash (node)) {
		hashmove = 0;
	} else if (hashhit (node, 0, ply, value, hashmove, alpha, beta)) {
		print_search (node, alpha, beta, hashmove, ply, value, _PS_HASH, _PS_Q_B);
		return value;		
	}
		
	nodes = g.fastnodes;
	hflags = upper_bound;
	
	legals = 0;
	flags = node -> flags;
	best = - INFINITY;
	first = ply << 7;

	g.reptable [g.repindex + ply] = node -> hashcode;

	if (! hashmove || _CAPTURE (hashmove)) {

		// captures		
		if (hashmove) {
			move = hashmove;
			legals ++;
			g.path [ply] = move;

			_fast_dobmove (node, move);
			if (! checkcheck_b (node, move)) {
				value = - w17_quiescence_w (node, - beta, - alpha, ply + 1, depth - 1);
			} else {
				value = - w17_quiescence_evade_w (node, - beta, - alpha, ply + 1, depth - 1);
			}

			_fast_undobmove (node, move, flags, fifty);

			if (g.stopsearch) { 
				print_search (node, alpha, beta, 0, ply, INVALID, _PS_TIME, _PS_Q_B);
				return INVALID;
			}		

			if (value > best) {			
				if (value > alpha) {
					if (value >= beta) {
						sort_cap_b [move & 4095] ++;
						hash_store (node, 0, ply, value, move, g.fastnodes - nodes, lower_bound);
						print_search (node, alpha, beta, move, ply, value, _PS_CUT, _PS_Q_B);
						return value;
					}
					hflags = exact;
					alpha = value;
					
                                        /* qpv update */
                                        update_pv(node, move, ply);
				}				
				best = value;
			}
		}

		last = _fast_gencapsb (node, first);

		if (hashmove) { 
			for (int i = first; i < last; i ++) {
				if (g.tmoves [i] == hashmove) {
					g.tmoves [i] = g.tmoves [-- last];
					break;
				}
			}
		}

		while (last > first)  {	
			move = w17_select_capture_b (node, first, last);	

			if (! legal_move_b (node, move)) {
				continue;
			}
			legals ++;
			g.path [ply] = move;
			_fast_dobmove (node, move);

			if (! checkcheck_b (node, move)) {
				value = - w17_quiescence_w (node, - beta, - alpha, ply + 1, depth - 1);
			} else {
				value = - w17_quiescence_evade_w (node, - beta, - alpha, ply + 1, depth - 1);
			}

			_fast_undobmove (node, move, flags, fifty);
			
			if (g.stopsearch) { 
				print_search (node, alpha, beta, 0, ply, INVALID, _PS_CUT, _PS_Q_B); 
				return INVALID;
			}
		
			if (value > best) {			
				if (value > alpha) {
					if (value >= beta) {
						sort_cap_b [move & 4095] ++;
						hash_store (node, 0, ply, value, move, g.fastnodes - nodes, lower_bound);
						print_search (node, alpha, beta, move, ply, value, _PS_CUT, _PS_Q_B);			
						return value;
					}
					hflags = exact;
					alpha = value;
					
                                        /* qpv_update */
                                        update_pv(node, move, ply);
				}
				hashmove = move;
				best = value;
			}
		}		
		
		if (legals) {
			hash_store (node, 0, ply, best, hashmove, g.fastnodes - nodes, hflags);
			print_search (node, alpha, beta, hashmove, ply, best, _PS_NORMAL, _PS_Q_B);
			return best;
		}
	}
	

	// de positie is stil voor wit (er zijn geen gedwongen zetten)

	value = w17_evaluate_b (node, ply, w17_eval_state(node));
	
	if (flags & W_ONLY_MATE_SEARCH) {
		return value;
	}

	if (value > alpha) { 
		if (value >= beta) {
			print_search (node, alpha, beta, 0, ply, value, _PS_CUT, _PS_Q_B);
			return value;
		}
		alpha = value;
	}
	
	best = alpha;
			
	node -> flags |= B_ONLY_MATE_SEARCH;
	flags = node -> flags;
			
	while (hashmove) {
		move = hashmove;
		legals ++;		

		g.path [ply] = move;

		_fast_dobmove (node, move);
		
		if (! checkcheck_b (node, move)) {			
			value = - w17_quiescence_w (node, - beta, - alpha, ply + 1, depth - 1);
		} else {
			if (!w17_quiescence_check_move_b (node, move, ply + 1)) {
				_fast_undobmove (node, move, flags, fifty);
				break;
			}
			value = - w17_quiescence_evade_w (node, - beta, - alpha, ply + 1, depth - 1); 
		}
		_fast_undobmove (node, move, flags, fifty);

		if (g.stopsearch) { 
			print_search (node, alpha, beta, 0, ply, INVALID, _PS_TIME, _PS_Q_B); 
			return INVALID;
		}
		
		if (value > best) {			
			if (value > alpha) {
				if (value >= beta) {
					sort_b [move & 4095] ++; 
					hash_store (node, 0, ply, value, move, g.fastnodes - nodes, lower_bound);
					print_search (node, alpha, beta, move, ply, value, _PS_CUT, _PS_Q_B); 
					return value;
				}
				hflags = exact;
				alpha = value;

				/* qpv_update */
                                update_pv(node, move, ply);
			}
			best = value;			
		}
		break;
	}

	last = _fast_gennoncapsb (node, first);

	if (hashmove) {
		for (int i = first; i < last; i ++) {
			if (g.tmoves [i] == hashmove) {
				g.tmoves [i] = g.tmoves [-- last];
				break;
			}
		}
	}

	
	while (last > first) {
		move = w17_select_noncapture_b (node, first, last, w17_eval_state(node));

		g_assert (move != hashmove); 

		if (! legal_move_b (node, move)) {
			continue;
		}		

		legals ++;

		if (! hashmove) {
			hashmove = move;
		}	

		g.path [ply] = move;

		_fast_dobmove (node, move);
		
		if (! checkcheck_b (node, move)) {			
			value = - w17_quiescence_w (node, - beta, - alpha, ply + 1, depth - 1);
		} else {
			if (!w17_quiescence_check_move_b (node, move, ply + 1)) {
				_fast_undobmove (node, move, flags, fifty);
				continue;
			}
			value = - w17_quiescence_evade_w (node, - beta, - alpha, ply + 1, depth - 1); 
		}
		_fast_undobmove (node, move, flags, fifty);

		if (g.stopsearch) {
			print_search (node, alpha, beta, 0, ply, INVALID, _PS_TIME, _PS_Q_B);
			return INVALID;
		}		

		if (value > best) {			
			if (value > alpha) {
				if (value >= beta) {
					sort_b [move & 4095] ++; 
					hash_store (node, 0, ply, value, move, g.fastnodes - nodes, lower_bound);
					print_search (node, alpha, beta, move, ply, value, _PS_CUT, _PS_Q_B);
					return value;
				}
				hflags = exact;
				alpha = value;

				/* qpv_update */
                                update_pv(node, move, ply); 
			}
			best = value;
			hashmove = move;
		}
		
	}
	
	if (! legals) {
		print_search (node, alpha, beta, 0, ply, MATE-ply, _PS_TRIVIAL, _PS_Q_B);
		return MATE - ply;
	}

	hash_store (node, 0, ply, best, hashmove, g.fastnodes - nodes, hflags);
	print_search (node, alpha, beta, hashmove, ply, best, _PS_NORMAL, _PS_Q_B);
	return best;
} 

int w17_quiescence_evade_w (TFastNode* node, int alpha, int beta, int ply, int depth) {
	int first,
		last,
		legals,
		value,
		best,		
		move,
		hashmove,
		hflags,
		nodes,
		flags,
		fifty = node -> fifty;

#ifdef DEBUG_INSPECT	
	int error = _fast_inspectnode (node);
	if (error ||  !attacked_by_pnbrqk (node, node->wkpos) || attacked_by_PNBRQK (node, node->bkpos)) {
		g_print ("error %d detected in w17_quiescence_evade_w\n", error);
		print_path (ply);
		g_error ("DEBUG_INSPECT error\n"); 
		return INVALID;
	}
#endif

	g.fastnodes ++;	
        g.pv[ply][ply] = 0;

	if (ply > g.maxply) {
		g.maxply = ply;
		memcpy (g.maxpath, g.path, ply * 4);
	}
		
	if (! (node -> wpawns) && ! (node -> wpieces)) {
		print_search (node, alpha, beta, 0, ply, MATE-ply, _PS_TRIVIAL, _PS_Q_E_W);
		return MATE - ply;
	}

	if (w17_trivial_draw_w (node, ply)) {
		print_search (node, alpha, beta, 0, ply, 0, _PS_TRIVIAL, _PS_Q_E_W);
                return 0;
	}

	if (depth <= 0) {
		print_search (node, alpha, beta, 0, ply, UNKNOWN, _PS_UNKNOWN, _PS_Q_E_W); 
                return w17_unknown_w (node);
	}

	if (ply >= _W17MAXPLY) {
		print_search (node, alpha, beta, 0, ply, beta, _PS_MAXPLY, _PS_Q_E_W);
                return beta;
	}

	// hash.
	if (! inhash (node)) {
		hashmove = 0;
	} else if (hashhit (node, 0, ply, value, hashmove, alpha, beta)) {
		print_search (node, alpha, beta, hashmove, ply, value, _PS_HASH, _PS_Q_E_W);
                return value;
	}
		
	nodes = g.fastnodes;
	hflags = upper_bound;

	legals = 0;
	flags = node -> flags;
	best = - INFINITY;
	first = ply << 7;

	g.reptable [g.repindex + ply] = node -> hashcode;
		
	// captures
	last = _fast_gencapsw (node, first);

	if ((! hashmove) || _CAPTURE (hashmove)) {
		 
		if (hashmove) {
			for (int i = first + 1; i < last; i ++) {
				if (g.tmoves [i] == hashmove) {
					g.tmoves [i] = g.tmoves [first];
					g.tmoves [first] = hashmove;					
					break;
				}
			}
		}

		while (last > first)  {
									
			move = w17_select_capture_w (node, first, last);	
			
			if (! legal_move_w (node, move)) {
				continue;
			}
		
			_fast_dowmove (node, move);
			if (attacked_by_pnbrqk (node, node -> wkpos)) {
				_fast_undowmove (node, move, flags, fifty);
				continue;
			}
			legals ++;

			g.path [ply] = move;

			if (! checkcheck_w (node, move)) {
				value = - w17_quiescence_b (node, - beta, - alpha, ply + 1, depth - 1);
			} else {
				value = - w17_quiescence_evade_b (node, - beta, - alpha, ply + 1, depth - 1);
			}
			
			_fast_undowmove (node, move, flags, fifty);

			if (g.stopsearch) { 
				print_search (node, alpha, beta, 0, ply, INVALID, _PS_TIME, _PS_Q_E_W);
				return INVALID;
			}		
			if (value > best) {			
				if (value > alpha) {
					if (value >= beta) {
						hash_store (node, 0, ply, value, move, g.fastnodes - nodes, lower_bound);
						print_search (node, alpha, beta, move, ply, value, _PS_CUT, _PS_Q_E_W);
						return value;
					}
					hflags = exact;
					alpha = value;
                                        
					/* qpv_update */
                                        update_pv(node, move, ply);
				}
				hashmove = move;
				best = value;
			}
		}
	}
					
	if (legals) {
		hash_store (node, 0, ply, best, hashmove, g.fastnodes - nodes, hflags);	
		print_search (node, alpha, beta, hashmove, ply, best, _PS_NORMAL, _PS_Q_E_W); 
		return best;
	}
			
	last = _fast_gennoncapsw (node, first);

	if (hashmove) {
		for (int i = first + 1; i < last; i ++) {
			if (g.tmoves [i] == hashmove) {
				g.tmoves [i] = g.tmoves [first];
				g.tmoves [first] = hashmove;					
				break;
			}
		}
	}
	
	for (int i = first; i < last; i ++) {
		move = g.tmoves [i];
		if (! legal_move_w (node, move)) {
			continue;
		}				
		_fast_dowmove (node, move);

		if (attacked_by_pnbrqk (node, node -> wkpos)) {
			_fast_undowmove (node, move, flags, fifty);
			continue;
		}
		legals ++;

		g.path [ply] = move;

		if (! checkcheck_w (node, move)) {			
			value = - w17_quiescence_b (node, - beta, - alpha, ply + 1, depth - 1);
		} else {
			value = - w17_quiescence_evade_b (node, - beta, - alpha, ply + 1, depth - 1); 
		}

		_fast_undowmove (node, move, flags, fifty);

		if (g.stopsearch) { 
			print_search (node, alpha, beta, 0, ply, INVALID, _PS_TIME, _PS_Q_E_W);
			return INVALID;
		}		
		if (value > best) {			
			if (value > alpha) {
				if (value >= beta) {
					hash_store (node, 0, ply, value, move, g.fastnodes - nodes, lower_bound);
					print_search (node, alpha, beta, move, ply, value, _PS_CUT, _PS_Q_E_W);
					return value;
				}
				hflags = exact;
				alpha = value;

				/* qpv_update */
                                update_pv(node, move, ply);
			}
			hashmove = move;
			best = value;
		}
	}
	
	if (! legals) {
		print_search (node, alpha, beta, 0, ply, MATE-ply, _PS_TRIVIAL, _PS_Q_E_W);
		return MATE - ply;
	}

	hash_store (node, 0, ply, best, hashmove, g.fastnodes - nodes, hflags);
	print_search (node, alpha, beta, hashmove, ply, best, _PS_NORMAL, _PS_Q_E_W);
        return best;
} 

int w17_quiescence_evade_b (TFastNode* node, int alpha, int beta, int ply, int depth) {
	int first,
		last,		
		value,
		best,
		legals,
		move,
		hashmove,
		hflags,
		nodes,
		flags,
		fifty = node -> fifty;

#ifdef DEBUG_INSPECT	
	int error = _fast_inspectnode (node);
	if (error ||  !attacked_by_PNBRQK (node, node->bkpos) || attacked_by_pnbrqk (node, node->wkpos)) {
		g_print ("error %d detected in w17_quiecence_evade_b\n", error);
		print_path (ply);
		g_error ("DEBUG_INSPECT error\n");
		return INVALID;
	}
#endif

	g.fastnodes ++;	
        g.pv[ply][ply] = 0;

	if (ply > g.maxply) {
		g.maxply = ply;
		memcpy (g.maxpath, g.path, ply * 4);
	}	
	
	if (! (node -> bpawns) && ! (node -> bpieces)) {
		print_search (node, alpha, beta, 0, ply, MATE-ply, _PS_TRIVIAL, _PS_Q_E_B); 
                return MATE - ply;
	}

	if (w17_trivial_draw_b (node, ply)) {
		print_search (node, alpha, beta, 0, ply, 0, _PS_TRIVIAL, _PS_Q_E_B);
                return 0;
	}

	if (depth <= 0) {
		print_search (node, alpha, beta, 0, ply, UNKNOWN, _PS_UNKNOWN, _PS_Q_E_B);
                return w17_unknown_b (node);
	}

	if (ply >= _W17MAXPLY) {
		print_search (node, alpha, beta, 0, ply, beta, _PS_MAXPLY, _PS_Q_E_B);
                return beta;
	}

	// hash.
	if (! inhash (node)) {
		hashmove = 0;
	} else if (hashhit (node, 0, ply, value, hashmove, alpha, beta)) {
		print_search (node, alpha, beta, hashmove, ply, value, _PS_HASH, _PS_Q_E_B);
                return value;
	}
		
	nodes = g.fastnodes;
	hflags = upper_bound;

	legals = 0;
	flags = node -> flags;
	best = - INFINITY;
	first = ply << 7;
	
	g.reptable [g.repindex + ply] = node -> hashcode;

	// captures 
	last = _fast_gencapsb (node, first);

	if (! hashmove || _CAPTURE (hashmove)) {
		
		if (hashmove) {
			for (int i = first + 1; i < last; i ++) {
				if (g.tmoves [i] == hashmove) {
					g.tmoves [i] = g.tmoves [first];
					g.tmoves [first] = hashmove;					
					break;
				}
			}
		}
		
		while (last > first)  {	
			
			
			move = w17_select_capture_b (node, first, last);	
			
			if (! legal_move_b (node, move)) {
				continue;
			}		
			_fast_dobmove (node, move);
			if (attacked_by_PNBRQK (node, node -> bkpos)) {
				_fast_undobmove (node, move, flags, fifty);
				continue;
			}
			legals ++;

			g.path [ply] = move;

			if (! checkcheck_b (node, move)) {
				value = - w17_quiescence_w (node, - beta, - alpha, ply + 1, depth - 1);
			} else {
				value = - w17_quiescence_evade_w (node, - beta, - alpha, ply + 1, depth - 1);
			}
			
			_fast_undobmove (node, move, flags, fifty);

			if (g.stopsearch) { 
				print_search (node, alpha, beta, 0, ply, INVALID, _PS_TIME, _PS_Q_E_B);
				return INVALID;
			}		
			if (value > best) {			
				if (value > alpha) {
					if (value >= beta) {						
						hash_store (node, 0, ply, value, move, g.fastnodes - nodes, lower_bound);
						print_search (node, alpha, beta, move, ply, value, _PS_CUT, _PS_Q_E_B);
						return value;
					}
					hflags = exact;
					alpha = value;

					/* qpv_update */
                                        update_pv(node, move, ply);
				}
				hashmove = move;
				best = value;
			}
		}					
	
		if (legals) {
			hash_store (node, 0, ply, best, hashmove, g.fastnodes - nodes, hflags);
			print_search (node, alpha, beta, hashmove, ply, best, _PS_NORMAL, _PS_Q_E_B); 
			return best;
		}
	}
	
		
	last = _fast_gennoncapsb (node, first);

	if (hashmove) {
		for (int i = first + 1; i < last; i ++) {
			if (g.tmoves [i] == hashmove) {
				g.tmoves [i] = g.tmoves [first];
				g.tmoves [first] = hashmove;					
				break;
			}
		}
	}
	
	for (int i = first; i < last; i ++) {
		move = g.tmoves [i];
		if (! legal_move_b (node, move)) {
			continue;
		}				
		_fast_dobmove (node, move);

		if (attacked_by_PNBRQK (node, node -> bkpos)) {
			_fast_undobmove (node, move, flags, fifty);
			continue;
		}
		legals ++;

		g.path [ply] = move;

		if (! checkcheck_b (node, move)) {			
			value = - w17_quiescence_w (node, - beta, - alpha, ply + 1, depth - 1);
		} else {
			value = - w17_quiescence_evade_w (node, - beta, - alpha, ply + 1, depth - 1); 
		}

		_fast_undobmove (node, move, flags, fifty);

		if (g.stopsearch) { 
			print_search (node, alpha, beta, 0, ply, INVALID, _PS_TIME, _PS_Q_E_B);
			return INVALID;
		}		
		if (value > best) {			
			if (value > alpha) {
				if (value >= beta) {					
					hash_store (node, 0, ply, value, move, g.fastnodes - nodes, lower_bound);
					print_search (node, alpha, beta, move, ply, value, _PS_CUT, _PS_Q_E_B);
					return value;
				}
				hflags = exact;
				alpha = value;

				/* qpv_update */
                                update_pv(node, move, ply);
			}
			hashmove = move;
			best = value;
		}
	}
	
	if (! legals) {
		print_search (node, alpha, beta, 0, ply, MATE-ply, _PS_TRIVIAL, _PS_Q_E_B); 
                return MATE - ply;
	}
	
	hash_store (node, 0, ply, best, hashmove, g.fastnodes - nodes, hflags);
	print_search (node, alpha, beta, hashmove, ply, best, _PS_NORMAL, _PS_Q_E_B);
        return best;
} 
