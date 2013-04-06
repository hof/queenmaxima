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

// Nalimov generated EGTB lookup. 


#include "egtb_lookup.h" 

square SqFindKing(square *matrix) 
{
	for (int i=0; i<64; i++) { 
		if (matrix[i]==KING) {
			return i; 
		}
	}
	return XX;
}

square SqFindFirst(square *matrix, char piece)
{
	for (int i=0; i<64; i++) { 
		if (matrix[i]==piece) { 
			return i; 
		}
	}
	return XX; 
}

square SqFindSecond(square *matrix, char piece)
{
	char cnt=0; 
	for (int i=0; i<64; i++) { 
		if (matrix[i]==piece) { 
			cnt++; 
			if (cnt==2) { 
				return i;
			}
		}
	}
	return XX; 
}

square SqFindThird(square *matrix, char piece)
{ 
	char cnt=0;
	for (int i=0; i<64; i++) { 
		if (matrix[i]==piece) { 
			cnt++;
			if (cnt==3) { 
				return i; 
			}
		}
	}
	return XX;
}

square SqFindOne(square *matrix, char piece)
{
	return SqFindFirst(matrix,piece);
}

// #include "tbindex.cpp" 

void loadWsquares(square *sqs, TFastNode *node)
{
	for (int i=0; i<64; i++) {
		sqs[i] = (node->matrix[i] <= KING) ? (node->matrix[i]) : 0; 
	}
}

void loadBsquares(square *sqs, TFastNode *node)
{
	for (int i=0; i<64; i++) { 
		sqs[i] = (node->matrix[i] > KING) ? (node->matrix[i] - KING) : 0;
	}
}

int EGTB_Lookup(TFastNode *node, int ply)
{
    return INVALID;
    /*
	int rgiCounters[10]; 
	int iTb; 
	color side; 
	int fInvert; 
	square psqW[64]; 
	square psqB[64]; 
	square sqEnP; 
	INDEX ind; 

	// init the counters 
	rgiCounters[0] = node->wpawns;
	rgiCounters[1] = node->wknights;
	rgiCounters[2] = node->wbishops; 
	rgiCounters[3] = node->wrooks;
	rgiCounters[4] = node->wqueens;
	rgiCounters[5] = node->bpawns;
	rgiCounters[6] = node->bknights;
	rgiCounters[7] = node->bbishops; 
	rgiCounters[8] = node->brooks;
	rgiCounters[9] = node->bqueens; 

	// check if we have the TB
	iTb = IDescFindFromCounters (rgiCounters); 
	if (iTb == 0) { 
		return INVALID; // fixme: invalid?  
	} 

	if (iTb > 0) { 
		// side to move 
		side = (node->flags & _WTM) ? 0 : 1; 
		fInvert = false; 
		loadWsquares(psqW, node); 
		loadBsquares(psqB, node); 
	} else { 
		side = (node->flags & _WTM) ? 1 : 0; // "other" side 
		fInvert = true; 
		loadWsquares(psqB, node); 
		loadBsquares(psqW, node); 
	}

	if (!FRegistered(iTb, side)) { 
		return L_bev_broken; 
	}

	// ep square 
	sqEnP = (_EPSQ (node)) ? (_EPSQ(node)) : XX; 

	ind = PfnIndCalc(iTb, side) (psqW, psqB, sqEnP, fInvert); 

	
	int val = L_TbtProbeTable (iTb, side, ind);

	// convert score 
	if (val == 0) { 
		return (node->flags & _WTM) ? g.drawscore_wtm : g.drawscore_btm; 
	} 
	int plysleft;
	if (val > 0) { 
		if (val==32767) { 
			return INVALID; 
		}
		plysleft = ply + ((32767-val) << 1);
		return MATE-plysleft;
	} else { 
		plysleft = ply + ((32767+val) << 1);
		return (-MATE)+plysleft; 
	}
	return INVALID; 
     */
}
