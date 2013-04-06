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
#include "hash.h" 
#include "parser.h"
#include "w0_evaluate.h"
#include "attack.h"
#include "tables.h"
#include "defines.h"

///////////
//bool isolated_*pawn (node, sq)
//probeert te weerleggen dat de pion op veld sq een geisoleerde pion is
bool isolated_wpawn (TFastNode * node, int sq)
{
	int 	dy, 
		sq2, 
		piece;

 	for (int i=0; i<node->wpawns; i++) { // find a buddy
        	sq2 = node->wpawnlist[i];
		if (abs (column (sq) - column (sq2)) != 1) {
               		continue; 
        	}
        	dy = row (sq) - row (sq2);
		if (dy==1) {
			return false;
		}
		if (dy<0 || dy>2) {
			continue;
		}
        	if (dy==0) {
			piece = node->matrix [sq+8];
			if (piece!=PAWN && piece!=BPAWN) {
				return false;
			}
			continue;
		}
		//so dy==2
		piece = node->matrix[sq2+8];
		if (piece!=PAWN && piece!=BPAWN) {
			return false;
		}
	}
	return true;
}

bool isolated_bpawn (TFastNode * node, int sq)
{
        int dy, sq2, piece;
        for (int i=0; i<node->bpawns; i++) { // find a buddy
                sq2 = node->bpawnlist[i];
                if (abs (column (sq) - column (sq2)) != 1) {
                        continue;
                }
                dy = row (sq2) - row (sq);
                if (dy==1) {
                        return false;
                }
                if (dy<0 || dy>2) {
                        continue;
                }
                if (dy==0) {
                        piece = node->matrix [sq+8];
                        if (piece!=PAWN && piece!=BPAWN) {
                                return false;
                        }
                        continue;
                }
                //so dy==2
                piece = node->matrix[sq2+8];
                if (piece!=PAWN && piece!=BPAWN) {
                        return false;
                }
        }
        return true;
}

bool true_isolani_white (TFastNode * node, int sq)
{
	int sq2;
	bool doubled = false;
      
        sq2 = sq + 8;
	while (sq2 <= h7) {
		if (node->matrix [sq2] == PAWN) {
			doubled = true;
			break;
		}
		sq2 += 8;
	}
	if (!doubled) {
		return false;
	}		
 	
	for (int i = 0; i < node->wpawns; i++) {
		sq2 = node->wpawnlist [i];
		if (ABS (column (sq) - column (sq2)) == 1) {
			return false;
		}
	}
	return true;
}

bool true_isolani_black (TFastNode * node, int sq)
{
        int sq2;
        bool doubled = false;

        sq2 = sq - 8;
        while (sq2 >= a2) {
                if (node->matrix [sq2] == BPAWN) {
                        doubled = true;
                        break;
                }
		sq2 -= 8;
        }
        if (!doubled) {
                return false;
        }

        for (int i = 0; i < node->bpawns; i++) {
                sq2 = node->bpawnlist [i];
                if (ABS (column (sq) - column (sq2)) == 1) {
                        return false;
                }
        }
        return true;
}

///////////
//bool pawn_in_front_* (node, sq)
//kijkt of er een pion voor het veld sq staat
bool pawn_in_front (TFastNode * node, int sq)
{
        int     sq2 = a7 + column (sq),
                piece;

        while (sq < sq2) {
                piece = node->matrix[sq2];
                if (piece==BPAWN || piece==PAWN) {
                        return true;
                }
                sq2 -= 8;
        }
        return false;
}

bool pawn_in_front_reversed (TFastNode * node, int sq)
{
        int     sq2 = a2 + column (sq),
                piece;

        while (sq > sq2) {
                piece = node->matrix[sq2];
                if (piece==PAWN || piece==BPAWN) {
                        return true;
                }
                sq2 += 8;
        }
	return false;
}

///////////
//bool connected_*passer (node, sq)
//kijkt of naast de passer op veld sq nog een passer staat 
bool connected_wpasser (TFastNode * node, int sq)
{
	int sq2;
	if (column (sq) > 0) { 
		sq2 = sq-1;
		if (node->matrix[sq2]==PAWN && passed_wpawn (node, sq2)) {
			return true;
		}
	}
	if (column (sq) < 7) {
		sq2 = sq+1;
		if (node->matrix[sq2]==PAWN && passed_wpawn (node, sq2)) {
			return true;
		}
	}
	return false;
}

bool connected_bpasser (TFastNode * node, int sq)
{
        int sq2;
        if (column (sq) > 0) {
                sq2 = sq-1;
                if (node->matrix[sq2]==BPAWN && passed_bpawn (node, sq2)) {
                        return true;
                }
        }
        if (column (sq) < 7) {
                sq2 = sq+1;
                if (node->matrix[sq2]==BPAWN && passed_bpawn (node, sq2)) {
                        return true;
                }
        }
        return false;
}

int evaluate_wpawns (TFastNode * node)
{
        int     sq,
                value = 0;
        bool    isolani,
                passer,
                pawninfront;

	node->wpassers = 0;
        for (int i=0; i<node->wpawns; i++) {
                sq = node -> wpawnlist [i];
                pawninfront = pawn_in_front (node, sq);
                passer = (!pawninfront) && passed_wpawn (node, sq);
                isolani = isolated_wpawn (node, sq);
                if (passer) {
			node->wpasserlist[node->wpassers++] = sq;
		} 
		if (!isolani) {
                        if (!passer) { //!isolani && !passer
				value += PAWN_TABLE[sq];
                        } else { // !isolani && passer
                                value += connected_wpasser (node, sq)? CONNECTED_PASSER[sq] : PASSER_TABLE[sq];
                        }
                } else {
			if (true_isolani_white (node, sq)) {
				value -= _true_isolani;
			}
                        if (passer) { //isolani && passer
                                value += ISOLATED_PASSER[sq];
                        } else { //isolani && !passer
                                value += pawninfront? ISOLANI_TABLE[sq] : WEAK_ISOLANI[sq];
                        }
                }
        }
        return value;
}

int evaluate_bpawns (TFastNode * node) 
{
	int	sq, 
		isq, 
		value = 0;
	bool 	isolani, 
		passer, 
		pawninfront;

	node->bpassers=0;
	for (int i=0; i<node->bpawns; i++) {
		sq = node -> bpawnlist [i];
		pawninfront = pawn_in_front_reversed (node, sq);		
		passer = (!pawninfront) && passed_bpawn (node, sq);
		if (passer) {
                        node->bpasserlist[node->bpassers++] = sq;
                }
		isolani = isolated_bpawn (node, sq);
		isq = _inverse (sq);
		if (!isolani) {
			if (!passer) { //!isolani && !passer
				value += PAWN_TABLE[isq];
			} else { // !isolani && passer
				value += connected_bpasser (node, sq)? CONNECTED_PASSER[isq] : PASSER_TABLE[isq];
			}
		} else { 
			if (true_isolani_black (node, sq)) {
				value -= _true_isolani;
			}
			if (passer) { //isolani && passer
				value += ISOLATED_PASSER[isq];
			} else { //isolani && !passer
				value += pawninfront? ISOLANI_TABLE[isq] : WEAK_ISOLANI[isq];
			}
		}
	}
	return value;
}

void passer_score (TFastNode * node)
{
	int i, sq, tsq, piece, dy, block;
	for (i=0; i<node->wpassers; i++) {
		sq = node->wpasserlist[i];
		if (sq < a4) {
			continue;
		}
		if (t.pieceattacks[KING][node->wkpos][sq]) {
			dy = 1 + row(node->wkpos) - row(sq);
			node->score += KINGSUPPORTSPASSER[dy];
		}
		tsq = sq+8;
		block=0;
		do {
			piece = node->matrix[tsq];
			if (piece>KING) {
				if (piece==BKNIGHT) {
					block=5;
					break;
				}
				block += 2;
			} else if (ABB (node, tsq) && !ABW (node, tsq)) {
				block++;
			}
			tsq += 8;
		} while (tsq<=h8 && block<5);
                if (block > 4) { 
                   block = 4; 
                }
		node->score -= BLOCKED_PASSER[block];
        }
	for (i=0; i<node->bpassers; i++) {
                sq = node->bpasserlist[i];
                if (sq > h4) {
                        continue;
                }
                if (t.pieceattacks[KING][node->bkpos][sq]) {
                        dy = 1 + row(sq) - row (node->bkpos);	
                        node->score -= KINGSUPPORTSPASSER[dy];
                }
                tsq = sq-8;
	        block=0;	
                do {
                        piece = node->matrix[tsq];
                        if (piece<=KING) {
                                if (piece==KNIGHT) {
                                        block=5;
                                        break;
                                }
                                block += 2;
                        } else if (ABW (node, tsq) && !ABB (node, tsq)) {
                                block++;
                        }
                        tsq -= 8;
                } while (tsq>=a1 && block<5);
                if (block > 4) { 
                   block = 4; 
                }
                node->score += BLOCKED_PASSER[block];
        }
}

void pawn_score (TFastNode * node)  
{
	int index, score=0;
	index = pawn_hash_index (node);
	if (!inpawnhash (node, index, score)) {
		score = evaluate_wpawns (node);
		score -= evaluate_bpawns (node);
		store_pawnhash (node, index, score);
	}
	node->score += score;
	//passers
	passer_score (node);
	//pawn_mobility
	if (node->matrix[e3] && node->matrix[e2]==PAWN) {
		node->score -= 50;
	}
	if (node->matrix[d3] && node->matrix[d2]==PAWN) {
		node->score -= 50;
	}
	if (node->matrix[c3] && node->matrix[c2]==PAWN) {
		node->score -= 20;
	}
	if (node->matrix[f3] && node->matrix[f2]==PAWN) {
		node->score -= 20;
	}
        if (node->matrix[e6] && node->matrix[e7]==BPAWN) {
                node->score += 50;
        }
        if (node->matrix[d6] && node->matrix[d7]==BPAWN) {
                node->score += 50;
        }
        if (node->matrix[c6] && node->matrix[c7]==BPAWN) {
                node->score += 20;
        }
        if (node->matrix[f6] && node->matrix[f7]==BPAWN) {
                node->score += 20;
        }
}







