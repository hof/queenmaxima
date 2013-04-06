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

#ifndef egtb_lookupH
#define egtb_lookupH
   
#include "fast.h" 

typedef int  INDEX;
typedef char square; 

#define NEW NEW
#define XX 64

typedef unsigned long ULONG;

// nalimov's functions 

extern "C" int IInitializeTb(char *pszPath); 
extern "C" int FTbSetCacheSize (void *buffer, ULONG cbSize);

// maxima's functions 

int EGTB_Lookup(TFastNode *node, int ply);

#endif
