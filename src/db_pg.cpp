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

#include <stdlib.h> 
#include <string.h> 
#include <stdio.h> 
#include "fast.h"
#include "parser.h"
#include "config.h"

#include <libpq-fe.h>
#include "db_pg.h"

using namespace std;
using namespace boost;

bool db_pg::connect(const char* hostname, const char* username, const char* password, const char *database, const unsigned int port)
{
    std::cerr << "db Postgres connecting to " << hostname << "\n"; 
    
    const char *serverversion; 
    
    std::ostringstream conninfo; 
    conninfo << "host = " << hostname << " dbname = " << database << " user = " << username << " password = " << password << " port = 5432 "; 
    
    m_connection = PQconnectdb(conninfo.str().c_str()); 
    
    if (PQstatus(m_connection) == CONNECTION_OK) { 
     
        std::cerr << "Connected to Postgres\n";
        
        serverversion = PQparameterStatus(m_connection, "server_version");
        std::cerr << "Server version: " << serverversion << "\n"; 
        m_connected = true; 
        
        return true; 
    } else { 
        m_connected = false; 
        std::cerr << "Unable to connect to Postgress\n"; 
        return false;
    }        
}

void db_pg::close() {     
    PQfinish(m_connection);     
}

// database_save_player 
// 
// looks up a player in the player table. if it doesn't find one, it inserts a player.
// returns the ID of the player. Updates the ENTERED (if new) and LASTGAME dates. 
// 
long db_pg::save_player(const char* name, const char* titles, const char* server)
{
	// determine the current date
 	time_t now = time(NULL); 
	tm *t = localtime(&now); 
	
	std::ostringstream query;
	query << "SELECT player_id FROM player WHERE name = '" << name << "'";
	
	// look up the player
	long player_id; 
	PGresult *result = PQexec(m_connection, query.str().c_str());
	if (PQresultStatus(result) != PGRES_TUPLES_OK) {
		cerr << "query1. save player\n";
		close();
	    exit(1);
	}
        
	if (PQntuples(result) == 0) { // no rows
	    // no rows in result set 		
		PQclear(result);
            
		// insert the player
	    query.str(""); 
	    query << "INSERT INTO player (name,titles,server,entered) VALUES ('" << name << "','" << titles;
	    query << "','" << server << "','" << (1900+t->tm_year) << "-" << t->tm_mon+1 << "-" << t->tm_mday << " ";
	    query << t->tm_hour << ":" <<  t->tm_min << ":" << t->tm_sec << "') RETURNING player_id";
	    result = PQexec(m_connection,query.str().c_str());
	    if ((PQresultStatus(result) != PGRES_TUPLES_OK)) {
	    	cerr << "query3. save_player\n";
	    	close();
	    	exit(1);
	    }
	    
		player_id = atoi(PQgetvalue(result,0,0));
		PQclear(result);

		std::cout << "new player id=" << player_id << " name: " << name << std::endl;

	} else {
		player_id = atoi(PQgetvalue(result,0,0));
		PQclear(result);
	}
	
	// update the stats for this player 
	query.str(""); 
	query << "UPDATE player SET titles = '" << titles << "', lastgame = '" << (1900+t->tm_year) << "-" << t->tm_mon+1 << "-" << t->tm_mday << " ";
	query << t->tm_hour << ":" <<  t->tm_min << ":" << t->tm_sec << "' WHERE player_id = " << player_id;

	result = PQexec(m_connection,query.str().c_str());
	if ((PQresultStatus(result) != PGRES_COMMAND_OK)) { 
	    cerr << "query4. save_player\n";
		close();
		exit(1);
	}
        
	PQclear(result);
 	return player_id;     
}
 
long db_pg::save_game(const int whitename, const int blackname, const int flags,
				 const int wildnumber, const int whiterating, 
				 const int blackrating, const int basetime, const int inc, 
				 const int rating_type, const int rated, string result_code)
{
    
    // determine the current date 
    time_t now = time(NULL); 
    tm *t = localtime(&now); 

    std::ostringstream query; 
    query << "INSERT INTO game (date, white_id, black_id, flags, wildnumber, whiterating, blackrating, basetime, increment, rating_type, rated, result_code) VALUES('";
    query << 1900+t->tm_year << "-" <<  t->tm_mon+1 << "-" << t->tm_mday << " " << t->tm_hour << ":" << t->tm_min << ":" << t->tm_sec << "',";
    query << whitename << "," << blackname << "," << flags << "," << wildnumber << ",";
    query << whiterating << "," << blackrating << "," << basetime << "," << inc << ",";
    query << rating_type << "," << rated <<  ",'" << result_code << "') RETURNING game_id ";

    PGresult *result = PQexec(m_connection,query.str().c_str());

    if ((PQresultStatus(result) != PGRES_TUPLES_OK)) {
        cerr << "query1. save_game\n";
        close();
        exit(1); 	
    }

    long res = atoi(PQgetvalue(result,0,0));
    PQclear(result); 

    std::cout << "saved game: " << res << std::endl;

    return res;        
}


long db_pg::save_move(const int game_id, const int ply, const int move, const int thinktime)
{
    std::ostringstream query;
    query << "INSERT INTO move (game_id,ply,move,movetime) VALUES (" << game_id << "," << ply << "," << move;
    query << "," << thinktime << ")";

    PGresult *result = PQexec(m_connection,query.str().c_str());
    if ((PQresultStatus(result) != PGRES_COMMAND_OK)) { 
        cerr << "query1. save_move\n";
        close();
        exit(1); 	
    }
    
    // return mysql_insert_id(m_connection); 
    return 0;  
}

void db_pg::update_stats(const char* tablename, const int player_id, const int rating, const int gameresult)
{
    // gameresult is calculated in main.cpp - it is NOT the same as the "gameresult" 
    // we get gtom ICC. It's different because we need to know if we won or lost. 
  
    // gameresult == 0 --> we lost
    // gameresult == 1 --> draw 
    // gameresult == 2 --> bugchess wins
    
    int win = 0, draw = 0, loss = 0, score = 0, minr = rating, maxr = rating, avg_rating = rating; 
    long total_rating = rating; 

    std::ostringstream query;
    query << "SELECT win,loss,draw,minr,maxr,total_rating,avg_rating  FROM " << tablename << " WHERE player_id = " << player_id; 
    
    PGresult *result = PQexec(m_connection,query.str().c_str());
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {  
        cerr << "query1. update_stats\n";
        close();
        exit(1);
    }
    
    if (PQntuples(result) == 0) { 
        PQclear(result); 
		
        // player not found. insert.
        switch(gameresult) {
        case 0:
        	// we lost
        	win++;
        	break;
        case 2:
        	// we won
        	loss++;
        	break;
        case 1:
        	// draw
        	draw++;
        	break;
        default:
        	return;
        }
        score = ((2*win+draw)*1000) / (2*(win+draw+loss));

        query.str("");
        query << "INSERT INTO " << tablename << " (player_id,win,draw,loss,score,minr,maxr,total_rating,avg_rating) VALUES (";
        query << player_id << "," << win << "," << draw << "," << loss << "," << score << "," << minr << "," << maxr << ",";
        query << (int)total_rating << "," << avg_rating << ")";
	
        result = PQexec(m_connection,query.str().c_str());
        if ((PQresultStatus(result) != PGRES_COMMAND_OK)) { 
            cerr << "query2. save stats\n";
            close();
            exit(1);
        }
	
    } else {
        
    	// update
    	win = atoi(PQgetvalue(result, 0, 0));
    	loss = atoi(PQgetvalue(result, 0, 1));
    	draw = atoi(PQgetvalue(result, 0, 2));
    	minr = atoi(PQgetvalue(result, 0, 3));
    	maxr = atoi(PQgetvalue(result, 0, 4));
    	total_rating = atoi(PQgetvalue(result, 0, 5));
    	avg_rating = atoi(PQgetvalue(result, 0, 6));
	
    	if (rating < minr) {
    		minr = rating;
    	}
    	if (rating > maxr) {
    		maxr = rating;
    	}
    	total_rating += rating;
    	switch(gameresult) {
    		case 0:
    			win++;
    			break;
    		case 2:
    			loss++;
    			break;
    		case 1:
    			draw++;
    			break;
    		default:
    			return;
    	}
 
    	score = ((2*win+draw)*1000) / (2*(win+draw+loss));
    	avg_rating = (int) ( total_rating/(win+loss+draw) );

        PQclear(result); 

        query.str("");
        query << "UPDATE " << tablename << " SET win=" << win << ",draw=" << draw << ",loss=" << loss << ",score=" << score;
        query << ",minr=" << minr << ",maxr=" << maxr << ",total_rating=" << (int)total_rating << ",avg_rating=" << avg_rating;
        query << " WHERE player_id=" << player_id;
	
        result = PQexec(m_connection,query.str().c_str());
        if ((PQresultStatus(result) != PGRES_COMMAND_OK)) {
            cerr << "!COMMAND_OK";
            close();
            exit(1);
        }
    }    
}

void db_pg::save_position(const char* tag, const int seq, const char* fen, const char* commands)
{
    std::ostringstream query; 
    query << "INSERT INTO positions (tag,seq,fen,commands) VALUES ('" << tag << "'," << seq << ",'" << fen << "','" << commands << "')"; 
    PGresult *result = PQexec(m_connection,query.str().c_str());
    if ((PQresultStatus(result) != PGRES_COMMAND_OK)) {
        cerr << "!COMMAND_OK";
        close();
        exit(1);
    }
}

bool db_pg::load_position(const char* tag, const int seq, std::string& fen, std::string& commands)
{
    std::ostringstream query; 
    query << "SELECT fen,commands FROM positions WHERE tag='" << tag << "' AND seq=" << seq;

    PGresult *result = PQexec(m_connection,query.str().c_str());
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {  
        cerr << "!TUPLES_OK";
        close();
        exit(1);
    }

    if (PQntuples(result) == 0) { 
    	// not found
        PQclear(result); 
        return false;
    } 
    
    fen = PQgetvalue(result, 0, 0); 
    commands = PQgetvalue(result, 0, 1); 	
    PQclear(result); 
    return true; 
}

bool db_pg::lookup_book(const _int64 hashcode, int& wwin, int& draw, int& bwin)
{	       
    // allow playing without a database 
    if (!m_connected) {
    	return false;
    }

    char query_buffer[1024];
    snprintf(query_buffer,1024,"SELECT wwin,draw,bwin FROM book WHERE hashcode = %Ld", hashcode);
    
    PGresult *result = PQexec(m_connection, query_buffer);
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {  
        cerr << "query1. lookup_book\n";
        close();
        exit(1);
    }
    
    if (PQntuples(result) == 0) { 
        PQclear(result); 
        return false;
    }
    
    wwin = atoi(PQgetvalue(result, 0, 0)); 
    draw = atoi(PQgetvalue(result, 0, 1)); 
    bwin = atoi(PQgetvalue(result, 0, 2)); 
    PQclear(result);     
    return true;
}

bool db_pg::learn_inbook(const _int64 hashcode, const int wildnumber, const int opponent_player_id)
{
    // allow playing without a database 
    if (!m_connected) {
    	return false;
    }

    char query_buffer[1024];

    if (wildnumber==17) {
		snprintf(query_buffer,1024,"SELECT hashcode FROM w17book WHERE hashcode = %Ld", hashcode);
    } else {
		snprintf(query_buffer,1024,"SELECT hashcode FROM learn WHERE hashcode = %Ld and player_id = %d",
				hashcode, opponent_player_id);
    }

    PGresult *result = PQexec(m_connection, query_buffer);
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {  
        cerr << "query1. learn_inbook\n";
        close();
        exit(1);
    }

    if (PQntuples(result) == 0) { 
        PQclear(result); 
        return false;
    }
    PQclear(result); 
    return true;
}

void db_pg::learn_update(const _int64 hashcode, const int winscore, const int lossscore,
				    const int nodes, const char avoid, const int wildnumber,
				    const int opponent_player_id, const int ply, const int move)
{
    // allow playing without a database 
    if (!m_connected) {
    	return;
    }

    char query_buffer[1024];

    if (wildnumber == 17) {
    	snprintf(query_buffer,1024,"SELECT nodes FROM w17book WHERE hashcode = %Ld", hashcode);
    } else {
    	snprintf(query_buffer,1024,"SELECT nodes FROM learn WHERE hashcode = %Ld and player_id = %d",
    			hashcode, opponent_player_id);
    }

    PGresult *result = PQexec(m_connection,query_buffer);
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {  
        cerr << "query1. learn_update\n";
        close();
        exit(1);
    }
    
    if (PQntuples(result) == 0) { 
        PQclear(result); 

        // insert
        if (wildnumber == 17) {
        	snprintf(query_buffer,1024,"INSERT INTO w17book (hashcode,win_score,loss_score,nodes,avoid) VALUES (%Ld,%d,%d,%d,%d)",
        			hashcode,winscore,lossscore,nodes,(int)avoid);
        } else {
        	std::ostringstream movestr;
        	_fast_SAN (movestr, move);
        	snprintf(query_buffer,1024,"INSERT INTO learn (hashcode,win_score,loss_score,nodes,avoid,player_id,ply,move) VALUES (%Ld,%d,%d,%d,%d,%d,%d,'%s')",
        			hashcode,winscore,lossscore,nodes,(int)avoid,opponent_player_id,ply, movestr.str().c_str());
        }

        result = PQexec(m_connection,query_buffer);
        if ((PQresultStatus(result) != PGRES_COMMAND_OK)) {
            cerr << "query2. learn_update\n";
            close();
            exit(1);
        }
        
    } else {
	
    	// update
    	int oldnodes = atoi(PQgetvalue(result,0,0));
    	PQclear(result);
		
    	int newnodes = nodes>oldnodes ? nodes : oldnodes;

    	if (wildnumber == 17) {
        	snprintf(query_buffer,1024,"UPDATE w17book SET win_score=%d,loss_score=%d,nodes=%d,avoid=%d WHERE hashcode=%Ld",
                    winscore,lossscore,newnodes,(int)avoid,hashcode);
    	} else {
        	snprintf(query_buffer,1024,"UPDATE learn SET win_score=%d,loss_score=%d,nodes=%d,avoid=%d WHERE hashcode=%Ld and player_id = %d",
                    winscore,lossscore,newnodes,(int)avoid,hashcode,opponent_player_id);
    	}
	
    	result = PQexec(m_connection,query_buffer);
    	if ((PQresultStatus(result) != PGRES_COMMAND_OK)) {
    		cerr << "query3. learn update\n";
    		close();
    		exit(1);
    	}
    }    
}

void db_pg::learn_retrieve(const _int64 hashcode, int& winscore, int& lossscore, int& nodes,
		char& avoid, const int wildnumber, const int opponent_player_id)
{
    // allow playing without a database 
    if (!m_connected) {
    	return;
    }

    char query_buffer[1024];
    if (wildnumber==17) {
        snprintf(query_buffer,1024,"SELECT win_score, loss_score, nodes, avoid FROM w17book WHERE hashcode = %Ld", hashcode);
    } else {
        snprintf(query_buffer,1024,"SELECT win_score, loss_score, nodes, avoid FROM learn WHERE hashcode = %Ld and player_id = %d", hashcode, opponent_player_id);
    }

    PGresult *result = PQexec(m_connection,query_buffer);
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {  
        cerr << "query1. learn_retrieve\n";
        close();
        exit(1);
    }

    if (PQntuples(result) == 0) { 
        PQclear(result); 
        return;
    } else {		
    	winscore = atoi(PQgetvalue(result, 0, 0));
    	lossscore = atoi(PQgetvalue(result, 0, 1));
    	nodes = atoi(PQgetvalue(result, 0, 2));
    	avoid = (char)atoi(PQgetvalue(result, 0, 3));
        PQclear(result); 
    }    
}

// w17 stuff 

void db_pg::w17_database_update_book (_int64 hashcode, int gameresult, int score)
{
/* flags: 
    bit 0-5: mate distance.
    bit 6: - mat
    bit 7: avoid this position */ 
	
    int wwin = 0; 
    int draw = 0; 
    int bwin = 0; 
    int flags = 0; 

    /* create the SQL statement */ 
    char query_buffer[1024]; 
    snprintf(query_buffer,1024,"SELECT wwin,draw,bwin,flags FROM w17book WHERE hashcode = %Ld", (_int64)hashcode);
	
    /* execute SQL query on the server */ 
    PGresult *result = PQexec(m_connection,query_buffer);
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {  
        cerr << "!TUPLES_OK";
        return; 
    }

    /* we only use one row so it's faster to use fetch_row (instead of mysql_store_result() */ 
    if (PQntuples(result) == 0) { 
        PQclear(result); 

        switch(gameresult) {
        case 1 : 
            wwin = 0; 
            draw = 0;
            bwin = 1; 
            break; 
        case 2 : 		      
            wwin = 0; 
            draw = 1;
            bwin = 0; 
            break; 
        case 3 : 
            wwin = 1;
            draw = 0; 
            bwin = 0; 
            break;
        default: cerr << "UNKNOWN GAMERESULT: aborting\n";
            break; 
        }

        flags = 0; 
        if MATE_VALUE(score) { 
                flags |= MATE-score; 
        }
        if MATED_VALUE(score) { 
                flags |= 64; 
        }
			
        /* entry staat niet in de database. compleet nieuw toevoegen */ 
        sprintf(query_buffer,"INSERT INTO w17book (hashcode,wwin,draw,bwin,flags) VALUES (%Ld,%d,%d,%d,%d)",hashcode,wwin,draw,bwin,flags);

        /* execute SQL query on the server */ 
        result = PQexec(m_connection,query_buffer);
        if ((PQresultStatus(result) != PGRES_COMMAND_OK)) {           
            cerr << "!COMMAND_OK";
            return;
        }
        return; 
        
    } else { 
 
        wwin = atoi(PQgetvalue(result, 0, 0)); 
        draw = atoi(PQgetvalue(result, 0, 1)); 
        bwin = atoi(PQgetvalue(result, 0, 2)); 
        flags = atoi(PQgetvalue(result, 0, 3)); 

        PQclear(result); 
		
        /* entry staat *AL* in de database. updaten */
        if MATE_VALUE(score) { 
                flags |= MATE-score; 
        }
        if MATED_VALUE(score) { 
                flags |= 64; 
        }
		
        switch(gameresult) {
        case 1 : 		       			
            bwin++; 
            snprintf(query_buffer,1024,"UPDATE w17book SET bwin=%d,flags=%d WHERE hashcode = %Ld",
            bwin,flags,hashcode);
            break; 
        case 2 : 		      			 
            draw++;			
            snprintf(query_buffer,1024,"UPDATE w17book SET draw=%d,flags=%d WHERE hashcode = %Ld",
            draw,flags,hashcode);
            break; 
        case 3 : 
            wwin++;		
            snprintf(query_buffer,1024,"UPDATE w17book SET wwin=%d,flags=%d WHERE hashcode = %Ld",
            wwin,flags,hashcode);       			 
            break;
        default: cerr << "UNKNOWN GAMERESULT: aborting\n";
            break; 
        }

        /* execute SQL query on the server */ 
        result = PQexec(m_connection,query_buffer);
        if ((PQresultStatus(result) != PGRES_COMMAND_OK)) {
            cerr << "!COMMAND_OK";
            return;
        }		
    } 	    
}

void db_pg::w17_database_set_avoid (int score, _int64 hashcode_after, int alevel)
{    
	if (alevel > 7 || (score < -(MATE-100) && 
			   score > -MATE)) {
		alevel = 7;
	}
		
	/* flags: bit 0-5 mate distance. 
	   bit 6 - 8 avoid this position */ 		
	alevel <<= 6;

	/* **** eerst kijken of deze positie al in het book staat */ 
	/* create the SQL statement */ 
	char query_buffer[1024]; 
	snprintf(query_buffer,1024,"SELECT flags FROM w17book WHERE hashcode = %Ld",
		   hashcode_after);

	/* execute SQL query on the server */ 
	PGresult *result = PQexec(m_connection,query_buffer);
	if (PQresultStatus(result) != PGRES_TUPLES_OK) {
		cerr << "!TUPLES_OK";
		return;
	}
		
	if (PQntuples(result) == 0) { 
		PQclear(result);
		cerr << "***** AVOID POSITION NOT IN w17BOOK!\n";
		return;
	}
	
	PQclear(result); 
				
	/*  maak query */ 
	snprintf(query_buffer,1024,"UPDATE w17book SET flags=%d WHERE hashcode = %Ld",
		alevel, hashcode_after);	

	/*  execute SQL query on the server */  
	result = PQexec(m_connection,query_buffer);
	if ((PQresultStatus(result) != PGRES_COMMAND_OK)) {
		cerr << "!COMMAND_OK";
	}    
} 

bool db_pg::w17_database_avoid (int score, _int64 hashcode_after)
{	
    /* return true if this position was already avoided */ 
    int wwin = 0, 
	bwin = 0, 
	draw = 0, 
	flags = 0;

	cout << "database_avoid_called\n";

    if (!w17_database_lookup_book (hashcode_after, wwin, draw, bwin, flags)) {
    	cout << "avoiding position not in book\n";
		return false;
    }
    
    if (flags & 63) {
		return false; /* forced win */
    }
    
    int alevel = (flags >> 7) & 8;	
    /* update database with higher avoid level */ 
    w17_database_set_avoid (score, hashcode_after, alevel + 1);
    
    if (alevel) {
    	return true;
    }
    return false; 
}

bool db_pg::w17_database_lookup_book (_int64 hashcode, int& wwin, int& draw, int& bwin, int& flags)
{
	if (! m_connection) {
		return false;
	}
	/* create the SQL statement */ 
	char query_buffer[1024]; 

	snprintf(query_buffer,1024,"SELECT wwin,draw,bwin,flags FROM w17book WHERE hashcode = %Ld", hashcode);

	/* execute SQL query on the server */ 
	PGresult *result = PQexec(m_connection,query_buffer);
	if (PQresultStatus(result) != PGRES_TUPLES_OK) {
		cerr << "!TUPLES_OK";
		return false;
	}
			
	if (PQntuples(result)==0) { 
		PQclear(result);
		return false;
	}

	wwin = atoi(PQgetvalue(result, 0, 0)); 
	draw = atoi(PQgetvalue(result, 0, 1)); 
	bwin = atoi(PQgetvalue(result, 0, 2)); 
	flags = atoi(PQgetvalue(result, 0, 3)); 

	PQclear(result);
	return true;
}

void db_pg::w17_database_set_age (int score, _int64 hashcode_after, int alevel)
{	
    // allow playing without a database 
    if (!m_connected) {
	return;
    }

    cout << "database_set_age\n";

    if (alevel > 7 || (score < -(MATE-100) && score > -MATE)) {
		alevel = 7;
    }
	
    /* flags: bit 0-5 mate distance.
       bit 6: - mat
       bit 7 - 9 avoid this position 
       bit 10 - 12 age this position */
 	    
    alevel <<= 10;

    /* **** eerst kijken of deze positie al in het book staat */ 
    /* create the SQL statement */ 
    char query_buffer[1024]; 
    snprintf(query_buffer,1024,"SELECT flags FROM w17book WHERE hashcode = %Ld",
	       hashcode_after);

    /* execute SQL query on the server */ 
    PGresult *result = PQexec(m_connection,query_buffer);
    if (PQresultStatus(result) != PGRES_TUPLES_OK) { 
        cerr << "!TUPLES_OK";
        return;
    }
	
    if (PQntuples(result) == 0) { 
        PQclear(result); 
        cerr << "***** AGE POSITION NOT IN w17BOOK!\n";
        return;
    }
	
    PQclear(result); 
				
    /* maak query */ 
    snprintf(query_buffer,1024,"UPDATE w17book SET flags=%d WHERE hashcode = %Ld",
	       alevel, hashcode_after);	

    /* execute SQL query on the server */  
    result = PQexec(m_connection,query_buffer);
    if ((PQresultStatus(result) != PGRES_COMMAND_OK)) {    
        cerr << "!COMMAND_OK";
        return;
    }     
}

void db_pg::w17_database_age (int score, _int64 hashcode_after, int alevel)
{	
    int wwin = 0, 
    bwin = 0, 
    draw = 0, 
    flags = 0;

    cout << "database_age called\n";

    if (! w17_database_lookup_book (hashcode_after, wwin, draw, bwin, flags)) {
        cout << "age position not in book\n";
		return;
    }

    if (flags) {
        return;
    }

    w17_database_set_age (score, hashcode_after, alevel);	
}

void db_pg::w17_database_unage(_int64 hashcode)
{
	/* create the SQL statement */ 
	char query_buffer[1024]; 
	snprintf(query_buffer,1024,"SELECT flags FROM w17book WHERE hashcode = %Ld", hashcode);

	/* execute SQL query on the server */ 
	PGresult *result = PQexec(m_connection,query_buffer);
	if (PQresultStatus(result) != PGRES_TUPLES_OK) {
		cerr << "!TUPLES_OK";
		return;
	}
		
	if (PQntuples(result) == 0) { 
		PQclear(result);
		return;
	}

	int flags = atoi(PQgetvalue(result,0,0)); 
	PQclear(result);
	
	// age 1 minder maken
	int age = flags >> 10;
	if (age && (flags == (age << 10))) {
		flags = (-- age) << 10;

		snprintf(query_buffer,1024,"UPDATE w17book SET flags=%d WHERE hashcode = %Ld",
				flags,hashcode);

		/* execute SQL query on the server */
		result = PQexec(m_connection,query_buffer);
		if ((PQresultStatus(result) != PGRES_COMMAND_OK)) {
			cerr << "!COMMAND_OK";
			return;
		}
	}    
} 
