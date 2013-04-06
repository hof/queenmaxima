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

bool attacked_by_P (TFastNode * node, int sq) 
{	
	int tsq = t.nextdir [BPAWN] [sq] [sq];
	do {
		if (node -> matrix [tsq] == PAWN) {
			return true;
		}
		tsq = t.nextdir [BPAWN] [sq] [tsq];
	} while (sq != tsq);	
	return false;
}

bool attacked_by_p (TFastNode * node, int sq) 
{		
	int tsq = t.nextdir [PAWN] [sq] [sq];
	do {
		if (node -> matrix [tsq] == BPAWN) {
			return true;
		}
		tsq = t.nextdir [PAWN] [sq] [tsq];
	} while (tsq != sq);	
	return false;
}

bool attacked_by_N (TFastNode * node, int sq) 
{
	for (int i = 0; i < node -> wknights; i ++) {
		if (t.pieceattacks [KNIGHT] [node -> wknightlist [i]] [sq]) {
			return true;
		}
	}
	return false;
}

bool attacked_by_B (TFastNode * node, int sq) 
{
	int ssq;
	for (int i = 0; i < node -> wbishops; i ++) {
		ssq = node -> wbishoplist [i];
		if (t.pieceattacks [BISHOP] [ssq] [sq] && nooccs (node, ssq, sq)) {
			return true;
		}
	}
	return false;
}

bool attacked_by_R (TFastNode * node, int sq) 
{
	int ssq;
	for (int i = 0; i < node -> wrooks; i ++) {
		ssq = node -> wrooklist [i];
		if (t.pieceattacks [ROOK] [ssq] [sq] && nooccs (node, ssq, sq)) {
			return true;
		}
	}
	return false;
}

bool attacked_by_Q (TFastNode * node, int sq) 
{
	int ssq;
	for (int i = 0; i < node -> wqueens; i ++) {
		ssq = node -> wqueenlist [i];
		if (t.pieceattacks [QUEEN] [ssq] [sq] && nooccs (node, ssq, sq)) {
			return true;
		}
	}
	return false;
}

bool attacked_by_BRQ (TFastNode * node, int sq)
{
	int ssq, i;
	for (i = 0; i < node -> wbishops; i ++) {
                ssq = node -> wbishoplist [i];
                if (t.pieceattacks [BISHOP] [ssq] [sq] && nooccs (node, ssq, sq)) {
                        return true;
                }
        }
        for (i = 0; i < node -> wrooks; i ++) {
                ssq = node -> wrooklist [i];
                if (t.pieceattacks [ROOK] [ssq] [sq] && nooccs (node, ssq, sq)) {
                        return true;
                }
        }
	for (i = 0; i < node -> wqueens; i ++) {
                ssq = node -> wqueenlist [i];
                if (t.pieceattacks [QUEEN] [ssq] [sq] && nooccs (node, ssq, sq)) {
                        return true;
                }
        }
	return false;
}

bool attacked_by_K (TFastNode * node, int sq) 
{
	return t.pieceattacks [KING] [node -> wkpos] [sq];	
}



bool attacked_by_n (TFastNode * node, int sq) 
{
	for (int i = 0; i < node -> bknights; i ++) {
		if (t.pieceattacks [KNIGHT] [sq] [node -> bknightlist [i]]) {
			return true;
		}
	}
	return false;
}

bool attacked_by_b (TFastNode * node, int sq) 
{
	int ssq;
	for (int i = 0; i < node -> bbishops; i ++) {
		ssq = node -> bbishoplist [i];
		if (t.pieceattacks [BISHOP] [sq] [ssq] && nooccs (node, sq, ssq)) {
			return true;
		}
	}
	return false;
}

bool attacked_by_r (TFastNode * node, int sq) 
{
	int ssq;
	for (int i = 0; i < node -> brooks; i ++) {
		ssq = node -> brooklist [i];
		if (t.pieceattacks [ROOK] [sq] [ssq] && nooccs (node, sq, ssq)) {
			return true;
		}
	}
	return false;
}

bool attacked_by_q (TFastNode * node, int sq) 
{
	int ssq;
	for (int i = 0; i < node -> bqueens; i ++) {
		ssq = node -> bqueenlist [i];
		if (t.pieceattacks [QUEEN] [sq] [ssq] && nooccs (node, sq, ssq)) {
			return true;
		}
	}
	return false;
}

bool attacked_by_k (TFastNode * node, int sq) 
{
	return t.pieceattacks [KING] [sq] [node -> bkpos];	
}

bool attacked_by_PNB (TFastNode * node, int sq) 
{
	return attacked_by_P (node, sq) ||
		attacked_by_N (node, sq) ||
		attacked_by_B (node, sq);
}

bool attacked_by_pnb (TFastNode * node, int sq) 
{
	return attacked_by_p (node, sq) ||
		attacked_by_n (node, sq) ||
		attacked_by_b (node, sq);
}

bool attacked_by_NBRQ (TFastNode * node, int sq)
{
	return attacked_by_N (node, sq) ||
		attacked_by_Q (node, sq) ||
		attacked_by_B (node, sq) ||
		attacked_by_R (node, sq);
}

bool attacked_by_nbrq (TFastNode * node, int sq)
{
	return attacked_by_n (node, sq) ||
		attacked_by_q (node, sq) ||
		attacked_by_b (node, sq) ||
		attacked_by_r (node, sq);
}

bool attacked_by_PNBRQK (TFastNode * node, int sq)
{
	return attacked_by_P (node, sq) || 
		attacked_by_N (node, sq) ||
		attacked_by_K (node, sq) ||
		attacked_by_Q (node, sq) || 
		attacked_by_B (node, sq) ||
		attacked_by_R (node, sq);
}


bool attacked_by_pnbrqk (TFastNode * node, int sq)
{
	return attacked_by_p (node, sq) || 
		attacked_by_n (node, sq) ||
		attacked_by_k (node, sq) ||
		attacked_by_q (node, sq) || 
		attacked_by_b (node, sq) ||
		attacked_by_r (node, sq);
}

bool attacked_by_pnbrq (TFastNode * node, int sq)
{
	return attacked_by_p (node, sq) || 
		attacked_by_n (node, sq) ||		
		attacked_by_q (node, sq) || 
		attacked_by_b (node, sq) ||
		attacked_by_r (node, sq);
}

bool attacked_by_pnbrk (TFastNode * node, int sq)
{
	return attacked_by_p (node, sq) ||
		attacked_by_n (node, sq) ||
		attacked_by_k (node, sq) ||
		attacked_by_b (node, sq) ||
		attacked_by_r (node, sq);
}

bool attacked_by_PNBRK (TFastNode * node, int sq)
{
	return attacked_by_P (node, sq) ||
		attacked_by_N (node, sq) ||
		attacked_by_K (node, sq) ||
		attacked_by_B (node, sq) ||
		attacked_by_R (node, sq);
}

bool attacked_by_PNBRQ (TFastNode * node, int sq)
{
	return attacked_by_P (node, sq) || 
		attacked_by_N (node, sq) ||		
		attacked_by_Q (node, sq) || 
		attacked_by_B (node, sq) ||
		attacked_by_R (node, sq);
}

bool attacked_by_nb (TFastNode * node, int sq) 
{
	return attacked_by_n (node, sq) || attacked_by_b (node, sq);
}

bool attacked_by_NB (TFastNode * node, int sq) 
{
	return attacked_by_N (node, sq) || attacked_by_B (node, sq);
}

bool attacked_by_rqk (TFastNode * node, int sq) 
{
	return attacked_by_k (node, sq) ||  attacked_by_q (node, sq) || attacked_by_r (node, sq);
}

bool attacked_by_RQK (TFastNode * node, int sq) 
{
	return attacked_by_K (node, sq) ||  attacked_by_Q (node, sq) || attacked_by_R (node, sq);
}

bool attacked_by_rq (TFastNode * node, int sq) 
{
	return attacked_by_q (node, sq) || attacked_by_r (node, sq);
}

bool attacked_by_RQ (TFastNode * node, int sq) 
{
	return attacked_by_Q (node, sq) || attacked_by_R (node, sq);
}

bool attacked_by_pnbr (TFastNode * node, int sq) 
{
	return attacked_by_p (node, sq) || attacked_by_n (node, sq) || 
		attacked_by_b (node, sq) || attacked_by_r (node, sq); 
}

bool attacked_by_PNBR (TFastNode * node, int sq) 
{
	return attacked_by_P (node, sq) || attacked_by_N (node, sq) || 
		attacked_by_B (node, sq) || attacked_by_R (node, sq); 
}


bool attacked_by_NBR (TFastNode * node, int sq) 
{
	return attacked_by_N (node, sq) || 
		attacked_by_B (node, sq) || attacked_by_R (node, sq); 
}

bool attacked_by_nbr (TFastNode * node, int sq) 
{
	return attacked_by_n (node, sq) || 
		attacked_by_b (node, sq) || attacked_by_r (node, sq); 
}

bool attacked_by_qk (TFastNode * node, int sq) 
{
	return attacked_by_k (node, sq) || attacked_by_q (node, sq);		
}

bool attacked_by_QK (TFastNode * node, int sq) 
{
	return attacked_by_K (node, sq) || attacked_by_Q (node, sq);		
}

bool blockable_by_p (TFastNode * node, int sq) 
{
	int piece;
	if (sq >= a7) {
		return false;
	}
	sq += 8;
	piece = node -> matrix [sq];
	if (piece == BPAWN) {
		return true;
	}
	if ((piece == 0) && sq >= a6 && sq <= h6) { // block by pushing pawn 2 squares
		return node -> matrix [sq + 8] == BPAWN;
	}			
	return false;
}

bool blockable_by_P (TFastNode * node, int sq) 
{
	int piece;
	if (sq <= h2) {
		return false;
	}
	sq -= 8;
	piece = node -> matrix [sq];
	if (piece == PAWN) {
		return true;
	}
	if ((piece == 0) && sq <= h3 && sq >= a3) { // block by pushing pawn 2 squares
		return node -> matrix [sq - 8] == BPAWN;
	}			
	return false;
}