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

#ifndef w17searchH
#define w17searchH

#include "fast.h"

int w17_pvs_root_w (TFastNode* node, int alpha, int beta, int depth, int last);
int w17_pvs_root_b (TFastNode* node, int alpha, int beta, int depth, int last);

int w17_pvs_b (TFastNode* node, int alpha, int beta, int ply, int depth);
int w17_pvs_w (TFastNode* node, int alpha, int beta, int ply, int depth);

int w17_pvs_evade_b (TFastNode* node, int alpha, int beta, int ply, int depth);
int w17_pvs_evade_w (TFastNode* node, int alpha, int beta, int ply, int depth);

int w17_next_search_w (TFastNode * node, int alpha, int beta, int ply, int depth, bool incheck, int legals);
int w17_next_search_b (TFastNode * node, int alpha, int beta, int ply, int depth, bool incheck, int legals);

int w17_iterate (TFastNode *);

int w17_root_drive_w (TFastNode * node, int value, int depth, int last);
int w17_root_drive_b (TFastNode * node, int value, int depth, int last);
// int root_drive_b (TFastNode * node, int value, int depth, int last);

int w17_newdepth_sre_w (TFastNode* node, int move, int ply, int depth); 
int w17_newdepth_sre_b (TFastNode* node, int move, int ply, int depth); 

int w17_newdepth_cap_w (TFastNode* node, int move, int first, int last, int ply, int depth); 
int w17_newdepth_cap_b (TFastNode* node, int move, int first, int last, int ply, int depth);
	
int w17_newdepth_silent_w (TFastNode* node, int move, int ply, int depth); 
int w17_newdepth_silent_b (TFastNode* node, int move, int ply, int depth); 

#endif 


