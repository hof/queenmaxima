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
#include "fast.h"  
#include "hash.h"
#include "w0_pawneval.h" 
#include "parser.h"
#include "w0_evaluate.h"
#include "attack.h"
#include "tables.h"
#include "defines.h"
#include "kingscore.h"
#include "rookscore.h"
#include "bishopscore.h"
#include "knightscore.h"
#include "queenscore.h"

///////////
//PRINT_EVAL
//shows a part of the evaluation on the screen

void print_score (TFastNode * node, int &oldscore, gchar * str)
{
	if (node->score != oldscore) {
		g_print ("*** ");		
		::print_score (node->score-oldscore);
		g_print ("...");
		g_print ("%s", str);
		oldscore = node->score;
	}
} 

///////////
//DRAW_HEURISTIC

bool draw_heuristic (TFastNode * node) 
{
	int wpp, bpp;
	if (node->wpawns==0 && node->score>0) {
		if (node->wpieces==2 && node->wknights==2) {
                        return true;
                }
		wpp = (node->wknights+node->wbishops)*3+node->wrooks*5+node->wqueens*10;
		bpp = (node->bknights+node->bbishops)*3+node->brooks*5+node->bqueens*10;
		if (wpp-bpp<=3) {
			return true;
		}
	}
	if (node->bpawns==0 && node->score<0) {
                if (node->bpieces==2 && node->bknights==2) {
                        return true;
                }
		wpp = (node->wknights+node->wbishops)*3+node->wrooks*5+node->wqueens*10;
                bpp = (node->bknights+node->bbishops)*3+node->brooks*5+node->bqueens*10;
		if (bpp-wpp<=3) {
			return true;
		}
        }
	return false;
}

void drawish_score (TFastNode * node)
{
	if (node->wpieces != 1) {
		return;
	}
	if (node->bpieces != 1) {
		return;
	}
	if (node->wqueens && node->bqueens) {
                node->score >>= 1; //queen-endgame
	} else if (node->wbishops && node->bbishops && \
		WHITE_SQUARE[node->wbishoplist[0]]!=WHITE_SQUARE[node->bbishoplist[0]]) {
		node->score >>= 1; //opposite bishops
	}
}

///////////
// MATERIAL_SCORE (node)
// evaluates the following terms:
// * having two more minor pieces
// * bishop-pair
// * trapped bishop at a2,h2,a7 or h7   

void material_score (TFastNode * node) 
{
	int 	wminors = node->wknights + node->wbishops,
		bminors = node->bknights + node->bbishops;
	
	if (wminors != bminors && node->wqueens==node->bqueens) {
		if (wminors - bminors > 1) {
			node->score += _more_minor_pieces;
		} else if (bminors - wminors > 1) {
			node->score -= _more_minor_pieces;
		} 
	}
	if (node->wqueens && node->wknights) {
		node->score += queen_knight_combo;
	}          
	if (node->bqueens && node->bknights) {
		node->score -= queen_knight_combo;
	}
	if (node -> wbishops != node -> bbishops) {
		if (node -> wbishops > 1) {
			node->score += BISHOP_PAIR [node->bpieces + node->bpawns];		
		}		
		if (node -> bbishops > 1) {
			node->score -= BISHOP_PAIR [node->wpieces + node->wpawns];		
		}		
	}
	if (node->matrix[a7]==BISHOP && (node->matrix[b7]==BPAWN || node->matrix[b6]==BPAWN)) {
		node->score -= node->matrix[c7]==BPAWN? _greek_gift : _greek_gift>>1;
	}
	if (node->matrix[h7]==BISHOP && (node->matrix[g7]==BPAWN || node->matrix[g6]==BPAWN)) {
		node->score -= node->matrix[f7]==BPAWN? _greek_gift : _greek_gift>>1;
	}
	if (node->matrix[a2]==BBISHOP && (node->matrix[b2]==PAWN || node->matrix[b3]==PAWN)) {
		node->score += node->matrix[c2]==PAWN? _greek_gift : _greek_gift>>1;
	}
	if (node->matrix[h2]==BBISHOP && (node->matrix[g2]==PAWN || node->matrix[g3]==PAWN)) {
		node->score += node->matrix[f2]==PAWN? _greek_gift : _greek_gift>>1;
	}
	if (node->matrix[a8]==KNIGHT && node->matrix[a7]==BPAWN &&\
		 (!attacked_by_P (node, node->matrix[c7])) && ABB(node, node->matrix[c7])) {
		node->score -= PAWN_VALUE;
	}
	if (node->matrix[h8]==KNIGHT && node->matrix[h7]==BPAWN &&\
                 (!attacked_by_P (node, node->matrix[f7])) && ABB(node, node->matrix[f7])) {
                node->score -= PAWN_VALUE;
        }
	if (node->matrix[a1]==BKNIGHT && node->matrix[a2]==PAWN &&\
                 (!attacked_by_p (node, node->matrix[c2])) && ABW(node, node->matrix[c2])) {
                node->score += PAWN_VALUE;
        }
	if (node->matrix[h1]==BKNIGHT && node->matrix[h2]==PAWN &&\
                 (!attacked_by_p (node, node->matrix[f2])) && ABW(node, node->matrix[f2])) {
                node->score += PAWN_VALUE;
        }
}        

///////////
//TRADE_SCORE (node)

void trade_score (TFastNode * node)   
{
	if (node->score > -KNIGHT_VALUE && node->score < KNIGHT_VALUE) {
		return;
	}
	if (node->wpawns==0 && node->bpawns==0) {
		return;
	}	
	if (node->score >= PAWN_VALUE) {
		node->score += 10*_trade_pieces-(node->bpieces+node->wpieces)*_trade_pieces;
		if (node->bqueens==0) {
			node->score += _trade_queen;
		}
	} else {
		node->score -= 10*_trade_pieces-(node->bpieces+node->wpieces)*_trade_pieces;
		if (node->wqueens==0) {
			node->score -= _trade_queen;
		}
	}
}

///////////
//WHITE_SCORE

int white_score (TFastNode * node, int ply, int alpha, int beta) 
{
	//int oldscore = 0;
	node->score=0;
	king_score (node);
	//print_score (node, oldscore, "king_score\n");
	material_score (node);
	//print_score (node, oldscore, "material_score\n");
	pawn_score (node);
	//print_score (node, oldscore, "pawn_score\n");
	knight_score (node);
	//print_score (node, oldscore, "knight_score\n");
	bishop_score (node);
	//print_score (node, oldscore, "bishop_score\n");
	rook_score (node);
	//print_score (node, oldscore, "rook_score\n");
	queen_score (node);
	//print_score (node, oldscore, "queen_score\n");
	trade_score (node);
	//print_score (node, oldscore, "trade_score\n");
	if (draw_heuristic (node)) {
		return g.drawscore_wtm;
	}
	drawish_score (node);
	//print_score (node, oldscore, "opp. bishop score\n");
	if (node->score+node->maximum_mobility_score<=alpha) {
		return alpha;
	}
	if (node->score-node->maximum_mobility_score>=beta) {
		return beta;
	}
	int score = node->score;
	bishop_mobility_score (node);
	//print_score (node, oldscore, "bishop mobility\n");
	rook_mobility_score (node);
	//print_score (node, oldscore, "rook mobility\n");
	queen_mobility_score (node);
	//print_score (node, oldscore, "queen mobility\n");
	if (ABS (node->score-score) > node->maximum_mobility_score) {
		node->maximum_mobility_score = ABS (node->score-score);
	}
	//g_print ("eval grand total: %d -> %d\n", node->score, node->score|127);
	//abort();
	return node->score | 127;
}

///////////
//BLACK_SCORE

int black_score (TFastNode * node, int ply, int alpha, int beta) 
{
	node->score = 0;
	king_score (node);
	material_score (node);
	pawn_score (node);
	knight_score (node);
	bishop_score (node);
	rook_score (node);
	queen_score (node);
	trade_score (node);
	if (draw_heuristic (node)) {
		return g.drawscore_btm;
	}
	drawish_score (node);
 	if (-node->score+node->maximum_mobility_score<=alpha) {
              return alpha;
        }
        if (-node->score-node->maximum_mobility_score>=beta) {
              return beta;
        }
	int score = node->score;
	bishop_mobility_score (node);
	rook_mobility_score (node);
	queen_mobility_score (node);
	if (ABS (node->score-score) > node->maximum_mobility_score) {
                node->maximum_mobility_score = ABS (node->score-score);
        }
	return -node->score | 127;
}
