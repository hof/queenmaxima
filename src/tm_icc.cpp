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

#define _second 1000 

int tm_icc (int mytime, int opptime, int increment) 
{
	int movesleft = (increment==0 && mytime<15*_second) ? 30 : 15,
		tmax = mytime / movesleft;
	if (mytime > opptime) {
		tmax += (mytime - opptime) / 5;
	} else if ((increment<3) && mytime<120*_second && mytime < opptime) {
	  //		tmax -= (opptime - mytime) / 2;
	  tmax -= (tmax/2);
	}
	if (increment && mytime > increment) {
		tmax += (4 * increment) / 5;
	}
	return tmax;
}
