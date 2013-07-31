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
#include <stdio.h> 
#include "hashcodes.h" 
#include "legality.h" 
#include "fast.h"
#include "attack.h"

bool _fast_moveokw (TFastNode* node, int move)  // returns "move would be generated"
{
	int ssq = SOURCESQ (move);
        int piece = PIECE (move);

	if ((!piece) || node -> matrix [ssq] != piece) { 
		return false;
	}
	
	int tsq = TARGETSQ (move);
	
	if (! (move & _LL(4294934528))) { //a normal noncapture
		if (node->matrix[tsq]) { 
			return false;
		}
		if (piece==PAWN) { 
			return !node->matrix [ssq+8];
		}
		return t.pieceattacks [piece] [ssq] [tsq] && (piece == KNIGHT || piece == KING || nooccs(node,ssq,tsq));
	} else if (!SPECIAL(move)) { //a normal capture
		if (node->matrix [tsq] != CAPTURED (move) + KING) { 
			return false;
		}   
		return t.pieceattacks [piece][ssq][tsq] && (piece == PAWN || piece==KNIGHT || piece==KING || nooccs(node,ssq,tsq));
	} else switch (SPECIALCASE (move)) {
	case _SHORT_CASTLE: 
		return _SCW (node) && node -> matrix [g1] == 0 && node -> matrix [f1] == 0;
	case _LONG_CASTLE: 
		return  _LCW (node) && node -> matrix [b1] == 0 && node -> matrix [c1] == 0 && node -> matrix [d1] == 0;
	case _EN_PASSANT:
		return _EPSQ (node) == tsq && node -> matrix [ssq] == PAWN;
	default: // promotion
		if (_CAPTURE (move)) {
			return node -> matrix [ssq] == PAWN && node -> matrix [tsq] == CAPTURED (move)+KING;
		} else {
			return node -> matrix [ssq] == PAWN && node -> matrix [tsq] == 0;
		}
	}
	BOOST_ASSERT (false);
	return false;
}

bool _fast_moveokb(TFastNode* node, int move) { // returns "move would be generated"
	int ssq = SOURCESQ (move),
            piece = PIECE(move);
  
	if (!piece || node -> matrix [ssq] != piece+KING) { 
		return false;
	}

	int tsq = TARGETSQ(move);
	if (! (move & _LL(4294934528))) { //not a special move or a capture
		if (node->matrix[tsq]) { 
			return false;
		}
		if (piece==PAWN) { 
			return !node->matrix[ssq-8];
		}
		return (t.pieceattacks [piece] [tsq] [ssq] && (piece == KNIGHT || piece == KING || nooccs (node, ssq, tsq)));
	} else if (!SPECIAL (move)) { //a normal capture
		if (node->matrix [tsq] != CAPTURED (move)) {
			return false;
		}
		return t.pieceattacks [piece][tsq][ssq] && (piece == PAWN || piece==KNIGHT || piece==KING || nooccs (node, ssq, tsq));
	} else switch (SPECIALCASE (move)) {
	case _SHORT_CASTLE: 
		return _SCB (node) && (node -> matrix [g8] == 0) && (node -> matrix [f8] == 0);
	case _LONG_CASTLE: 
		return  _LCB (node) && (node -> matrix [b8] == 0) && (node -> matrix [c8] == 0) && (node -> matrix [d8] == 0);
	case _EN_PASSANT:
		return (_EPSQ (node) == tsq) && (node -> matrix [ssq] == BPAWN);
	default: // promotion
		if (_CAPTURE (move)) {
			return (node -> matrix [ssq] == BPAWN) && (node -> matrix [tsq] == CAPTURED (move));
		} else {
			return (node -> matrix [ssq] == BPAWN) && (node -> matrix [tsq] == 0);
		}
	}
	BOOST_ASSERT (false);
	return false;
}

bool inspect_move_legality_b (TFastNode* node, int move) 
{
	bool result;
	int flags = node -> flags,
            fifty = node -> fifty;
	if (move == ENCODESCB) {
		return ! attacked_by_PNBRQK (node, e8) && ! attacked_by_PNBRQK (node, f8) && ! attacked_by_PNBRQK (node, g8); 
	}
	if (move == ENCODELCB) {
		return ! attacked_by_PNBRQK (node, e8) && ! attacked_by_PNBRQK (node, d8) && ! attacked_by_PNBRQK (node, c8);
	}
	_fast_dobmove (node, move);
	result = ! attacked_by_PNBRQK (node, node -> bkpos);
	_fast_undobmove (node, move, flags, fifty);	
	return result;
}

bool inspect_move_legality_w (TFastNode* node, int move) 
{
	bool result;
	int flags = node -> flags,
            fifty = node -> fifty;
	if (move == ENCODESCW) {
		return ! attacked_by_pnbrqk (node, e1) && ! attacked_by_pnbrqk (node, f1) && ! attacked_by_pnbrqk (node, g1); 
	}
	if (move == ENCODELCW) {
		return ! attacked_by_pnbrqk (node, e1) && ! attacked_by_pnbrqk (node, d1) && ! attacked_by_pnbrqk (node, c1);
	}
	_fast_dowmove (node, move);
	result = ! attacked_by_pnbrqk (node, node -> wkpos);
	_fast_undowmove (node, move, flags, fifty);	
	return result;
}

bool legal_move (TFastNode* node, int move) {
	if (node -> flags & _WTM) {
		return legal_move_w(node, move);
	} else {
		return legal_move_b(node, move);
	}
}

bool legal_move_w (TFastNode* node, int move) // returns "white move is legal in node"
{
	int sq = SOURCESQ (move),
		offset_ssq_wk,
		offset_ssq_tsq,
		piece = PIECE (move);

	if (piece < KING) { 
		// voor promoties, en_passant en normale zetten geldt:
		offset_ssq_wk = t.direction [sq] [node->wkpos];	
		if (! offset_ssq_wk) {
			return true; // niet op een lijn met de koning. vaak knalt ie er hier meteen weer uit.
		}
		
		offset_ssq_tsq = t.direction [sq] [TARGETSQ (move)];
		if (offset_ssq_wk == offset_ssq_tsq || offset_ssq_wk == - offset_ssq_tsq) {
			return true; // stuk blijft op dezelfde lijn
		}	       
			
		if (! SPECIAL (move) || SPECIALCASE (move) != _EN_PASSANT || (offset_ssq_wk != 1 && offset_ssq_wk != -1)) {
			
			// uitzonderingen met en_passant komen alleen voor als koning en pion op hor. lijn staan (toch?)
			
			// omdat er bij enpassant slaan twee stukken van de horizontale lijn verdwijnen.
			sq += offset_ssq_wk;
			while (sq != node -> wkpos) {
				if (node -> matrix [sq]) {
					return true; // niet gepind want er staat een stuk (w/z) tussen
				}
				sq += offset_ssq_wk;
			}
			sq = t.sqonline [node -> wkpos] [SOURCESQ (move)];
			while (sq != node -> wkpos) {
				BOOST_ASSERT (sq >= 0 && sq <= 63);
				piece = node -> matrix [sq];
				if (piece) {			       
					return piece < BBISHOP || ! t.pieceattacks [piece - KING] [sq] [node->wkpos];
				}	
				sq = t.sqonline [node -> wkpos] [sq];
			} 
			return true;
		}
		// en passant en koning, pion en geslagen pion op 1 horizontale lijn
		int csq = TARGETSQ (move) - 8;
		sq += offset_ssq_wk;
		while (sq != node->wkpos) {
			if (node -> matrix [sq] && (sq != csq)) {
				return true; // niet gepind want er staat een stuk (w/z) tussen
			}
			sq += offset_ssq_wk;
		}	
		sq = t.sqonline [node -> wkpos] [SOURCESQ (move)];
		while (sq != node -> wkpos) {			
			BOOST_ASSERT (sq >= 0 && sq <= 63);
			piece = node -> matrix [sq];
			if (piece && (sq != csq)) {
				return piece < BROOK || ! t.pieceattacks [piece - KING] [sq] [node -> wkpos];
			}
			sq = t.sqonline [node -> wkpos] [sq];
		}	
		return true;
	}
	// de koning zet
	if (attacked_by_pnbrqk (node, TARGETSQ (move))) {
		return false;
	}
	if (! SPECIAL (move)) {
		return true;
	}
	if (attacked_by_pnbrqk (node, e1)) {
		return false;
	}
	if (move == ENCODESCW) {
		return ! attacked_by_pnbrqk (node, f1);
	}
	return ! attacked_by_pnbrqk (node, d1);
}

bool legal_move_b (TFastNode* node, int move) // returns "black move is legal in node"
{
	int sq = SOURCESQ (move),
		offset_ssq_bk,
		offset_ssq_tsq,
		piece = PIECE (move);

	if (piece < KING) { 
		// voor promoties, en_passant en normale zetten geldt:
		offset_ssq_bk = t.direction [sq] [node->bkpos];	
		if (! offset_ssq_bk) {
			return true; // niet op een lijn met de koning. vaak knalt ie er hier meteen weer uit.
		}
		offset_ssq_tsq = t.direction [sq] [TARGETSQ (move)];
		if (offset_ssq_bk == offset_ssq_tsq || offset_ssq_bk == - offset_ssq_tsq) {
			return true; // stuk blijft op dezelfde lijn
		}		
		sq += offset_ssq_bk;	
		if (! SPECIAL (move) || SPECIALCASE (move) != _EN_PASSANT || (offset_ssq_bk != 1 && offset_ssq_bk != -1)) {
			
			// uitzonderingen met en_passant komen alleen voor als koning en pion op hor. lijn staan (toch?)
			// omdat er bij enpassant slaan twee stukken van de horizontale lijn verdwijnen.												
			while (sq != node -> bkpos) {
				if (node -> matrix [sq]) {
					return true; // niet gepind want er staat een stuk (w/z) tussen
				}
				sq += offset_ssq_bk;
			}

			sq = t.sqonline [node -> bkpos] [SOURCESQ (move)];		
			while (sq != node -> bkpos) {				
				BOOST_ASSERT (sq >= 0 && sq <= 63);
				piece = node -> matrix [sq];
				if (piece) {			       
					return piece > QUEEN ||  piece < BISHOP || ! t.pieceattacks [piece] [sq] [node->bkpos];
				}
				sq = t.sqonline [node -> bkpos] [sq];
			}
			return true;
		}
		// en passant en koning, pion en geslagen pion op 1 horizontale lijn
		int csq = TARGETSQ (move) + 8;
		while (sq != node -> bkpos) {
			if (node -> matrix [sq] && sq != csq) {
				return true; // niet gepind want er staat een stuk (w/z) tussen
			}
			sq += offset_ssq_bk;
		}	
		sq = t.sqonline [node -> bkpos] [ SOURCESQ (move)];
		while (sq != node -> bkpos) {
			BOOST_ASSERT (sq >= 0 && sq <= 63);
			piece = node -> matrix [sq];
			if (piece && sq != csq) {
				return piece > QUEEN || piece < ROOK || ! t.pieceattacks [piece] [sq] [node -> bkpos];
			}
			sq = t.sqonline [node -> bkpos] [sq];
		}
		return true;
	}
	// de koning zet
	if (attacked_by_PNBRQK (node, TARGETSQ (move))) {
		return false;
	}
	if (! SPECIAL (move)) {
		return true;
	}
	if (attacked_by_PNBRQK (node, e8)) {
		return false;
	}
	if (move == ENCODESCB) {
		return ! attacked_by_PNBRQK (node, f8);
	}
	return ! attacked_by_PNBRQK (node, d8);	
}

int _fast_inspectnode (TFastNode* node) {
	_int64 hashcode = 0,
		pawncode = 0;
	int i; 

	if ((bool)((node -> hashcode & 1) > 0) != (bool)((node -> flags & _WTM) >0)) {
		return 500;
	}
	if (node->wpawns > 8 || node->wpawns < 0) {
		return 1;
	}
	if (node->wknights + node->wpawns > 10 || node->wknights < 0) {
		return 2;
	}
	if (node->wbishops + node->wpawns > 10 || node->wbishops < 0) {
		return 3;
	}	
	if (node->wrooks + node->wpawns > 10 || node->wrooks < 0) {
		return 4;
	}
	if (node->wqueens + node->wpawns > 10 || node->wqueens < 0) {
		return 5;
	}
	if (node->wkpos < 0 || node->wkpos > 63) {
		return 6;
	}
	if (node->bpawns > 8 || node->bpawns < 0) {
		return 11;
	}
	if (node->bknights + node->bpawns > 10 || node->bknights < 0) {
		return 12;
	}
	if (node->bbishops + node->bpawns > 10 || node->bbishops < 0) {
		return 13;
	}	
	if (node->brooks + node->bpawns > 10 || node->brooks < 0) {
		return 14;
	}
	if (node->bqueens + node->bpawns > 10 || node->bqueens < 0) {
		return 15;
	}
	if (node->bkpos < 0 || node->bkpos > 63) {
		return 16;
	}
	int sq;
        char matrix[64];

	memset (matrix, 0, sizeof (matrix));
	for (i = 0; i < node->wpawns; i++) {
		sq = node->wpawnlist [i];
		hashcode ^= hashnumbers [PAWN - 1] [sq];
		pawncode ^= hashnumbers [PAWN - 1] [sq];
		if (node->index [sq] != i) {
			std::cout << boost::format("node->index[%d] = %d ; i = %d\n") % sq % node->index[sq] % i;
			return 21;
		}			
		matrix [sq] = PAWN;
		if (node->matrix [sq] != PAWN) {
			return 41;
		}
	}
	for (i = 0; i < node->wknights; i++) {		
		sq = node->wknightlist [i];
		hashcode ^= hashnumbers [KNIGHT - 1] [sq];
		if (node->index [sq] != i) {
			return 22;
		}			
		matrix [sq] = KNIGHT;
		if (node->matrix [sq] != KNIGHT) {
			return 42;
		}
	}	
	for (i = 0; i < node->wbishops; i++) {		
		sq = node->wbishoplist [i];
		hashcode ^= hashnumbers [BISHOP - 1] [sq];
		if (node->index [sq] != i) {
			return 23;
		}			
	        matrix [sq] = BISHOP;
		if (node->matrix [sq] != BISHOP) {
			return 43;
		}
	}
	for (i = 0; i < node->wrooks; i++) {
		sq = node->wrooklist [i];
		hashcode ^= hashnumbers [ROOK - 1] [sq];
		if (node->index [sq] != i) {
			return 24;
		}			
		matrix [sq] = ROOK;
		if (node->matrix [sq] != ROOK) {
			return 44;
		}
	}
	for (i = 0; i < node->wqueens; i++) {
		sq = node->wqueenlist [i];
		hashcode ^= hashnumbers [QUEEN - 1] [sq];
		if (node->index [sq] != i) {
			return 25;
		}			
		matrix [sq] = QUEEN;
		if (node->matrix [sq] != QUEEN) {
			return 45;
		}
	}
	matrix [node->wkpos] = KING;
	hashcode ^= hashnumbers [KING - 1] [node -> wkpos];
	if (node->matrix [node->wkpos] != KING) {
		return 46;
	}
	for (i = 0; i < node->bpawns; i++) {
		sq = node->bpawnlist [i];
		hashcode ^= hashnumbers [BPAWN - 1] [sq];
		pawncode ^= hashnumbers [BPAWN - 1] [sq];
		if (node->index [sq] != i) {
			return 31;
		}			
		matrix [sq] = BPAWN;
		if (node->matrix [sq] != BPAWN) {
			return 51;
		}
	}
	for (i = 0; i < node->bknights; i++) {
		sq = node->bknightlist [i];
		hashcode ^= hashnumbers [BKNIGHT - 1] [sq];
		if (node->index [sq] != i) {
			return 32;
		}			
		matrix [sq] = BKNIGHT;
		if (node->matrix [sq] != BKNIGHT) {
			return 52;
		}
	}	
	for (i = 0; i < node->bbishops; i++) {
		sq = node->bbishoplist [i];
		hashcode ^= hashnumbers [BBISHOP - 1] [sq];
		if (node->index [sq] != i) {
			return 33;
		}			
		matrix [sq] = BBISHOP;
		if (node->matrix [sq] != BBISHOP) {
			return 53;
		}
	}
	for (i = 0; i < node->brooks; i++) {
		sq = node->brooklist [i];
		hashcode ^= hashnumbers [BROOK - 1] [sq];
		if (node->index [sq] != i) {
			return 34;
		}			
		matrix [sq] = BROOK;
		if (node->matrix [sq] != BROOK) {
			return 54;
		}
	}
	for (i = 0; i < node->bqueens; i++) {
		sq = node->bqueenlist [i];
		hashcode ^= hashnumbers [BQUEEN - 1] [sq];
		if (node->index [sq] != i) {
			return 35;
		}			
		matrix [sq] = BQUEEN;
		if (node->matrix [sq] != BQUEEN) {
			return 55;
		}
	}
	matrix [node->bkpos] = BKING;
	hashcode ^= hashnumbers [BKING - 1] [node -> bkpos];
	if (node->matrix [node->bkpos] != BKING) {
		return 56;
	}
	if (memcmp (matrix, node->matrix, sizeof (matrix))) {
		return 100;
	}            

	if (_SCW(node)) { 
		hashcode ^= _LL(0x47bc71a493da706e);
	}
	if (_SCB(node)) {
		hashcode ^= _LL(0x6fed622e98f98b7e);
	}
	if (_LCW(node)) {
		hashcode ^= _LL(0x6338be439fd357dc);
	}
	if (_LCB(node)) {
		hashcode ^= _LL(0xce107ca2947d2d58);
	}
	if (_EPSQ(node)) {
		hashcode ^= ephash [_EPSQ (node)];
	}
	if (node-> flags & _WTM) {
		hashcode |= 1;
	} else {
		hashcode &= _LL(0xFFFFFFFFFFFFFFFE);
	}
	if (hashcode != node -> hashcode) {
		std::cout << boost::format("hashcode = %Ld, node -> hashcode = %Ld\n") % hashcode % node -> hashcode;
		return 200;
	}

	if (pawncode != node -> pawncode) {
		std::cout << boost::format("pawncode = %Ld, node -> pawncode = %Ld\n") % pawncode % node -> pawncode;
		return 201;
	}

//	if (_result_value < -CHESS_INF || _result_value > CHESS_INF) {
//		return 300;
//	}
        
	return 0;
}
