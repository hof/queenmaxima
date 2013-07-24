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

#ifndef dbpgH
#define dbpgH

#include "config.h"

#include <libpq-fe.h>
#include "db_base.h"

class db_pg : public db_base
{
 private:
  PGconn* m_connection;
  bool   m_connected;   
 public:
  long db_get_max(const char *table, const char *field);
  db_pg() : m_connection(NULL), m_connected(false) { };
  bool connect(const char* hostname, const char* username, const char* password, const char *database, const unsigned int port = 0);
  void close();
  long save_player(const char* name, const char* titles, const char* server);
  long save_game(const int whitename, const int blackname, const int flags, 
		 const int wildnumber, const int whiterating, 
		 const int blackrating, const int basetime, const int inc, 
		 const int rating_type, const int rated);
  long save_move(const int gamenumber, const int ply, const int move, const int thinktime);
  void update_stats(const char* tablename, const int player_id, const int rating, const int gameresult);
  void save_position(const char* tag, const int seq, const char* fen, const char* commands);
  bool load_position(const char* tag, const int seq, std::string& fen, std::string& commands);
  bool lookup_book(const _int64 hashcode, int& wwin, int& draw, int& bwin);
  bool learn_inbook(const _int64 hashcode, int wildnumber);
  void learn_update(const _int64 hashcode, const int winscore, const int lossscore, 
		    const int nodes, const char avoid, const int wildnumber);
  void learn_retrieve(const _int64 hashcode, int& winscore, int& lossscore, int& nodes, char& avoid, const int wildnumber);
  
  // w17 
  bool w17_database_lookup_book (_int64  hashcode, int& wwin, int& draw, int& bwin, int& flags);
  void w17_database_age (int score, _int64 hashcode_after, int alevel);
  void w17_database_set_age (int score, _int64 hashcode_after, int alevel); 
  bool w17_database_avoid (int score, _int64 hashcode_after);
  void w17_database_set_avoid (int score, _int64 hashcode_after, int alevel);
  void w17_database_update_book (_int64 hashcode, int gameresult, int score);
  void w17_database_unage(_int64 hashcode); 
  
};

#endif 
