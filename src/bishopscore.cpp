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
#include "tables.h"
#include "attack.h" 

void bishop_score (TFastNode * node) 
{
	int sq, i;
	for (i=0; i < node->wbishops; i++) {
		sq = node->wbishoplist[i];
		node->score += BISHOP_TABLE[sq];
	}
	for (i=0; i < node->bbishops; i++) {
                sq = node->bbishoplist[i];
                node->score -= BISHOP_TABLE[_inverse(sq)];
        }
}

void bishop_mobility_score (TFastNode * node)
{	
	int	i,
		ssq, 
		tsq, 
		piece, 
		mobility, 
		activity; 

	for (i = 0; i < node -> wbishops; i++) {
	        ssq = node->wbishoplist[i];
		mobility = 0;
		activity = 0;
		tsq = t.nextpos[BISHOP][ssq][ssq];
		do {
			if (t.pieceattacks[KING][node->bkpos][tsq]) {
				node->score += BISHOP_ON_KING[node->shaky_bking];
			}
			if (tsq >= a7) {
				node->score += _active_bishop;
			}
			piece = node->matrix [tsq];
			if ((!piece) || (piece==QUEEN) || (piece==KNIGHT)) {
				mobility ++;
				tsq = t.nextpos [BISHOP] [ssq] [tsq];
			} else {
				if (piece==PAWN && CENTRUM[tsq] && tsq>ssq) {
					node->score -= _inactive_bishop;
				}	
				if (piece > KING && (piece != BPAWN || (!attacked_by_p (node, tsq)))) {
					activity ++;
				}
				tsq = t.nextdir [BISHOP] [ssq] [tsq];
			}						
		} while (ssq != tsq);
		node->score += BISHOP_MOBILITY [mobility];
		node->score += ACTIVITY [activity];
	}
	for (i = 0; i < node -> bbishops; i++) {
	        ssq = node->bbishoplist[i];	
		mobility = 0;
		activity = 0;
		tsq = t.nextpos[BISHOP][ssq][ssq];
		do {
			if (tsq <= h2) {
				node->score -= _active_bishop;
			}
			if (t.pieceattacks[KING][node->wkpos][tsq]) {
                                node->score -= BISHOP_ON_KING[node->shaky_wking];
                        }
			piece = node -> matrix [tsq];
			if ((!piece) || (piece==BQUEEN) || (piece==BKNIGHT)) {
				mobility ++;
				tsq = t.nextpos [BISHOP] [ssq] [tsq];				
			} else {
				if (piece==BPAWN && CENTRUM[tsq] && tsq<ssq) {
                                        node->score += _inactive_bishop;
                                }
				if (piece < KING && (piece != PAWN || (!attacked_by_P (node, tsq)))) {
					activity ++;
				}
				tsq = t.nextdir [BISHOP] [ssq] [tsq];
			}						
		} while (ssq != tsq);
		node->score -= BISHOP_MOBILITY [mobility];
		node->score -= ACTIVITY [activity];
	}
}
