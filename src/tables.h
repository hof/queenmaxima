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

#ifndef TABLES_H
#define TABLES_H

extern int WEAK_ISOLANI[64];
extern int ISOLANI_TABLE[64];
extern int PAWN_TABLE[64];
extern int ISOLATED_PASSER[64];
extern int PASSER_TABLE[64];
extern int CONNECTED_PASSER[64];
extern int BISHOP_MOBILITY[15];
extern int ROOK_MOBILITY[15];
extern int QUEEN_MOBILITY[15];
extern int ACTIVITY [8];
extern int KING_CENTRE_TABLE [64];
extern int PASSER_ON_ITS_OWN [64]; 
extern int PAWN_ON_ITS_OWN [64]; 
extern int PASSER_WITH_BUDDY [64];
extern int KNIGHT_TABLE [64];
extern int BISHOP_TABLE[64];
extern int SHAKY_KING[6];
extern char CENTRUM[64];
extern int ROOK_TABLE [64];
extern int QUEEN_TABLE [64];
extern char WHITE_SQUARE [64];
extern int BISHOP_PAIR [16];
extern int ROOK_ON_KING[6];
extern int BISHOP_ON_KING[6];
extern int KNIGHT_ON_KING[6];
extern int QUEEN_ON_KING[6];
extern int KINGSUPPORTSPASSER[3];
extern int BLOCKED_PASSER[5];
#endif
