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

#ifndef dbbaseH
#define dbbaseH

#include <string>
#include "defines.h"

    class db_base 
	{
	public:
		virtual bool connect(const char* hostname, const char* username, const char* password, const char *database, const unsigned int port=0) { return false; };
		virtual void close() { };
		virtual long save_player(const char* name, const char* titles, const char* server) { return 0;};
		virtual long save_game(const int whitename, const int blackname, const int flags, 
				const int wildnumber, const int whiterating, 
			   const int blackrating, const int basetime, const int inc, 
			   const int rating_type, const int rated, std::string result_code ) { return 0; };
		virtual long save_move(const int gamenumber, const int ply, const int move, const int thinktime) { return 0; };
		virtual void update_stats(const char* tablename, const int player_id, const int rating, const int gameresult) {};
		virtual void save_position(const char* tag, const int seq, const char* fen, const char* commands) {};
		virtual bool load_position(const char* tag, const int seq, std::string& fen, std::string& commands) { return false;};
		virtual bool lookup_book(const _int64 hashcode, int& wwin, int& draw, int& bwin) {return false;};
		virtual bool learn_inbook(const _int64 hashcode, int wildnumber) {return false;};
		virtual void learn_update(const _int64 hashcode, const int winscore, const int lossscore, 
			const int nodes, const char avoid, const int wildnumber) {};
		virtual void learn_retrieve(const _int64 hashcode, int& winscore, int& lossscore, int& nodes, char& avoid, const int wildnumber) { };
		
		// w17 
		virtual bool w17_database_lookup_book (_int64 hashcode, int& wwin, int& draw, int& bwin, int& flags) { return false; };
		virtual void w17_database_age (int score, _int64 hashcode_after, int alevel) { };
		virtual void w17_database_set_age (int score, _int64 hashcode_after, int alevel) { }; 
		virtual bool w17_database_avoid (int score, _int64 hashcode_after) { return false; };
		virtual void w17_database_set_avoid (int score, _int64 hashcode_after, int alevel) { };
		virtual void w17_database_update_book (_int64 hashcode, int gameresult, int score) { };
		virtual void w17_database_unage(_int64 hashcode) { };

	};     

#endif 
