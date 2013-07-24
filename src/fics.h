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

#ifndef FICS_H 
#define FICS_H 

#define ICSTOURNEY 1
#define ICSPROMPT 2
#define ICSISHOUT 3
#define ICSSTYLE12 4
#define ICSCREATING 5
#define ICSGAME 6
#define ICSCHALLENGE 7
#define ICSREMOTECOMMAND 8
#define ICSTELL 9
#define ICSSHOUT 10
#define ICSSSHOUT 11
#define ICSUNKNOWN 12

void fics_connection_cb();

#endif 
