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

#ifndef W0_SEARCH
#define W0_SEARCH 

int	drive_wtm (TFastNode *, int, int, int);
int	next_search_w (TFastNode * node, int move, int alpha, int beta, int ply, int depth, bool incheck, int truelegals, int legals);
int	MTDf_wtm (TFastNode * node, int value, int depth, int last);
int	next_q_b (TFastNode * node, int alpha, int beta, int ply, int depth, bool incheck, int legals);
int	aspiration_wtm (TFastNode * node, int value, int depth, int last);
bool	q_move_w (TFastNode * node, int move, int ply);
bool	q_move_b (TFastNode * node, int move, int ply);

int	pvs_b (TFastNode* node, int alpha, int beta, int ply, int depth);
int	pvs_w (TFastNode* node, int alpha, int beta, int ply, int depth);

int	pvs_evade_b (TFastNode* node, int alpha, int beta, int ply, int depth);
int	pvs_evade_w (TFastNode* node, int alpha, int beta, int ply, int depth);

int	pvs_root_w (TFastNode* node, int alpha, int beta, int depth, int last);
int	pvs_root_b (TFastNode* node, int alpha, int beta, int depth, int last);

int	quiescence_w (TFastNode * node, int alpha, int beta, int ply, int depth);
int	quiescence_b (TFastNode * node, int alpha, int beta, int ply, int depth);

int	quiescence_evade_w (TFastNode * node, int alpha, int beta, int ply, int depth);
int	quiescence_evade_b (TFastNode * node, int alpha, int beta, int ply, int depth);

int	iterate (TFastNode *);

int	rootdrive_b (TFastNode *, int, int, int);

#endif 







