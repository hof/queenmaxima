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
#include "defines.h"
#include "attack.h"
#include "tables.h"

void rook_score (TFastNode * node) 
{
	int 	sq, tsq, piece, i;

	for (i=0; i<node->wrooks; i++) {
		sq = node->wrooklist[i];
		node->score += ROOK_TABLE[sq];
		if (sq >= a7 && node->bkpos >= a8) {
			node->score += _rook_on_7th;
		}
		if (open_file (node, sq)) {
			node->score += _rook_on_open_file;
		}
		// check for rook activity (this is hashable)
	        tsq = sq+8;
		while (tsq<=h7) {
			piece=node->matrix[tsq];
			if (piece==PAWN) {
				node->score -= _inactive_rook;	
				break;
			}
			if (piece==BPAWN) {
				if (!attacked_by_pnb (node, tsq)) {
					node->score += _active_rook;
				}
				break;
			}
			tsq += 8;
		}		
	}
	for (i=0; i<node->brooks; i++) {
                sq = node->brooklist[i];
                node->score -= ROOK_TABLE[_inverse(sq)];
        	if (sq <= h2 && node->wkpos <= h1) {
                        node->score -= _rook_on_7th;
                }
                if (open_file (node, sq)) {
                        node->score -= _rook_on_open_file;
                }
		// check for rook activity (this is hashable)
                tsq = sq-8;
                while (tsq>=a2) {
                        piece=node->matrix[tsq];
                        if (piece==BPAWN) {
                                node->score += _inactive_rook;
                                break;
                        }
                        if (piece==PAWN) {
                                if (!attacked_by_PNB (node, tsq)) {
                                        node->score -= _active_rook;
                                }
                                break;
                        }
                        tsq -= 8;
                }

	}
	// connected rooks
	if (node->wrooks==2) {
		int 	sq1 = node->wrooklist[0],
			sq2 = node->wrooklist[1];
		if (t.pieceattacks [ROOK][sq1][sq2] && nooccs (node, sq1, sq2)) {
			node->score += row(sq1)==row(sq2) ? _connected_rooks : _verticle_connection; 
		}
	}
	if (node->brooks==2) {
		int 	sq1 = node->brooklist[0],
		    	sq2 = node->brooklist[1];
		if (t.pieceattacks [ROOK][sq1][sq2] && nooccs (node, sq1, sq2)) {
			node->score -= row(sq1)==row(sq2) ? _connected_rooks : _verticle_connection;
		}
	}
}

///////////
//ROOK_MOBILITY_SCORE (node)
void rook_mobility_score (TFastNode * node)
{
	int 	ssq, 
		tsq, 
		piece, 
		activity, 
		mobility,
		i;
	
	for (i = 0; i < node->wrooks; i++) {
	        ssq = node->wrooklist[i];		
		mobility = 0;
		activity = 0;
		tsq = t.nextpos[ROOK][ssq][ssq];
		do {
			if (t.pieceattacks[KING][node->bkpos][tsq]) {
				node->score += ROOK_ON_KING[node->shaky_bking];
			};
			piece = node->matrix[tsq];
			if ((!piece) || (piece==QUEEN) || (piece==ROOK)) {
				mobility ++;
				tsq = t.nextpos[ROOK][ssq][tsq];				
			} else {
				if (piece==PAWN) {
					node->score -= _inactive_rook;
				}	
				if (piece > KING && (piece==BQUEEN || (!attacked_by_pnb (node, tsq)))) {
					activity++;
				}
				tsq = t.nextdir[ROOK][ssq][tsq];
			}						
		} while (ssq != tsq);
	 	node->score += ROOK_MOBILITY [mobility];
		node->score += ACTIVITY [activity];	
	}
	for (i = 0; i < node->brooks; i++) {
	        ssq = node -> brooklist [i];		
		mobility = 0;
		activity = 0;
		tsq = t.nextpos [ROOK] [ssq] [ssq];
		do {
			if (t.pieceattacks[KING][node->wkpos][tsq]) {
				node->score -= ROOK_ON_KING [node->shaky_wking];
			}
			piece = node->matrix[tsq];
			if ((!piece) || (piece==BROOK) || (piece==BQUEEN))  {
				mobility ++;
				tsq = t.nextpos [ROOK] [ssq] [tsq];				
			} else {
				if (piece==BPAWN) {
                                        node->score += _inactive_rook;
                                }
				if (piece < KING && (piece==QUEEN || (!attacked_by_PNB (node, tsq)))) {
					activity++;
				}
				tsq = t.nextdir[ROOK][ssq][tsq];
			}						
		} while (ssq != tsq);
		node->score -= ROOK_MOBILITY [mobility];
		node->score -= ACTIVITY [activity];
	}
}
















