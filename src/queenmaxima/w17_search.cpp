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
#include "w17_search.h"
#include "legality.h" 
#include "hash.h"  
#include "w17.h"
#include "w17_quiescence.h"
#include "attack.h"
#include "parser.h" 

int max_mate_depth (TFastNode * node) 
{
	int w = node -> wpieces + node -> wpawns,
		b = node -> bpieces + node -> bpawns;
	if (w > b) {
		return 2 + (w << 1);
	}
	return 2 + (b << 1);	
}

int w17_next_search_w (TFastNode * node, int alpha, int beta, int ply, int depth, bool incheck, int legals)
{
	int value;
	       
	if (legals) {
		if (depth < FULL_PLY) {
			g.pv[ply][ply]=0; 
			if (! incheck) {
				value = - w17_quiescence_b (node, - alpha - 1, - alpha, ply, g.maxq);
				if (value > alpha && value < beta) {
					value = - w17_quiescence_b (node, - beta, - alpha, ply, g.maxq);
				}
			} else {
				value = - w17_quiescence_evade_b (node, - alpha - 1, - alpha, ply, g.maxq);
				if (value > alpha && value < beta) {
					value = - w17_quiescence_evade_b (node, - beta, - alpha, ply, g.maxq);
				}
			}
		} else {
			if (! incheck) {
				value = - w17_pvs_b (node, - alpha - 1, - alpha, ply, depth);
				if (value > alpha && value < beta) {
					value = - w17_pvs_b (node, - beta, - alpha, ply, depth);
				}
			} else {
				value = - w17_pvs_evade_b (node, - alpha - 1, - alpha, ply, depth);
				if (value > alpha && value < beta) {
					value = - w17_pvs_evade_b (node, - beta, - alpha, ply, depth);
				}
			}
		}
	} else {
		if (depth < FULL_PLY) {
			g.pv[ply][ply]=0; 
			if (! incheck) {				
				value = - w17_quiescence_b (node, - beta, - alpha, ply, g.maxq);
			} else {				
				value = - w17_quiescence_evade_b (node, - beta, - alpha, ply, g.maxq);	
			}
		} else {
			if (! incheck) {				
				value = - w17_pvs_b (node, - beta, - alpha, ply, depth);
			} else {				
				value = - w17_pvs_evade_b (node, - beta, - alpha, ply, depth);	
			}		
		}
	}
	return value;
}

int w17_next_search_b (TFastNode * node, int alpha, int beta, int ply, int depth, bool incheck, int legals)
{
	int value;
	if (legals) {
		if (depth < FULL_PLY) {
			g.pv[ply][ply]=0; 
			if (! incheck) {
				value = - w17_quiescence_w (node, - alpha - 1, - alpha, ply, g.maxq);
				if (value > alpha && value < beta) {
					value = - w17_quiescence_w (node, - beta, - alpha, ply, g.maxq);
				}
			} else {
				value = - w17_quiescence_evade_w (node, - alpha - 1, - alpha, ply, g.maxq);
				if (value > alpha && value < beta) {
					value = - w17_quiescence_evade_w (node, - beta, - alpha, ply, g.maxq);
				}
			}
		} else {
			if (! incheck) {
				value = - w17_pvs_w (node, - alpha - 1, - alpha, ply, depth);
				if (value > alpha && value < beta) {
					value = - w17_pvs_w (node, - beta, - alpha, ply, depth);
				}
			} else {
				value = - w17_pvs_evade_w (node, - alpha - 1, - alpha, ply, depth);
				if (value > alpha && value < beta) {
					value = - w17_pvs_evade_w (node, - beta, - alpha, ply, depth);
				}
			}
		}
	} else {
		if (depth < FULL_PLY) {
			g.pv[ply][ply]=0; 
			if (! incheck) {				
				value = - w17_quiescence_w (node, - beta, - alpha, ply, g.maxq);
			} else {				
				value = - w17_quiescence_evade_w (node, - beta, - alpha, ply, g.maxq);	
			}
		} else {
			if (! incheck) {				
				value = - w17_pvs_w (node, - beta, - alpha, ply, depth);
			} else {				
				value = - w17_pvs_evade_w (node, - beta, - alpha, ply, depth);	
			}		
		}
	}
	return value;
}

int w17_root_drive_w (TFastNode * node, int value, int depth, int last) 
{
	int alpha, 
		beta;
//		delta = 10;

		alpha = -INFINITY;
		beta = +INFINITY; 

	do {		
//		alpha = value - delta;
//		beta = value + delta;					  	       
		value = w17_pvs_root_w (node, alpha, beta, depth, last);
		if (value > alpha && value < beta) {
			return value;
		}	
		if (g.stopsearch) {
			return INVALID;
		}
		if (value <= alpha) {
			g_warning ("value <= alpha\n");
		} else {
			g_warning ("value >= beta\n");
		}		
		//		delta += delta;
	} while (true);
	return INVALID;
}

int w17_root_drive_b (TFastNode * node, int value, int depth, int last) 
{
	int alpha, 
		beta;
//		delta = 10;

		alpha = -INFINITY; 
		beta = +INFINITY; 

	do {		
//		alpha = value - delta;
//		beta = value + delta;		
		value = w17_pvs_root_b (node, alpha, beta, depth, last);
		if (value > alpha && value < beta) {
			return value;
		}	
		if (g.stopsearch) {
			return INVALID;
		}
		if (value <= alpha) {
			g_warning ("value <= alpha\n");
		} else {
			g_warning ("value >= beta\n");
		}	
//		delta += delta;
	} while (true);
	return INVALID;
}

int w17_iterate (TFastNode *node) 
{
	int last = 0,
		value = INVALID,
		mm;
	int legals = 0; 
	
	_fast_init_iterate(node); 
	w17_init_iterate(node); 

	/* w17 win by losing everything */ 
	if ((!node->wpawns && !node->wpieces) || (!node->bpawns && !node->bpieces)) { 
		g_print("returning w17_iterate because of 1 side no pawns and pieces\n"); 
		return 0; 
	}

	if (node->flags & _WTM) {
		last = w17_genrootmoves_w (node);

		legals = last; 

		if (last == 0) {		       
			return 0; /* geen legal moves */ 
		}

		if (last > 1) {
		    
		    /* check if there are bookmoves */ 
		    if (g.checkbook && select_bookmove(node,last)) { 				
                        whisper_bookmove ();
                        return last;
		    } else { 
                        g.checkbook = false;
                    }
			
                    node -> flags |= _ROOT_WTM;
                    //	node -> flags &= ~(B_ONLY_MATE_SEARCH);
                    //node -> flags |= W_ONLY_MATE_SEARCH;

                    rootmove_sort (0, last);
                    g.maxq = 6;
                    mm = max_mate_depth (node);
                    value = 0;
                    for (int depth = 10; depth < 50 * FULL_PLY; depth += FULL_PLY)
                    {
                        do {
                            //value = w17_pvs_root_w (node, -INFINITY, INFINITY, depth, last);
                            value = w17_root_drive_w (node, value, depth, last);
                            //g_print("value=%d g.fastnodes=%d\n",value,g.fastnodes);

                            if (g.stopsearch) {
                                break;
                            }

                            g.rootscore = value;

                            // unknown //fixme: what is this??
                            rootmove_sort (1, last);
                            if (g.maxq >= mm) {
                                break;
                            }

                            g.maxq += 2;

                        } while (depth <= 10);

                        int choices = 0;
                        for (int i = 0; i < last; i ++) {
                            if (! g.rootmoves [i]. matevalue) {
                                choices ++;
                                if (choices > 1) {
                                    break;
                                }
                            }
                        }
												
                        if (choices == 0) {
                            /* geen choices, alles is mat */
                            /* todo: misschien nog ff sorteren op langste tak */
                            /* fixme: kortste tak moet als eerste gevonden worden */
                            g.stopsearch = true;
                            break;
                        }
				
                        if (g.stopsearch || g.crisis) {
                            break;
                        }
				
                        g_assert (value > - INFINITY && value < INFINITY);
                        g.iteration++;
                    }
		} else { 
			g.rootmoves[0].forced_move = true;

                        /* set the pv, and make sure there is no pondermove in
                         * the pv by making it only 1 move long */
                        g.pv[0][0] = g.rootmoves[0].move;
                        g.pv[0][1] = 0;

			return 1; 
		}
	} else { // blacks move
		last = w17_genrootmoves_b (node);
		legals = last; 

		node -> flags &= ~(_ROOT_WTM);
		//node -> flags &= ~(W_ONLY_MATE_SEARCH);
		//node -> flags |= B_ONLY_MATE_SEARCH; 

		if (last == 0) {			
			return 0; /* geen legal moves */ 
		}
		if (last > 1) {

			/* check if there is a bookmove */ 
			if (g.checkbook && select_bookmove(node,last)) { 						
			    whisper_bookmove ();
			    return last; 
			} else { 
                            g.checkbook = false;
			}
			
			rootmove_sort (0, last);
			g.maxq = 6;
			mm = max_mate_depth (node);
			value = 0;
			for (int depth = 10; depth < 50 * FULL_PLY; depth += FULL_PLY) {			
				do {
					//value = w17_pvs_root_b (node, -INFINITY, INFINITY, depth, last);
					value = w17_root_drive_b (node, value, depth, last);
					if (g.stopsearch) {
						break;
					}
					g.rootscore = value;
					rootmove_sort (1, last);
					if (g.maxq >= mm) {
						break;
					}
					g.maxq += 2;
				} while (depth <= 10);

				int choices = 0;
				for (int i = 0; i < last; i ++) {
					if (! g.rootmoves [i]. matevalue) {
						choices ++;
						if (choices > 1) {
							break;
						}
					}
				}

				if (choices == 0) {
				        /* geen choices, alles is mat */ 
				        /* todo: misschien nog ff sorteren op langste tak */ 
 					g.stopsearch = true;
					break;
				}

				if (g.stopsearch || g.crisis) {
					break;
				}
				g_assert (value > - INFINITY && value < INFINITY);
												
				g.iteration++;
				
			}
		} else { 
			g.rootmoves[0].forced_move = true; 		

                        /* set the pv, and make sure there is no pondermove in
                         * the pv by making it only 1 move long */
                        g.pv[0][0] = g.rootmoves[0].move;
                        g.pv[0][1] = 0;

                        return 1;
		}
	}	
	return legals; /* value; */ 
}

int w17_newdepth_sre_w (TFastNode* node, int move, int ply, int depth) 
{			
	return depth - FULL_PLY;
}

int w17_newdepth_sre_b (TFastNode* node, int move, int ply, int depth) 
{		
	return depth - FULL_PLY;	
}

int w17_newdepth_cap_w (TFastNode* node, int move, int first, int last, int ply, int depth) 
{				
	return depth - FULL_PLY;	
}

int w17_newdepth_cap_b (TFastNode* node, int move, int first, int last, int ply, int depth) 
{				
	return depth - FULL_PLY;
}

int w17_newdepth_silent_w (TFastNode* node, int move, int ply, int depth) 
{							
	return depth - FULL_PLY;
}

int w17_newdepth_silent_b (TFastNode* node, int move, int ply, int depth) 
{				
	return depth - FULL_PLY;	
}

int w17_pvs_b (TFastNode* node, int alpha, int beta, int ply, int depth) {
	int best = -INFINITY,
		bestmove = 0,
		hashmove,
		move = 0, // fixme: why does it give warnings if move is unitialized??
		flags = node->flags,
		fifty = node -> fifty,
		hflags,		
		nodes,
		value,
		last,
		legals = 0,
		newdepth,		
		first = ply << 7,
		index;
	
	g.fastnodes++;
				
	if (ply >= _W17MAXPLY) {
		g.pv[ply][ply]=0;
		print_search (node, alpha, beta, 0, ply, w17_evaluate_b (node, ply, w17_eval_state(node)), _PS_MAXPLY, _PS_PVS_B);
		return w17_evaluate_b (node,ply, w17_eval_state(node));
	}

	
#ifdef DEBUG_INSPECT	
	int error = _fast_inspectnode (node);
	if (error || attacked_by_PNBRQK (node, node -> bkpos) || attacked_by_pnbrqk (node, node->wkpos)) {
		g_print ("error %d detected in w17_pvs_b \n", error);
		print_path (ply);
		g_error ("DEBUG_INSPECT error\n"); 	
		return INVALID;
	}	
#endif	
		
	/* mate by no material */ 
	if (!node->bpawns && !node->bpieces) {	
		g.pv[ply][ply]=0; 
		print_search (node, alpha, beta, 0, ply, MATE-ply, _PS_TRIVIAL, _PS_PVS_B);	
		return MATE-ply;
	}

	if (w17_trivial_draw_b (node, ply)) {
		g.pv[ply][ply]=0; 
		print_search (node, alpha, beta, 0, ply, 0, _PS_TRIVIAL, _PS_PVS_B);
		return 0;
	}

	g_assert( depth >= FULL_PLY ); 
		        					
	// hash.
	if (!inhash (node)) {
		hashmove = 0;
	} else if (hashhit (node, depth, ply, value, hashmove, alpha, beta)) {
		g_assert (value > - INFINITY && value < INFINITY);
		print_search (node, alpha, beta, hashmove, ply, value, _PS_HASH, _PS_PVS_B); 
		return value;		
	}	
	
	g.reptable [g.repindex + ply] = node -> hashcode;
	
	// try captures. skip if hashmove is a noncapture.
	if (!hashmove || _CAPTURE (hashmove)) {

		last = _fast_gencapsb (node, first);

#ifdef DEBUG_INSPECT
		int debuglegals = count_legals_b (node, first, last);
#endif

		if (last > first) { // there are captures.
			
			if ((last - first) == 1) { // there is only one move. 
				
				move = g.tmoves [first];		

#ifdef DEBUG_LEGALITY
				if (legal_move_b (node, move) != inspect_move_legality_b (node, move)) {
					g_print ("path: ");
					print_path (ply);
					g_print ("\n move: ");
					print_move (move);
					g_print ("\nlegal_move_b: %d\n", (int) legal_move_b (node, move));
					g_print ("inspect: %d\n", (int) inspect_move_legality_b (node, move));
					g_error ("DEBUG_LEGALITY error\n");
				}
#endif
				
				if (legal_move_b (node, move)) {
					
					_fast_dobmove (node, move);
					newdepth = w17_newdepth_sre_b (node, move, ply, depth);
					g.path [ply] = move;

					value = w17_next_search_b (node, alpha, beta, ply + 1, newdepth, checkcheck_b (node, move), 0);
					_fast_undobmove (node, move, flags, fifty);		
		
					if (g.stopsearch) {
						print_search (node, alpha, beta, 0, ply, INVALID, _PS_TIME, _PS_PVS_B);
						return INVALID;
					}
		
					legals ++;

					if (value > alpha) {
						if (value >= beta) {
							g_assert (value > - INFINITY && value < INFINITY);
							print_search (node, alpha, beta, move, ply, value, _PS_CUT, _PS_PVS_B);		
							return value;
						}
						alpha = value;						
						update_pv (node, move, ply); 

					}					

					g_assert (value > - INFINITY && value < INFINITY);
					print_search (node, alpha, beta, move, ply, value, _PS_NORMAL, _PS_PVS_B); 

#ifdef DEBUG_INSPECT
	                                if (debuglegals != legals) {		 
						g_print ("hash = %d, nodes = %d, debuglegals = %d, legals = %d, hashmove = %d\n", (int) node -> hashcode,  g.fastnodes, debuglegals, legals, hashmove);
						print_path (ply);
						g_assert_not_reached ();
					}
#endif
					return value;						
				}      
			} else { // more then one (capture) move. sorting becomes an issue.
								
				nodes = g.fastnodes;
				hflags = upper_bound;
				
				// try hashmove				
				if (hashmove) {
#ifdef DEBUG_LEGALITY
					if (legal_move_b (node, hashmove) != inspect_move_legality_b (node, hashmove)) {
						g_print ("path: ");
						print_path (ply);
						g_print ("\n move: ");
						print_move (hashmove);
						g_print ("\nlegal_move_b: %d\n", (int) legal_move_b (node, hashmove));
						g_print ("inspect: %d\n", (int) inspect_move_legality_b (node, hashmove));
						g_error ("DEBUG_LEGALITY error\n");
					}
#endif

					_fast_dobmove (node, hashmove);

					newdepth = w17_newdepth_cap_b (node, hashmove, first, last, ply, depth);
					g.path [ply] = hashmove;
					legals = 1;

					best = w17_next_search_b (node, alpha, beta, ply + 1, newdepth, checkcheck_b (node, hashmove), 0);				
					_fast_undobmove (node, hashmove, flags, fifty);

					if (g.stopsearch) {
						print_search (node, alpha, beta, 0, ply, INVALID, _PS_TIME, _PS_PVS_B); 
						return INVALID;
					}
					g_assert (best > - INFINITY && best < INFINITY);
					if (best > alpha) {
						if (best >= beta) {
							hash_store (node, depth, ply, best, hashmove, g.fastnodes - nodes, lower_bound);
							g_assert (best > - INFINITY && best < INFINITY);
							print_search (node, alpha, beta, hashmove, ply, best, _PS_CUT, _PS_PVS_B);
							g.cutoffs_b [legals]++; 
						       	return best;
						}
						alpha = best;
						hflags = exact;
						update_pv (node, hashmove, ply);  
					}
					bestmove = hashmove; 
				}   
				
				// try moves from capture generation				
				if (legals) { //haal reeds onderzochte legale zetten uit de lijst		
					index = first;
					while (index < last) {
						move = g.tmoves [index ++];
						if (move == hashmove) {
							g.tmoves [-- index] = g.tmoves [-- last];
							continue;
						}			
					}
				}
				
				
				while (last > first) { 					

					move = w17_select_capture_b (node, first, last); // fixme: ook proberen zonder deze sortering
				
					g_assert( legals <= 128); 
	
#ifdef DEBUG_LEGALITY
					if (legal_move_b (node, move) != inspect_move_legality_b (node, move)) {
						g_print ("path: ");
						print_path (ply);
						g_print ("\n move: ");
						print_move (move);
						g_print ("\nlegal_move_b: %d\n", (int) legal_move_b (node, move));
						g_print ("inspect: %d\n", (int) inspect_move_legality_b (node, move));
						g_error ("DEBUG_LEGALITY error\n");
					}
#endif
					
					if (! legal_move_b (node, move)) {						
						continue;
					}
					_fast_dobmove (node, move);
					
					newdepth = w17_newdepth_cap_b (node, move, first, last, ply, depth);
            					
					g.path [ply] = move;

					value = w17_next_search_b (node, alpha, beta, ply + 1, newdepth, checkcheck_b (node, move), legals);				
					 					
					_fast_undobmove (node, move, flags, fifty);
					
					if (g.stopsearch) { 
						print_search (node, alpha, beta, 0, ply, value, _PS_TIME, _PS_PVS_B); 
						return INVALID;
					}
					g_assert (value > - INFINITY && value < INFINITY);
					legals++;
					
					if (value > best) {
						if (value > alpha) {
							if (value >= beta) {
								hash_store (node, depth, ply, value, move, g.fastnodes - nodes, lower_bound);
								g_assert (value > - INFINITY && value < INFINITY);
								print_search (node, alpha, beta, move, ply, value, _PS_CUT, _PS_PVS_B);
								g.cutoffs_b [legals] ++; 
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
				
				if (legals) { 

#ifdef DEBUG_INSPECT
					if (debuglegals != legals) {		 
						g_print ("hash = %d, nodes = %d, debuglegals = %d, legals = %d, hashmove = %d\n", (int) node -> hashcode,  g.fastnodes, debuglegals, legals, hashmove);
						print_path (ply);
						g_assert_not_reached ();
					}
#endif
 
					g_assert (best > - INFINITY && best < INFINITY);
					print_search (node, alpha, beta, bestmove, ply, best, _PS_NORMAL, _PS_PVS_B); 
					return best;
				}
			} /* else meerdere caps */ 									
		} /* er zijn captures */
	} /* geen hashmove of hashmove is een capture */ 
		
	// this node has no legal captures 									
	nodes = g.fastnodes;
	hflags = upper_bound;	
	
	// try hashmove
	if (hashmove) {
		g.path [ply] = hashmove;
		legals = 1;		
		_fast_dobmove (node, hashmove);
		newdepth = w17_newdepth_silent_b (node, move, ply, depth);
		best = w17_next_search_b (node, alpha, beta, ply + 1, newdepth, checkcheck_b (node, hashmove), 0);		     
		_fast_undobmove (node, hashmove, flags, fifty);		
		if (g.stopsearch) { 
			print_search (node, alpha, beta, 0, ply, best, _PS_TIME, _PS_PVS_B); 
			return INVALID;
		}	
		g_assert (best > - INFINITY && best < INFINITY);	

		if (best > alpha) {		       
			if (best >= beta) {
				hash_store (node, depth, ply, best, hashmove, g.fastnodes - nodes, lower_bound);
				g_assert (best > - INFINITY && best < INFINITY);
				print_search (node, alpha, beta, move, ply, best, _PS_CUT, _PS_PVS_B); 
				g.cutoffs_b [legals] ++; 
				return best;
			}
			hflags = exact;
			alpha = best;
			update_pv (node, move, ply); 
		}
		bestmove = hashmove; 
	}

	
	
	first = ply << 7;  
	last = _fast_gennoncapsb (node, first);

#ifdef DEBUG_INSPECT
	int debuglegals = count_legals_b (node, first, last);
#endif

	g_assert(first + 128 > last); 

	if (legals) { //haal reeds onderzochte legale zetten uit de lijst		
		index = first;
		while (index < last) {
			move = g.tmoves [index ++];
			if (move == hashmove) {
				g.tmoves [-- index] = g.tmoves [-- last];
				continue;
			}			
		}
	}
		
	while (last > first) { 

		move = _fast_selectmove (first, last); // fixme: ook proberen zonder deze sortering

#ifdef DEBUG_LEGALITY
		if (legal_move_b (node, move) != inspect_move_legality_b (node, move)) {
			g_print ("path: ");
			print_path (ply);
			g_print ("\n move: ");
			print_move (move);
			g_print ("\nlegal_move_b: %d\n", (int) legal_move_b (node, move));
			g_print ("inspect: %d\n", (int) inspect_move_legality_b (node, move));
			g_error ("DEBUG_LEGALITY error\n");
		}
#endif
		if (!legal_move_b (node, move)) {				 
			continue;
		}

		_fast_dobmove (node, move);

		newdepth = w17_newdepth_silent_b (node, move, ply, depth);

		g.path [ply] = move; 

		value = w17_next_search_b (node, alpha, beta, ply + 1, newdepth, checkcheck_b (node, move), legals);	
						
		_fast_undobmove (node, move, flags, fifty);
		
		if (g.stopsearch) { 
			print_search (node, alpha, beta, 0, ply, INVALID, _PS_TIME, _PS_PVS_B); 
			return INVALID;
		}		
		g_assert (value > - INFINITY && value < INFINITY);
		legals++;
		
		if (value > best) {
			if (value > alpha) {
				if (value >= beta) {
					hash_store (node, depth, ply, value, move, g.fastnodes - nodes, lower_bound);
					g_assert (value > - INFINITY && value < INFINITY);
					print_search (node, alpha, beta, move, ply, value, _PS_CUT, _PS_PVS_B); 
					g.cutoffs_b [legals] ++; 
					return value;
				}
				hflags = exact;
				alpha = value;
				update_pv (node, move, ply); 
			}
			bestmove = move;
			best = value;
		}
					
	} /* while (last > first) */ 
	
#ifdef DEBUG_INSPECT
	if (debuglegals != legals) {		 
		g_print ("hash = %d, nodes = %d, debuglegals = %d, legals = %d, hashmove = %d\n", (int) node -> hashcode,  g.fastnodes, debuglegals, legals, hashmove);
		print_path (ply);
		g_assert_not_reached ();
	}
#endif

	/* no legal moves --> mate */ 
	if (!legals) { 	       
		g.pv[ply][ply]=0; 
		print_search (node, alpha, beta, 0, ply, MATE-ply, _PS_TRIVIAL, _PS_PVS_B); 
		return MATE-ply; 
	}

       	hash_store (node, depth, ply, best, bestmove, g.fastnodes - nodes, hflags);
	g_assert (best > - INFINITY && best < INFINITY);
	print_search (node, alpha, beta, bestmove, ply, best, _PS_NORMAL, _PS_PVS_B); 
	return best; 
}

int w17_pvs_w (TFastNode* node, int alpha, int beta, int ply, int depth) 
{
	int best = -INFINITY,
		bestmove = 0,
		move = 0, //fixme: why initialize?
		flags = node->flags,
		fifty = node -> fifty,
		value,
		hashmove,
		hflags,
		nodes,
		last,
		legals = 0,  
		newdepth,
		first = ply << 7,
		index;	
       
	g.fastnodes++;	
			
	if (ply >= _W17MAXPLY) {
		g.pv[ply][ply]=0; 
		print_search (node, alpha, beta, 0, ply, w17_evaluate_w (node, ply, w17_eval_state(node)), _PS_MAXPLY, _PS_PVS_W);
		return w17_evaluate_w (node,ply,w17_eval_state(node));		
	}
		

#ifdef DEBUG_INSPECT
	int error = _fast_inspectnode (node);
	if (error || attacked_by_pnbrqk (node, node->wkpos) || attacked_by_PNBRQK (node, node -> bkpos)) {
		g_print ("error %d detected in w17::pvs_w \n", error);
		print_path (ply);
		g_error ("DEBUG_INSPECT error\n"); 	       
		return INVALID;
	}
#endif				
	
	/* mate by no material */ 
	if (!node->wpawns && !node->wpieces) { 
		g.pv[ply][ply]=0; 
		print_search (node, alpha, beta, 0, ply, 0, _PS_TRIVIAL, _PS_PVS_W); 
		return MATE-ply;
	}
	
	if (w17_trivial_draw_w (node, ply)) {
		g.pv[ply][ply]=0; 
		print_search (node, alpha, beta, 0, ply, 0, _PS_TRIVIAL, _PS_PVS_W); 
		return 0;
	}

	g_assert(depth >= FULL_PLY); 
				
	// hash.
	if (!inhash (node)) {
		hashmove = 0;
	} else if (hashhit (node, depth, ply, value, hashmove, alpha, beta)) {
		g_assert (value > - INFINITY && value < INFINITY);
		print_search (node, alpha, beta, hashmove, ply, value, _PS_HASH, _PS_PVS_W); 
		return value;		
	}	
		
	g.reptable [g.repindex + ply] = node -> hashcode;

	// try captures. skip if hashmove is a noncapture 
	if (!hashmove || _CAPTURE (hashmove)) {

		last = _fast_gencapsw (node, first); 	      

#ifdef DEBUG_INSPECT
		int debuglegals = count_legals_w (node, first, last);
#endif
		
		if (last > first) {
			
			if ((last - first) == 1) { // there is only one move
				
				move = g.tmoves [first];
#ifdef DEBUG_LEGALITY
				if (legal_move_w (node, move) != inspect_move_legality_w (node, move)) {
					g_print ("path: ");
					print_path (ply);
					g_print ("\n move: ");
					print_move (move);
					g_print ("\nlegal_move_w: %d\n", (int) legal_move_w (node, move));
					g_print ("inspect: %d\n", (int) inspect_move_legality_w (node, move));
					g_error ("DEBUG_LEGALITY error\n");
				}
#endif
				if (legal_move_w (node, move)) {
										
					_fast_dowmove (node, move);
					newdepth = w17_newdepth_sre_w (node, move, ply, depth);
					g.path [ply] = move;

					value = w17_next_search_w (node, alpha, beta, ply + 1, newdepth, checkcheck_w (node, move), 0);	     				
					_fast_undowmove (node, move, flags, fifty);
					
					if (g.stopsearch) {
						print_search (node, alpha, beta, 0, ply, INVALID, _PS_TIME, _PS_PVS_W); 
						return INVALID;
					}

					legals ++;

					if (value > alpha) {
						if (value >= beta) {
							g_assert (value > - INFINITY && value < INFINITY);
							print_search (node, alpha, beta, move, ply, value, _PS_CUT, _PS_PVS_W); 
							return value;
						}					
						alpha = value;
						update_pv (node, move, ply); 
					}
										
					g_assert (value > - INFINITY && value < INFINITY);
					print_search (node, alpha, beta, move, ply, value, _PS_NORMAL, _PS_PVS_W);  

#ifdef DEBUG_INSPECT
	                                if (debuglegals != legals) {		 
						g_print ("hash = %d, nodes = %d, debuglegals = %d, legals = %d, hashmove = %d\n", (int) node -> hashcode,  g.fastnodes, debuglegals, legals, hashmove);
						print_path (ply);
						g_assert_not_reached ();
					}
#endif
					return value;					
				}
			} else { // more than one (capture) move. sorting becomes an issue.
				
				nodes = g.fastnodes;
				hflags = upper_bound;
				
				// try hashmove
				
				if (hashmove) {

#ifdef DEBUG_LEGALITY
					if (legal_move_w (node, hashmove) != inspect_move_legality_w (node, hashmove)) {
						g_print ("path: ");
						print_path (ply);
						g_print ("\n move: ");
						print_move (hashmove);
						g_print ("\nlegal_move_w: %d\n", (int) legal_move_w (node, hashmove));
						g_print ("inspect: %d\n", (int) inspect_move_legality_w (node, hashmove));
						g_error ("DEBUG_LEGALITY error\n");
					}
#endif

					_fast_dowmove (node, hashmove);

					newdepth = w17_newdepth_cap_w (node, hashmove, first, last, ply, depth);
					g.path [ply] = hashmove;
					legals = 1;

					best = w17_next_search_w (node, alpha, beta, ply + 1, newdepth, checkcheck_w (node, hashmove), 0);						
					_fast_undowmove (node, hashmove, flags, fifty);

					if (g.stopsearch) { 
						print_search (node, alpha, beta, 0, ply, INVALID, _PS_TIME, _PS_PVS_W); 
						return INVALID;
					}
					g_assert (best > - INFINITY && best < INFINITY);
					if (best > alpha) {
						if (best >= beta) {
							hash_store (node, depth, ply, best, hashmove, g.fastnodes - nodes, lower_bound);
							g_assert (best > - INFINITY && best < INFINITY);
							print_search (node, alpha, beta, hashmove, ply, best, _PS_CUT, _PS_PVS_W);
							g.cutoffs_w [legals] ++; 
						       	return best;
						}
						alpha = best;
						hflags = exact;
						update_pv (node, hashmove, ply); 
					}
					bestmove = hashmove; 
				} 
  
				// try moves from capture generation
				if (legals) { //haal reeds onderzochte legale zetten uit de lijst		
					index = first;
					while (index < last) {
						move = g.tmoves [index ++];
						if (move == hashmove) {
							g.tmoves [-- index] = g.tmoves [-- last];
							continue;
						}			
					}
				}
							
				while (last > first) { 		
				
					move = w17_select_capture_w (node, first, last); // fixme: ook proberen zonder deze sortering
	
					g_assert(legals <= 128); 

					
#ifdef DEBUG_LEGALITY
					if (legal_move_w (node, move) != inspect_move_legality_w (node, move)) {
						g_print ("path: ");
						print_path (ply);
						g_print ("\n move: ");
						print_move (move);
						g_print ("\nlegal_move_w: %d\n", (int) legal_move_w (node, move));
						g_print ("inspect: %d\n", (int) inspect_move_legality_w (node, move));
						g_error ("DEBUG_LEGALITY error\n");
					}
#endif
					if (!legal_move_w (node, move)) { 											 
						continue;
					}
					
					_fast_dowmove (node, move);
					
					newdepth = w17_newdepth_cap_w (node, move, first, last, ply, depth);
					
					g.path [ply] = move; 

					value = w17_next_search_w (node, alpha, beta, ply + 1, newdepth, checkcheck_w (node, move), legals);	
												
					_fast_undowmove (node, move, flags, fifty);
					
					if (g.stopsearch) { 
						print_search (node, alpha, beta, 0, ply, value, _PS_TIME, _PS_PVS_W); 
						return INVALID;
					}
					g_assert (value > - INFINITY && value < INFINITY);					
					legals++;
					
					if (value > best) {
						if (value > alpha) {
							if (value >= beta) {
								hash_store (node, depth, ply, value, move, g.fastnodes - nodes, lower_bound);
								g_assert (value > - INFINITY && value < INFINITY);
								print_search (node, alpha, beta, move, ply, value, _PS_CUT, _PS_PVS_W);
								g.cutoffs_w [legals] ++; 
								return value;
							}
							alpha = value;
							hflags = exact;
							update_pv (node, move, ply); 
						}
						best = value;
						bestmove = move;
					}
													
				} /* while (last > first) */ 
		
				if (legals) { 

#ifdef DEBUG_INSPECT
					if (debuglegals != legals) {		 
						g_print ("hash = %d, nodes = %d, debuglegals = %d, legals = %d, hashmove = %d\n", (int) node -> hashcode,  g.fastnodes, debuglegals, legals, hashmove);
						print_path (ply);
						g_assert_not_reached ();
					}
#endif

					g_assert (best > - INFINITY && best < INFINITY);
					print_search (node, alpha, beta, bestmove, ply, best, _PS_NORMAL, _PS_PVS_W); 
					return best;
				}
			} /* else meerdere caps */ 			
		} /* er zijn captures */ 
	} /* geen hashmove of hashmove is een capture */ 
					
	// this node hash no legal captures 							
	nodes = g.fastnodes;
	hflags = upper_bound;

	// try hashmove
	if (hashmove) {
		g.path [ply] = hashmove;
		legals = 1;
		_fast_dowmove (node, hashmove);
		newdepth = w17_newdepth_silent_w (node, move, ply, depth);
		best = w17_next_search_w (node, alpha, beta, ply + 1, newdepth, checkcheck_w (node, hashmove), 0);			
		_fast_undowmove (node, hashmove, flags, fifty);		
		if (g.stopsearch) {
			print_search (node, alpha, beta, 0, ply, best, _PS_TIME, _PS_PVS_W); 
			return INVALID;
		}	
		g_assert (best > - INFINITY && best < INFINITY);	

		if (best > alpha) {		       
			if (best >= beta) {
				hash_store (node, depth, ply, best, hashmove, g.fastnodes - nodes, lower_bound);
				g_assert (best > - INFINITY && best < INFINITY);
				print_search (node, alpha, beta, move, ply, best, _PS_CUT, _PS_PVS_W); 
				g.cutoffs_w [legals] ++; 
				return best;
			}
			hflags = exact;
			alpha = best;
			update_pv (node, move, ply); 
		}
		bestmove = hashmove; 
	}


	// try moves (non captures) from generation
	
	first = ply << 7;  
	last = _fast_gennoncapsw (node, first);

#ifdef DEBUG_INSPECT
	int debuglegals = count_legals_w (node, first, last);
#endif

	g_assert(first + 128 > last); 

	if (legals) { //haal reeds onderzochte legale zetten uit de lijst		
		index = first;
		while (index < last) {
			move = g.tmoves [index ++];
			if (move == hashmove) {
				g.tmoves [-- index] = g.tmoves [-- last];
				continue;
			}
		}
	}
	
	while (last > first) { 
	
		move = _fast_selectmove (first, last); // fixme: ook proberen zonder deze sortering

	
#ifdef DEBUG_LEGALITY
		if (legal_move_w (node, move) != inspect_move_legality_w (node, move)) {
			g_print ("path: ");
			print_path (ply);
			g_print ("\n move: ");
			print_move (move);
			g_print ("\nlegal_move_w: %d\n", (int) legal_move_w (node, move));
			g_print ("inspect: %d\n", (int) inspect_move_legality_w (node, move));
			g_error ("DEBUG_LEGALITY error\n");
		}
#endif
		
		if (!legal_move_w (node, move)) {			 
			continue;
		}
				
		_fast_dowmove (node, move);
		
		newdepth = w17_newdepth_silent_w (node, move, ply, depth);
		
		g.path [ply] = move; 

		value = w17_next_search_w (node, alpha, beta, ply + 1, newdepth, checkcheck_w (node, move), legals);	
								       	
		_fast_undowmove (node, move, flags, fifty);
		
		if (g.stopsearch) { 
			print_search (node, alpha, beta, 0, ply, INVALID, _PS_TIME, _PS_PVS_W); 
			return INVALID;
		}
		g_assert (value > - INFINITY && value < INFINITY);		
		legals++;
		
		if (value > best) {
			if (value > alpha) {
				if (value >= beta) {
					hash_store (node, depth, ply, value, move, g.fastnodes - nodes, lower_bound);	      
					g_assert (value > - INFINITY && value < INFINITY);
					print_search (node, alpha, beta, move, ply, value, _PS_CUT, _PS_PVS_W); 
					g.cutoffs_w [legals] ++; 
					return value;
				}
				hflags = exact;
				alpha = value;
				update_pv (node, move, ply); 
			}
			bestmove = move;
			best = value;
		}
			
	} /* while (last > first) */ 

#ifdef DEBUG_INSPECT
	if (debuglegals != legals) {		 
		g_print ("hash = %d, nodes = %d, debuglegals = %d, legals = %d, hashmove = %d\n", (int) node -> hashcode,  g.fastnodes, debuglegals, legals, hashmove);
		print_path (ply);
		g_assert_not_reached ();
	}
#endif
	
	/* no legal moves --> mate */ 
	if (!legals) { 
		g.pv[ply][ply]=0; 
		print_search (node, alpha, beta, 0, ply, MATE-ply, _PS_TRIVIAL, _PS_PVS_W); 
		return MATE-ply; 
	}

	hash_store (node, depth, ply, best, bestmove, g.fastnodes - nodes, hflags);	
	g_assert (best > - INFINITY && best < INFINITY);
	print_search (node, alpha, beta, bestmove, ply, best, _PS_NORMAL, _PS_PVS_W); 
	return best; 
}


int w17_avoid_adjustment (int index, int best) {
	if (! g.rootmoves [index]. avoid || best > 10000 || best < -10000) {	       
		return 0;
	}
	switch (g.rootmoves [index]. avoid) {
	case 1: return - 200;
	case 2: return - 400;
	case 3: return - 600;
	case 4: return - 800;
	case 5: return - 1000;
	case 6: return - 1200;
	default: return - 5000;
	}
	return 0;
}

int w17_fifty_adjust (TFastNode * node)
{
	int value = - node -> fifty / 2;
	if (node -> fifty > 5) {
		value -= (node -> fifty / 5) * 10;
	}
	return value;
}

int w17_pvs_root_w (TFastNode* node, int alpha, int beta, int depth, int last) 
{	 
	int best,
		newdepth,
		move, 
		flags,
		fifty,
		nodes = g.fastnodes,
		value;	

#ifdef PRINT_SEARCH 	
	g_print ("w17_pvs_root_w (depth = %d / %d, nodes = %d, evasions = %d)\n", depth / FULL_PLY, g.maxq, g.fastnodes,0);
#endif

	// g_print ("%d/%d ", depth / FULL_PLY, g.maxq); 	
	// search first sucsessor with open window (-beta, -alpha)
	
	move = g.rootmoves [0]. move;
	
	flags = node -> flags;
	fifty = node -> fifty;
	
	g.path [0] = move;
	
	if (g.rootmoves [0]. draw) {
		g.pv[1][1]=0; 
		best = -10000;
	} else if (g.rootmoves[0]. exact_score) {
		g.pv[1][1]=0; 
		best = g.rootmoves[0].value; 
	} else {
	
		_fast_dowmove (node, move);

		newdepth = depth-FULL_PLY;

		best = w17_next_search_w (node, alpha, beta, 1, newdepth, checkcheck_w (node, move), 0);	
		
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
			
	best += w17_avoid_adjustment (0, best);
	best += w17_fifty_adjust (node);
	
	root_new_best (node, 0, best);
	
	if (best > alpha) {		
		alpha = best;				
		if (MATE_VALUE (best)) {
			found_mate_w (node);			
			g_assert (best > - INFINITY && best < INFINITY);			
			return best;
		}		
	}
		
	g_assert (best > - INFINITY && best < INFINITY);

	// search remaining sucsessors with closed window (-alpha - 1, -alpha). 
	// research if value outside window.
	
	for (int i = 1; i < last; i++) {
		
		move = g.rootmoves [i]. move;
		
		g.path [0] = move;

		if (g.rootmoves [i]. draw) {
			g.pv[1][1]=0; 
			value = -10000;
		} else if (g.rootmoves [i]. exact_score) {
			g.pv[1][1]=0; 
			value = g.rootmoves [i]. value; 
		} else if (! g.rootmoves [i]. matevalue) {
			
			nodes = g.fastnodes;
			
			_fast_dowmove (node, move);			

			newdepth = depth-FULL_PLY;

			value = w17_next_search_w (node, alpha, beta, 1, newdepth, checkcheck_w (node, move), 1);		
		
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
						
			value += w17_avoid_adjustment (i, value);
			value += w17_fifty_adjust (node);
			
		} else {
			g.pv[1][1]=0;
			value = g.rootmoves [i]. matevalue;
		}
		
		g_assert (value > - INFINITY && value < INFINITY);
				
		if (value > best) {
			root_new_best (node, i, value); 
			if (value > alpha) { 				
				alpha = value;
			}
			best = value;									
			
			if (MATE_VALUE (value)) {				
				found_mate_w (node);				
				g_assert (best > - INFINITY && best < INFINITY);
				return best;
			}
		}
	} 

	g_assert (best > - INFINITY && best < INFINITY);
	return best;  
}


int w17_pvs_root_b (TFastNode* node, int alpha, int beta, int depth, int last) 
{	 
	int best,
		move, 
		flags,
		value,
		fifty,
		nodes = g.fastnodes,
		newdepth;

#ifdef PRINT_SEARCH	
	g_print ("w17_pvs_root_b (depth = %d / %d, nodes = %d, evasions = %d)\n", depth / FULL_PLY, g.maxq, g.fastnodes,0);
#endif 

	// g_print ("%d/%d ", depth / FULL_PLY, g.maxq); 
	
	// search first sucsessor with open window (-beta, -alpha)
	
	move = g.rootmoves [0]. move;
	
	flags = node -> flags;
	fifty = node -> fifty;

	g.path [0] = move;
	
	if (g.rootmoves [0]. draw) {
		g.pv[1][1]=0; 
		best = -10000;
	} else if (g.rootmoves [0]. exact_score) { 
		g.pv[1][1]=0;
		best = g.rootmoves [0]. value; 
	} else {

		_fast_dobmove (node, move);	
		
		newdepth = depth-FULL_PLY;

		best = w17_next_search_b (node, alpha, beta, 1, newdepth, checkcheck_b (node, move), 0);	
		
		_fast_undobmove (node, move, flags, fifty);
		
		if (g.stopsearch) {
			return INVALID;
		}
	}

	g.rootmoves [0]. value = best;
	g.rootmoves [0]. nodes = g.fastnodes - nodes;
	g.rootmoves [0]. unknown = IS_UNKNOWN (best);

	if (MATED_VALUE (best)) {
		g.rootmoves [0]. matevalue = best;
	}
	
	best += w17_avoid_adjustment (0, best);
	best += w17_fifty_adjust (node);

	root_new_best (node, 0, best); 

	if (best > alpha) {
		alpha = best;		
		if (MATE_VALUE (best)) {
			found_mate_b (node);
			g_assert (best > - INFINITY && best < INFINITY);
			return best;
		}
	}
		
	g_assert (best > - INFINITY && best < INFINITY);
	
	// search remaining sucsessors with closed window (-alpha - 1, -alpha). 
	// research if value outside window.
	
	for (int i = 1; i < last; i++) {
				
		move = g.rootmoves [i]. move;
		
		g.path [0] = move;
		
		if (g.rootmoves [i]. draw) {
			g.pv[1][1]=0;
			value = -10000;
		} else if (g.rootmoves [i]. exact_score) { 
			g.pv[1][1]=0; 
			value = g.rootmoves [i]. value;
		} else if (! g.rootmoves [i]. matevalue) {
		
			nodes = g.fastnodes;
	
			_fast_dobmove (node, move);
						
			newdepth = depth-FULL_PLY;
			
			value = w17_next_search_b (node, alpha, beta, 1, newdepth, checkcheck_b (node, move), 1);								
			_fast_undobmove (node, move, flags, fifty);
			
			if (g.stopsearch) { 
				return INVALID;
			}
  
			g.rootmoves [i]. value = value;
			g.rootmoves [i]. nodes += g.fastnodes - nodes;
			g.rootmoves [i]. unknown = IS_UNKNOWN (value);

			if MATED_VALUE (value) {
				g.rootmoves [i]. matevalue = value;
			}			
			
			value += w17_avoid_adjustment (i, value);
			value += w17_fifty_adjust (node);

		} else {
			g.pv[1][1]=0; 
			value = g.rootmoves [i]. matevalue;
		}

		g_assert (value > - INFINITY && value < INFINITY);
				
		if (value > best) {
			best = value;
			root_new_best (node, i, value); 
			if (value > alpha) {  
				alpha = value;
			}
			if (MATE_VALUE (value)) {
				found_mate_b (node);
				g_assert (best > - INFINITY && best < INFINITY);
				return best;
			}
		}
	} 
	g_assert (best > - INFINITY && best < INFINITY);
	return best;  
}

int w17_pvs_evade_b (TFastNode* node, int alpha, int beta, int ply, int depth) {
	int best = -INFINITY,
		move,
		hashmove,
		flags = node->flags,
		fifty = node -> fifty,
		value,
		last,
		hflags,		
		nodes,
		legals = 0,		
		first = ply << 7;

	depth -= FULL_PLY;

	g.fastnodes ++;
	
			
	if (ply >= _W17MAXPLY) {
		print_search (node, alpha, beta, 0, ply, w17_evaluate_b (node, ply, w17_eval_state(node)), _PS_MAXPLY, _PS_PVS_E_B); 
		g.pv[ply][ply]=0;
		return w17_evaluate_b (node,ply, w17_eval_state(node));
	}
		
#ifdef DEBUG_INSPECT	
	int error = _fast_inspectnode (node);
	if (error ||  !attacked_by_PNBRQK (node, node->bkpos) || attacked_by_pnbrqk (node, node->wkpos)) {
		g_print ("error %d detected in w17_pvs_evade_b\n", error);
		print_path (ply);
		g_error ("DEBUG_LEGALITY error\n"); 
		return INVALID;
	}
#endif
		       
	/* mate by no material */ 
	if (!node->bpawns && !node->bpieces) {
		g.pv[ply][ply]=0; 
		print_search (node, alpha, beta, 0, ply, MATE-ply, _PS_TRIVIAL, _PS_PVS_E_B); 
		return MATE-ply;
	}
	
	if (w17_trivial_draw_b (node, ply)) {
		g.pv[ply][ply]=0; 
		print_search (node, alpha, beta, 0, ply, 0, _PS_TRIVIAL, _PS_PVS_E_B); 
		return 0;
	}


	// hash
	if (!inhash (node)) {
		hashmove = 0;
	} else if (hashhit (node, depth, ply, value, hashmove, alpha, beta)) {
		g_assert (value > - INFINITY && value < INFINITY);
		print_search (node, alpha, beta, hashmove, ply, value, _PS_HASH, _PS_PVS_E_B);
		return value;		
	}	

	g.reptable [g.repindex + ply] = node -> hashcode;
			
	// fixme: ideetje. opslaan in hash of er 1 legale zet is en movegeneratie 
	// overslaan indien dat het geval is.
	
	last = _fast_gencapsb (node, first);

#ifdef DEBUG_INSPECT
	int debuglegals = count_legals_b (node, first, last);
#endif

	if (last > first) { // there are captures.

		if ((last - first) == 1) { // there is only one move

			move = g.tmoves [first];	
					
			if (legal_move_b (node, move)) {

				_fast_dobmove (node, move);

				if (!attacked_by_PNBRQK (node, node->bkpos)) {

					g.path [ply] = move;

					value = w17_next_search_b (node, alpha, beta, ply + 1, depth - FULL_PLY, checkcheck_b (node, move), 0);	
								
					_fast_undobmove (node, move, flags, fifty);

					if (g.stopsearch) {
						print_search (node, alpha, beta, 0, ply, INVALID, _PS_TIME, _PS_PVS_E_B); 
						return INVALID;
					}	
					
					legals ++;

					if (value > alpha) {
						if (value >= beta) {
							g_assert (value > - INFINITY && value < INFINITY);		
							print_search (node, alpha, beta, move, ply, value, _PS_CUT, _PS_PVS_E_B);
							return value;
						}
						alpha = value;					      
						update_pv (node, move, ply); 
					}

					g_assert (value > - INFINITY && value < INFINITY);
					print_search(node, alpha, beta, move, ply, value, _PS_NORMAL, _PS_PVS_E_B); 

#ifdef DEBUG_INSPECT
	                                if (debuglegals != legals) {		 
						g_print ("hash = %d, nodes = %d, debuglegals = %d, legals = %d, hashmove = %d\n", (int) node -> hashcode,  g.fastnodes, debuglegals, legals, hashmove);
						print_path (ply);
						g_assert_not_reached ();
					}
#endif

					return value;
				} else {
					_fast_undobmove (node, move, flags, fifty);
				}
			}      
		} else { // more then one move. sorting becomes an issue.
						
			nodes = g.fastnodes;
			hflags = upper_bound;
			
					       												
			while (last > first) { 					

				move = w17_select_capture_b (node, first, last); // fixme: ook proberen zonder deze sortering

				g_assert( legals <= 128 ); 
								 
				if (! legal_move_b (node, move)) {				
					continue;
				}
				
				_fast_dobmove (node, move);
				
				if (!attacked_by_PNBRQK (node, node->bkpos)) {
	
					g.path [ply] = move;

					value = w17_next_search_b (node, alpha, beta, ply + 1, depth - FULL_PLY, checkcheck_b (node, move), legals);	
		    					
					_fast_undobmove (node, move, flags, fifty);
					
					if (g.stopsearch) { 
						return INVALID;
					}
					g_assert (value > - INFINITY && value < INFINITY);
					legals++;
					
					if (value > best) {
						if (value > alpha) {
							if (value >= beta) {
								g_assert (value > - INFINITY && value < INFINITY);
								hash_store (node, depth, ply, value, move, g.fastnodes - nodes, lower_bound);
								print_search( node, alpha, beta, move, ply, value, _PS_CUT, _PS_PVS_E_B); 
								g.cutoffs_b [legals] ++; 
								return value;
							}
							alpha = value;
							hflags = exact;
							update_pv (node, move, ply); 
						}
						hashmove = move;
						best = value;
					}
				} else {
					_fast_undobmove (node, move, flags, fifty);
				}
		            							
			}
					
      			if (legals) { 

#ifdef DEBUG_INSPECT
				if (debuglegals != legals) {		 
					g_print ("hash = %d, nodes = %d, debuglegals = %d, legals = %d, hashmove = %d\n", (int) node -> hashcode,  g.fastnodes, debuglegals, legals, hashmove);
					print_path (ply);
					g_assert_not_reached ();
				}
#endif
				g_assert (best > - INFINITY && best < INFINITY);
				print_search( node, alpha, beta, hashmove, ply, best, _PS_NORMAL, _PS_PVS_E_B);
				return best;
			}
		} /* else meerdere caps */ 
	} /* er zijn captures */ 
	
	
	// this node has no legal captures
	nodes = g.fastnodes;
	hflags = upper_bound;
			
	
	// try moves (non captures) from generation
	
	first = ply << 7;  
	last = _fast_gennoncapsb (node, first);

#ifdef DEBUG_INSPECT
	debuglegals = count_legals_b (node, first, last);
#endif
	
	g_assert(first + 128 > last); 

		
	while (last > first) { 

		move = _fast_selectmove (first, last); // fixme: ook proberen zonder deze sortering
			
		if (!legal_move_b (node, move)) {				
			continue;
		}

		_fast_dobmove (node, move);

		if (!attacked_by_PNBRQK (node, node->bkpos)) {
			
			g.path [ply] = move; 

			value = w17_next_search_b (node, alpha, beta, ply + 1, depth - FULL_PLY, checkcheck_b (node, move), legals);	
			
			_fast_undobmove (node, move, flags, fifty);
			
			if (g.stopsearch) { 
				return INVALID;
			}		
			g_assert (value > - INFINITY && value < INFINITY);
			legals++;
			
			if (value > best) {
				if (value > alpha) {
					if (value >= beta) {
						g_assert (value > - INFINITY && value < INFINITY);
						hash_store (node, depth, ply, value, move, g.fastnodes - nodes, lower_bound);
						print_search( node, alpha, beta, move, ply, value, _PS_CUT, _PS_PVS_E_B); 
						g.cutoffs_b [legals] ++; 
						return value;
					}					
					hflags = exact;
					alpha = value;
					update_pv (node, move, ply); 
				}
				hashmove = move;
				best = value;
			}
		} else {
			_fast_undobmove (node, move, flags, fifty);
		}
		
	} /* while (last > first) */ 

#ifdef DEBUG_INSPECT
	if (debuglegals != legals) {		 
		g_print ("hash = %d, nodes = %d, debuglegals = %d, legals = %d, hashmove = %d\n", (int) node -> hashcode,  g.fastnodes, debuglegals, legals, hashmove);
		print_path (ply);
		g_assert_not_reached ();
	}
#endif	

	/* no legal moves --> mate */ 
	if (!legals) { 
		g.pv[ply][ply] = 0; 
		print_search (node, alpha, beta, 0, ply, MATE-ply, _PS_TRIVIAL, _PS_PVS_E_B); 
		return MATE-ply; 
	}
	g_assert (best > - INFINITY && best < INFINITY);
	hash_store (node, depth, ply, best, hashmove, g.fastnodes - nodes, hflags);
	print_search (node, alpha, beta, hashmove, ply, best, _PS_NORMAL, _PS_PVS_E_B); 
	return best; 
}

int w17_pvs_evade_w (TFastNode* node, int alpha, int beta, int ply, int depth) 
{
	int best = -INFINITY,
		hflags,
		hashmove,
		nodes,
		move,
		flags = node->flags,
		fifty = node -> fifty,
		value,
		last,
		legals = 0,  
		first = ply << 7;

	
	depth -= FULL_PLY;

	g.fastnodes++;
	
	
	if (ply >= _W17MAXPLY) {
		print_search (node, alpha, beta, 0, ply, w17_evaluate_w (node, ply, w17_eval_state(node)), _PS_MAXPLY, _PS_PVS_E_W); 
		g.pv[ply][ply]=0; 
		return w17_evaluate_w (node,ply,w17_eval_state(node));
	}
		

#ifdef DEBUG_INSPECT
	int error = _fast_inspectnode (node);
	if (error ||  !attacked_by_pnbrqk (node, node->wkpos) || attacked_by_PNBRQK (node, node -> bkpos)) {
		g_print ("error %d detected in w17_pvs_evade_w\n", error);
		print_path (ply);
		g_error ("DEBUG_LEGALITY error\n"); 	       
		return INVALID;	       
	}
#endif				
		 
	if (!node->wpawns && !node->wpieces) {
		g.pv[ply][ply]=0; 
		print_search (node, alpha, beta, 0, ply, MATE-ply, _PS_TRIVIAL, _PS_PVS_E_W); 
		return MATE-ply;
	}
		
	if (w17_trivial_draw_w (node, ply)) {
		g.pv[ply][ply]=0; 
		print_search (node, alpha, beta, 0, ply, 0, _PS_TRIVIAL, _PS_PVS_E_W); 
		return 0;
	}
	
	// hash 
	if (!inhash (node)) {
		hashmove = 0;
	} else if (hashhit (node, depth, ply, value, hashmove, alpha, beta)) {
		g_assert (value > - INFINITY && value < INFINITY);
		print_search (node, alpha, beta, hashmove, ply, value, _PS_HASH, _PS_PVS_E_W); 
		return value;		
	}		
	
	g.reptable [g.repindex + ply] = node -> hashcode;

	// fixme: ideetje. opslaan in hash of er 1 legale zet is en movegeneratie 
	// overslaan indien dat het geval is.

	last = _fast_gencapsw (node, first); 	      

#ifdef DEBUG_INSPECT
	int debuglegals = count_legals_w (node, first, last);
#endif
	
	if (last > first) {

		if ((last - first) == 1) { // there is only one move
			move = g.tmoves [first];
			if (legal_move_w (node, move)) {
			
				_fast_dowmove (node, move);
				
				if (!attacked_by_pnbrqk (node, node->wkpos)) {
					
					g.path [ply] = move;

					value = w17_next_search_w (node, alpha, beta, ply + 1, depth - FULL_PLY, checkcheck_w (node, move), legals);				       
					_fast_undowmove (node, move, flags, fifty);

					if (g.stopsearch) {
						print_search (node, alpha, beta, 0, ply, INVALID, _PS_TIME, _PS_PVS_E_W);
						return INVALID;
					}
					
					legals ++;

					if (value > alpha) {
						if (value >= beta) {
							g_assert (value > - INFINITY && value < INFINITY);
						  	print_search (node, alpha, beta, move, ply, value, _PS_CUT, _PS_PVS_E_W);       
							return value;
						}						
						update_pv (node, move, ply); 
					}

					g_assert (value > - INFINITY && value < INFINITY);
					print_search(node, alpha, beta, move, ply, value, _PS_NORMAL, _PS_PVS_E_W); 
					return value;

				} else {
					_fast_undowmove (node, move, flags, fifty);
				}
			}
		} else { // more than one move. sorting becomes an issue. 					
			
			nodes = g.fastnodes;
			hflags = upper_bound;
					
			while (last>first) {
				move = w17_select_capture_w (node, first, last); // fixme: ook proberen zonder deze sortering
				
				g_assert( legals <= 128 ); 

				if (!legal_move_w (node, move)) {								
					continue;
				}
				
				_fast_dowmove (node, move);

				if (!attacked_by_pnbrqk (node, node->wkpos)) {

					g.path [ply] = move; 

					value = w17_next_search_w (node, alpha, beta, ply + 1, depth - FULL_PLY, checkcheck_w (node, move), legals);	
										
					_fast_undowmove (node, move, flags, fifty);
					
					if (g.stopsearch) { 
						return INVALID;
					}
					g_assert (value > - INFINITY && value < INFINITY);
					
					legals++;
					
					if (value > best) {
						if (value > alpha) {
							if (value >= beta) {
								g_assert (value > - INFINITY && value < INFINITY);
								hash_store (node, depth, ply, value, move, g.fastnodes - nodes, lower_bound);
								print_search( node, alpha, beta, move, ply, value, _PS_CUT, _PS_PVS_E_W);
								g.cutoffs_w [legals]++; 
								return value;
							}
							alpha = value;
							hflags = exact;
							update_pv (node, move, ply);
						}
						hashmove = move;
						best = value;
					}
				} else {
					_fast_undowmove (node, move, flags, fifty);
				}
			
			}
			if (legals) { 

#ifdef DEBUG_INSPECT
				if (debuglegals != legals) {		 
					g_print ("hash = %d, nodes = %d, debuglegals = %d, legals = %d, hashmove = %d\n", (int) node -> hashcode,  g.fastnodes, debuglegals, legals, hashmove);
					print_path (ply);
					g_assert_not_reached ();
				}
#endif

				g_assert (best > - INFINITY && best < INFINITY);
				print_search( node, alpha, beta, hashmove, ply, best, _PS_NORMAL, _PS_PVS_E_W); 
				return best;
			}			
		} /* else meerdere captures */ 
	} /* er zijn captures */ 

	// no legal captures
	hflags = upper_bound;
	nodes = g.fastnodes;						
	
	
	// try moves (non captures) from generation
	
	first = ply << 7;  
	last = _fast_gennoncapsw (node, first);

#ifdef DEBUG_INSPECT
	debuglegals = count_legals_w (node, first, last);
#endif
	g_assert(first + 128 > last); 

	
	while (last > first) { 

		move = _fast_selectmove (first, last); // fixme: ook proberen zonder deze sortering 	

		if (!legal_move_w (node, move)) {				 
			continue;
		}
				
		_fast_dowmove (node, move);
		
		if (!attacked_by_pnbrqk (node, node->wkpos)) {
						
			g.path [ply] = move; 

			value = w17_next_search_w (node, alpha, beta, ply + 1, depth - FULL_PLY, checkcheck_w (node, move), legals);

			_fast_undowmove (node, move, flags, fifty);
			
			if (g.stopsearch) { 
				return INVALID;
			}
			g_assert (value > - INFINITY && value < INFINITY);
			legals++;
			
			if (value > best) {
				if (value > alpha) {
					if (value >= beta) {
						g_assert (value > - INFINITY && value < INFINITY);
						hash_store (node, depth, ply, value, move, g.fastnodes - nodes, lower_bound);
						print_search( node, alpha, beta, move, ply, value, _PS_CUT, _PS_PVS_E_W); 
						g.cutoffs_w [legals] ++; 
						return value;
					}
					hflags = exact;
					alpha = value;
					update_pv (node, move, ply); 
				}
				hashmove = move;
				best = value;
			}
		} else {
			_fast_undowmove (node, move, flags, fifty);
		}
				
	} /* while (last > first) */ 

#ifdef DEBUG_INSPECT
	if (debuglegals != legals) {		 
		g_print ("hash = %d, nodes = %d, debuglegals = %d, legals = %d, hashmove = %d\n", (int) node -> hashcode,  g.fastnodes, debuglegals, legals, hashmove);
		print_path (ply);
		g_assert_not_reached ();
	}
#endif	
	
	/* no legal moves --> mate */ 
	if (!legals) { 
		g.pv[ply][ply] = 0; 
		print_search (node, alpha, beta, 0, ply, MATE-ply, _PS_TRIVIAL, _PS_PVS_E_W);  
		return MATE-ply; 
	}
	g_assert (best > - INFINITY && best < INFINITY);
	hash_store (node, depth, ply, best, hashmove, g.fastnodes - nodes, lower_bound);
	print_search (node, alpha, beta, hashmove, ply, best, _PS_NORMAL, _PS_PVS_E_W); 
	return best; 
}






