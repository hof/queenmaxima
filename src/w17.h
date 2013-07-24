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

#ifndef W17_H 
#define W17_H 

extern int state_count[8]; 
/* defines */ 
#define _W17MAXPLY MAXPLY

/* evaluate states */ 
#define W17_PIECES_PAWNS 1
#define W17_PIECES 2
#define W17_WNOPAWNS 3
#define W17_BNOPAWNS 4
#define W17_NOPIECES 5
#define W17_NOWPIECES 6
#define W17_NOBPIECES 7

void	print_state_stats();

/* global tables */ 
extern int sort_w [4096];
extern int sort_b [4096];
extern int sort_cap_w [4096];
extern int sort_cap_b [4096];
extern int sort_eva_w [4096];
extern int sort_eva_b [4096];

/* global functions */ 

#define PIECES_PAWNS 1
#define PIECES 2
#define WNOPAWNS 3
#define BNOPAWNS 4
#define NOPIECES 5
#define NOWPIECES 6
#define NOBPIECES 7

int    w17_eval_state(TFastNode *node);
void   w17_print_state_stats();

bool   w17_lookup_books_wtm (TFastNode* node, int& score, int& avoid, int tmax);
bool   w17_lookup_books_btm (TFastNode* node, int& score, int& avoid, int tmax);

int    w17_genrootmoves_w (TFastNode* node);
int    w17_genrootmoves_b (TFastNode* node);

bool   w17_trivial_draw_w (TFastNode* node, int ply);
bool   w17_trivial_draw_b (TFastNode* node, int ply);

void   w17_init_iterate(TFastNode* node);
void   w17_new_game(); 

int    w17_fast_select_capture_w (TFastNode* node, int first, int & last);
int    w17_fast_select_capture_b (TFastNode* node, int first, int & last);
int    w17_fast_select_noncapture_w (TFastNode* node, int first, int & last, int eval_state);
int    w17_fast_select_noncapture_b (TFastNode* node, int first, int & last, int eval_state);

int    w17_evaluate_w(TFastNode *node, int ply, int eval_state);
int    w17_evaluate_b(TFastNode *node, int ply, int eval_state); 

int    w17_unknown_w (TFastNode *node);
int    w17_unknown_b (TFastNode *node); 

bool   w17_quiescence_check_move_w (TFastNode* node, int move, int ply); 
bool   w17_quiescence_check_move_b (TFastNode* node, int move, int ply); 

int    w17_qd_w (TFastNode * node, int alpha, int beta, int ply); 
int    w17_qd_b (TFastNode * node, int alpha, int beta, int ply); 

int    w17_select_noncapture_w (TFastNode* node, int first, int & last, int eval_state);
int    w17_select_noncapture_b (TFastNode* node, int first, int & last, int eval_state); 

int    w17_select_capture_b (TFastNode* node, int first, int & last);
int    w17_select_capture_w (TFastNode* node, int first, int & last);

int    w17_select_noncap_w_dump (TFastNode* node, int first, int &last, int eval_state);
int    w17_select_noncap_b_dump (TFastNode* node, int first, int &last, int eval_state);

int    w17_select_noncap_w_bigpiecesfirst (TFastNode* node, int first, int &last, int eval_state);
int    w17_select_noncap_b_bigpiecesfirst (TFastNode* node, int first, int &last, int eval_state);

int    w17_select_noncap_w_history (TFastNode* node, int first, int &last, int eval_state); 
int    w17_select_noncap_b_history (TFastNode* node, int first, int &last, int eval_state); 

int    w17_evaluate_pawns_w (TFastNode * node);
int    w17_evaluate_pawns_b (TFastNode * node);

int    w17_max_mate_depth (TFastNode * node);

int    w17_next_search_w (TFastNode * node, int alpha, int beta, int ply, int depth, bool incheck, int legals);
int    w17_next_search_b (TFastNode * node, int alpha, int beta, int ply, int depth, bool incheck, int legals);

#endif 
