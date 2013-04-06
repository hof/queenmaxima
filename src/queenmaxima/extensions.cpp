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
#include "w0.h"
#include "attack.h"
#include "extensions.h"

int depth_wtm (TFastNode * node, int ply, int depth, int move, bool check, int legals)
{
	if (legals==1 && !_CAPTURE(move)) {
		return depth-1;
	}
	if (check) {
		return depth-1 ;
	}
	if (PIECE(move)==PAWN) {
		if (TARGETSQ (move) >= a7) {
			if (TARGETSQ(move)<=h7) {
				return depth-1;
			}
			if (SPECIALCASE(move)==_QUEEN_PROMOTION) {
				return depth-1;
			}
		} else if (TARGETSQ(move)>=a6 && node->bpieces<2 &&
			passed_wpawn(node, TARGETSQ(move))) { 
			return depth-1;
		}
	}
	if (g.nullmate[ply-1]) {
		return depth-1;
	}
	if (_CAPTURE (move) && TARGETSQ(move)==TARGETSQ(g.path[ply-2]) &&
         piece_val[CAPTURED(move)]==piece_val[CAPTURED(g.path[ply-2])] &&
         profcap_w (node, move)) {
                return depth-1;
        }
	if (Q_mates_k (node)) {
		return depth-1;
	}
        return depth-10;
}

int depth_btm (TFastNode * node, int ply, int depth, int move, bool check, int legals)
{
	if (legals==1 && !_CAPTURE(move)) {
		return depth-1;
	}
	if (check) {
		return depth-1;
	}
	if (PIECE(move)==PAWN) {
                if (TARGETSQ (move) <= h2) {
                        if (TARGETSQ(move)>=a2) {
                                return depth-1;
                        }
                        if (SPECIALCASE(move)==_QUEEN_PROMOTION) {
                                return depth-1;
                        }
                } else if (TARGETSQ (move) <= h3 && node->wpieces<2 &&\
			passed_bpawn (node, TARGETSQ (move))) {
			return depth-1;
		}
        }
	if (g.nullmate[ply-1]) {
		return depth-1;
	}
	if (_CAPTURE (move) && TARGETSQ(move)==TARGETSQ(g.path[ply-2]) &&\
         piece_val[CAPTURED(move)]==piece_val[CAPTURED(g.path[ply-2])] &&\
         profcap_b (node, move)) {
                return depth-1;
        }
	if (q_mates_K (node)) {
		return depth-1;
	}
	return depth-10;
}

