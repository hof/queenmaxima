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

#ifndef fastH
#define fastH

#include <string>
#include <iostream>
#include <sstream>
#include <cstring>
#include <stdlib.h> 
#include <cmath>
#include <boost/assert.hpp>
#include <boost/asio.hpp>
#include <boost/format.hpp>

#include <time.h>
#include "defines.h" 
#include "db_pg.h"

/* defines */ 

#define MSECS_PER_SEC       1000
#define MSECS_PER_MIN       (60 * MSECS_PER_SEC)
#define MSECS_PER_HOUR      (60 * MSECS_PER_MIN)

#define NKILLERS            2

#define MAXPLY 50

#define MAX_WQCHECKS 2
#define MAX_BQCHECKS 2

#define a1 0
#define b1 1
#define c1 2
#define d1 3
#define e1 4
#define f1 5
#define g1 6
#define h1 7
#define a2 8
#define b2 9
#define c2 10
#define d2 11
#define e2 12
#define f2 13
#define g2 14
#define h2 15
#define a3 16
#define b3 17
#define c3 18
#define d3 19
#define e3 20
#define f3 21
#define g3 22
#define h3 23
#define a4 24
#define b4 25
#define c4 26
#define d4 27
#define e4 28
#define f4 29
#define g4 30
#define h4 31
#define a5 32
#define b5 33
#define c5 34
#define d5 35
#define e5 36
#define f5 37
#define g5 38
#define h5 39
#define a6 40
#define b6 41
#define c6 42
#define d6 43
#define e6 44
#define f6 45
#define g6 46
#define h6 47
#define a7 48
#define b7 49
#define c7 50
#define d7 51
#define e7 52
#define f7 53
#define g7 54
#define h7 55
#define a8 56
#define b8 57
#define c8 58
#define d8 59
#define e8 60
#define f8 61
#define g8 62
#define h8 63

#define CHESS_INF    9999999
#define INVALID      1234567
#define ILLEGAL      1313131
#define MATE         1000000
#define FULL_PLY     10
#define HALF_PLY     ((FULL_PLY) / 2)
#define MAXDEPTH     (MAXPLY*FULL_PLY)
#define MAXQDEPTH    50
#define EXTENSION    (FULL_PLY)
#define ABS(x)	     std::abs(x)
#define MIN(x,y)	 std::min(x,y)
#define MAX(x,y)	 std::max(x,y)

#define MATE_VALUE(x)      ((x)>= (MATE-1000) && (x)<= (MATE+1000))
#define MATED_VALUE(x)     ((x)<=-(MATE-1000) && (x)>=-(MATE+1000))

#define INVERSE(sq)         (8*(7-row(sq))+column(sq))

#define column(sq)          ((sq) & 7)
#define row(sq)             ((sq) >> 3)

#define PAWN_VALUE   1000
#define KNIGHT_VALUE 3000
#define BISHOP_VALUE 3000
#define ROOK_VALUE   5000
#define QUEEN_VALUE  9000
#define KING_VALUE   30000

#define EMPTY   0
#define PAWN    1
#define KNIGHT  2
#define BISHOP  3
#define ROOK    4
#define QUEEN   5
#define KING    6

#define BPAWN   7
#define BKNIGHT 8 
#define BBISHOP 9
#define BROOK   10
#define BQUEEN  11 
#define BKING   12 

#define UNKNOWN (-MATE/2)
#define IS_UNKNOWN(value) (value < (UNKNOWN+MAXPLY) && value > (UNKNOWN-MAXPLY))

#define FMAX(x, y)          ((x)>(y) ? (x) : (y))
#define FMIN(x, y)          ((x)>(y) ? (y) : (x))

#define _QUEEN_PROMOTION    1
#define _ROOK_PROMOTION     2
#define _BISHOP_PROMOTION   3
#define _KNIGHT_PROMOTION   4
#define _SHORT_CASTLE       5
#define _LONG_CASTLE        6
#define _EN_PASSANT         7
#define _PLUNKMOVE          8
#define _CHECKING           (1 << 30)        

#define SOURCESQ(m)         ((m) & 63)
#define TARGETSQ(m)         (((m)>> 6) & 63)
#define PIECE(m)            (((m)>>12) & 7)
#define CAPTURED(m)         (((m)>>15) & 7)
#define SPECIAL(m)          ((m) & ((1 << 18) | (1 << 19) | (1 << 20) | (1 << 21) | (1 << 22)))

// #define NOT_EP_OR_CASTLE(m) (unsigned (m) < unsigned (_SHORT_CASTLE << 18))

#define SPECIALCASE(m)      (((m) >> 18) & 16383)

#define CASTLE_K          64
#define CASTLE_Q          128
#define CASTLE_k          256
#define CASTLE_q          512
#define _WTM              1024
#define W_ONLY_MATE_SEARCH 2048
#define B_ONLY_MATE_SEARCH 4096
#define _ROOT_WTM         16384
#define NONULL            65536

#define ROOT_WTM(n)   ((n) -> flags & _ROOT_WTM)

#define _CAPTURE(m)          ((m) & (1<<15|1<<16|1<<17))

#define _ENPASSANT(m)        (((m) >> 18) == _EN_PASSANT)
#define _ISPLUNK(m)          (((m) >> 18) == _PLUNKMOVE)

#define _EPSQ(node)          ((node)->flags & 63)
#define _CASTLEW(node)       ((node)->flags & (64|128))
#define _CASTLEB(node)       ((node)->flags & (256|512))
#define _SCW(node)           ((node)->flags & 64)
#define _LCW(node)           ((node)->flags & 128)
#define _SCB(node)           ((node)->flags & 256)
#define _LCB(node)           ((node)->flags & 512)

#define ENCODEMOVE(ssq,tsq,pc)           ((ssq)|((tsq)<<6)|((pc)<<12))
#define ENCODECAPTURE(ssq,tsq,pc,cpc)    ((ssq)|((tsq)<<6)|((pc)<<12)|((cpc)<<15))
#define ENCODESPECIAL(ssq,tsq,pc,cpc,sp) ((ssq)|((tsq)<<6)|((pc)<<12)|((cpc)<<15)|(sp<<18))
#define ENCODEEP(ssq,tsq)                ((ssq)|((tsq)<<6)|(PAWN<<12)|(PAWN<<15)|(_EN_PASSANT<<18))
#define ENCODESCW                        (e1 | (g1 << 6) | (KING << 12) | (_SHORT_CASTLE << 18))
#define ENCODELCW                        (e1 | (c1 << 6) | (KING << 12) | (_LONG_CASTLE << 18))
#define ENCODESCB                        (e8 | (g8 << 6) | (KING << 12) | (_SHORT_CASTLE << 18))
#define ENCODELCB                        (e8 | (c8 << 6) | (KING << 12) | (_LONG_CASTLE << 18))
#define ENCODEPLUNK(tsq,pc)              ((tsq)|((tsq)<<6)|((pc)<<12)|(_PLUNKMOVE << 18))

#define _PS_HASH     0
#define _PS_CUT      1
#define _PS_TRIVIAL  2
#define _PS_NORMAL   3
#define _PS_TIME     4
#define _PS_MAXPLY   5
#define _PS_NULLCUT  6
#define _PS_EVAL     7
#define _PS_UNKNOWN  8
#define _PS_ERROR    9

#define _PS_PVS_W    0
#define _PS_PVS_B    1
#define _PS_PVS_E_W  2
#define _PS_PVS_E_B  3
#define _PS_Q_W      4
#define _PS_Q_B      5
#define _PS_Q_E_W    6
#define _PS_Q_E_B    7

/* structures and datatypes */ 

// the only globals left
extern int _maxd;
extern int window1;
extern int window2;
extern const int avoidscore[4];

struct TRootMove {
	int  move;
	int  matevalue;
	bool bookmove;
	int  bookvalue;
	int  value;
	int  avoid;
	int  nodes;
	int  fifty;
	bool draw;
	bool unknown;
	bool exact_score;
	bool forced_move;
};

typedef boost::date_time::microsec_clock< boost::posix_time::ptime > msecc_t;

struct TTimer {
	long long startTime;

	TTimer() { startTime = current_timestamp(); };

	void start() { startTime = current_timestamp(); };
	void stop() { };
	int elapsed() {
		long long now = current_timestamp();
		long long elapsed = (now - startTime);
		return (int)elapsed;
	};
	long long current_timestamp() {
	    struct timeval te;
	    gettimeofday(&te, NULL); // get current time
	    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // caculate milliseconds
	    return milliseconds;
	};
};

struct TFastNode 
{
  _int64 hashcode;
  _int64 pawncode;
  unsigned char matrix[64];
  unsigned char index[64];
  unsigned char wpawnlist[16];
  unsigned char wpawns;
  unsigned char bpawnlist[16];
  unsigned char bpawns;
  unsigned char wknightlist[20];
  unsigned char wknights;
  unsigned char bknightlist[20];
  unsigned char bknights;
  unsigned char wbishoplist[20];
  unsigned char wbishops;
  unsigned char bbishoplist[20];
  unsigned char bbishops;
  unsigned char wrooklist[20];
  unsigned char wrooks;
  unsigned char brooklist[20];
  unsigned char brooks;
  unsigned char wqueenlist[18];
  unsigned char wqueens;
  unsigned char bqueenlist[18];
  unsigned char bqueens;
  unsigned char wkpos;
  unsigned char bkpos;
  char wpieces;
  char bpieces;
  unsigned char fifty;
  int  flags;
  
  // evaluation stuff
  unsigned char wpassers;
  unsigned char bpassers;
  char wpasserlist[8];
  char bpasserlist[8];
  int score;
  int maximum_mobility_score;
  int shaky_wking;
  int shaky_bking;
};

	bool select_bookmove(TFastNode *node, int mc);
	void store_cutoff (int move, int ply, int depth);

	bool q_mates_K (TFastNode * node);
	bool Q_mates_k (TFastNode * node);

	bool black_octopus (TFastNode * node, int sq);
	bool white_octopus (TFastNode * node, int sq);

	bool black_small_octopus (TFastNode * node, int sq);
	bool white_small_octopus (TFastNode * node, int sq);

	bool passed_wpawn (TFastNode * node, int sq);
	bool passed_bpawn (TFastNode * node, int sq); 

	bool quiet_w (TFastNode * node);
	bool quiet_b (TFastNode * node);

	int count_legals_w (TFastNode * node, int first, int last);
	int count_legals_b (TFastNode * node, int first, int last);
	void update_pv (TFastNode *node, int move, int ply);

	bool open_file (TFastNode * node, int sq);

	bool stalemated_K (TFastNode * node);
	bool stalemated_k (TFastNode * node);

	// global vars
	struct Vars {
	    TTimer  timer;
	    _int64  stoptime; 
	    _int64  maxtime;
            _int64  timeleft; 
	    char    maxply;
	    char    maxq;	
	    bool    stopsearch;
	    unsigned int     fastnodes;
	    int     iteration;
	    int     result_value;
	    int     repindex;
	    bool    checkbook; 
	    bool	crisis;
	    bool    winning;
	    int rootscore; 
	    int last_known_nps; 
	    int tmax; 

		db_pg* dbhandle;

		// callbacks
		//void (*depth_completed_callback)();
		//void (*search_completed_callback)();

		/* add 1 ply for the termintating 0 */
		int		pv [MAXPLY+1] [MAXPLY+1];

		int     path[MAXPLY];
		int     evalpath [MAXPLY];
		int     maxpath [51];
		bool    nullmate [MAXPLY];
		int     nullmate_move [MAXPLY];
		bool    unquiet_ext [MAXPLY];

		int	cutoffs_w [100];
		int	cutoffs_b [100];

	    int drawscore_wtm;
	    int drawscore_btm;

		TRootMove  rootmoves [255];
		int     tmoves [((MAXPLY + 2) << 7) + 128];
		int     megasort [((MAXPLY + 2) << 7) + 128];
		int     tsort [4096]; // index = ssq | tsq << 6 dus: (move & 4095)
		_int64  reptable [150];
	}; 

	struct Tables { 
		bool	edge [64];	
		char    direction [64] [64];
		char    squares_to_edge [64] [64];
		char    nextpos[8][64][64];
		char	nextdir[8][64][64];
		bool	pieceattacks[7][64][64];
		char    sqonline [64] [64]; /* fixme: maybe it's faster to use the 0x88 board representation.*/
		char    queen_kisses_king [64] [64];
	}; 

	extern Vars g; 
	extern Tables t; 

	extern const int piece_val [13];	
	extern const int piece_values [13];


void    add_time ();
void   display_time (_int64 time) ;
bool   claimdraw (TFastNode * node, int draw);
bool   draw_by_rep (TFastNode * node, int ply);
int    _fast_resolve_pv(int *dest);
void   _fast_new_game(); 
void   _fast_AddPiece(TFastNode* node, char piece, unsigned char sq);
void   donullmove (TFastNode * node);
void   undonullmove (TFastNode * node, int oldflags, int oldfifty);
int    _fast_distance (int sq1, int sq2);
void   found_mate_w (TFastNode *node);
void   found_mate_b (TFastNode *node);
void   _fast_clearnode(TFastNode *node);
void   _fast_matrix2node(TFastNode *node,unsigned char matrix[64],int flags,int fifty);
int    _fast_genmovesw(TFastNode*,int);
int    _fast_genmovesb(TFastNode*,int);

	extern char pawn_control [64];

	int get_move_from_db (TFastNode * node);

	int   _fast_gencapsw (TFastNode*, int);
	int   _fast_gencapsb (TFastNode*, int);
	int   _fast_gennoncapsw (TFastNode*, int);
	int   _fast_gennoncapsb (TFastNode*, int);

	void  _fast_dowmove(TFastNode*, int);
	void  _fast_dobmove(TFastNode*, int);
	void  _fast_undowmove(TFastNode*, int, int, int);
	void  _fast_undobmove(TFastNode*, int, int, int);

	void  rootmove_sort (int first, int last);
	
	int   _fast_selectmove(int first, int &last);

	int   _fast_selectcapture (int first, int &last, int sq);

	int   megaselect (int first, int & last);

	void  print_path (int);
	void  print_pv (); 
	int   print_pv_addstr (std::ostringstream &);
	void  print_cutoffs ();

	void  root_new_best (TFastNode *node, int index, int value);
	void  _fast_init_iterate (TFastNode*);

	bool  nooccs (TFastNode* node, int ssq, int tsq);

	bool  checkcheck_w (TFastNode*, int);
	bool  checkcheck_b (TFastNode*, int);

	void  rootmove_sort (int first, int last);

	void  _tgoodmovesw_add(int,int);
	void  _tgoodmovesb_add(int,int);

	void  goodcapsw_add (int, int);
	void  goodcapsb_add (int, int);

	#ifndef PRINT_SEARCH
	#define print_search(a,b,c,d,e,f,g,h) /* debug */
	#define print_search_entry(a, b, c, d) /* debug */ 
	#endif 

	#ifdef PRINT_SEARCH 
	void print_search (TFastNode * node, int alpha, int beta, int move, int ply, int score, int type, int proc);
	void print_search_entry (TFastNode *node, int type, int ply, int depth);
	#endif

#endif












