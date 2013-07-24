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

#ifndef DEFINES_H
#define DEFINES_H

/* 
 *    defines for multiple platform support 
 */

#define _LL(longlong_constant)  longlong_constant##LL
#define _int64                  long long int
#define ASSERT(b)               g_assert (b)

/*
 *    defined values
 */

#define _greek_gift              (2*PAWN_VALUE)
#define _more_minor_pieces       (PAWN_VALUE)
#define _connected_on_7th        (PAWN_VALUE/2)
#define _trade_queen             (PAWN_VALUE/2)
#define _king_attack             (PAWN_VALUE/2)
#define _true_isolani            (PAWN_VALUE/2)
#define _king_passer_support     100 
#define _trade_pieces            20
#define _short_castle_ok         100
#define _short_castled_ok        220
#define _long_castle_ok          80
#define _long_castled_ok         160
#define _connected_rooks         10
#define _verticle_connection     80
#define _rook_on_open_file       100
#define _rook_on_7th             200
#define _octopus                 (PAWN_VALUE/2)
#define _blocked_centre_pawn     20
#define _inactive_bishop         100
#define _inactive_rook           50
#define _active_rook             50 
#define _active_bishop           50
#define queen_knight_combo	 50 
/*
 *   defined functions
 */

#ifndef PRINT_EVAL
#define print_eval(node, oldscore, score, str)
#endif

#define _btm_and_root_btm(n)    (((n) -> flags & _WTM == false) && ((n) -> flags & _ROOT_WTM == false))
#define _wtm_and_root_wtm(n)    ((n) -> flags & _WTM && (n) -> flags & _ROOT_WTM)
#define _inverse(sq)            (8 * (7 - row (sq)) + column (sq))

#endif
