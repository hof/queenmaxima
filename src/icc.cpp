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

#include <iostream>
#include <stdlib.h>
#include "icc.h" 
#include "main.h" 
#include "parser.h" 
#include "w17.h" 
#include "engine_impl.h" 
#include "tm_icc.h" 

const std::string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
    return buf;
}

void connect_to_ics(const char *host, const int port) 
{ 
    struct hostent *hp;
    struct sockaddr_in server; 

    hp = gethostbyname(host); 
    if (hp==NULL) {  
    	std::cerr << "maxima failed to resolve hostname\n";
    	exit(1);
    	return;
    }
 
    memset( (char *)&server, 0, sizeof (server));
    memcpy( (char *)&server.sin_addr, hp->h_addr, hp->h_length);
    server.sin_family = hp->h_addrtype;
    server.sin_port = htons((unsigned short)port);

    MainForm.socket_connection = socket(hp->h_addrtype,SOCK_STREAM,0);  
    if (MainForm.socket_connection<0) { 
    	std::cerr << "could not create socket\n";
    }

    int ecode=0; 
    ecode = connect(MainForm.socket_connection, (sockaddr *)&server, sizeof (server)); 
    if (ecode<0) {
    	std::cerr << "connect error: " << ecode << "\n";
    	return;
    }

    MainForm.lost_connection = false; 
}

bool processing_adjourned_moves = false; 
int  nmoves_to_follow = 0; 
int  aborts = 0; 
bool graceful_shutdown = false; 

void process_datagram()
{
	int id = atoi(MainForm.dgram_fields[0].c_str()); 
	int move = 0; 
	int challenger_rating;
	int gameresult = 0; 
	bool max_has_white;
	bool remote_cmd = false; 

	switch (id) { 
	case DG_WHO_AM_I:		
		std::cout << "icc logged in as " << MainForm.dgram_fields[1] << "\n"; 
		MainForm.myname = MainForm.dgram_fields[1];
                
		// autoseek
		if (MainForm.autoseek) {
		    send(MainForm.socket_connection, "seek1\nseek2\nseek3\n", 18, 0);
		    std::cout << currentDateTime() << " icc autoseeking\n";
		}
                
		break; 
	case DG_OPEN:
		break; 	
	case DG_MY_GAME_STARTED:
		/* 
		   Form    (gamenumber whitename blackname wild-number rating-type rated
		            white-initial white-increment black-initial black-increment
		            played-game {ex-string} white-rating black-rating game-id
		            white-titles black-titles irregular-legality irregular-semantics
		            uses-plunkers)
		*/ 
		MainForm.gamenumber = atoi(MainForm.dgram_fields[1].c_str()); 
		MainForm.whitename = MainForm.dgram_fields[2]; 
		MainForm.blackname = MainForm.dgram_fields[3]; 
		MainForm.wildnumber = atoi(MainForm.dgram_fields[4].c_str()); 
		MainForm.rating_type = MainForm.dgram_fields[5]; 	
		MainForm.rated = (MainForm.dgram_fields[6] == "1"); 
		MainForm.basetime = atoi(MainForm.dgram_fields[7].c_str())*60*1000; /* in msecs */ 
		MainForm.increment = atoi(MainForm.dgram_fields[8].c_str())*1000; /* in msecs */ 
		/* also set black and whitetime */ 
		MainForm.whitetime = MainForm.basetime; 
		MainForm.blacktime = MainForm.basetime; 
		MainForm.whiterating = atoi(MainForm.dgram_fields[13].c_str()); 
		MainForm.blackrating = atoi(MainForm.dgram_fields[14].c_str());
		MainForm.white_titles = MainForm.dgram_fields[16]; 
		MainForm.black_titles = MainForm.dgram_fields[17]; 

		max_has_white = (MainForm.myname == MainForm.whitename);

		if (max_has_white) {
			MainForm.opponent_player_id = MainForm.dbhandle->save_player(MainForm.blackname.c_str(),
					MainForm.black_titles.c_str(), "icc");
		} else {
			MainForm.opponent_player_id = MainForm.dbhandle->save_player(MainForm.whitename.c_str(),
					MainForm.white_titles.c_str(), "icc");
		}

		// init engine
		_fast_new_game(); 
		if (MainForm.wildnumber==17) { 
			w17_new_game(); 
		}

		// calculate drawscore 
		if (max_has_white) {
			g.drawscore_wtm = (MainForm.blackrating - MainForm.whiterating) * 3;
			if (g.drawscore_wtm > 1000) { 
			    g.drawscore_wtm = 1000; 
			}
			if (g.drawscore_wtm < -1000) { 
			    g.drawscore_wtm = -1000; 
			}
			g.drawscore_btm = -g.drawscore_wtm;
		} else {
			g.drawscore_btm = (MainForm.whiterating - MainForm.blackrating) * 3;
			if (g.drawscore_btm > 1000) {
			    g.drawscore_btm = 1000; 
			}
			if (g.drawscore_btm < -1000) { 
			    g.drawscore_btm = -1000; 
			}
			g.drawscore_wtm = -g.drawscore_btm;
		}
		std::cout << "icc starting game " << MainForm.whitename << " (" << MainForm.whiterating << ") vs. "; 
		std::cout << MainForm.blackname << " (" << MainForm.blackrating << ")\n"; 
		// std::cout << "icc drawscore_wtm=" << g.drawscore_wtm << " drawscore_btm=" << g.drawscore_btm << "\n";

		break; 
	case DG_MY_GAME_RESULT:
		/* Form: (gamenumber become-examined game_result_code score_string2
  		   description-string) */
	    
		stop_thinking(); 

		gameresult = 0; 

		if (engine_rootply > 5) { 

			std::cout << "GAME_RESULT: " << MainForm.dgram_fields[3] << " " << MainForm.dgram_fields[4] << std::endl;

			if (MainForm.dgram_fields[4] == "1-0") { 
				game_ended(3, MainForm.dgram_fields[3]); /* white wins */
			} else if (MainForm.dgram_fields[4] == "0-1") {
				game_ended(1, MainForm.dgram_fields[3]); /* black wins */
			} else if (MainForm.dgram_fields[4] == "1/2-1/2") { 
				game_ended(2, MainForm.dgram_fields[3]); /* draw */
			}
			aborts = 0; 
		} else { 
			std::cerr << "WARNING: game not saved. It's too short\n"; 
			aborts++; 

			// check for too many abort, may be a bug 
			if (aborts==5) { 
				std::cerr << ("Too many aborted games. \n"); 
				exit(0); 
			}
		}
                
		// check if we have to exit		
		if (graceful_shutdown) { 
		    std::cout << "icc graceful shutdown\n";
		    send(MainForm.socket_connection,"exit\n",5,0); 
		}

		// autoseek 
		if (MainForm.autoseek) {
		    send(MainForm.socket_connection, "seek1\nseek2\nseek3\n", 18, 0);
		    std::cout << currentDateTime() << " icc autoseeking\n";
		}	
		break; 
	case DG_SEND_MOVES:
		/* Form: (gamenumber algebraic-move smith-move time clock) */ 
		/* last 4 fields are optional, we only use algebraic */ 
		/* we ues all fields. */ 
		// g_print("DG_SEND_MOVES movestring=%s\n",MainForm.dgram_fields[3]->str); 
		
		/* parse de move */ 
		move = ParseSmith(&engine_rootnode, MainForm.dgram_fields[3].c_str());
		// g_print("-*-");

		if (nmoves_to_follow) { 
			std::cout << "DG_SEND_MOVE: adjourned. move=";
			print_move(move); 			
			nmoves_to_follow--; 
			std::cout << boost::format("moves_left=%d\n") % nmoves_to_follow;
			game_move_forward(move);

			if (nmoves_to_follow) { 			
				MainForm.gameply++; /* anders dubbeltelling hieronder */ 
				MainForm.relayply++;
			}
			move = 0; 
		}

		if (!nmoves_to_follow) { 

			MainForm.gameply++;
			if (MainForm.playoneven && MainForm.gameply %2 == 0) { 	

				MainForm.relayply++;
                            
				game_search (MainForm.gameply, move, MainForm.whitetime, MainForm.blacktime, MainForm.increment);
                                                                                   
			} else if ((!MainForm.playoneven) && (MainForm.gameply % 2 != 0)) {    
			    
				MainForm.relayply++;
                            
				game_search (MainForm.gameply, move, MainForm.whitetime, MainForm.blacktime, MainForm.increment);
                                                        
			}
		}        
		break;
	case DG_MOVE_LIST: 
		/* Format: (gamenumber initial-position 
		            {white-move1-info}{black-move1-info}
                            {white-move2-info} ...) */	
		std::cout << "DG_MOVE_LIST";
		break; 
	case DG_FEN:
		/* Format: (gamenumber {FEN-string}) */ 
		/* beginpositie setten in MainForm.fastposition */ 
		std::cout << "DG_FEN\n";

		// lame check to see if it is a continued game
		if (std::string("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1") == MainForm.dgram_fields[2]) { 
			MainForm.continued_game = false; 
		} else {
			MainForm.continued_game = true; 
		}
		set_fen(MainForm.dgram_fields[2].c_str()); 
		MainForm.gameply = 0; 
		MainForm.relayply = 0;
		MainForm.playoneven = false; 

		max_has_white = (MainForm.myname == MainForm.whitename);
		// calculate drawscore 
		if (max_has_white) {
			g.drawscore_wtm = (MainForm.blackrating - MainForm.whiterating) * 3;
			if (g.drawscore_wtm > 1000) { 
			    g.drawscore_wtm = 1000; 
			}
			if (g.drawscore_wtm < -1000) { 
			    g.drawscore_wtm = -1000; 
			}
			g.drawscore_btm = -g.drawscore_wtm;
		} else {
			g.drawscore_btm = (MainForm.whiterating - MainForm.blackrating) * 3;
			if (g.drawscore_btm > 1000) {
			    g.drawscore_btm = 1000; 
			}
			if (g.drawscore_btm < -1000) { 
			    g.drawscore_btm = -1000; 
			}
			g.drawscore_wtm = -g.drawscore_btm;
		}

		// check if we have to start thinking  
		if (engine_rootnode.flags & _WTM && max_has_white) {
		    MainForm.playoneven = true;
		    game_search(0, 0, MainForm.whitetime, MainForm.blacktime, MainForm.increment);
		} else if (!max_has_white) {
		    MainForm.playoneven = true; 
		    game_search(0, 0, MainForm.whitetime, MainForm.blacktime, MainForm.increment);
		}
		break; 
	case DG_MSEC: 
	    // (gamenumber color msec running)

		if (MainForm.dgram_fields[2] == "W") {
		// update time of white 
		MainForm.whitetime = atoi(MainForm.dgram_fields[3].c_str()); 		       
	    } else { 
		// update time of black
		MainForm.blacktime = atoi(MainForm.dgram_fields[3].c_str()); 		       
	    }		

	    break; 
	case DG_MATCH:
	    /* 
	       Form:   (challenger-name challenger-rating challenger-titles
	       receiver-name   receiver-rating   receiver-titles
	       wild-number rating-type is-it-rated is-it-adjourned
	       challenger-time-control receiver-time-control
	       challenger-color-request [assess-loss assess-draw assess-win]
	       fancy-time-control)
	    */ 
	    challenger_rating = atoi (MainForm.dgram_fields[2].c_str());
	    
	    if (MainForm.autoaccept) { 
	    	send(MainForm.socket_connection,"accept ",7, 0);
	    	send(MainForm.socket_connection,MainForm.dgram_fields[1].c_str(),MainForm.dgram_fields[1].length(), 0);
	    	send(MainForm.socket_connection,"\n",1, 0);
	    }
	    break; 
	case DG_KIBITZ: 
	    /* Form: (gamenumber playername titles kib/whi ^Y{kib string^Y}) */ 
	    // g_print("DG_KIBITZ\n"); 
	    // g_print("%s kibitzes: %s\n",MainForm.dgram_fields[2]->str, MainForm.dgram_fields[5]->str);  
	    break; 
	case DG_PERSONAL_TELL:
	    /* Form: (playername titles ^Y{tell string^Y} type) */ 
		
	    remote_cmd = false; 
		
	    // check if this a remote command 
	    if (MainForm.dgram_fields[1] == "hof") { 
	    	remote_cmd = true;
	    }
	    if (MainForm.dgram_fields[1] == "Hajewiet") { 
	    	remote_cmd = true;
	    }
	    if (MainForm.dgram_fields[1] == "fridus") { 
	    	remote_cmd = true;
	    }
	    
	    // g_print("%s tells you: %s\n", MainForm.dgram_fields[1]->str, MainForm.dgram_fields[3]->str);
	    
	    if (remote_cmd) { 
	    	if (MainForm.dgram_fields[3] == "graceful") {
	    		graceful_shutdown = true;
	    	}
	    	send(MainForm.socket_connection, MainForm.dgram_fields[3].c_str(), MainForm.dgram_fields[3].length(), 0);
	    	send(MainForm.socket_connection, "\r\n",2, 0);
	    }		
	    break; 
	case DG_POSITION_BEGIN:
		/*  (gamenumber {initial-FEN} nmoves-to-follow) */ 
		
		// g_print("DG_POSITION_BEGIN: FEN=%s nmoves=%s\n",MainForm.dgram_fields[2]->str,MainForm.dgram_fields[3]->str); 
		nmoves_to_follow = atoi(MainForm.dgram_fields[3].c_str()); 
		MainForm.gameply = 0; 
		MainForm.relayply = 0;
		MainForm.playoneven = false; 
		if (!nmoves_to_follow) { 					 
		    // check if we need to start searching 
		    MainForm.continued_game = false; 		
			set_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"); 
			if (MainForm.myname == MainForm.whitename) { 
				MainForm.playoneven = true; 
				// fixme: time manager 
				game_search(0, 0, MainForm.whitetime, MainForm.blacktime, MainForm.increment);			
			}						       

		} else { 
			// continued game 
			set_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
			MainForm.continued_game = false; 
			if (MainForm.myname == MainForm.whitename) { 
				MainForm.playoneven = true; 
			}
		}
		break; 
	default:
		// for (int i=0; i<=MainForm.dgram_field_count;i++) { 
		//	g_print("dgram field[%d] = %s\n",i,MainForm.dgram_fields[i]->str);
		//}
		break;
	}
}

void clean_datagram() 
{
	MainForm.dgram_fields.clear(); 
}

// 
// mini icc datagram parser. 
void icc_connection_cb()
{ 
    char buffer[8092];  
    int bytesread = recv(MainForm.socket_connection, buffer,8092, 0); 
    if (!bytesread) {
    	// end of file from socket
    	MainForm.lost_connection = true;
    	return;
    }
    for (int i=0; i<bytesread;i++) { 
	switch(MainForm.ICCstate) {
	case 0 : // state 0: wait for login  		
	    switch(buffer[i]) { 
	    case 10 : 
		break; 
	    case 13 : 			
		MainForm.ICSCurrentLine = ""; 								
		break;
	    default:
		MainForm.ICSCurrentLine += buffer[i]; 
		
		// check login
		if (MainForm.ICSCurrentLine == "login:") {			 
		    send(MainForm.socket_connection,
"level2settings=100000000010000110000000101001010111100000000000000000001000000000000000000000000000000000000000000001\n",118, 0); 
//                                                                                    ^             
//                                                                                    |
//                                                                                   70 
//
		    std::string login_name = MainForm.properties[std::string("-icc-user")];
		    if (login_name == "guest") { 
		    	// do a guest login
		    	send(MainForm.socket_connection, "g\n\nset prompt 0\nset style 13\n", 29, 0);
		    	MainForm.ICCstate = 1; /* alleen bij een guest login */
		    } else {
		    	send(MainForm.socket_connection, login_name.c_str(), login_name.length(), 0);
		    	send(MainForm.socket_connection, "\n", 1, 0);
		    }
		}
				
		// check password
		if (MainForm.ICSCurrentLine == "password:") { 
		    std::string passwd = MainForm.properties[std::string("-icc-password")];
		    send(MainForm.socket_connection, passwd.c_str(), passwd.length(), 0);
		    send(MainForm.socket_connection, "\n", 1, 0); 
		    send(MainForm.socket_connection,
			 "delphi456852\nset prompt 0\nset interface internet enabled Maxima\nset style 13\n", 77, 0 );
		    send(MainForm.socket_connection, 
			 "seek1\nseek2\nseek3\n",18, 0);
		    MainForm.ICCstate = 1;
		}
		break; 
	    }
	    break; 
	case 1 : // state 1: look for escape code 
	    if (buffer[i]=='\031') { 
		MainForm.ICCstate = 2;
	    }
	    break; 
	case 2 : // state 2: look for a field (after ^Y)
	    switch(buffer[i]) { 
	    case '(' : 
		// start of a new datagram                               				
		MainForm.ICCstate = 4; 
		break; 
	    case ')': 
		// end of datagram
		process_datagram(); 
		clean_datagram(); 
		MainForm.ICCstate = 1;
		break; 
	    case '{' : 
		// start of a string field 	 			  
		MainForm.ICCstate = 5; 
		break; 
	    case '}' : 
		// end of escaped string field 
		MainForm.dgram_fields.push_back(MainForm.current_field);
		MainForm.current_field = "";
		MainForm.ICCstate = 3; 
		break; 
	    }
	    break;
	case 3: // state 3: search for the next field (after a field is read) 
	    switch (buffer[i]) { 
	    case ' ' : 
		// skip 
		break; 
	    case '{' :
		// new string field 			  
       		MainForm.ICCstate = 5; 
		break;
	    case '\031' : 
		// start of an escaped string field 
		MainForm.ICCstate = 2; 
		break;
	    default:
		// start a new field 	 
		MainForm.current_field = buffer[i]; 
		MainForm.ICCstate = 4;
		break; 
	    }
	    break; 								
	case 4: // read a field 
	    switch(buffer[i]) { 
	    case ' ' :          
		// end of field 
		MainForm.dgram_fields.push_back(MainForm.current_field);
		MainForm.current_field = "";
		MainForm.ICCstate = 3; 
		break; 
	    case '\031' :
		// end of field 
	       	MainForm.dgram_fields.push_back(MainForm.current_field);
		MainForm.current_field = "";
		MainForm.ICCstate = 2; 
		break;
	    default:
		MainForm.current_field += buffer[i];
		break; 
	    }
	    break; 	
	case 5: // reading of a { } and ^{ ^} string string 
	    switch(buffer[i]) { 
	    case '}' : 
		// end of a string
		MainForm.dgram_fields.push_back(MainForm.current_field);
		MainForm.current_field = "";
		MainForm.ICCstate = 3; 
		break; 
	    case '\031': 
		// process the escape 
		MainForm.ICCstate = 2; 
		break;
	    default:
		MainForm.current_field += buffer[i]; 
		break; 				
	    }
	    break; 
	}
    }
}
