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

#include <string>
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include "main.h" 
#include "fast.h"
#include "parser.h"  
#include "hashcodes.h" 

void whisper_bookmove ()
{
  bool w0learn = false; 
  std::ostringstream str;  
    
  if (!g.rootmoves[0].bookmove) {
    return;
  }
 
  str << "whisper book "; 
  switch (MainForm.wildnumber) {
  case 0:
    if (g.rootmoves[0].bookvalue>10000000) {
      g.rootmoves[0].bookvalue -= 10000000;
      w0learn = true; 
      str << "(learn) ";
    } else {
      str << "(GM) ";
    }
    break;
  case 17: 
    str << "(losers) ";
    break;
  }
	 
  _fast_LAN (str, g.rootmoves[0]. move);
  str << " (score=" << g.rootmoves[0].bookvalue << ")\n";
  
  if (w0learn) { 
    g.rootmoves[0].bookvalue -= 10000000;
  }

  // send(MainForm.socket_connection, str.str().c_str(), str.str().length(), 0);
  std::cout << str.str(); 
}

void tell_crisis (int time)
{
    std::cout << "engine crisis time=" << time << "\n"; 
}

void print_iteration (int elapsed)
{
	char sign = g.rootscore>=0? '+' : '-';
	int pvlen = 0; 
	if (elapsed >= 0) {
		std::ostringstream scorestr; 
		if (MATE_VALUE (g.rootscore)) {
			scorestr << "#" << (MATE-g.rootscore+1)/2;
		} else if (MATED_VALUE (g.rootscore)) {
			scorestr << "-#" << (MATE+g.rootscore+1)/2;
		} else { 
			scorestr << sign << ABS (g.rootscore/1000.0);
		}
       		std::ostringstream str; 
		std::ostringstream str2;
		
		str << scorestr.str() << " d" << g.iteration << " (" << int(g.fastnodes/1000) << "Kn " << elapsed << ") "; 
	        pvlen = print_pv_addstr (str);
		str << std::endl; 

		if (MainForm.white_titles == "C" && MainForm.black_titles == "C") {
		  str2 << "whisper " << str.str();
		} 
		//send(MainForm.socket_connection, str2.str().c_str(), str2.str().length(), 0);
	}
}

void print_move (int move) {
	_print_SAN (move);	
}

void print_score (int score) 
{	
	if (score < 0) {
		g_print ("-");
		score = - score;		
	} else {
		g_print ("+");
	}
	int pawns = score / 1000,
		millipawns = score - 1000 * pawns;
	if (millipawns > 99) {
		g_print ("%d.%d", pawns, millipawns);
	}
	else if (millipawns > 9) {
		g_print ("%d.0%d", pawns, millipawns);
	} else {
		g_print ("%d.00%d", pawns, millipawns); 
	}
}

void print_square (int square) {	
	g_print("%c%c",	FileToChar (square), RankToChar (square));
}

void print_piece (int piece) {
	switch (piece) {		
	case PAWN:
		g_print ("P");
		break;
	case KNIGHT:
		g_print ("N");
		break;
	case BISHOP:
		g_print ("B");
		break;
	case ROOK:
		g_print ("R");
		break;
	case QUEEN:
		g_print ("Q");
		break;
	case KING:
		g_print ("K");
		break;
	case BPAWN:
		g_print ("p");
		break;
	case BKNIGHT:
		g_print ("n");
		break;
	case BBISHOP:
		g_print ("b");
		break;
	case BROOK:
		g_print ("r");
		break;
	case BQUEEN:
		g_print ("q");
		break;
	case BKING:
		g_print ("k");
		break;
	default:
		g_print ("?");
		break;
	}
}

void _write_LAN(int connection, int move) 
{
	char buffer[25]; 
	char piece = PieceToChar(PIECE(move));
	char seperator = _CAPTURE(move)? 'x':'-';
	
	if (!SPECIAL(move)) {
		if (piece==' ') { 
			g_snprintf(buffer,25,"%c%c%c%c%c\n",
				FileToChar(SOURCESQ(move)),
				RankToChar(SOURCESQ(move)),
				seperator,
				FileToChar(TARGETSQ(move)),
				RankToChar(TARGETSQ(move)));   

			send(connection,buffer,6, 0); 

			return;
		} else { 
			g_snprintf(buffer,25,"%c%c%c%c%c%c\n",
				piece,
				FileToChar(SOURCESQ(move)),
				RankToChar(SOURCESQ(move)),
				seperator,
				FileToChar(TARGETSQ(move)),
				RankToChar(TARGETSQ(move)));   

			send(connection,buffer,7, 0);
			return;
		}
	}
	switch SPECIALCASE(move) {
	case _PLUNKMOVE:
		g_snprintf(buffer,25,"%c@%c%c\n", 
			   piece,
			   FileToChar(TARGETSQ(move)),
			   RankToChar(TARGETSQ(move))); 

		send(connection,buffer,5, 0); 

		return; 
	case _SHORT_CASTLE:
		send(connection,"O-O\n",4, 0); 
		return;
	case _LONG_CASTLE:

		send(connection,"O-O-O\n",6, 0);
		return;
	case _EN_PASSANT:
		g_snprintf(buffer,25,"%c%cx%c%c\n",
			FileToChar(SOURCESQ(move)),
			RankToChar(SOURCESQ(move)),
			FileToChar(TARGETSQ(move)),
			RankToChar(TARGETSQ(move))); 
		send(connection,buffer,6, 0); 
		return;
	case _QUEEN_PROMOTION:
		g_snprintf(buffer,25,"%c%c%c%c%c=Q\n",
			FileToChar(SOURCESQ(move)),
			RankToChar(SOURCESQ(move)),
			seperator,
			FileToChar(TARGETSQ(move)),
			RankToChar(TARGETSQ(move)));   
		send(connection,buffer,8, 0); 
		return;
	case _ROOK_PROMOTION: 
		g_snprintf(buffer,25,"%c%c%c%c%c=R\n",
			FileToChar(SOURCESQ(move)),
			RankToChar(SOURCESQ(move)),
			seperator,
			FileToChar(TARGETSQ(move)),
			RankToChar(TARGETSQ(move)));   
		send(connection,buffer,8, 0); 
		return;
	case _BISHOP_PROMOTION:
		g_snprintf(buffer,25,"%c%c%c%c%c=B\n",
			FileToChar(SOURCESQ(move)),
			RankToChar(SOURCESQ(move)),
			seperator,
			FileToChar(TARGETSQ(move)),
			RankToChar(TARGETSQ(move)));   
		send(connection,buffer,8, 0); 
		return;
	case _KNIGHT_PROMOTION: 
		g_snprintf(buffer,25,"%c%c%c%c%c=N\n",
			FileToChar(SOURCESQ(move)),
			RankToChar(SOURCESQ(move)),
			seperator,
			FileToChar(TARGETSQ(move)),
			RankToChar(TARGETSQ(move)));   
		send(connection,buffer,8, 0); 
		return;
	default: 
	        g_error("Not Sent. %c%c%c%c%c%c - Illegal move representation.\n",
			piece,
			FileToChar(SOURCESQ(move)),
			RankToChar(SOURCESQ(move)),
			seperator,
			FileToChar(TARGETSQ(move)),
			RankToChar(TARGETSQ(move)));   
		return;
	} 
}

void _print_SAN (int move)
{
        char piece = PieceToChar(PIECE (move));
	char from1 = FileToChar (SOURCESQ (move));
	char to1 = FileToChar (TARGETSQ (move));
	char to2 = RankToChar (TARGETSQ (move));
        if (move == 0) {
                g_print ("null");
                return;
        }

        if (!SPECIAL(move)) {
                if (piece == ' ') { //pawn
			if (_CAPTURE (move)) {
				g_print ("%cx%c%c", from1, to1, to2);
			} else {
				g_print ("%c%c", to1, to2);
			}
                } else {
			if (_CAPTURE (move)) {
				g_print ("%cx%c%c",piece, to1, to2);
			} else {
				g_print ("%c%c%c", piece, to1, to2);
			}
                }
        } else {
                switch SPECIALCASE(move) {
                case _PLUNKMOVE:
                        g_print("%c@%c%c", piece, to1, to2);
                        break;
                case _SHORT_CASTLE:
                        g_print("O-O");
                        break;
                case _LONG_CASTLE:
                        g_print("O-O-O");
                        break;
                case _EN_PASSANT:
                        g_print("%cx%c%cep", from1, to1, to2);
                        break;
                case _QUEEN_PROMOTION:
			if (_CAPTURE (move)) {
                        	g_print("%cx%c%c=Q", from1, to1, to2);
			} else {
				g_print("%c%c=Q", to1, to2);
			}
                        break;
                case _ROOK_PROMOTION:
			if (_CAPTURE (move)) {
                                g_print("%cx%c%c=R", from1, to1, to2);
                        } else {
                                g_print("%c%c=R", to1, to2);
                        }
                        break;
                case _BISHOP_PROMOTION:
			if (_CAPTURE (move)) {
                                g_print("%cx%c%c=B", from1, to1, to2);
                        } else {
                                g_print("%c%c=B", to1, to2);
                        }
                        break;
                case _KNIGHT_PROMOTION:
			if (_CAPTURE (move)) {
                                g_print("%cx%c%c=N", from1, to1, to2);
                        } else {
                                g_print("%c%c=N", to1, to2);
                        }
                        break;
                default:
			g_print ("???");
                        return;
                }
        }

}



void _print_LAN(int move) 
{
	char piece = PieceToChar (PIECE (move));
	char seperator = _CAPTURE(move)? 'x':'-';
	
	if (move == 0) {
		g_print ("null");
		return;
	}

	if (!SPECIAL(move)) {
		if (piece == ' ') { 
			g_print("%c%c%c%c%c",
				FileToChar(SOURCESQ(move)),
				RankToChar(SOURCESQ(move)),
				seperator,
				FileToChar(TARGETSQ(move)),
				RankToChar(TARGETSQ(move)));			
			
		} else { 
			g_print ("%c%c%c%c%c%c",
				piece,
				FileToChar(SOURCESQ(move)),
				RankToChar(SOURCESQ(move)),
				seperator,
				FileToChar(TARGETSQ(move)),
				RankToChar(TARGETSQ(move)));   
			
		}		
	} else {
		switch SPECIALCASE(move) {
		case _PLUNKMOVE:
			g_print("%c@%c%c", 
				piece,
				FileToChar(TARGETSQ(move)),
				RankToChar(TARGETSQ(move))); 		
			break; 
		case _SHORT_CASTLE:
			g_print("O-O"); 
			break;
		case _LONG_CASTLE:
			g_print("O-O-O");
			break;
		case _EN_PASSANT:
			g_print("%c%cx%c%c",
				FileToChar(SOURCESQ(move)),
				RankToChar(SOURCESQ(move)),
				FileToChar(TARGETSQ(move)),
				RankToChar(TARGETSQ(move))); 
			break;
		case _QUEEN_PROMOTION:
			g_print("%c%c%c%c%c=Q",
				FileToChar(SOURCESQ(move)),
				RankToChar(SOURCESQ(move)),
				seperator,
				FileToChar(TARGETSQ(move)),
				RankToChar(TARGETSQ(move)));   
			break;
		case _ROOK_PROMOTION: 
			g_print("%c%c%c%c%c=R",
				FileToChar(SOURCESQ(move)),
				RankToChar(SOURCESQ(move)),
				seperator,
				FileToChar(TARGETSQ(move)),
				RankToChar(TARGETSQ(move)));   
			break;
		case _BISHOP_PROMOTION:
			g_print("%c%c%c%c%c=B",
				FileToChar(SOURCESQ(move)),
				RankToChar(SOURCESQ(move)),
				seperator,
				FileToChar(TARGETSQ(move)),
				RankToChar(TARGETSQ(move)));   
			break;
		case _KNIGHT_PROMOTION: 
			g_print("%c%c%c%c%c=N",
				FileToChar(SOURCESQ(move)),
				RankToChar(SOURCESQ(move)),
				seperator,
				FileToChar(TARGETSQ(move)),
				RankToChar(TARGETSQ(move)));   
			break;
		default: 
			g_error("%c%c%c%c%c%c - Illegal move representation.",
				piece,
				FileToChar(SOURCESQ(move)),
				RankToChar(SOURCESQ(move)),
				seperator,
				FileToChar(TARGETSQ(move)),
				RankToChar(TARGETSQ(move)));   
			return;
		}
	}

	if (move & _CHECKING) {
		g_print ("+");
	}	
}

void _fast_SAN (std::ostringstream& str, int move)
{
        char piece = PieceToChar(PIECE (move));
	char from1 = FileToChar (SOURCESQ (move));
	char to1 = FileToChar (TARGETSQ (move));
	char to2 = RankToChar (TARGETSQ (move));
        if (move == 0) {
                str << "null";
                return;
        }

        if (!SPECIAL(move)) {
                if (piece == ' ') { //pawn
			if (_CAPTURE (move)) {
                            // g_print ("%cx%c%c", from1, to1, to2);
                            str << from1 << 'x' << to1 << to2; 
			} else {                            
                            // g_print ("%c%c", to1, to2);
                            str << to1 << to2; 
			}
                } else {
			if (_CAPTURE (move)) {
                            // g_print ("%cx%c%c",piece, to1, to2);
                            str << piece << 'x' << to1 << to2; 
			} else {
                            // g_print ("%c%c%c", piece, to1, to2);
                            str << piece << to1 << to2; 
			}
                }
        } else {
                switch SPECIALCASE(move) {
                case _PLUNKMOVE:
                        // g_print("%c@%c%c", piece, to1, to2);
                        str << piece << '@' << to1 << to2;  
                        break;
                case _SHORT_CASTLE:
                        // g_print("O-O");
                        str << "O-O"; 
                        break;
                case _LONG_CASTLE:
                        // g_print("O-O-O");
                        str << "O-O-O";
                        break;
                case _EN_PASSANT:
                        // g_print("%cx%c%cep", from1, to1, to2);
                        str << from1 << 'x' << to1 << to2 << "ep"; 
                        break;
                case _QUEEN_PROMOTION:
			if (_CAPTURE (move)) {
                            // g_print("%cx%c%c=Q", from1, to1, to2);
                            str << from1 << 'x' << to1 << to2 << "=Q"; 
			} else {
                            // g_print("%c%c=Q", to1, to2);
                            str << to1 << to2 << "=Q"; 
			}
                        break;
                case _ROOK_PROMOTION:
			if (_CAPTURE (move)) {
                            // g_print("%cx%c%c=R", from1, to1, to2);
                            str << from1 << 'x' << to1 << to2 << "=R"; 
                        } else {
                            // g_print("%c%c=R", to1, to2);
                            str << to1 << to2 << "=R"; 
                        }
                        break;
                case _BISHOP_PROMOTION:
			if (_CAPTURE (move)) {
                            // g_print("%cx%c%c=B", from1, to1, to2);
                            str << from1 << 'x' << to1 << to2 << "=B"; 
                        } else {
                            // g_print("%c%c=B", to1, to2);
                            str << to1 << to2 << "=B"; 
                        }
                        break;
                case _KNIGHT_PROMOTION:
			if (_CAPTURE (move)) {
                            // g_print("%cx%c%c=N", from1, to1, to2);
                            str << from1 << 'x' << to1 << to2 << "=N"; 
                        } else {
                            // g_print("%c%c=N", to1, to2);
                            str << to1 << to2 << "=N"; 
                        }
                        break;
                default:
			str << "???";
                        return;
                }
        }
}

void _fast_LAN(std::ostringstream& str, int move) 
{
 	char piece = PieceToChar(PIECE (move));
        char from1 = FileToChar (SOURCESQ (move));
	char from2 = RankToChar (SOURCESQ (move));
        char to1 = FileToChar (TARGETSQ (move));
        char to2 = RankToChar (TARGETSQ (move));
        if (move == 0) {
                str << "null";
                return;
        }

        if (!SPECIAL(move)) {
                if (piece == ' ') { //pawn
                        if (_CAPTURE (move)) {
				str << from1 << from2 << "x" << to1 << to2;                                 
                        } else {
				str << from1 << from2 << "-" << to1 << to2;                                 
                        }
                } else {
                        if (_CAPTURE (move)) {
				str << piece << from1 << from2 << "x" << to1 << to2;                                 
                        } else {
				str << piece << from1 << from2 << "-" << to1 << to2;                                 
                        }
                }
        } else {
                switch SPECIALCASE(move) {
                case _PLUNKMOVE:
			str << piece << "@" << to1 << to2;                         
                        break;
                case _SHORT_CASTLE:
			str << "O-O";                         
                        break;
                case _LONG_CASTLE:
			str << "O-O-O";                         
                        break;
                case _EN_PASSANT:
			str << from1 << from2 << "x" << to1 << to2 << "ep";                         
                        break;
                case _QUEEN_PROMOTION:
                        if (_CAPTURE (move)) {
				str << from1 << from2 << "x" << to1 << to2 << "=Q";                                 
                        } else {
				str << from1 << from2 << "-" << to2 << to2 << "=Q";                                 
                        }
                        break;
		case _ROOK_PROMOTION:
                        if (_CAPTURE (move)) {
				str << from1 << "x" << to1 << to2 << "=R";                                 
                        } else {
				str << to1 << to2 << "=R";                                 
                        }
                        break;
                case _BISHOP_PROMOTION:
                        if (_CAPTURE (move)) {
				str << from1 << "x" << to1 << to2 << "=B";                                 
                        } else {
				str << to1 << to2 << "=B";                                 
                        }
                        break;
                case _KNIGHT_PROMOTION:
                        if (_CAPTURE (move)) {
				str << from1 << "x" << to1 << to2 << "=N";                                 
                        } else {
				str << to1 << to2 << "=N";                                 
                        }
                        break;
                default:
			str << "<move exception>";                         
                        return;
                }
        }
}

int ParseXboardMove(TFastNode* node, const char*  str)
{
	/* coordinated algebraic notation */ 
	/* Normal moves: e2e4  */
	/* Pawn promotion: e7e8q  */ 
	/* Castling: e1g1, e1c1, e8g8, e8c8 */ 
	/* Bughouse drop: P@h3 */ 
	/* ICS Wild 0/1 castling: d1f1, d1b1, d8f8, d8b8  */ 
	/* FischerRandom castling: o-o, o-o-o (future)  */
	/* node is the position in which this move is made */ 
	
	char column = str[0]; 
	column -= 'a';
	char row = str[1]; 
	row -= '1'; 
	unsigned char ssq = (row<<3 | column);
	column = str[2]; 
	column -= 'a';
	row = str[3]; 
	row -= '1'; 
	unsigned char tsq = (row<<3 | column);

	if (ssq==e1 && tsq==g1 && node->matrix[e1]==KING) { 
		return ENCODESCW;
	}

	if (ssq==e8 && tsq==g8 && node->matrix[e8]==BKING) { 
		return ENCODESCB;
	}

	if (ssq==e1 && tsq==c1 && node->matrix[e1]==KING) { 
		return ENCODELCW;
	}

	if (ssq==e8 && tsq==c8 && node->matrix[e8]==BKING) {
		return ENCODELCB;
	}

	char cap = node->matrix[tsq] % KING; 

	if (strlen(str)==5) { 
		/* must be a promotion */ 
		g_print("promotion\n"); 
		switch(str[4]) { 
		case 'n' :
				/* promotion to KNIGHT */ 
				return ENCODESPECIAL(ssq,tsq,PAWN,cap,_KNIGHT_PROMOTION);
		case 'b' :
				/* promotion to BISHOP */ 
				return ENCODESPECIAL(ssq,tsq,PAWN,cap,_BISHOP_PROMOTION); 			
		case 'r':
				/* promotion to ROOK */ 
				return ENCODESPECIAL(ssq,tsq,PAWN,cap,_ROOK_PROMOTION); 
		case 'q':
				return ENCODESPECIAL(ssq,tsq,PAWN,cap,_QUEEN_PROMOTION); 
		}

		g_print("parse promotion \n"); 

	}

	if (cap) { 
		if (node->flags & _WTM) { 
			g_print("parsexboard_wcap string=%s ssq=%d tsq=%d cap=%d\n",str,ssq,tsq, cap); 
			return ENCODECAPTURE(ssq,tsq,node->matrix[ssq],cap);
		} else {
			g_print("parsexboard_bcap string=%s ssq=%d tsq=%d cap=%d\n",str,ssq,tsq, cap); 
			return ENCODECAPTURE(ssq,tsq,node->matrix[ssq]-KING,cap); 
		}			
	} else { 

		/* no capture. by matrix[tsq]==0 but it still can be 
		   an EP capture */ 
		if (_EPSQ(node) == tsq && (node->matrix[ssq]==PAWN || node->matrix[ssq]==BPAWN)) {
			return  ENCODEEP(ssq,tsq); 
		} 
			
		if (node->flags & _WTM) {
			return ENCODEMOVE(ssq,tsq,node->matrix[ssq]);
		} else {
			return ENCODEMOVE(ssq,tsq,node->matrix[ssq]-KING);
		}
	}

}

int ParseSmith(TFastNode *node, const char* str) 
{
	/* very simple notation used on ICC */ 
	/* 2 chars fromsq */ 
	/* 2 chars tosq */ 
	/* <optional> cap piece */ 
	/* <optional> prmopiece */ 
	/* cap indicator one of these :pnbrqkEcC */ 
	/* c = short castle */ 
	/* C = long castle */ 
	/* promopiece = NBRQ */ 

	char column = str[0]; 
	column -= 'a';
	char row = str[1]; 
	row -= '1'; 
	unsigned char ssq = (row<<3 | column);
	column = str[2]; 
	column -= 'a';
	row = str[3]; 
	row -= '1'; 
	char tsq = (row<<3 | column); 
	
	char cap = 0; 
	unsigned char index = 4; 
	while (index < strlen(str)) { 
		switch(str[index]) { 
		case 'p' : 
			cap = PAWN; 
			break; 
		case 'n' : 
			cap = KNIGHT; 
			break; 
		case 'b' :
			cap = BISHOP; 
			break; 
		case 'r' :
			cap = ROOK; 
			break; 
		case 'q' :
			cap = QUEEN; 
			break; 
		case 'k' :
			cap = KING; /* yeah sure. */ 
			break; 
		case 'E' :
			/* EP move */ 
			return  ENCODEEP(ssq,tsq); 
		case 'c' :
			/* short castle */ 
			if (ssq < 8) { /* must be white */ 
				return ENCODESCW; 
			} else { 
				return ENCODESCB; 
			}
		case 'C' :
			/* long castle */ 
			if (ssq < 8) { /* must be white */ 
				return ENCODELCW; 
			} else {
				return ENCODELCB;
			}
		case 'N' : 
			/* promotion to KNIGHT */ 
			return ENCODESPECIAL(ssq,tsq,PAWN,cap,_KNIGHT_PROMOTION);
		case 'B' :
			/* promotion to BISHOP */ 
			return ENCODESPECIAL(ssq,tsq,PAWN,cap,_BISHOP_PROMOTION); 			
		case 'R':
			/* promotion to ROOK */ 
			return ENCODESPECIAL(ssq,tsq,PAWN,cap,_ROOK_PROMOTION); 
		case 'Q':
			return ENCODESPECIAL(ssq,tsq,PAWN,cap,_QUEEN_PROMOTION); 

		}
		index++;
	}
	if (cap) { 
		if (node->flags & _WTM) { 
			return ENCODECAPTURE(ssq,tsq,node->matrix[ssq],cap);
		} else {
			return ENCODECAPTURE(ssq,tsq,node->matrix[ssq]-KING,cap); 
		}			
	} else { 
		if (node->flags & _WTM) {
			return ENCODEMOVE(ssq,tsq,node->matrix[ssq]);
		} else {
			return ENCODEMOVE(ssq,tsq,node->matrix[ssq]-KING);
		}
	}
}

/* 
 * fenton 
 * @node: node to store the position in  
 * @fenstr: string holding fen representation of a position 
 * 
 * convert a string containing a fen representation to a TNode* 
 *
 */ 
void fenton(TFastNode* node, const char* fenstr) 
{
/*
  +------------------------------------------------+
  | Initialize                                     |
  +------------------------------------------------+
*/
	char offset=a8;
        unsigned char pos=a8;
	bool wtm=true;
	int i,end=strlen(fenstr);
	_fast_clearnode(node);
/*
+------------------------------------------------+
| Read piece placement data                      |
+------------------------------------------------+
*/     
     for (i=0; i<end; i++) { 
	     switch (fenstr[i]) {
	     case ' ': offset = -1; break;
	     case '/': offset -= char(8); pos = offset; break;
	     case '1': pos+=1; break;
	     case '2': pos+=2; break;
	     case '3': pos+=3; break;
	     case '4': pos+=4; break;
	     case '5': pos+=5; break;
	     case '6': pos+=6; break;
	     case '7': pos+=7; break;
	     case 'p': _fast_AddPiece(node,BPAWN,pos++); break;
	     case 'n': _fast_AddPiece(node,BKNIGHT,pos++); break;
	     case 'b': _fast_AddPiece(node,BBISHOP,pos++); break;
	     case 'r': _fast_AddPiece(node,BROOK,pos++); break;
	     case 'q': _fast_AddPiece(node,BQUEEN,pos++); break;
	     case 'k': _fast_AddPiece(node,BKING,pos++); break;
	     case 'P': _fast_AddPiece(node,PAWN,pos++); break;
	     case 'N': _fast_AddPiece(node,KNIGHT,pos++); break;
	     case 'B': _fast_AddPiece(node,BISHOP,pos++); break;
	     case 'R': _fast_AddPiece(node,ROOK,pos++); break;
	     case 'Q': _fast_AddPiece(node,QUEEN,pos++); break;
	     case 'K': _fast_AddPiece(node,KING,pos++); break;
	     }
	     if (offset<0) {i++; break;};
     }
/*
+------------------------------------------------+
| Read side to move (if none, then white)        |
+------------------------------------------------+
*/
     for (bool done=false; i<=end; i++) {
	     switch (fenstr[i]) {
	     case '-':
	     case 'W':
	     case 'w':
		     done = true;
		     break;
	     case 'B':
	     case 'b':
		     done = true;
		     wtm = false;
	     }
	     if (done) {i++; break;};
     }
/*
+------------------------------------------------+
| Read castling status and ep-square             |
+------------------------------------------------+
*/
     for (; i<=end; i++) {
	     switch (fenstr[i]) {
	     case 'a': 
		     if (i==end) break; 
		     i++; 
		     if (fenstr[i]>='1' && fenstr[i] <= '8')
			     node->flags |= char((fenstr[i]-'1')*8);   
		     else i=end; 
		     break;
	     case 'b': 
		     if (i==end) break; 
		     i++; 
		     if (fenstr[i]>='1' && fenstr[i]<='8')
			     node->flags |= char((fenstr[i]-'1')*8+1); 
		     else i=end; 
		     break;
	     case 'c': 
		     if (i==end) break; 
		     i++; 
		     if (fenstr[i]>='1' && fenstr[i]<='8')
			     node->flags |= char((fenstr[i]-'1')*8+2); 
		     else i=end; 
		     break;
	     case 'd': 
		     if (i==end) break; 
		     i++; 
		     if (fenstr[i]>='1' && fenstr[i]<='8')
			     node->flags |= char((fenstr[i]-'1')*8+3); 
		     else i=end; 
		     break;
	     case 'e': 
		     if (i==end) break; 
		     i++; 
		     if (fenstr[i]>='0' && fenstr[i]<='8')
			     node->flags |= char((fenstr[i]-'1')*8+4); 
		     else i=end; 
		     break;
	     case 'f': 
		     if (i==end) break; 
		     i++; 
		     if (fenstr[i]>='0' && fenstr[i]<='8')
			     node->flags |= char((fenstr[i]-'1')*8+5); 
		     else i=end; 
		     break;
	     case 'g': 
		     if (i==end) break; 
		     i++; 
		     if (fenstr[i]>='0' && fenstr[i]<='8')
			     node->flags |= char((fenstr[i]-'1')*8+6); 
		     else i=end; 
		     break;
	     case 'h': 
		     if (i==end) break; 
		     i++; 
		     if (fenstr[i]>='0' && fenstr[i]<='8')
			     node->flags |= char((fenstr[i]-'1')*8+7); 
		     else i=end; break;
#define CASTLE_K          64
#define CASTLE_Q          128
#define CASTLE_k          256
#define CASTLE_q          512

	     case 'k': 
		     if (!_SCB(node)) {
			     node -> flags |= CASTLE_k; 
			     HashSCB(node); 
		     }
		     break;
	     case 'q': 
		     if (!_LCB(node)) { 
			     node -> flags |= CASTLE_q; 
			     HashLCB(node);
		     } 
		     break;
	     case 'K': 
		     if (!_SCW(node)) {
			     node -> flags |= CASTLE_K; 
			     HashSCW(node);
		     } 
		     break;
	     case 'Q': 
		     if (!_LCW(node)) {
			     node -> flags |= CASTLE_Q; 
			     HashLCW(node); 
		     }
		     break;
	     case ' ':
	     case '-':
		     break;
	     default: 
		     i=end;
	     }
     }
/*
+------------------------------------------------+
| Forget about half-move clock and movenumber    |
+------------------------------------------------+
*/
     if (node->flags & 63) { 
	     node->hashcode ^= ephash[node->flags & 63];
     }
     if (wtm) { 
	     node->hashcode |= 1;
	     node->flags |= _WTM; 
     } else {
	     node->hashcode = (node->hashcode  & _LL(0xFFFFFFFFFFFFFFFE));	     
     }
     g.reptable [node -> fifty] = node -> hashcode;
}

void ntofen(std::string& str, TFastNode* node)
{
  // append fen to str 
  std::ostringstream s; 
  _fast_ntofen(s,node); 
  str = str + s.str(); 
}

void _fast_ntofen(std::ostringstream& str, TFastNode* node) 
{
     int offset=56;
     int es=0;
     for (int r=7; r>=0; r--) {
	     for (int f=0; f<=7; f++) {
		     int sq = offset+f;
		     switch (node->matrix[sq]) {
		     case PAWN:
			     if (es) {
				     str << es; 				     
				     es = 0; 
			     }
			     str << "P"; 			     
			     break;
		     case PAWN+KING:
			     if (es) { 
				     str << es; 				     
				     es = 0; 
			     }
			     str << "p"; 
			     break;
		     case KNIGHT:
			     if (es) { 
				     str << es;
				     es = 0; 
			     }
			     str << "N"; 
			     break;
		     case KNIGHT+KING:
			     if (es) { 
				     str << es;
				     es = 0; 
			     }
			     str << "n";
			     break;
		     case BISHOP:
			     if (es) {
				     str << es; 
				     es = 0;
			     }
			     str << "B";
			     break;
		     case BISHOP+KING:
			     if (es) { 
				     str << es;  
				     es = 0; 
			     }
			     str << "b";
			     break;
		     case ROOK:
			     if (es) { 
				     str << es;
				     es = 0; 
			     }
			     str << "R"; 
			     break;
		     case ROOK+KING:
			     if (es) {
				     str << es;  
				     es = 0; 
			     } 
			     str << "r";
			     break;
		     case QUEEN:
			     if (es) {
				     str << es; 
				     es = 0; 
			     }
			     str << "Q";
			     break;
		     case QUEEN+KING:
			     if (es) { 
				     str << es;
				     es = 0; 
			     }
			     str << "q";
			     break;
		     case KING:
			     if (es) {
				     str << es;
				     es = 0; 
			     }
			     str << "K";
			     break;
		     case KING+KING:
			     if (es) { 
				     str << es;
				     es = 0; 
			     }
			     str << "k";
			     break;
		     default: es++;
		     }
	     }
	     if (es) {
		     str << es; 
		     es=0;
	     }
	     if (r>0) { 
		     str << "/"; 
	     }
	     offset -= 8;
     }
     
     if (node->flags & _WTM) { 
	     str << " w "; 
     } else { 
	     str << " b ";  
     }

     if (_CASTLEW(node) | _CASTLEB(node)) {
	     if _SCW(node) {
		     str << "K";
	     }
	     if _LCW(node) {
		     str << "Q";
	     }
	     if _SCB(node) {
		     str << "k";
	     }
	     if _LCB(node) {
		     str << "q";
	     }
	     str << " "; 	     
     } else {
	     str << "-";	     
     }

     if ( _EPSQ(node)) {
	     str << _EPSQ(node); 
     } else {
	     str << "- "; 
     }

     str << "0 1";      
}

void parse_epd(std::string& src, std::string& fen, std::string& commands)
{
	/* simpel. 1e 4 velden zijn voor de fen, de rest commands */ 
	int i=0; 
	int len=src.length(); 

	/* piece placement data */ 
	for (i=0; i<len && src[i] != ' '; i++) { 
		fen += src[i]; 
	} 
	i++;
	fen += ' '; 
	
	for (;i<len && src[i] != ' '; i++) {
		fen += src[i];
	}
	i++; 
	fen += ' '; 
	
	for (;i<len && src[i] != ' '; i++) { 
		fen += src[i]; 
	}

	i++; 
	fen += ' '; 
	
	for (;i<len && src[i] != ' ';i++) { 
		fen += src[i]; 
	}

	i++; 

	/* lees de commands */ 
	for (;i<len && src[i] != 13 && src[i] != 10;i++) { 
		if (src[i]=='"') { 		       
			commands += "'"; 
		} else { 
			commands += src[i]; 
		}
	}
}
