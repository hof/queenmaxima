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
/* All rights reserved.
 * 
 * fics.cpp:  parsing of data received from the free internet chess server (freechess.org)    
 * 
 */

#include <stdlib.h> 

#include "fics.h" 
#include "main.h"
#include "parser.h" 


/* 
 * Parse12Move
 * @node: the node in which the move is played. 
 * @str: the string containinh the move
 
 * 
 * move looks like P/e2-e4
 *
 */ 
int Parse12Move(TFastNode* node, std::string& str, std::string& pretty) 
{
//     if (str=="none") return 0;
//     if (str=="o-o") return WTM(node)? ShortCastleWhite : ShortCastleBlack;
//     if (str=="o-o-o") return WTM(node)? LongCastleWhite : LongCastleBlack;

//     char ssq = char((str[3]-'a')+8*(str[4]-'1'));
//     char tsq = char((str[6]-'a')+8*(str[7]-'1'));

//     __int64 btsq = stob(tsq);
//     int piece = node->matrix[ssq];
//     if (piece>KING) piece -= KING;

//     if (piece != ICSCharToPiece(str[1])) {
//     return 0;
//     }
//
//     int captured = node->matrix[tsq];
//     if (captured>KING) {
//	     captured -= KING;
//     }
//     if (piece==PAWN) {
//	     char prop=0;
//	     if (btsq & (rank_1|rank_8)) {
//		     switch (UpCase(pretty[pretty.Length()])) {
//		     case 'N' : 
//			     prop = (char)KNIGHT; 
//			     break;
//		     case 'B' : prop = (char)BISHOP; break;
//     case 'R' : prop = (char)ROOK; break;
//      case 'Q' : prop = (char)QUEEN; break;
//      default  : prop = (char)QUEEN; break;
//   }
//  }
//  if (column(ssq)==column(tsq)) return (prop)? PromotionMove(ssq,tsq,prop) : NormalMove(ssq,tsq,PAWN);
//  if (tsq==node->ep_square) return EnPassantMove(ssq,node->ep_square);
//  return (prop)? ProCapMove(ssq,tsq,prop,captured) : CaptureMove(ssq,tsq,PAWN,captured);
// }
//
// return CaptureMove(ssq,tsq,piece,captured);

	     return 0; 
}


/* fast parse functions for style12 parsing */ 

void inline parse_copy_str(const std::string& src, int& pos, std::string& dest)
{ 
    // we are in a <12> so we know the token ends when we see a ' ' 
    // pos must be *on* the first char of the token 
    // fixme: use stl function 
//    dest = "";
//    while (src[pos] != ' ') {
//	dest += src[pos++];
//    }
}

void inline parse_copy_int(const std::string& src, int& pos, int& result) 
{
    // fixme: use stringstream 
//    char buffer[25];
//    int bpos = 0;
//    /* skip to the next token. pos points to start of next token */
//    while (src[pos] != ' ') {
//	buffer[bpos++] = src[pos++];
//    }
//    buffer[bpos] = 0;
//    result = atoi(buffer);
//    pos++;
}

void inline parse_skip_token(const std::string& src, int& pos)
{
//    while (src[pos++] != ' ') {}
//    pos++;
}

/* style 12 parsing */ 

#define north  8
#define east   1
#define west  -1
#define south -8

void Parse12Full(std::string& str, TFastNode* node)
{
	/* we create a new node in node. generally, node is */ 
	/* MainForm.position. so we have to clear the node  */ 
	/* first. */
//	_fast_clearnode(node); 
//	ClearNode(node); 

	/* the following code is optimized for icc style 12 */ 
	/* breaks if it's called on a non style 12 string   */ 
//	int pos=a8;  
//	for (int i=5; i<=75; i++) {
//		switch(str->str[i]) {
//		case ' ': 
//			pos = pos - 17; 
//			break; 
//		case 'K' :
//			_fast_AddPiece(TFastNode* node, char piece, char sq) { 
//			AddWhiteKing(node,pos,stob(pos));
//			break;
//		case 'Q' : 
//			AddWhiteQueen(node,pos,stob(pos));
//			break;
//		case 'R' : 
//			AddWhiteRook(node,pos,stob(pos));
//			break;
//		case 'B' : 
//			AddWhiteBishop(node,pos,stob(pos));
//			break;
//		case 'N' : 
//			AddWhiteKnight(node,pos,stob(pos));
//			break;
//		case 'P' : 
//			AddWhitePawn(node,pos,stob(pos));
//			break;
//		case '-' : 
//			break;
//		case 'k' : 
//			AddBlackKing(node,pos,stob(pos));
//			break;
//		case 'q' : 
//			AddBlackQueen(node,pos,stob(pos));
//			break;
//		case 'r' : 
//			AddBlackRook(node,pos,stob(pos));
//			break;
//		case 'b' : 
//			AddBlackBishop(node,pos,stob(pos));
//			break;
//		case 'n' : 
//			AddBlackKnight(node,pos,stob(pos));
//			break;
//		case 'p' : 
//			AddBlackPawn(node,pos,stob(pos));
//			break;
//		}
//		pos++;
//	}
//	   
//	/* str->str[77] indicates 'W' white to move or 'B' black to move */ 
//	bool wtm = (str->str[77]=='W'); 
//	if (wtm) {
//		node->hashcode |= 1;
//	}
//
	/* the next token in the string is either a "-1" or a file number */ 
	/* 0-7 (for a-h, in which a double pawn push was made.            */ 
//	int nexttoken;        /* place in str->str where next token starts */  
//	if (str->str[79] == '-') {
		/* no e.p. file */ 
//		nexttoken = 82; 
//	} else {
		/* set epsquare in node */ 
//		nexttoken = 81; 
//		node->ep_square = (wtm) ? ((str->str[79]-'0')+5*north) : ((str->str[79]-'0')+2*north);
//		HashEP(node);		
//	}

	/* castling stuff */ 
//	if (str->str[nexttoken]=='1') {
//		AddSCW(node);
//	}
//	if (str->str[nexttoken+2]=='1') {
//		AddLCW(node);
//	}
//	if (str->str[nexttoken+4]=='1') {
//		AddSCB(node);
//	}
//	if (str->str[nexttoken+6]=='1') {
//		 AddLCB(node);
//	}
	
	/* at this point, we have to parse. */ 
	/* the fields are not of fixed length. */ 
	
//	nexttoken += 8; 
//	parse_copy_int(str,nexttoken,&node->fifty); 
       
	/* game number */ 
//	parse_copy_int(str,nexttoken,&MainForm.gamenumber); 

	/* whitename */ 
//	parse_copy_str(str,nexttoken,MainForm.whitename); 

	/* blackname */ 
//	parse_copy_str(str,nexttoken,MainForm.blackname); 

	/* our relation to this game */ 
//	int relation; 
//	parse_copy_int(str,nexttoken,&relation); 
	
	/* time controls */ 
//	parse_copy_int(str,nexttoken,&MainForm.basetime);
//	parse_copy_int(str,nexttoken,&MainForm.increment);

	/* strength of the players */ 
//	parse_skip_token(str,nexttoken);
//	parse_skip_token(str,nexttoken);

	/* times */ 
//	parse_copy_int(str,nexttoken,&MainForm.whitetime); 
//	parse_copy_int(str,nexttoken,&MainForm.blacktime); 

	/* ok - check relation and decide if we have to start thinking. */ 
	/* do we have to think? */ 

//	if (relation==2) { 
//		/* calculating a d1 position */ 
//		return;
//	} 


//	if (relation==1 /* RELATION_MYMOVE */ ) {
//		
//		resume_engine_thread();
//	} else { 
//		if (node->fifty == 0) { 
//			_repindex_w = 0; 
//			_repindex_b = 0; 
//			printf("reptable reset in Parse12Full() NOTMYMOVE\n"); 
//		}
//		if (wtm) { 
//			_reptable_w [_repindex_w++ ] = node->hashcode; 
//		} else { 
//			_reptable_b [_repindex_b++ ] = node->hashcode; 
//		}
//		printf("node->fifty = %d\n",node->fifty); 
//	}
//		
//
}

void Parse12Fast(std::string& str)
{
/* voorbeeld style 12 : 

<12> -r---rk- -bR--pp- -p--p--p p-p-n--- ----P--- P-P--P-- BP--KNPP ---R---- B -1 0 0 0 0 1 389 Alfreema Wyrm 0 5 0 23 23 209 210 22 N/h3-f2 (0:14) Nf2 1

<12> -r---rk- --R--pp- bp--p--p p-p-n--- ----P--- P-P--P-- BP--KNPP ---R---- W -1 0 0 0 0 2 389 Alfreema Wyrm 0 5 0 23 23 209 203 23 B/b7-a6 (0:07) Ba6+ 1

<12> -------Q ------R- ---P---- ------Pk -----K-- --B----- -------- -------- B -1 0 0 0 0 0 507 guest1151 guest460 1 3 0 19 0 27 27 53 P/h7-h8 (0:03) h8=Q# 1
 */
}

void ParseCreating(std::string& str)
{
// Creating: hof (1520) Mran (1596) rated blitz 4 0
}

void ParseGame(std::string& str)
{
// {Game 183 (hof vs. Mran) Mran resigns} 1-0
// {Game 183 (hof vs. Mran) Creating rated blitz match.} *
}

void ParseChallenge(std::string& str)
{
}

void ParseTell(std::string& str)
{
}

/*
 * indentify_ics_string
 * @str: buffer containing the last line read from icc 
 * @first12: indicates if, after this line we have to fully parse style 12 game info 
 *           (meaning that a new game has started) 
 * @prevType: pevious result of this function
 *

 * detects and returns the type of string received from the icc
 * connection. 
 */
int identify_ics_string(std::string& line, bool& first12, int prevType)
{
//	char w1[256];
//	char w2[256];
//
//	if (line.length() == 0 || line[0]==' ' || line[0]==10) {
//		return ICSUNKNOWN;
//	}
//
//	unsigned int pos=0;
//	unsigned int wpos=0;
//
//	while (line[pos] != ' ' && line[pos] != 10 && pos <= line.length()) {
//		w1[pos] = line[pos];
//		pos++;
//	}
//	w1[pos] = 0;
//
//	while (line[pos] == ' ' && line[pos] != 10 && pos <= line.length()) {
//		pos++;
//	}
//
//	while (line[pos] != ' ' && line[pos] != 10) {
//		w2[wpos] = line[pos];
//		wpos++;
//		pos++;
//	}
//	w2[wpos] = 0;
//
//	/* first 2 words of the line are parsed. */
//	/* let's see what we have */
//
//	char ch1 = w1[0];
//	if (ch1=='\\') {
//		return prevType;
//	}
//	if (ch1=='>') {
//		return ICSTOURNEY;
//	}
//	if (!strcmp(w1,"-->")) {
//		return ICSISHOUT;
//	}
//	if (!strcmp(w1,"<12>")) {
//		return ICSSTYLE12;
//	}
//	if (!strcmp(w1,"Creating:")) {
//		first12 = true;
//		return ICSCREATING;
//	}
//	if (!strcmp(w1,"{Game")) {
//		return ICSGAME;
//	}
//	if (!strcmp(w1,"Challenge:")) {
//		return ICSCHALLENGE;
//	}
//	if (w2==NULL) {
//		return ICSUNKNOWN;
//	}
//	if (!strcmp(w2,"tells")) {
//		return ICSTELL;
//	}
//	if (!strcmp(w2,"says:")) {
//		return ICSTELL;
//	}
//	if (!strcmp(w2,"shouts:")) {
//		return ICSSHOUT;
//	}
//	if (!strcmp(w2,"s-shouts:")) {
//		return ICSSSHOUT;
//	}
	return ICSUNKNOWN;
}

void fics_connection_cb()
{

	/* read a number of bytes from the socket */ 
//	char buffer[1024];
//	int stringtype = MainForm.prevType; 
//	int bytesread = read(MainForm.socket_connection, &buffer,1024); 

//	for (int i=0; i<bytesread;i++) { 
//		switch(buffer[i]) {
//		case 10 : break;
//		case 13 : 
//			g_string_append_c(MainForm.ICSCurrentLine,10);
//			stringtype = identify_ics_string(MainForm.ICSCurrentLine,
//							 MainForm.first12,
//							 MainForm.prevType);
//			/* check the type of the line */ 
//			switch(stringtype) {
//			case ICSTOURNEY: 
//				gtk_text_insert (GTK_TEXT(MainForm.console), 
//						 MainForm.console_fixed_font ,
//						 NULL,NULL,
//						 MainForm.ICSCurrentLine->str,
//						 MainForm.ICSCurrentLine->len);
//				break;
//			case ICSISHOUT:
//				gtk_text_insert (GTK_TEXT(MainForm.console), 
//						 MainForm.console_fixed_font ,
//						 NULL,NULL,
//						 MainForm.ICSCurrentLine->str, 
//						 MainForm.ICSCurrentLine->len);  
//				break;
//			case ICSSTYLE12:
//				gtk_text_insert (GTK_TEXT(MainForm.console), 
//						 MainForm.console_fixed_font ,
//						 NULL,NULL,
//						 MainForm.ICSCurrentLine->str, 
//						 MainForm.ICSCurrentLine->len);   
//				if (MainForm.first12) {
//					Parse12Full(MainForm.ICSCurrentLine,MainForm.fastposition);
					// MainForm.first12 = false; 
//				} else { 
//					Parse12Fast(MainForm.ICSCurrentLine);
//				}
//			
//				break;
//			case ICSCREATING: 
//				ParseCreating(MainForm.ICSCurrentLine);
//				new_game(); 
//				gtk_text_insert (GTK_TEXT(MainForm.console), 
//						 MainForm.console_fixed_font ,
//						 NULL,NULL,
//						 MainForm.ICSCurrentLine->str,
//						 MainForm.ICSCurrentLine->len);  
//				break;
//			case ICSGAME:
//				ParseGame(MainForm.ICSCurrentLine);
//				if (moveinfo_gameresult) {
//					game_ended ();
//				}
//				gtk_text_insert (GTK_TEXT(MainForm.console), 
///						 MainForm.console_fixed_font ,
//						 NULL,NULL,
//						 MainForm.ICSCurrentLine->str,
//						 MainForm.ICSCurrentLine->len);  
//				MainForm.first12 = true;
//				if (moveinfo_gameresult) {
//					MainForm.autoseekaccept = false; 
//				}
				//if (moveinfo_gameresult==4) {
//					write(MainForm.socket_connection, 
//					      "seek1\nseek2\nseek3\n", 18);
				//}
//				break;
//			case ICSCHALLENGE: 
//				ParseChallenge(MainForm.ICSCurrentLine);
//				gtk_text_insert (GTK_TEXT(MainForm.console), 
//						 MainForm.console_fixed_font ,
//						 NULL,NULL,
//						 MainForm.ICSCurrentLine->str,
//						 MainForm.ICSCurrentLine->len);
//				if (MainForm.autoseekaccept) { 
//					write(MainForm.socket_connection, "accept\n", 7); 
//				}
	//			break;
//			case ICSTELL: 
//				ParseTell(MainForm.ICSCurrentLine);
//				gtk_text_insert (GTK_TEXT(MainForm.console), 
//						 MainForm.console_fixed_font ,
//						 NULL,NULL,
//						 MainForm.ICSCurrentLine->str,
//						 MainForm.ICSCurrentLine->len); 					      
//				break;
//			case ICSSHOUT: 
//				gtk_text_insert (GTK_TEXT(MainForm.console), 
//						 MainForm.console_fixed_font ,
//						 NULL,NULL,
//						 MainForm.ICSCurrentLine->str,
//						 MainForm.ICSCurrentLine->len); 
	//			break;
	//		case ICSSSHOUT: 
//				gtk_text_insert (GTK_TEXT(MainForm.console), 
//						 MainForm.console_fixed_font ,
//						 NULL,NULL,
//						 MainForm.ICSCurrentLine->str,
//						 MainForm.ICSCurrentLine->len); 
//				break;
//			case ICSUNKNOWN: 
//				gtk_text_insert (GTK_TEXT(MainForm.console), 
//					 MainForm.console_fixed_font ,
//						 NULL,NULL,
//						 MainForm.ICSCurrentLine->str,
//						 MainForm.ICSCurrentLine->len);  
//				break;
				
//			}
//			g_string_assign(MainForm.ICSCurrentLine,"");
//			break; 
//		default:
//			g_string_append_c(MainForm.ICSCurrentLine,buffer[i]); 
//			/* auto-login */ 
//			if (!strcmp(MainForm.ICSCurrentLine->str,"login:")) {
//				write(MainForm.socket_connection, "BugChess\n", 9);
//				//write(MainForm.socket_connection, "g\n\nset prompt 0\nset style 12\n", 29);  
//			}
//			if (!strcmp(MainForm.ICSCurrentLine->str,"password:")) { 
//				write(MainForm.socket_connection,
//				      "delphi456852\nset prompt 0\nset style 12\n", 39);  
//			}
//			break; 
//		}
//	}	
}







