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

void queen_score (TFastNode * node) 
{
	int 	sq, i;
	
	for (i=0; i<node->wqueens; i++) {
		sq = node->wqueenlist[i];
		node->score += QUEEN_TABLE[sq];
	}
	for (i=0; i<node->bqueens; i++) {
                sq = node->bqueenlist[i];
                node->score -= QUEEN_TABLE[_inverse(sq)];
        }
}

void queen_mobility_score (TFastNode * node)
{
	int 	piece, 
		ssq, 
		tsq, 
		mobility, 
		activity,
		i;
	
	for (i = 0; i < node->wqueens; i++) {
		ssq = node -> wqueenlist [i];
		tsq = t.nextpos [QUEEN][ssq][ssq];
		mobility = 0;
		activity = 0;
		do {
			if (t.pieceattacks[KING][node->bkpos][tsq]) {
				node->score += QUEEN_ON_KING [node->shaky_bking];
			}
			piece = node->matrix[tsq];
			if ((!piece) || (piece>PAWN && piece<KING)) {
				mobility++;
				tsq = t.nextpos [QUEEN][ssq][tsq];
			} else {
				if (piece>BPAWN && !attacked_by_pnbr (node, tsq)) {
					activity++;
				}
				tsq = t.nextdir [QUEEN][ssq][tsq];
			}
		} while (ssq != tsq);
		node->score += QUEEN_MOBILITY [mobility];
		node->score += ACTIVITY [activity];
	}
	for (i = 0; i < node->bqueens; i++) {
		ssq = node -> bqueenlist [i];
		tsq = t.nextpos [QUEEN][ssq][ssq];
		mobility = 0;
		activity = 0;
		do {
			if (t.pieceattacks[KING][node->wkpos][tsq]) {
                                node->score -= QUEEN_ON_KING [node->shaky_wking];
                        }
			piece = node->matrix[tsq];
			if (!piece || (piece>PAWN && piece<KING)) {
				mobility++;
				tsq = t.nextpos [QUEEN][ssq][tsq];
			} else {
				if (piece<KING && piece>PAWN && !attacked_by_PNBR (node, tsq)) {
					activity++;
				}
				tsq = t.nextdir [QUEEN][ssq][tsq];
			}
		} while (ssq != tsq);
		node->score -= QUEEN_MOBILITY [mobility];
		node->score -= ACTIVITY [activity];
	}
}
