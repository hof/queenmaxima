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

#ifndef LEGALITY_H
#define LEGLAITY_H 

#include "fast.h" 

bool	_fast_moveokw(TFastNode*,int);
bool	_fast_moveokb(TFastNode*,int);

bool	inspect_move_legality_b (TFastNode* node, int move);
bool	inspect_move_legality_w (TFastNode* node, int move);

bool	legal_move_w (TFastNode*, int);
bool	legal_move_b (TFastNode*, int);

int	_fast_inspectnode(TFastNode* node);

#endif 

