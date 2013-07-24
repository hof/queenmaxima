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

void knight_score (TFastNode * node)
{
	int	sq, i; 
	for (i = 0; i < node -> wknights; i++) {
	        sq = node->wknightlist[i];
		node->score += KNIGHT_TABLE[sq];
		if (_fast_distance (sq, node->bkpos)<5) {
			node->score += KNIGHT_ON_KING [node->shaky_bking];
		}		
		if (sq > a6 && sq<h6) {
			if (white_octopus (node, sq)) {
				node->score += _octopus;
			} else if (white_small_octopus (node, sq)) {
				node->score += _octopus>>1;
			}
		}
	}
	for (i = 0; i < node->bknights; i++) {
	        sq = node->bknightlist[i];
		node->score -= KNIGHT_TABLE [_inverse(sq)];
		if (_fast_distance (sq, node->wkpos)<5) {
                        node->score -= KNIGHT_ON_KING [node->shaky_bking];
                }
		if (sq < h3 && sq>a3) {
			if (black_octopus (node, sq)) {
				node->score -= _octopus;
			} else if (black_small_octopus (node, sq)) {
				node->score -= _octopus/2;
			} 
		}
	}
}
