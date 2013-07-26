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

bool checkcheck_w (TFastNode* node, int move) { //check if white move checks black king
	char offset,	
		offset_ssq_tsq;
		
	unsigned char piece, sq, csq;

	if (!SPECIAL (move)) {
		piece = PIECE (move);
		sq = TARGETSQ (move);
		if (!t.pieceattacks [piece] [sq] [node->bkpos]) {		       
			goto AFTREKSCHAAK;
		}
		if (piece < BISHOP) {
			return true;
		}	
		// check if no piece in between
		offset = t.direction [sq] [node->bkpos];
		sq += offset;		
		while (sq != node -> bkpos) {
			BOOST_ASSERT (sq >= 0 && sq <= 63);
			if (node->matrix [sq]) {				
				goto AFTREKSCHAAK;
			}			
			sq += offset;
		}		
		return true;		
	} else {
		switch (SPECIALCASE (move)) {
		case _EN_PASSANT:
			sq = TARGETSQ (move);
			if (t.pieceattacks [PAWN] [sq] [node->bkpos]) {
				return true;
			}
			csq = sq - 8;
			offset = t.direction [node -> bkpos] [csq];
			if (offset) {
				sq = node -> bkpos + offset;
				if (node -> matrix [sq]) {
					return false;
				}
				sq = t.sqonline [node -> bkpos] [sq];
				while (sq != node -> bkpos) {				
					BOOST_ASSERT (sq >= 0 && sq <= 63);
					piece = node -> matrix [sq];
					if (piece) {
						if (piece < KING && piece > KNIGHT && t.pieceattacks [piece] [sq] [node->bkpos]) {
							return true;
						}
						goto AFTREKSCHAAK;
					}
					sq = t.sqonline [node -> bkpos] [sq];
				} 
			}
			break;
		case _QUEEN_PROMOTION:
			sq = TARGETSQ (move);
			if (!t.pieceattacks [QUEEN] [sq] [node->bkpos]) {
				goto AFTREKSCHAAK;
			}
			offset = t.direction [sq] [node->bkpos];
			sq += offset;
			while (sq != node->bkpos) {
				piece = node -> matrix [sq];
				if (piece) {
					goto AFTREKSCHAAK;
				}
				sq += offset;
			}
			return true;
		case _SHORT_CASTLE:
			if (!t.pieceattacks [ROOK] [f1] [node->bkpos]) {
				return false;
			}
			offset = t.direction [f1] [node->bkpos];
			sq = f1 + offset;
			do {
				if (node->matrix [sq]) {
					return false;
				}
				sq += offset;
			} while (sq != node->bkpos);
			return true;		       
		case _LONG_CASTLE:
			 if (!t.pieceattacks [ROOK] [d1] [node->bkpos]) {
				return false;
			}
			offset = t.direction [d1] [node->bkpos];
			sq = d1 + offset;
			do {
				if (node->matrix [sq]) {
					return false;
				}
				sq += offset;
			} while (sq != node->bkpos);
			return true;		     		
		case _ROOK_PROMOTION:
			sq = TARGETSQ (move);
			if (!t.pieceattacks [ROOK] [sq] [node->bkpos]) {
				goto AFTREKSCHAAK;
			}
			offset = t.direction [sq] [node->bkpos];
			sq += offset;
			while (sq != node->bkpos) {
				piece = node -> matrix [sq];
				if (piece && sq != SOURCESQ (move)) {
					goto AFTREKSCHAAK;
				}
				sq += offset;
			}
			return true;
		case _BISHOP_PROMOTION:
			sq = TARGETSQ (move);
			if (!t.pieceattacks [BISHOP] [sq] [node->bkpos]) {
				goto AFTREKSCHAAK;
			}
			offset = t.direction [sq] [node->bkpos];
			sq += offset;
			while (sq != node->bkpos) {
				piece = node -> matrix [sq];
				if (piece) {
					goto AFTREKSCHAAK;
				}
				sq += offset;
			}
			return true;
			
		case _KNIGHT_PROMOTION:
			sq = TARGETSQ (move);
			if (!t.pieceattacks [KNIGHT] [sq] [node->bkpos]) {
				goto AFTREKSCHAAK;
			}			
			return true;
		case _PLUNKMOVE:
			piece = PIECE (move);
			sq = TARGETSQ (move);
			if (!t.pieceattacks [piece] [sq] [node->bkpos]) {		       
				return false; /* geen aftrekschaak mogelijk */ 
			}
			if (piece < BISHOP) { /* dus wel attacks */ 
				return true;
			}	
			// check if no piece in between
			offset = t.direction [sq] [node->bkpos];
			sq += offset;		
			while (sq != node -> bkpos) {
				BOOST_ASSERT (sq >= 0 && sq <= 63);
				if (node->matrix [sq]) {				
					return false;
				}			
				sq += offset;
			}		
			return true;		
			
		}
	}
	
 AFTREKSCHAAK:		
	offset = t.direction [node->bkpos] [SOURCESQ (move)];
	if (!offset) {
		return false;
	}
	offset_ssq_tsq = t.direction [SOURCESQ (move)] [TARGETSQ (move)];	
	if ((offset_ssq_tsq == offset) || (offset_ssq_tsq == (- offset))) {
		return false;
	}
	sq = node -> bkpos + offset;
	if (node -> matrix [sq]) {
		return false;
	}
	sq = t.sqonline [node->bkpos] [sq];
	while (sq != node -> bkpos) {		
		BOOST_ASSERT (sq >= 0 && sq <= 63);
		piece = node->matrix [sq];
		if (piece) {
			return piece < KING && piece > KNIGHT && t.pieceattacks [piece] [sq] [node->bkpos];
		}	
		sq = t.sqonline [node -> bkpos] [sq];
	}	
	return false;	
}



bool checkcheck_b (TFastNode* node, int move) 
{ //check if black move checked white king
	char offset,	
		offset_ssq_tsq;

        unsigned char piece,
                csq,
                sq;
        
	if (!SPECIAL (move)) {
		piece = PIECE (move);
		sq = TARGETSQ (move);
		if (!t.pieceattacks [piece] [node->wkpos] [sq]) {		       
			goto AFTREKSCHAAK;
		}
		if (piece < BISHOP) {
			return true;
		}	
		// check if no piece in between
		offset = t.direction [sq] [node->wkpos];
		sq += offset;		
		while (sq != node -> wkpos) {
			if (node->matrix [sq]) {				
				goto AFTREKSCHAAK;
			}			
			sq += offset;
		}		
		return true;		
	} else {
		switch (SPECIALCASE (move)) {
		case _EN_PASSANT:
			sq = TARGETSQ (move);
			if (t.pieceattacks [PAWN] [node->wkpos] [sq]) {
				return true;
			}
			csq = sq + 8;
			offset = t.direction [node -> wkpos] [csq];
			if (offset) {
				sq = node -> wkpos + offset;
				if (node -> matrix [sq]) {
					break;
				}
				sq = t.sqonline [node->wkpos] [sq];
				while (sq != node -> wkpos) {				
					BOOST_ASSERT (sq >= 0 && sq <= 63);
					piece = node -> matrix [sq];
					if (piece) {
						if (piece > BKNIGHT && t.pieceattacks [piece - KING] [sq] [node->wkpos]) {
							return true;
						}
						goto AFTREKSCHAAK;
					}
					sq = t.sqonline [node -> wkpos] [sq];
				}
			}
			break;
		case _QUEEN_PROMOTION:
			sq = TARGETSQ (move);
			if (!t.pieceattacks [QUEEN] [sq] [node->wkpos]) {
				goto AFTREKSCHAAK;
			}
			offset = t.direction [sq] [node->wkpos];
			sq += offset;
			while (sq != node->wkpos) {
				piece = node -> matrix [sq];
				if (piece) {
					goto AFTREKSCHAAK;
				}
				sq += offset;
			}
			return true;
		case _SHORT_CASTLE:
			if (!t.pieceattacks [ROOK] [f8] [node->wkpos]) {
				return false;
			}
			offset = t.direction [f8] [node->wkpos];
			sq = f8 + offset;
			do {
				if (node->matrix [sq]) {
					return false;
				}
				sq += offset;
			} while (sq != node->wkpos);
			return true;		       
		case _LONG_CASTLE:
			 if (!t.pieceattacks [ROOK] [d8] [node->wkpos]) {
				return false;
			}
			offset = t.direction [d8] [node->wkpos];
			sq = d8 + offset;
			do {
				if (node->matrix [sq]) {
					return false;
				}
				sq += offset;
			} while (sq != node->wkpos);
			return true;		     		
		case _ROOK_PROMOTION:
			sq = TARGETSQ (move);
			if (!t.pieceattacks [ROOK] [sq] [node->wkpos]) {
				goto AFTREKSCHAAK;
			}
			offset = t.direction [sq] [node->wkpos];
			sq += offset;
			while (sq != node->wkpos) {
				piece = node -> matrix [sq];
				if (piece) {
					goto AFTREKSCHAAK;
				}
				sq += offset;
			}
			return true;
		case _BISHOP_PROMOTION:
			sq = TARGETSQ (move);
			if (!t.pieceattacks [BISHOP] [sq] [node->wkpos]) {
				goto AFTREKSCHAAK;
			}
			offset = t.direction [sq] [node->wkpos];
			sq += offset;
			while (sq != node->wkpos) {
				piece = node -> matrix [sq];
				if (piece) {
					goto AFTREKSCHAAK;
				}
				sq += offset;
			}
			return true;
			
		case _KNIGHT_PROMOTION:
			sq = TARGETSQ (move);
			if (!t.pieceattacks [KNIGHT] [sq] [node->wkpos]) {
				goto AFTREKSCHAAK;
			}			
			return true;

		case _PLUNKMOVE:
			piece = PIECE (move);
			sq = TARGETSQ (move);
			if (!t.pieceattacks [piece] [node->wkpos] [sq]) {		       
				return false;
			}
			if (piece < BISHOP) {
				return true;
			}	
			// check if no piece in between
			offset = t.direction [sq] [node->wkpos];
			sq += offset;		
			while (sq != node -> wkpos) {
				if (node->matrix [sq]) {				
					return false;
				}			
				sq += offset;
			}		
			return true;		
		}
	}
	
 AFTREKSCHAAK:	
	sq = SOURCESQ (move);	
	offset = t.direction [node->wkpos] [sq];
	if (!offset) {
		return false;
	}
	offset_ssq_tsq = t.direction [sq] [TARGETSQ (move)];	
	if ((offset_ssq_tsq == offset) || (offset_ssq_tsq == (- offset))) {
		return false;
	}
       	sq = node -> wkpos + offset;
	if (node -> matrix [sq]) {
		return false;
	}	
	sq = t.sqonline [node -> wkpos] [sq];
	while (sq != node -> wkpos) {
		piece = node->matrix [sq];
		if (piece) {
			return piece > BKNIGHT && t.pieceattacks [piece - KING] [sq] [node->wkpos];
		}
		sq = t.sqonline [node -> wkpos] [sq];
	}
	return false;	
}
