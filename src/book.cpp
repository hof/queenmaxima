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

#include "fast.h" 
#include "engine_impl.h"
#include "parser.h"

// experimental book learning. 
//
// goals:
// 1. learn wich openings are "good" (have a high win/loss ratio)
// 2. learn openings from the opponent
// 3. avoid positions with score drops
// 4. move at once if this position is searched long enough before
//
// fields in learning table:
//	hashcode, win_score, loss_score, nodes, avoid
//
// hashcode: position-id after a move "move_x"
// win_score: increased if:
//   max won the game and max played move_x (goal 1)
//   opponent won the game and opponent played move_x (goal 1 and 2)
// loss_score: increased if max lost the game and max played move_x (goal 1)
// nodes: max. number of nodes searched (goal 3)
// avoid: counts how many times this score drop is encountered (goal 4)
//  

void w17_learning_update (int endply, int gameresult)
{
    int bugchess_firstply = 1; 
	if (!engine_records[1].maxima_thinking) { /* fixme why starts at 1? */ 
	    bugchess_firstply = 2; 
	}
	
	int score;
	bool addloss = true;
	for (int i = 1; i <= endply; i++) {
	    
	    if (engine_records[i].maxima_thinking) { 
			    score = engine_records [i]. score;	

				g.dbhandle->w17_database_update_book ( engine_records [i]. hashcode_after_move, 
								       gameresult, score);	
				

				if (score < -100 && addloss) {
				    if (i > bugchess_firstply) {
					    g.dbhandle->w17_database_age (engine_records[bugchess_firstply].score,
									      engine_records[bugchess_firstply].hashcode_after_move,3);
					}
					if (i > bugchess_firstply+2) {
					    g.dbhandle->w17_database_age (engine_records[bugchess_firstply+2].score,
									      engine_records[bugchess_firstply+2].hashcode_after_move,2);
					}
					if (i > bugchess_firstply+4) {
					    g.dbhandle->w17_database_age (engine_records[bugchess_firstply+4].score, 
									      engine_records[bugchess_firstply+4].hashcode_after_move,1);
					}
					int j = i; 
					while (j>0 && g.dbhandle->w17_database_avoid(engine_records[j].score, 
										     engine_records[j].hashcode_after_move)) { 
					    j = j - 2; 
					}			
					addloss = false;		       			
				}
				
			}
	}
}    

void learning_book_update_score (int winner_first_ply, int endply, int score, int wildnumber, int opponent_player_id)
{
	// update the positions in the learning table with the score (from 
	// the saved moves array starting at winner_first_ply to endply. 
	// 
	int     n = 1;

    while (n <= endply) {
				
		/* only use nodes if maxima was thinking */
		int winscore=0, lossscore=0, nodes; 
		if (engine_records[n].maxima_thinking) {
			nodes = engine_records[n].nodes;
		} else { 
			n++;
			continue;
		}
	
		//g_print ("updating learning; score += %d; move = ", score);
		//print_move (engine_records[n].move);
		//g_print ("\n");

		if (!g.dbhandle->learn_inbook(engine_records[n].hashcode_after_move, wildnumber,
				opponent_player_id)) {

			//add new moves to book
			if ((n%2) == (winner_first_ply%2)) { //set win_score
				g.dbhandle->learn_update (engine_records[n].hashcode_after_move, score, 0, nodes, 0, wildnumber,
						opponent_player_id, n, engine_records[n].move);
			} else { //set loss_score
				g.dbhandle->learn_update (engine_records[n].hashcode_after_move, 0, score, nodes, 0, wildnumber,
						opponent_player_id, n, engine_records[n].move);
			}
			
			if (nodes) {
				score--;
			}		
 
			if (score <= 0) { //decrease the score and stop adding if score <= 0
				break;
			}
		} else {

			// update the score of the position already in the table 
			char avoid; // actually not used (a dummy).
			g.dbhandle->learn_retrieve(engine_records[n].hashcode_after_move, winscore, lossscore, nodes, avoid,
					wildnumber, opponent_player_id);

			if ((n%2) == (winner_first_ply%2)) { //update win_score
				g.dbhandle->learn_update(engine_records[n].hashcode_after_move, winscore+score, lossscore, nodes,
						avoid, wildnumber, opponent_player_id, n, engine_records[n].move);
			} else { //update loss_score
				g.dbhandle->learn_update(engine_records[n].hashcode_after_move, winscore, lossscore+score, nodes,
						avoid, wildnumber, opponent_player_id, n, engine_records[n].move);
			}
		}
		n ++;
	}
}

// just get the score from the book. 
// we need to know the nps. can use a higher 
// fixed value first to be safe, say 125k
// 
// but ehh, moves from opponents are never used in this way??!!!
// so forget the nps stuff for now. 

bool database_lookup_learn(_int64 hashcode, int &bookscore, int &book_avoid, int min_score, int min_nodes,
		int wildnumber, int opponent_player_id)
{
	int score=0, winscore=0, lossscore=0, nodes=0;
	char avoid = 0;

	if (g.dbhandle->learn_inbook(hashcode, wildnumber, opponent_player_id)) {
		g.dbhandle->learn_retrieve(hashcode, winscore, lossscore, nodes, avoid, wildnumber, opponent_player_id);
		score = winscore - lossscore;
		if (avoid) {
			bookscore = 0;
			book_avoid = avoid;
			return true;
		}
		if (winscore >= min_score) {
			bookscore = score;
			book_avoid = 0;
			return true;
		}
//		if (nodes >= min_nodes) {
//			bookscore = score;
//			book_avoid = 0;
//			return true;
//		}
	}
	return false;
}

void learn_avoid (int maxima_firstply, int lastply, bool max_won, int wildnumber, int opponent_player_id)
{
	// set's avoid for a move. learns not to play this move again 
	//
	// loop over all of maximas moves until a score drop occurs. 
	// we set the avoid score to 1. if the avoid score is n (n>1) 
	// we are going to avoid the n move before this one too. 
	//
	// for now we only avoid on the first score drop. 
	// 
	// because we never play avoided moves and dont know scores 
	// from the opp, this recursive update will never happen 
	// 

	int n = maxima_firstply; 
	int score1, score2;      
	while (n+2 <= lastply) {
		score1 = engine_records[n].score;
		score2 = engine_records[n+2].score;
		if (score2 < -3000 || (score2<0 && !max_won && score1-PAWN_VALUE >= score2)) { 	
			int winscore=0, lossscore=0, nodes=0; 
			char avoid=0; 

			std::cout << "avoid learning; move = ";
			print_move (engine_records[n].move);
            std::cout << "\n";

			if (g.dbhandle->learn_inbook(engine_records[n].hashcode_after_move, wildnumber, opponent_player_id)) {
				g.dbhandle->learn_retrieve(engine_records[n].hashcode_after_move, winscore, lossscore,nodes,
						avoid, wildnumber, opponent_player_id);
				g.dbhandle->learn_update(engine_records[n].hashcode_after_move,winscore,lossscore,nodes,avoid+1,
						wildnumber, opponent_player_id, n, engine_records[n].move);

				//avoid position before this one too 
				n -= 2;
				if (n < maxima_firstply) {
					break;
				}
				if (g.dbhandle->learn_inbook(engine_records[n].hashcode_after_move, wildnumber, opponent_player_id)) {
					g.dbhandle->learn_retrieve (engine_records[n].hashcode_after_move, winscore, lossscore,nodes,
							avoid, wildnumber, opponent_player_id);
					g.dbhandle->learn_update (engine_records[n].hashcode_after_move, winscore, lossscore,nodes, avoid+1,
							wildnumber, opponent_player_id, n, engine_records[n].move);
					break;
				} else {
					g.dbhandle->learn_update (engine_records[n].hashcode_after_move, winscore, lossscore, nodes, 1,
							wildnumber, opponent_player_id, n, engine_records[n].move);
				}
			} else { 
				g.dbhandle->learn_update(engine_records[n].hashcode_after_move,winscore,lossscore,nodes,1,
						wildnumber, opponent_player_id, n, engine_records[n].move);
				break; 
			}
			break;
		}	
		n += 2;
	}  
}

int learn_update_score (int maxima_rating, int opprating, bool computer, bool max_won) 
{
	int score=0;
	if (max_won) { 
		if (opprating >= maxima_rating+100) { //max beat a better opponent. very good!
			score = 4;
		} else if (opprating >= maxima_rating-100) { //max beat equal opponent
			score = 3;
		} else if (opprating >= maxima_rating-300) { //max wins as expected
			score = 2;
		} else {
			score = 1; //max wins very much as expected
		}	
	} else { //max lost the game
		if (opprating >= maxima_rating+100) { //opp. wins as expected
			score = 10;
		} else if (opprating >= maxima_rating-100) { //roughly equal strength
			score = 20;
		} else if (opprating >= maxima_rating-300) { //max should have won the game
			score = 40;
		} else {
			score = 60; //max should very much have won the game
		}	
	}
	if (computer) { //score is doubled because the game is more blunder-proof 
		score <<= 1;
	}
	return score;
} 
