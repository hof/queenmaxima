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

#ifndef BOOK_H
#define BOOK_H

// update the w17 learning table 
void w17_learning_update (int endply, int gameresult);

// update the learning table 
void learning_book_update_score (int winner_first_ply, int endply, int score, int wildnumber);

// lookup stuff in the learning table 
bool database_lookup_learn(_int64 hashcode, int &bookscore, int &book_avoid, int min_score, int min_nodes, int wildnumber);

// mark bad positions 
void learn_avoid (int maxima_firstply, int lastply, bool max_won, int wildnumber); 

int learn_update_score (int myrating, int opprating, bool computer, bool max_won);

#endif

