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

#ifndef HASH_H
#define HASH_H 

#include "fast.h"

/*
+=========================================+
| Hash tables                             |
+-----------------------------------------+
*/

#define hash_checksum   _LL(0xFFFFFFFFFFFF0000)
#define hash_flags      _LL(0x000000000000FFFF)

#define MBREDUCTION (10 * 1024 * 1024)

#define worthless       0
#define upper_bound     1
#define lower_bound     2
#define exact           3
#define HF_EXTEND       4
#define HF_W17          16
#define HF_SPECIAL      32

#define KEY(x)          ((x & hash_checksum))
#define entrytype(x)    (x & 3)
#define INDEX(x)        (unsigned(x & _hash_index))

#define pawn_hash_index(node)       (unsigned((node->pawncode) & _pindex))

    struct THashEntry {
	_int64 key; //48 bits for checksum, 16 for flags
/*
+---------------------------------------------------------+
| __int64 key: 48 MSB checksum                            |
|              16 LSB flags:                              |
|                 bit 0 and 1: entry type                 |
|                 bit 2:       extend                     |
|                 bit 3:       age                        |
|                 bit 4:       w17                        |
|                 bit 5:       special                    |
+---------------------------------------------------------+
*/

	int move;
	int value;
	int depth;
	char maxq;
	char rootply; 
	unsigned nodes;
#ifdef DEBUG_HASH
	char matrix [64];
#endif
    };

    struct TPawnHashEntry {
	_int64 pawn_hashcode; 
	char wpassers;
        char bpassers;
        char wpasserlist[8];
        char bpasserlist[8];
	int score;
    };

    extern THashEntry         *hashtable_0;   
    extern THashEntry         *hashtable_1;   
    extern TPawnHashEntry     *pawn_hashtable;
    extern int                _hashsize; 
    extern int                _pawnhashsize; 
    extern unsigned int       _hash_index; 
    extern unsigned int       _pindex; 

    void  InitHash(bool smallesthash = false);
    bool  inhash (TFastNode*);
    void  hash_store (TFastNode*, int, int, int, int, unsigned, int);
    bool  hashhit (TFastNode*, int, int, int &, int &, int &, int &); 
    int   hash_move (TFastNode* node);

    bool  inpawnhash (TFastNode * node, int index, int &score);
    void  store_pawnhash (TFastNode * node, int index, int score);

#endif
