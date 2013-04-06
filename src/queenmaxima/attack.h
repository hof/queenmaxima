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


#ifndef attackH
#define attackH

#define ABW(node, sq) (attacked_by_P(node,sq)||attacked_by_N(node,sq)||\
			attacked_by_B(node,sq)||attacked_by_R(node,sq)||\
			attacked_by_Q(node,sq)||attacked_by_K(node,sq))
#define ABB(node, sq) (attacked_by_p(node,sq)||attacked_by_n(node,sq)||\
			attacked_by_b(node,sq)||attacked_by_r(node,sq)||\
			attacked_by_q(node,sq)||attacked_by_K(node,sq))

	bool attacked_by_N (TFastNode * node, int sq);
	bool attacked_by_n (TFastNode * node, int sq);
	bool attacked_by_B (TFastNode * node, int sq);
	bool attacked_by_b (TFastNode * node, int sq);

	bool attacked_by_Q (TFastNode * node, int sq);
	bool attacked_by_q (TFastNode * node, int sq);
	bool attacked_by_PNBRQK (TFastNode * node, int sq);
	bool attacked_by_pnbrqk (TFastNode * node, int sq);
	bool attacked_by_pnb (TFastNode * node, int sq);
	bool attacked_by_PNB (TFastNode * node, int sq);
	bool attacked_by_nb (TFastNode * node, int sq);
	bool attacked_by_NB (TFastNode * node, int sq);
	bool attacked_by_p (TFastNode * node, int sq);
	bool attacked_by_P (TFastNode * node, int sq);
	bool attacked_by_rq (TFastNode * node, int sq);
	bool attacked_by_RQ (TFastNode * node, int sq);
	bool attacked_by_rqk (TFastNode * node, int sq);
	bool attacked_by_qk (TFastNode * node, int sq);
	bool attacked_by_QK (TFastNode * node, int sq);
	bool attacked_by_RQK (TFastNode * node, int sq);
	bool attacked_by_pnbr (TFastNode * node, int sq);
	bool attacked_by_PNBR (TFastNode * node, int sq);
	bool attacked_by_NBRQ (TFastNode * node, int sq);
	bool attacked_by_nbrq (TFastNode * node, int sq);
	bool attacked_by_pnbrq (TFastNode * node, int sq);
	bool attacked_by_PNBRQ (TFastNode * node, int sq);
	bool attacked_by_nbr (TFastNode * node, int sq);
	bool attacked_by_NBR (TFastNode * node, int sq);
	bool blockable_by_p (TFastNode * node, int sq);
	bool blockable_by_P (TFastNode * node, int sq);
	bool attacked_by_k (TFastNode * node, int sq);
	bool attacked_by_K (TFastNode * node, int sq);
	bool attacked_by_R (TFastNode * node, int sq);
	bool attacked_by_r (TFastNode * node, int sq);
	bool attacked_by_pnbrk (TFastNode * node, int sq);
	bool attacked_by_PNBRK (TFastNode * node, int sq);
	bool attacked_by_BRQ (TFastNode * node, int sq);

#endif

