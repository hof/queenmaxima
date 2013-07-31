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

#ifndef W0_H
#define W0_H

#define _W0MAXPLY MAXPLY

int next_q_w (TFastNode * node, int alpha, int beta, int ply, int depth, bool incheck, int legals);
int next_search_b (TFastNode * node, int move, int alpha, int beta, int ply, int depth, bool incheck, int truelegals, int legals);
int get_move_from_db (TFastNode * node); 
int selectmove (int first, int & last);
bool try_egtb (TFastNode * node, int ply);

void init_evasions_w (TFastNode * node, int first, int & last, int killerindex, int hashmove);
void init_evasions_b (TFastNode * node, int first, int & last, int killerindex, int hashmove);

bool profcap_w (TFastNode * node, int move);
bool profcap_b (TFastNode * node, int move);

int select_evasion (int first, int & last);
int select_qmove (int first, int & last);

void sort_qmoves_btm (TFastNode * node, int first, int last, int ply);

int newdepth_w (TFastNode * node, int ply, int depth, int move, bool check);
int newdepth_b (TFastNode * node, int ply, int depth, int move, bool check); 

int book_evaluate_w (TFastNode* node, int move, int index);
int book_evaluate_b (TFastNode* node, int move, int index);

bool game_ended (TFastNode * node);

bool trivial_draw_w (TFastNode* node, int ply);
bool trivial_draw_b (TFastNode* node, int ply);

int genrootmoves_w (TFastNode * node);
int genrootmoves_b (TFastNode * node);

int minisearch_w (TFastNode * node, int ssq, int tsq);
int minisearch_b (TFastNode * node, int ssq, int tsq);

int genq_w (TFastNode * node, int index);
int genq_b (TFastNode * node, int index);

bool lookup_books (TFastNode* node, int& score, int& avoid, int tmax, int move);

#endif 
