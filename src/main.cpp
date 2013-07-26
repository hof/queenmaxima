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
#include <stdio.h> 

#include <pthread.h>
#include <signal.h> 
#include <sys/time.h>
#include <sys/types.h> 
#include <fcntl.h>
#include <unistd.h>  
#include "config.h" 
#include "main.h"
#include "fast.h"
#include "db_pg.h"
#include "parser.h" 
#include "w0_search.h" 
#include "hash.h"
#include "engine_impl.h" 
#include "run_test.h"
#include "tm_icc.h"
#include "book.h" 
#include "fics.h" 
#include "icc.h"
#include "w17_search.h"
#include "w17.h" 
#include "legality.h" 

/*  ---- global vars */ 

TMainForm MainForm; 

void game_ended(int gameresult, std::string result_code)
{	
    if (MainForm.properties[std::string("-db-save-game")]=="false")
    {
    	return;
    }

    int maxima_firstply; 
    bool max_white = (MainForm.myname == MainForm.whitename);
    maxima_firstply = max_white? 1 : 2;

    /* check if maxima won or lost */ 
    int result_flag = 0; 
    if (gameresult == 3 && !max_white) { 
    	/* maxima lost */
    	result_flag = 0;
    } else if (gameresult == 1 && max_white) { 
    	/* maxima lost */
    	result_flag = 0;
    } else if (gameresult == 2) {
    	result_flag = 1;
    } else { 
    	result_flag = 2;
    }

    int white_id = MainForm.dbhandle->save_player(MainForm.whitename.c_str(), MainForm.white_titles.c_str() , "icc");
    int black_id = MainForm.dbhandle->save_player(MainForm.blackname.c_str(), MainForm.black_titles.c_str() , "icc"); 

    if (white_id==-1) { 
	return; 
	/* hmm error. cancel save */ 
    }
    if (black_id==-1) { 
	/* hmm error. cancel save */ 
	return; 
    }

    int rating_type = 0; 
    if (MainForm.rating_type == "Blitz") { 
    	rating_type = 1;
    } else if (MainForm.rating_type == "Standard") { 
    	rating_type = 2;
    } else if (MainForm.rating_type == "Loser's") { 
    	rating_type = 3;
    }

    int game_id = -1; 
    
    if (MainForm.dbhandle != NULL) { 
        game_id = MainForm.dbhandle->save_game(white_id,black_id, result_flag,
                                               MainForm.wildnumber, MainForm.whiterating,
                                               MainForm.blackrating, MainForm.basetime,
                                               MainForm.increment,rating_type,MainForm.rated,
                                               result_code);
    }
    
    if (game_id==-1) { 
    	return;
    }

    // ok save the game 
    for (int i=1; i<engine_rootply ;i++) { 
    	MainForm.dbhandle->save_move(game_id,i,engine_records[i].move, engine_records[i].thinktime);
    } 

    // ok split the w0 and w17 learning 
    if (MainForm.wildnumber == 0) {

	// update the learn table; 
	// result flag stuff is unclean and should be made more clear
	int winner_firstply; //note: unclear engine_info starts at 1
	bool max_won = true;
	
	if (result_flag == 0) { //maxima lost this game
	    max_won = false;	
	} else if (result_flag == 1) { //count a draw as a win or a loss, based on drawscore
	    if (max_white && g.drawscore_wtm < 0) {
		max_won = false;
	    } else if (!max_white && g.drawscore_btm<0) {
		max_won = false;
	    }                               
	} 
	
	if (max_won) {
	    winner_firstply = max_white? 1 : 2;
	} else {
	    winner_firstply = max_white? 2 : 1; 
	}
	
	if (engine_rootply > 511) {
	    engine_rootply = 511;
	}
	
	
	learn_avoid (maxima_firstply, engine_rootply-1, max_won, MainForm.wildnumber );

	// determine the win_score and loss_score for use in the learnbook. 
	// win_score is used if maxima won. 
	// 
	int score = 0; //score to place in the learning table "win_score" or "loss_score" field
	
	bool computer;

	if (max_white) { 
	    computer = (MainForm.black_titles == "C");
	    score = learn_update_score (MainForm.whiterating, MainForm.blackrating, computer, max_won);
	} else {
	    computer = (MainForm.white_titles =="C");
	    score = learn_update_score (MainForm.blackrating, MainForm.whiterating, computer, max_won);
	} 
	
	//update learn-book adh van die score	
	learning_book_update_score (winner_firstply, engine_rootply-1, score, MainForm.wildnumber );
	
    } 

    if (MainForm.wildnumber==17) { 
	// w17 old style learning 
	w17_learning_update (engine_rootply-1, gameresult);
    }
 
    /* ok check if we have to upate the stats */ 
    if (MainForm.rated) { 
	switch (rating_type) { 
	case 0:
	    /* Bullet */ 
	    if (MainForm.myname == MainForm.blackname) { 
		/* opp is white */ 
		MainForm.dbhandle->update_stats("bullet_stats",white_id, MainForm.whiterating, 
						result_flag);
	    } else { 
		/* opp is black */ 
		MainForm.dbhandle->update_stats("bullet_stats",black_id, MainForm.blackrating, 
						result_flag); 
	    }
	    break; 
	case 1: 
	    /* Blitz */ 
	    if (MainForm.myname == MainForm.blackname) { 
		/* opp is white */ 
		MainForm.dbhandle->update_stats("blitz_stats",white_id, MainForm.whiterating, 
						result_flag);
	    } else { 
		/* opp is black */ 
		MainForm.dbhandle->update_stats("blitz_stats",black_id, MainForm.blackrating, 
						result_flag); 
	    }
	    break; 
	case 2: 
	    /* Standard */ 
	    if (MainForm.myname == MainForm.blackname) { 
		/* opp is white */ 
		MainForm.dbhandle->update_stats("std_stats",white_id, MainForm.whiterating, 
						result_flag);
	    } else { 
		/* opp is black */ 
		MainForm.dbhandle->update_stats("std_stats",black_id, MainForm.blackrating, 
						result_flag); 
	    }
	    break; 
	case 3:
	    /* Loser's */ 			 
	    if (MainForm.myname == MainForm.blackname) { 
		/* opp is white */ 
		MainForm.dbhandle->update_stats("losers_stats",white_id, MainForm.whiterating, 
						result_flag);
	    } else { 
		/* opp is black */ 
		MainForm.dbhandle->update_stats("losers_stats",black_id, MainForm.blackrating, 
						result_flag); 
	    }
	    break; 
	}                
    }
}

void main_init()
{
    init_engine ();

    if (MainForm.properties[std::string("-db-save-game")]=="true")
    {

        MainForm.dbhandle = new db_pg();

        MainForm.dbhandle->connect(MainForm.properties[std::string("-db-host")].c_str(),
        		       MainForm.properties[std::string("-db-user")].c_str(),
        		       MainForm.properties[std::string("-db-password")].c_str(),
        		       MainForm.properties[std::string("-db-database")].c_str() );

        set_dbhandle(MainForm.dbhandle);
       	std::cout << "Connected to PostgreSQL database\n";

    }
    
    /* MainForm init */
    MainForm.ICSCurrentLine = ""; 
    MainForm.whitename      = "maxima"; 
    MainForm.blackname      = "";
    MainForm.white_titles   = "";
    MainForm.black_titles   = "";
    MainForm.myname         = "maxima"; 
    MainForm.rating_type    = "Blitz";
    MainForm.basetime       = 5*60; 
    MainForm.increment      = 0; 
    MainForm.ICCstate       = 0; 
    MainForm.autoaccept     = true;
    MainForm.autoseek       = true;
}

void Master_Exit()
{
    g.dbhandle->close(); 
    free (hashtable_0);   
    free (hashtable_1);   
    free (pawn_hashtable);
}

void process_xboard_command(std::string& command)
{
    if (command == "xboard") { 
	// init phase 
	return;
    }

    if (command == "new") { 
	// new game 
	stop_thinking();
	set_fen("rnbqkbnr/pppppppp/////PPPPPPPP/RNBQKBNR w kqKQ");
	return; 
    }

    if (command.find("variant ")==0) { 
	// set variant type 
	return;
    }

    if (command == "random") { 
	// random - gnu chess command do nothing 
	return; 
    }

    if (command == "quit") { 
	stop_thinking(); 
	Master_Exit(); 
	exit (0); 
    }

    if (command == "force") { 
	// enter xboard "force" mode 
	return; 
    }

    if (command == "white") { 
	// set white to move 
	return;
    }

    if (command == "black") { 
	// set black to move 
	return;
    }

    if (command.find("level ")==0) { 
	// set tc 
	std::cout << "setting level\n"; 
	return;
    }

    if (command.find("st ")==0) { 
	// limit tc 
	return;
    }

    if (command.find("sd ")==0) { 
	// limit search depth 
	return;
    }

    if (command.find("time ")==0) {
	// set engines clock 
	
	// fixme: we are black :) 
	int time; 
	sscanf(command.c_str(), "time %d", &time); 
	std::cout << "_maxima_time_ " << time << "\n"; 

	MainForm.blacktime = time*10;
	return;
    }

    if (command.find("otim ")==0) {
	// set the opponents time 

	// fixme: opp is white 
	int time; 
	sscanf(command.c_str(), "otim %d", &time); 
	std::cout << "_maxima_otim_ " << time << std::endl; 
	MainForm.whitetime = time*10; 
	return;
    }

    if (command == "?") { 
	// move now 
	return;
    }

    if (command == "draw") { 
	// draw offer 
	return;
    }

    if (command.find("result ")==0) { 
	// game result  
	return; 
    }

    if (command == "edit") { 
	// go in edit postition mode 
	return;
    }

    // fixme: add the edit commands 
	
    if (command == "hint") { 
	// user asks for a hint 
	return; 
    }

    if (command == "bk") { 
	// user selected bk 
	return; 
    }

    if (command == "undo") { 
	// undo 
	return; 
    }

    if (command == "remove") {
	// undo a move
	return;
    }

    if (command == "hard") { 
	// turn on pondering 
	return;
    }

    if (command == "easy") {
	// turn off pondering 
	return; 
    }

    if (command == "post") { 
	// turn on thinking info  
	return;
    }

    if (command == "nopost") { 
	// turn off thinking info 
	return;
    }

    if (command == "analyze") { 
	// analyze mode 
	return;
    }

    // must be a move  
    // fixme: we are black  
    MainForm.gameply++; 		
    int move = ParseXboardMove(&engine_rootnode, command.c_str());
    std::cout << "_parsed_move="; 
    _print_LAN(move);
    std::cout << "\n"; 
    int tmax = tm_icc (MainForm.blacktime , MainForm.whitetime , MainForm.increment); 
    std::cout << "tmax = " << tmax << "\n"; 
    std::cout << "going to search\n"; 

    game_search (MainForm.gameply, move, MainForm.whitetime, MainForm.blacktime, MainForm.increment);

}

int main_xboard(int argc, char *argv[])
{
    std::cout << "xboard not (yet) supported int the gcc/linux version\n"; 
    return 0;  
}

int main_runtest(int argc, char *argv[])
{
    std::cout << "fixme: local var in main_runtest\n"; 	
    TFastNode* node = new TFastNode();
    run_test (node, std::string(argv[2]), atoi (argv[3]));
    delete node;
    return 0; 
}

int main_icc(int argc, char *argv[])
{
    connect_to_ics(MainForm.properties[std::string("-icc-host")].c_str(),
		   atoi(MainForm.properties[std::string("-icc-port")].c_str()));

    while (!MainForm.lost_connection) { 
		fd_set rfds;
		fd_set exds;

		FD_ZERO(&rfds);
		FD_ZERO(&exds);

		FD_SET(MainForm.socket_connection, &rfds);
		FD_SET(MainForm.socket_connection, &exds);

		int select_res;
		if ((select_res = select(MainForm.socket_connection+1, &rfds, NULL, NULL, NULL ))) {
			if (select_res == -1) {
				continue;
			}
			if (FD_ISSET(MainForm.socket_connection, &rfds)) {
					icc_connection_cb();
			}
		}
    }

    std::cout << "main icc connection lost\n";
    return 1; 
}

int main_test(int argc, char *argv[])
{
    TFastNode * node = new TFastNode ();
 	
    std::cout << "maxima test\n";	
    g.maxtime = 10000000;
    g.stoptime = g.maxtime;
    g.stopsearch = false;
    g.checkbook = false;
//	fenton(node, string("r1b2r1R/1p2bpk1/4p1p1/4P1N1/p2p4/5Q2/qPP2PP1/1NKR4 w - -")); 
//	fenton(node, string("b2qr1k1/5ppp/pn6/1p1n1N2/3PR3/P2B4/1B3PPP/2Q3K1 w - - 0 1"));
//      fenton(node, "3r2r1/1k1b3n/1pp2q1b/3P1Np1/R4pp1/1Q3R1P/1PP5/2B4K w - -");

    // fenton(node, "1nb1k1n1/3p1pp1/8/1P5r/4p3/8/3PPPP1/1NBQKBN1 w - - 0 12");
    // fenton(node, "1nb1k1n1/3p1pp1/8/8/1r2p3/8/3PPPP1/1N1QKBN1 w - - 0 14");
    // fenton(node, "1nb1k1n1/3p1pp1/8/8/4p3/8/3PPPP1/1rQ1KBN1 w - - 0 1");

    fenton(node, "rnb1kbnr/1p1p3p/p1p3p1/4p3/1P6/P1P5/3PP1PP/RNBQKBR1 w Qkq e6 0 8");
    
    // fenton(node, "1nb2knr/1p1p3p/2p3p1/8/2r5/4p3/6PP/4K1R1 w - - 0 18");

    // na 9.cxb5
    //fenton(node, "rnb1k1nr/1p1p3p/p5p1/1p2p3/8/B1P5/3PP1PP/RN1QKBR1 w Qkq - 0 10");
    //fenton(node, "rnb1k1nr/3p3p/p5p1/8/8/B3p3/p5PP/1N2K1R1 w kq - 0 16");
    // fenton(node, "rnb1k1nr/3p3p/p5p1/8/8/B3p3/6PP/1r2KR2 w kq - 0 17");
    //fenton(node, "rnb1k1nr/3p3p/p5p1/8/8/B3p3/p5PP/1N2KR2 b kq - 1 16");

    //g_timer_reset(g.timer);
    //g_timer_start(g.timer);
    //w17_iterate (node);

    //int first = 0, last = 0, move = 0;
    //last = _fast_gencapsw (node, first);

    //while (last > first)  {
    //    move = w17_select_capture_w (node, first, last);

     //   _print_LAN(move);

     //   if (! legal_move_w (node, move)) {
     //       std::cout << " illegal move ";
     //   }

     //   std::cout << std::endl;
    //}

    //for (int i=first; i<last; i++) {
    //    std::cout << i << ": ";
    //    _print_LAN(g.tmoves[i]);
    //    std::cout << std::endl;
    //}

    /* test postgres code */
    MainForm.dbhandle = new db_pg();

    MainForm.dbhandle->connect("localhost",
        		       "bugchess", // user  
        		       "bugchess", // password 
        		       "bugchess" ); // database 

    set_dbhandle(MainForm.dbhandle);       	
    
    
    MainForm.dbhandle->close(); 
    
    delete node;        
    return 0; 
}

void display_usage() 
{
    std::cout << "maxima chess engine (" << MAXIMA_VERSION << ") "__DATE__ << " "__TIME__"\n";
    std::cout << "Copyright (C) 1996-2013 Erik van het Hof and Hermen Reitsma. All rights reserved.\n" 
	"\nusage: maxima [options] mode\n\n" 
	" \n"
	" modes: \n" 
	"    xboard  - run in xboard mode. requires winboard or xboard\n" 
	"    icc     - connect to icc and seek games\n" 
	"    analyze - analyze a position\n"
	"    runtest - run a testset\n\n"
	" options: \n"
	"    -icc-host=<hostname>        icc hostname. (default=chessclub.com)\n"
	"    -icc-port=<port>            icc port. (default 5000)\n"
	"    -icc-user=<name>            username to login with on icc\n"
	"    -icc-password=<password>    password to use on icc\n"
	"    -db-host=<hostname>         hostname for postgres database\n"
	"                                default=localhost\n"
	"    -db-user=<name>             username for postgres database\n"
	"                                default=maxima\n"
	"    -db-password=<password>     password for postgres database\n"
	"    -db-database=<database>     database to use\n\n"
	"                                default=maxima\n"
	"    -db-save-game=<bool>        save game in database\n"
	"                                default=true\n\n";
}

int main(int argc, char *argv[])
{	 
    if (argc<2) {
    	display_usage();
    	return 0;
    }

    int ret = 0; 

    // set default properties 
    MainForm.properties[std::string("-icc-host")] = std::string("chessclub.com"); 
    MainForm.properties[std::string("-icc-port")] = std::string("5000"); 
    MainForm.properties[std::string("-db-save-game")] = std::string("true");
    MainForm.properties[std::string("-mysql-host")] = std::string("localhost");
    MainForm.properties[std::string("-mysql-user")] = std::string("maxima");
    MainForm.properties[std::string("-mysql-password")] = std::string("maxima");
    MainForm.properties[std::string("-mysql-database")] = std::string("maxima");

    for (int i=1; i<argc-1; i++) {
	std::string param = argv[i]; 
	if (param.find("=")) {
	    std::string keyword = param.substr(0, param.find("="));
	    std::string value = param.substr(param.find("=")+1, param.length()+1-param.find("="));
	    std::cout << "main property " << keyword; 
	    std::cout << " = " << value << "\n"; 
	    MainForm.properties[keyword] = value; 
	}
    }

    if (!strcmp(argv[argc-1],"test")) { 
	ret = main_test(argc,argv);
        return ret; 
    }
    
    main_init();

    // commandline parsing 
    if (!strcmp(argv[argc-1],"xboard")) { 
    	ret = main_xboard(argc,argv);
    } else if (!strcmp(argv[argc-1],"icc")) { 
    	ret = main_icc(argc,argv);
    } else if (!strcmp(argv[argc-1],"runtest")) {
    	ret = main_runtest(argc,argv);
    } else { 	
    	display_usage();
    }

    std::cout << "called elapsed: " << g.timer.elapsed() << std::endl;

    Master_Exit(); 
	
    return ret; 
}
