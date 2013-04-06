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
#include "tables.h"
#include "defines.h"
#include "attack.h"

char INIT_SHAKY[64] = 
{
	0, 0, 0, 2, 2, 0, 0, 0,
	0, 1, 2, 2, 2, 2, 1, 0,
	2, 2, 4, 4, 4, 4, 2, 2,
	4, 4, 6, 6, 6, 6, 4, 4,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8 
};

void set_shakyking_w (TFastNode * node)
{
	int 	col,
		piece,
		tsq;

	if (node->wkpos>h6) {
        	node->shaky_wking = 5;
        	return;
        }

	node->shaky_wking = INIT_SHAKY[node->wkpos];
	
	//pawn shelter in front of king
	col = column(node->wkpos);
	tsq = node->wkpos + 8;
	piece = node->matrix[tsq];
        if (piece!=PAWN && piece!=BISHOP) {
        	node->shaky_wking++;
		if (piece>=BPAWN) {
			node->shaky_wking++;
		}
                tsq += 8;
                piece = node->matrix[tsq];
                if (piece!=PAWN && piece!=BISHOP) {
                	node->shaky_wking++;
			if (piece>=BPAWN) {
                        	node->shaky_wking++;
                	}
                }
	}
	if (col>0) {
		tsq = node->wkpos + 7;
		piece = node->matrix[tsq];
		if (piece!=PAWN && piece!=BISHOP) {
			node->shaky_wking++;
			if (piece>=BPAWN) {
                        	node->shaky_wking++;
                	}
			tsq += 8;
			piece = node->matrix[tsq];
			if (piece!=PAWN && piece!=BISHOP) {
				node->shaky_wking++;
				if (piece>=BPAWN) {
                                	node->shaky_wking++;
                        	}
			}
		}
	}
	if (col<7) {
                tsq = node->wkpos + 7;
                piece = node->matrix[tsq];
                if (piece!=PAWN && piece!=BISHOP) {
                        node->shaky_wking++;
                        if (piece>=BPAWN) {
                                node->shaky_wking++;
                        }
                        tsq += 8;
                        piece = node->matrix[tsq];
                        if (piece!=PAWN && piece!=BISHOP) {
                                node->shaky_wking++;
                                if (piece>=BPAWN) {
                                        node->shaky_wking++;
                                }
                        }
                }
        }
	node->shaky_wking >>= 1;

	if (node->shaky_wking>5) {
		node->shaky_wking=5;
	}
}

void set_shakyking_b (TFastNode * node) 
{
	int     col,
                piece,
                tsq;


        if (node->bkpos<a6) {
        	node->shaky_bking = 5;
                return;
        } 
        	
	node->shaky_wking = INIT_SHAKY[_inverse(node->bkpos)];

	
	//pawn shelter in front of king
        col = column(node->bkpos);
        tsq = node->bkpos - 8;
        piece = node->matrix[tsq];
        if (piece!=BPAWN && piece!=BBISHOP) {
                node->shaky_bking++;
                if (piece && piece<BPAWN) {
                        node->shaky_bking++;
                }
                tsq -= 8;
                piece = node->matrix[tsq];
                if (piece!=BPAWN && piece!=BBISHOP) {
                        node->shaky_bking++;
                        if (piece && piece<BPAWN) {
                                node->shaky_bking++;
                        }
                }
        }
        if (col>0) {
                tsq = node->bkpos - 9;
                piece = node->matrix[tsq];
                if (piece!=BPAWN && piece!=BBISHOP) {
                        node->shaky_bking++;
                        if (piece && piece<BPAWN) {
                                node->shaky_bking++;
                        }
                        tsq -= 8;
                        piece = node->matrix[tsq];
                        if (piece!=BPAWN && piece!=BBISHOP) {
                                node->shaky_bking++;
                                if (piece && piece<BPAWN) {
                                        node->shaky_bking++;
                                }
                        }
                }
        }
	if (col<7) {
                tsq = node->bkpos - 7;
                piece = node->matrix[tsq];
                if (piece!=BPAWN && piece!=BBISHOP) {
                        node->shaky_bking++;
                        if (piece && piece<BPAWN) {
                                node->shaky_bking++;
                        }
                        tsq -= 8;
                        piece = node->matrix[tsq];
                        if (piece!=BPAWN && piece!=BBISHOP) {
                                node->shaky_bking++;
                                if (piece && piece<BPAWN) {
                                        node->shaky_bking++;
                                }
                        }
                }
        }
        node->shaky_bking >>= 1;

        if (node->shaky_bking>5) {
                node->shaky_bking=5;
        }
}

bool pawnwall_ok_ks_w (TFastNode * node) 
{
	if (node -> matrix [g2] == PAWN) {
		if (node -> matrix [h2] == PAWN) {
			return true;
		}		
		if (node -> matrix [f2] == PAWN && node -> matrix [h3] == PAWN) {
			return true;
		}
		
	} else if (node -> matrix [g2] == BISHOP) {
		if (node -> matrix [g3] != PAWN) {
			return false;
		}
		if (node -> matrix [h2] == PAWN) {
			return true;
		}
		if (node -> matrix [f2] == PAWN && node -> matrix [h3] == PAWN) {		
			return true;			
		}					
	}		
	return false;
}

bool pawnwall_ok_qs_w (TFastNode * node) 
{
	if (node -> matrix [c2] != PAWN) {
		return node->matrix [a2] == PAWN &&\
			(node -> matrix [b2] == PAWN ||\
			 (node -> matrix [b2] == BISHOP && node -> matrix [b3] == PAWN));
	}
	if (node -> matrix [a2] != PAWN && node -> matrix [a3] != PAWN) {
		return false;
	}
	if (node -> matrix [b2] == PAWN) {				
		return true;
	}						
	if (node -> matrix [b2] != BISHOP) {
		return false;
	}
	if (node -> matrix [b3] != PAWN) {
		return false;
	}		
	return true;
}

bool pawnwall_ok_qs_b (TFastNode * node) 
{	
	if (node -> matrix [c7] != BPAWN) {
		return node->matrix [a7] == BPAWN &&\
			(node -> matrix [b7] == BPAWN ||\
			 (node -> matrix [b7] == BBISHOP && node -> matrix [b6] == BPAWN));
	}
	if (node -> matrix [a7] != BPAWN && node -> matrix [a6] != BPAWN) {
		return false;
	}
	if (node -> matrix [b7] == BPAWN) {				
		return true;
	}						
	if (node -> matrix [b7] != BBISHOP) {
		return false;
	}
	if (node -> matrix [b6] != BPAWN) {
		return false;
	}		
	return true;
}
						
int evaluate_castle_w (TFastNode * node)
{	
	int value = 0;
	if (_SCW (node)) { // wit kan kort rokeren (rokadestellng is niet om zeep) 	
		if (pawnwall_ok_ks_w (node) && (! attacked_by_pnbrqk (node, f1)) && (!attacked_by_pnbrqk (node, g1))) {			
			value += _short_castle_ok;			
		}	
	} else if (_LCW (node)) { 
		if (pawnwall_ok_qs_w (node) && (! attacked_by_pnbrqk (node, d1)) && (!attacked_by_pnbrqk (node, c1))) {
			value += _long_castle_ok;
		}
	} else { // we kunnen niet meer rokeren. staat de koning goed? (en geen rook ingesloten?)
		if (node -> wkpos == g1 || node -> wkpos == h1 || node -> wkpos == h2) {
			if (pawnwall_ok_ks_w (node)) {
				if (node -> matrix [h1] != ROOK && node -> matrix [h2] != ROOK) {
					value += _short_castled_ok;
				}				
			}
		}
		if (pawnwall_ok_qs_w (node)) {			
			if (node -> wkpos == a1) {
				value += _long_castled_ok;
			}
			if (node -> matrix [a1] != ROOK && node -> matrix [b1] != ROOK) {
				if (node -> wkpos == c1 || node -> wkpos == b1) {
					value += _long_castled_ok;
				}					
			}
		}		
	}
	return value;
}

bool pawnwall_ok_ks_b (TFastNode * node) 
{
	if (node -> matrix [g7] == BPAWN) {
		if (node -> matrix [h7] == BPAWN) {
			return true;
		}		
		if (node -> matrix [f7] == BPAWN && node -> matrix [h6] == BPAWN) {
			return true;
		}
		
	} else if (node -> matrix [g7] == BBISHOP) {
		if (node -> matrix [g6] != BPAWN) {
			return false;
		}
		if (node -> matrix [h7] == BPAWN) {
			return true;
		}
		if (node -> matrix [f7] == BPAWN && node -> matrix [h6] == BPAWN) {
			return true;			
		}					
	}		
	return false;
}
						
int evaluate_castle_b (TFastNode * node)
{	
	int value = 0;
	if (_SCB (node)) { // we kunen kort rokeren.. maar is dat wijs?	
		if (pawnwall_ok_ks_b (node) && (!attacked_by_PNBRQK (node, f8)) && (!attacked_by_PNBRQK (node, g8))) {
			value += _short_castle_ok;			
		}		
	} else if (_LCB (node)) {
		if (pawnwall_ok_qs_b (node) && (!attacked_by_PNBRQK (node, d8)) && (!attacked_by_PNBRQK (node, c8))) {
			value += _long_castle_ok;
		}			
	} else { // we kunnen niet meer rokeren. staat de koning goed? (en geen rook ingesloten?)
		if (node -> bkpos == g8 || node -> bkpos == h8 || node -> wkpos == h7) {
			if (pawnwall_ok_ks_b (node)) {
				if (node -> matrix [h8] != BROOK && node -> matrix [h7] != BROOK) {
					value += _short_castled_ok;
				}				
			}
		}
		if (pawnwall_ok_qs_b (node)) {			
			if (node -> bkpos == a8) {
				value += _long_castled_ok;
			}
			if (node -> matrix [a8] != BROOK && node -> matrix [b8] != BROOK) {
				if (node -> bkpos == c8 || node -> bkpos == b8) {
					value += _long_castled_ok;
				}					
			}
		}	
	}
	return value;
}

///////////
//KING_SCORE (node)
void king_score (TFastNode * node)
{
	int score = 0;
	if ((node->bpieces>3) || (node->bqueens && node->bpieces>2)) {
		set_shakyking_w (node);
		score = evaluate_castle_w (node) + SHAKY_KING[node->shaky_wking];
		node->score += score;	
	} else {
		node->shaky_wking = 0;
		node->score += KING_CENTRE_TABLE [node->wkpos];		
	}
	if ((node->wpieces>3) || (node->wqueens && node->wpieces>2)) {
		set_shakyking_b (node);
		score = evaluate_castle_b (node) + SHAKY_KING[node->shaky_bking];
		node->score -= score;		
	} else {
		node->shaky_bking = 0;
		node->score -= KING_CENTRE_TABLE [_inverse(node->bkpos)];
	}
}








