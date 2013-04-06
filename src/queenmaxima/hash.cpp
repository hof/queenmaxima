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

// #include <math.h> 
#include "hash.h" 
#include "legality.h"  
#include "parser.h" 
#include "hash.h" 
#include "engine_impl.h" 

THashEntry         *hashtable_0;    
THashEntry         *hashtable_1;     
TPawnHashEntry     *pawn_hashtable; 
// egtb_hash_entry    *egtb_hash;       
int                _hashsize = 0; 
int                _pawnhashsize= 0; 
unsigned int       _hash_index = 0; 
unsigned int       _pindex = 0; 

struct TSystemInfo {
	long uptime; 
	unsigned long loads[3]; 
	unsigned long totalram; 
	unsigned long freeram; 
	unsigned long sharedram; 
	unsigned long bufferram; 
	unsigned long totalswap; 
	unsigned long freeswap; 
	unsigned short procs; 
	char _f[22]; 
} system_info; 

void InitHash(bool smallesthash) 
{

	if (smallesthash) { 
		_hashsize = 1; 
		_pawnhashsize = 1; 
	} else { 


		/* get total system memory */ 
		// if (sysinfo((struct sysinfo *)&system_info)) { 
		//	g_error("could not determine system memory limits"); 
		// }

		srand( 1 /* system_info.uptime */ ); 
		
		//int mbreduction = MBREDUCTION; 
//		if (system_info.totalram > (200*1024*1024)) { 
//			mbreduction = 0; 
//		}

//		g_print("engine system memory: %dMB\n", (int) ( system_info.totalram / (1024 * 1024))); 
		
		//_hashsize = (int) (log (40 * ((system_info. totalram - mbreduction) / (sizeof (THashEntry) * 2) / 100)) / log (2)); 
		//_pawnhashsize = (int) (log(5 * ((system_info. totalram - mbreduction) / sizeof (TPawnHashEntry) / 100)) / log (2)); 
		
		_hashsize = 20;
		_pawnhashsize = 20;

		//		g_print("engine hashtable: %d MB (%d entries)\n", 
		//(int)(((1<<_hashsize)*2*sizeof(THashEntry))/(1024*1024)),1<<_hashsize); 
		
		//g_print("engine pawn-hashtable: %d MB (%d entries)\n", 
		//(int) ((1<<_pawnhashsize)*sizeof(TPawnHashEntry))/(1024*1024),1<<_pawnhashsize);

	}

	/* ---- alloc hashtables */ 

	hashtable_0 = g_new0(THashEntry,(1<< _hashsize)); 
	hashtable_1 = g_new0(THashEntry,(1<< _hashsize)); 
	pawn_hashtable = g_new0(TPawnHashEntry, (1<<_pawnhashsize)); 

	/* ---- init van hashsize afhankelijke vars ---- */ 

	_hash_index = ((1 << _hashsize) - 1); 
	_pindex = ((1<<_pawnhashsize)-1);
}


bool inpawnhash (TFastNode * node, int index, int &score)
{ 	 	
	int i;
	if (pawn_hashtable [index].pawn_hashcode == node->pawncode) { 
		score = pawn_hashtable [index].score; 		
		node->wpassers = pawn_hashtable[index].wpassers;
		node->bpassers = pawn_hashtable[index].bpassers;
		for (i=0; i<node->wpassers; i++) { 
			node->wpasserlist[i] = pawn_hashtable[index].wpasserlist[i];
		}
		for (i=0; i<node->bpassers; i++) {
			node->bpasserlist[i] = pawn_hashtable[index].bpasserlist[i];
		}
		return true;
	}
	return false; 
}

void store_pawnhash(TFastNode * node, int index, int score) 
{ 
	int i;
	pawn_hashtable [index]. pawn_hashcode = node -> pawncode;
	pawn_hashtable [index]. score = score;
	for (i=0; i<node->wpassers; i++) {
        	pawn_hashtable[index].wpasserlist[i] = node->wpasserlist[i];
        }
        for (i=0; i<node->bpassers; i++) {
        	pawn_hashtable[index].bpasserlist[i] = node->bpasserlist[i];
        }
}

bool inhash (TFastNode* node) 	
{	
	 	
	unsigned int index = INDEX (node -> hashcode);
	
	g_assert (index >= 0 && index <= _hash_index);
	
#ifdef DEBUG_HASH
	bool result = KEY (node -> hashcode) == KEY (hashtable_1 [index]. key);
        if (result) {
		if (memcmp (node->matrix, hashtable_1 [index]. matrix, 64)) {			
			g_assert_not_reached ();
		}		
		return true;
	}
	result = KEY (node -> hashcode) == KEY (hashtable_0 [index]. key);
	if (result) {
		if (memcmp (node->matrix, hashtable_0 [index]. matrix, 64)) {
			g_assert_not_reached ();			
		}		
		return true;
	}
#endif

	if (KEY (node -> hashcode) == KEY (hashtable_1 [index]. key) ||
	    KEY (node -> hashcode) == KEY (hashtable_0 [index]. key)) { 				
		return true; 
	}
	
	return false; 
}

int hash_move (TFastNode* node) 
{
	int index = INDEX (node -> hashcode);
	if (KEY (node -> hashcode) == KEY (hashtable_0 [index]. key)) {
		return hashtable_0 [index]. move;
	}
	return hashtable_1 [index]. move;
}


bool hashhit (TFastNode* node, int depth, int ply, int & value, int & move, int & alpha, int & beta) 
{	
	int index = INDEX (node -> hashcode);			

	if (KEY (node -> hashcode) == KEY (hashtable_0 [index]. key)) {
		
		move = hashtable_0 [index]. move;
		
		
		if (depth <= hashtable_0 [index]. depth && hashtable_0 [index]. maxq >= g.maxq 
			&& hashtable_0[index].rootply == engine_rootply) {
						
			value = hashtable_0 [index]. value;
			
			g_assert (value > - INFINITY && value < INFINITY);
			
			if (MATE_VALUE (value)) {
				value -= ply;
			} else if (MATED_VALUE (value)) {
				value += ply;
			}
			
			switch (entrytype (hashtable_0 [index]. key)) {
			case lower_bound:				
#ifdef PRINT_SEARCH
				g_print("inhash t0  hc=%10x n=%d: move=",(int)node->hashcode,g.fastnodes);
				print_move(move); 
				g_print(" value=%d ",value); 
#endif
				if (value >= beta) {
#ifdef PRINT_SEARCH 
					g_print("lower_bound (value>=beta) return true\n"); 
#endif 
					return true;
				}
				return false; //fixme
				if (value > alpha) {
#ifdef PRINT_SEARCH
					g_print("lower_bound (value>alpha) adjusting alpha\n"); 
#endif
					alpha = value;
				} else { 
#ifdef PRINT_SEARCH
					g_print("lower_bound -no bound change-\n");
#endif
				}
				return false;
			case upper_bound:
#ifdef PRINT_SEARCH
				g_print("inhash t0 hc=%10x n=%d: move=",(int)node->hashcode,g.fastnodes);
				print_move(move); 
				g_print(" value=%d ",value); 
#endif
				if (value <= alpha) {
#ifdef PRINT_SEARCH 
					g_print("upper_bound (value<=alpha) return true\n"); 
#endif 
					return true;
				}
				 return false; //fixme
				if (value < beta) {
#ifdef PRINT_SEARCH
					g_print("upper_bound (value<beta) adjusting beta\n"); 
#endif
					beta = value;
				} else { 
#ifdef PRINT_SEARCH 
					g_print("upper_bound -no bound change-\n"); 
#endif
				}
				return false;
			case exact:
#ifdef PRINT_SEARCH
				g_print("inhash t0  hc=%10x n=%d: move=",(int)node->hashcode,g.fastnodes);
				print_move(move); 
				g_print(" value=%d exact\n",value); 
#endif
				g.pv [ply + 1] [ply + 1] = 0;				
				update_pv (node, move, ply);
				return true;
			}
		} 
		
	} else { 
		
		move = hashtable_1 [index]. move;
		if (depth <= hashtable_1 [index]. depth && hashtable_1 [index]. maxq >= g.maxq
			&& hashtable_1[index].rootply == engine_rootply) {
						
			value = hashtable_1 [index]. value;
			
			g_assert (value > - INFINITY && value < INFINITY);
			
			if (MATE_VALUE (value)) {
				value -= ply;
			} else if (MATED_VALUE (value)) {
				value += ply;
			}

			switch (entrytype (hashtable_1 [index]. key)) {
			case lower_bound:				
#ifdef PRINT_SEARCH
				g_print("inhash t1  hc=%10x n=%d: move=",(int)node->hashcode,g.fastnodes);
				print_move(move); 
				g_print(" value=%d ",value); 
#endif
				if (value >= beta) {
#ifdef PRINT_SEARCH 
					g_print("lower_bound (value>=beta) return true\n"); 
#endif 
					return true;
				}
				return false; //return
				if (value > alpha) {
#ifdef PRINT_SEARCH
					g_print("lower_bound (value>alpha) adjusting alpha\n"); 
#endif
					alpha = value;
				} else { 
#ifdef PRINT_SEARCH
					g_print("lower_bound -no bound change-\n");
#endif
				}
				return false;
			case upper_bound:
#ifdef PRINT_SEARCH
				g_print("inhash t1  hc=%10x n=%d: move=",(int)node->hashcode,g.fastnodes);
				print_move(move); 
				g_print(" value=%d ",value); 
#endif
				if (value <= alpha) {
#ifdef PRINT_SEARCH 
					g_print("upper_bound (value<=alpha) return true\n"); 
#endif 
					return true;
				}
				return false; //fixme
				if (value < beta) {
#ifdef PRINT_SEARCH
					g_print("upper_bound (value<beta) adjusting beta\n"); 
#endif
					beta = value;
				} else { 
#ifdef PRINT_SEARCH
					g_print("upper_bound -no bound change-\n");
#endif
				}					
				return false;
			case exact:
#ifdef PRINT_SEARCH
				g_print("inhash t1  hc=%10x n=%d: move=",(int)node->hashcode,g.fastnodes);
				print_move(move); 
				g_print(" value=%d exact\n",value); 
#endif
				g.pv [ply + 1] [ply + 1] = 0;
				update_pv (node, move, ply);
				return true;
			}
		} 			
	}
	return false;
}



void hash_store (TFastNode* node, int depth, int ply, int value, int move, unsigned nodes, int bound) {
/*
  +----------------------------------------------+
  | Store node in hashtable using the TwoBig1    |
  | replacement strategy                         |
  +----------------------------------------------+
*/
  g_assert (value > - INFINITY && value < INFINITY);

#ifdef PRINT_SEARCH
	g_print("hash_store: hc=%10x m= ",(int)node->hashcode); 
	print_move(move);
	g_print(" value=%d depth=%d nodes=%d bound=",value,depth,nodes); 
	switch(bound) { 
	case lower_bound: 
		g_print("lower_bound\n"); 
		break; 
	case upper_bound:
		g_print("upper_bound\n"); 
		break; 
	case exact:
		g_print("exact\n"); 
	}
#endif
	
#ifdef DEBUG_INSPECT

	if (ABS(value) > INFINITY) {
		g_print ("%d", value);
	}
	g_assert (value > - INFINITY && value < INFINITY);
	if (node -> flags & _WTM) {		

		if (SPECIAL(move) && SPECIALCASE(move)==_PLUNKMOVE) { 
			
			g_assert( node -> matrix[TARGETSQ(move)] == 0 ); 

		} else { 
	
			if (node -> matrix [SOURCESQ (move)] != PIECE (move)) {
				g_print("\nnode -> matrix [SOURCESQ (move)] != PIECE (move)\npath = "); 
				print_path (ply);
				g_print("\nmove = "); 
				print_move(move); 
				g_print("fast_inspectnode returns: %d\n",_fast_inspectnode(node)); 
				g_assert_not_reached ();
			}
			
			if ( ! legal_move_w (node, move)) { 
				g_print("\n!legal_move_w (node, move)\npath = "); 
				print_path (ply);
				g_print("\nmove = "); 
				print_move(move); 
				g_print("fast_inspectnode returns: %d\n",_fast_inspectnode(node)); 
				g_assert_not_reached ();
			}
		}
	} else { /* black to move */ 		 

		if (SPECIAL(move) && SPECIALCASE(move)==_PLUNKMOVE) { 
			g_assert( node -> matrix[TARGETSQ(move)] == 0); 
		} else {

			if (node -> matrix [SOURCESQ (move)] != PIECE (move) + KING || ! legal_move_b (node, move)) {
				g_print("path = ");
				print_path (ply);
				g_print("; move = "); 
				print_move(move); 
				g_print (" nodes = %d", g.fastnodes);
				g_assert_not_reached ();
			}	
		}

	}
	g_assert (ply > 0 && ply < MAXPLY);
	g_assert (depth < MAXPLY * FULL_PLY);
	g_assert (nodes >= 0 && nodes < (unsigned) g.fastnodes);
	g_assert (bound >= 0 && bound <= 5);
	g_assert (value > - INFINITY && value < INFINITY);

#endif
	int index = INDEX (node -> hashcode);

/*
  nieuw 20 okt 1999: 
  exact niet replacen door !exact tenzij:
  * nieuwe entry meer nodes en ook exact
  * nieuwe entry meer nodes en hogere depth
  !exact replacen door exact als:
  * meer nodes
  * gelijke of hogere depth
  
*/
// 04-feb-00 (key & AGE) vervangen door .rootply != engine_rootply 

//	if (hashtable_0 [index]. key & HF_AGE || 
	if (hashtable_0 [index]. rootply != engine_rootply || 
	    ((entrytype (hashtable_0 [index]. key) != exact) && (nodes > hashtable_0 [index]. nodes || (bound == exact && depth >= hashtable_0 [index]. depth))) ||	  
	    ((entrytype (hashtable_0 [index]. key) == exact) && (nodes > hashtable_0 [index]. nodes && (bound == exact || depth > hashtable_0 [index]. depth)))) {
		
//	if (nodes > hashtable_0 [index]. nodes) {
		if (KEY (node -> hashcode) != KEY (hashtable_0 [index]. key)) {
		
			hashtable_1 [index] = hashtable_0 [index];
			
		}
		hashtable_0 [index]. key = KEY (node -> hashcode) | bound;		
		hashtable_0 [index]. depth = depth;
		hashtable_0 [index]. value = value;
		hashtable_0 [index]. move = move;
		hashtable_0 [index]. nodes = nodes;	
		hashtable_0 [index]. maxq = g.maxq;
		hashtable_0 [index]. rootply = engine_rootply; 
		hashtable_0 [index]. value = value;
		if (MATE_VALUE (value)) {
			hashtable_0 [index]. value += ply;
		} else if (MATED_VALUE (value)) {
			hashtable_0 [index]. value -= ply;
		}
#ifdef DEBUG_HASH
		memcpy (hashtable_0 [index]. matrix, node -> matrix, 64);
#endif
	} else {
		hashtable_1 [index]. key = KEY (node -> hashcode) | bound;		
		hashtable_1 [index]. depth = depth;
		hashtable_1 [index]. value = value;
		hashtable_1 [index]. move = move;
		hashtable_1 [index]. nodes = nodes;		
		hashtable_1 [index]. maxq = g.maxq;
		hashtable_1 [index]. rootply = engine_rootply; 
		hashtable_1 [index]. value = value;
		if (MATE_VALUE (value)) {
			hashtable_1 [index]. value += ply;
		} else if (MATED_VALUE (value)) {
			hashtable_1 [index]. value -= ply;
		}	
#ifdef DEBUG_HASH
		memcpy (hashtable_1 [index]. matrix, node -> matrix, 64);
#endif
	}	
}










