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

#ifndef ICC_H 
#define ICC_H 

#include <string.h>
#include <glib.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <netdb.h>

#define DG_WHO_AM_I 0
#define DG_OPEN 10
#define DG_MY_GAME_STARTED 15
#define DG_MY_GAME_RESULT 16
#define DG_MY_GAME_ENDED 17 
#define DG_SEND_MOVES 24
#define DG_MOVE_LIST 25
#define DG_KIBITZ 26
#define DG_MATCH 29
#define DG_PERSONAL_TELL 31
#define DG_MSEC 56
#define DG_FEN 70
#define DG_POSITION_BEGIN 101 

void connect_to_ics(const char *host, const int port);
void icc_connection_cb();

#endif 

