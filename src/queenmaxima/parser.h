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

#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <sstream>

#define PieceToChar(piece) (char("  NBRQK"[piece]))
#define FileToChar(sq)     (char(((sq)&7)+97))
#define RankToChar(sq)     (char(((sq)>>3)+49))

void whisper_bookmove ();
void tell_crisis (int time);

int ParseSmith(TFastNode* node, const char* str);
int ParseMove(TFastNode* node, std::string& str);
int ParseXboardMove(TFastNode* node, const char* str);

void print_iteration (int elapsed); 
void fenton(TFastNode* node, const char* fenstr);
void _fast_LAN(std::ostringstream& str, int move);
void _print_LAN(int move);
void _print_SAN(int move);
void _fast_SAN (std::ostringstream& str, int move); 
void _write_LAN(int connection, int move); 
void ntofen(std::string& str, TFastNode* node);
void _fast_ntofen(std::ostringstream& str, TFastNode* node); 
void print_square (int square);
void print_piece (int piece);
void parse_epd(std::string& src, std::string& fen, std::string& commands);
void print_score (int score);
void print_move (int move);

#endif
