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

#ifndef HASHCODES_H 
#define HASHCODES_H 

#include "defines.h" 

#define HashPiece(node,pc,sq)       (node->hashcode ^= hashnumbers[pc-1][sq])
#define HashEP(node)                (node->hashcode ^= ephash[node->ep_square])
#define HashSCW(node)               (node->hashcode ^= _LL(0x47bc71a493da706e))
#define HashLCW(node)               (node->hashcode ^= _LL(0x6338be439fd357dc))
#define HashSCB(node)               (node->hashcode ^= _LL(0x6fed622e98f98b7e))
#define HashLCB(node)               (node->hashcode ^= _LL(0xce107ca2947d2d58))

extern  _int64 ephash[64];
extern  _int64 hashnumbers[12][64];

#endif 
