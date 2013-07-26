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

#include <sys/time.h>
#include <signal.h>
#include <pthread.h>
#include <sys/socket.h>

#include "fast.h"
#include "w0.h" 
#include "w17.h" 
#include "main.h" 
#include "w0_search.h" 
#include "w17_search.h"
#include "parser.h" 
#include "legality.h" 
#include "init.h"
#include "hash.h"
#include "tm_icc.h"

/* engine vars */ 
pthread_t         *engine_thread;
int               engine_pondermove;
int               engine_lastmove;  
TFastNode         engine_node; 
TFastNode         engine_rootnode; 
int               engine_rootply;  
Tengine_records   engine_records[512];

/* states of the working thread: 
 
   - s_threadstate -
   THINKING: 
   PONDERING: 
   PONDERSTOPPING: pondering is stoppend by search(). s_ponderhit indicates 
                   if threre is a ponderhit or not. 
   STOPPING: This game is over. will sleep until the state is reset in 
             setFen() 

   - s_ponderhit - 
   indicates ponderhit 
   
   - statemutex - 
   this mutex is locked when the s_threadstate var is read or changed 

   - _stopsearch - 
   used to stop the search 

   - cond - 
   condition on which the thread waits when it sleeps 

*/ 

#define THREAD_STOPPING 1
#define THREAD_PONDERING 2
#define THREAD_THINKING 3
#define THREAD_PONDERSTOPPING 4 

int s_threadstate = THREAD_STOPPING; 
bool s_ponderhit = false; 
pthread_cond_t *cond; // = PTHREAD_COND_INITIALIZER;
pthread_mutex_t *statemutex; // = PTHREAD_MUTEX_INITIALIZER; 

void set_dbhandle(db_base* dbptr)
{
    if (g.dbhandle) {
	delete g.dbhandle; 
    }
    g.dbhandle = dbptr; 
}

void engine_update_stats()
{
    /* simple routine to copy statistics from the engine in an array */ 
    
    if (engine_rootply< 512) { 
	
	engine_records[engine_rootply].maxima_thinking = true; 
	engine_records[engine_rootply].move = g.rootmoves[0].move; 
	engine_records[engine_rootply].hashcode_after_move = engine_rootnode.hashcode; 
	engine_records[engine_rootply].score = /*g.rootscore;*/ g.rootmoves[0].value; 	       
	engine_records[engine_rootply].thinktime = (int)(g.timer.elapsed () * 1000);
	engine_records[engine_rootply].nodes = g.fastnodes;
//		if (g.fastnodes > g.last_known_nps && engine_records[engine_rootply].thinktime>1000) {
	unsigned long nodes = g.fastnodes*1000;
	if (engine_records[engine_rootply].thinktime > 0) { 
	    g.last_known_nps = nodes / engine_records[engine_rootply].thinktime;
	} else {
	    g.last_known_nps = 40000; 
	}
	//g.last_known_nps = (g.fastnodes / engine_records[engine_rootply].thinktime)*1000; 
	//std::cout << "engine nps = " << g.last_known_nps << "\n"; 
	if (g.last_known_nps < 0) { 
	    std::cerr << "engine nodes = " << g.fastnodes << " time=" <<  engine_records[engine_rootply].thinktime << "\n";
	    //std::cout << "engine debug nps=" << g.last_known_nps << "\n"; 
	}
//		}		 				
    }
}

void stop_thinking()
{	
    /* we must stop the engine because the game has ended. */ 
    pthread_mutex_lock(statemutex);
    { 
		if (s_threadstate != THREAD_STOPPING) {
			s_threadstate = THREAD_STOPPING;
			g.stopsearch = true;
			pthread_cond_signal(cond);
		} else {
			std::cout << "Thread already stopped\n";
		}
    }
    pthread_mutex_unlock(statemutex); 
}

int determine_pondermove () 
{
    int pmove;

    if (g.checkbook) {
    	return 0;
    	//pmove = get_move_from_db (&engine_node);
    } else {
    	/* get pondermove from the pv */
    	pmove = g.pv[0][1];
    }
    
    if (pmove == 0 && MainForm.wildnumber==0) { //guess a move only for w0
        std::cout << "warning: nothing to ponder\n";
        if (engine_node.flags & _WTM) {
        	if (genrootmoves_w (&engine_node)) {
        		pmove = g.rootmoves[0].move;
        	}
        } else {
        	if (genrootmoves_b (&engine_node)) {
        		pmove = g.rootmoves[0].move;
        	}
        }
    }	
    return pmove;
}

void play_move()
{
    /* stuff the thread needs to do when done thinking and when it has a ponderhit */
    if (engine_node.flags & _WTM) { 
    	_fast_dowmove (&engine_node, g.rootmoves[0].move);
    	_fast_dowmove (&engine_rootnode, g.rootmoves [0]. move);
    } else { 		 
    	_fast_dobmove (&engine_node, g.rootmoves[0].move);
    	_fast_dobmove (&engine_rootnode, g.rootmoves [0]. move);
    }
	
    /* de reptable updaten met de actually played move */ 
    g.reptable [engine_rootnode.fifty] = engine_rootnode.hashcode;	
    engine_lastmove = g.rootmoves[0].move; 

    /* return move to ui */
    _write_LAN(MainForm.socket_connection,g.rootmoves[0].move);

    /* relay the move */
    MainForm.relayply++; 
        
    std::cout << "\n";

    if (MainForm.wildnumber == 17) {
        if (g.rootmoves[0].forced_move) {
            std::cout << "forced move\n";
        }
    }

    /* clean the draw */
    if (g.rootmoves[0].draw) { 
    	if (claimdraw(&engine_rootnode,1)) {
    		send(MainForm.socket_connection,"draw\n",5,0);
    	}
    }

    /* nu we een zet hebben gedaan moeten we de engine statistieken updaten. */ 
    engine_update_stats(); 
    engine_rootply++;

    /* determine pondermove */ 		
    engine_pondermove = determine_pondermove ();
	
    if (engine_pondermove) {
    	if (engine_node.flags & _WTM) {
    		_fast_dowmove (&engine_node, engine_pondermove);
    	} else {
    		_fast_dobmove (&engine_node, engine_pondermove);
    	}
    }
}

void *engine_thread_execute(void *ptr)
{
    while (true) { 
		
	pthread_mutex_lock(statemutex);
	{
	    /* set !stopsearch if we are going to think or ponder */
	    if (s_threadstate == THREAD_PONDERING || s_threadstate == THREAD_THINKING) {
			g.stopsearch = false; 
	    } else {
			g.stopsearch = true; 
	    }
	}
	pthread_mutex_unlock(statemutex); 

	/* ---- vanaf dit punt kan stopsearch worden gewijzigd door search() of stop() */
	g.timer.start();

	if (MainForm.wildnumber==0) { 
	    iterate(&engine_node);
	} else if (MainForm.wildnumber==17) { 

		int legals = w17_iterate(&engine_node);

		if (!legals) {
			std::cout << "w17: stop search; no legals\n";
			// g.stopsearch = true;
			stop_thinking();
		}
	}

	g.timer.stop();
	/* ---- tot dit punt */ 
		
	pthread_mutex_lock(statemutex); 
	{		       			     
	    switch(s_threadstate) { 
	    case THREAD_STOPPING: 	
	        // std::cout << "* thread state: THREAD_STOPPING\n";
	        pthread_cond_wait(cond, statemutex);
                /* setFen handles the new state and engine_rootnode */ 				
			break;
	    case THREAD_PONDERING:
		case THREAD_PONDERSTOPPING:
			if (s_threadstate==THREAD_PONDERING) {
				pthread_cond_wait(cond, statemutex);
			}
			if (s_threadstate != THREAD_STOPPING && s_ponderhit) {
				play_move();
				s_threadstate = THREAD_PONDERING;
				g.maxtime = g.stoptime = 100000000; /* max pondertime */
			}
			if (s_threadstate != THREAD_STOPPING && !s_ponderhit) {
				/* geen ponderhit, reset engine_node */
				if (engine_node.hashcode != engine_rootnode.hashcode) {
				memcpy( &engine_node, &engine_rootnode, sizeof(TFastNode));
				}
				s_threadstate = THREAD_THINKING;
			}
				
		break; 
	    case THREAD_THINKING:
	    	play_move();
	    	s_threadstate = THREAD_PONDERING;
	    	g.maxtime = g.stoptime = 100000000; /* max pondertime */
		break;
	    default:				
	    	std::cerr << boost::format("Illegal thread state (%d)\n") % s_threadstate;
		break;

	    }
	}
	pthread_mutex_unlock(statemutex); 

    }
    return NULL;
}

void game_search(int ui_ply, int ui_lastmove, int whitetime, int blacktime, int increment)
{		
    // calc tmax
    // engine rootmove is still other side to move 
    if (engine_rootnode.flags & _WTM) {
        g.timeleft = blacktime;
	g.tmax = tm_icc (blacktime, whitetime, increment);
    } else {
        g.timeleft = whitetime; 
	g.tmax = tm_icc (whitetime, blacktime, increment);
    }

    pthread_mutex_lock(statemutex); 
    {
	switch (s_threadstate) {
	case THREAD_STOPPING:
	    break; 
	case THREAD_PONDERING: 
	    if (ui_lastmove==engine_pondermove) { 		
			/* ponderhit */
			s_ponderhit = true;

			/* ponderhit - update de rootnode */
			if (engine_rootnode.flags & _WTM) {
				_fast_dowmove(&engine_rootnode, ui_lastmove);
			} else {
				_fast_dobmove(&engine_rootnode, ui_lastmove);
			}

			/* update game info */
			engine_records[engine_rootply].maxima_thinking = false;
			engine_records[engine_rootply].hashcode_after_move = engine_rootnode.hashcode;
			engine_records[engine_rootply++].move = ui_lastmove;
					
			/* de reptable updaten met de actually played move */
			g.reptable [engine_rootnode.fifty] = engine_rootnode.hashcode;
	
			/* pas tmax aan */
			int msecs = g.timer.elapsed ();
			g.stoptime = g.maxtime = g.tmax - msecs;

			/* resume de thread */
			s_threadstate = THREAD_PONDERSTOPPING;
			pthread_cond_signal(cond);

	    } else { 
	    	/* geen ponderhit */
	    	s_ponderhit = false;
				
	    	/* update de engine_rootnode */
	    	if (ui_lastmove && (ui_lastmove != engine_lastmove)) {
										
				if (engine_rootnode.flags & _WTM) {
					_fast_dowmove(&engine_rootnode, ui_lastmove);
				} else {
					_fast_dobmove(&engine_rootnode, ui_lastmove);
				}

				/* update game info */
				engine_records[engine_rootply].maxima_thinking = false;
				engine_records[engine_rootply].hashcode_after_move = engine_rootnode.hashcode;
				engine_records[engine_rootply++].move = ui_lastmove;

				/* de reptable updaten met de actually played move */
				g.reptable [engine_rootnode.fifty] = engine_rootnode.hashcode;

			} else {
				// g_print("* starting with first move\n");
			}
					
			/* set tmax voor de nieuwe search nadat we de ponderactie gestopt hebben */
			g.maxtime = g.tmax;
			g.stoptime = g.tmax / 2;
					
			g.stopsearch = true;

			s_threadstate = THREAD_PONDERSTOPPING;
			pthread_cond_signal(cond);
				
	    }
	    break; 
	case THREAD_THINKING:
	    std::cerr << "Illegal THREAD_THINKING state in search();";
	    break; 
	case THREAD_PONDERSTOPPING:
	    std::cerr << "Illegal THREAD_PONDERSTOPPING state in search();";
	    break; 
	}
    }
    pthread_mutex_unlock(statemutex); 
}

void set_fen(const char* fen)
{
    pthread_mutex_lock(statemutex); 
    {
	if (s_threadstate != THREAD_STOPPING) { 
	    std::cerr << boost::format("Thread not stopped before newgame in SetFen. state=%d\n") % s_threadstate;
	}
	s_threadstate = THREAD_PONDERING; 
	engine_lastmove = 0; 
	engine_pondermove = -1; 		
	fenton(&engine_rootnode,fen);
	memcpy(&engine_node,&engine_rootnode,sizeof(TFastNode)); 
	g.last_known_nps = 100000;
	engine_rootply=1; /* fixme: why starts at 1 */ 
	g.checkbook = true;
    }
    pthread_mutex_unlock(statemutex); 
}

void game_move_forward(int move)
{
    if (engine_rootnode.flags & _WTM) { 
    	_fast_dowmove (&engine_rootnode, move);
    } else { 		 
    	_fast_dobmove (&engine_rootnode, move);
    }
	
    engine_records[engine_rootply].maxima_thinking = false; 
    engine_records[engine_rootply].hashcode_after_move = engine_rootnode.hashcode; 
    engine_records[engine_rootply].move = move; 

    engine_rootply++;
}

void signal_handler(int signal) 
{	
    if (s_threadstate==THREAD_THINKING) {

		int msecs = g.timer.elapsed ();
		if (msecs >= g.stoptime) {
			if (g.iteration < 2 && !g.crisis) {
				/* crisis: double the time */
				std::cout << "engine crisis msecs=" << msecs << "\n";
				std::cout << "engine crisis before: maxtime=" << g.maxtime << " stoptime=" << g.stoptime << "\n";
				g.crisis = true;
				g.maxtime = MIN(g.maxtime * 3, (g.timeleft-msecs)/2);
				g.stoptime = g.maxtime;
				std::cout << "engine crisis after: maxtime=" << g.maxtime << "\n";
				tell_crisis(g.maxtime/1000);
			} else {
				/* stop the search */
				g.stopsearch = true;
			}
			
		}
    }
}

void init_engine()
{
    // init the engine data structures and hash tables 
    Master_Init();  		
    g.crisis = false; 

    engine_lastmove = 0; 
    engine_pondermove = -1; 

    /* init sync stuff */ 
    cond = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));
    statemutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    engine_thread = (pthread_t*)malloc(sizeof(pthread_t));

    pthread_mutex_init(statemutex, NULL); 
    pthread_cond_init(cond, NULL); 

    /* create worker thread */ 
    pthread_mutex_lock(statemutex); 
    {
    	g.stopsearch = true;
    	s_threadstate = THREAD_STOPPING;

    	fenton(&engine_rootnode,"rnbqkbnr/pppppppp/////PPPPPPPP/RNBQKBNR w kqKQ");
    	memcpy(&engine_node, &engine_rootnode,sizeof(TFastNode));

    	pthread_create(engine_thread,NULL,engine_thread_execute,NULL );
    }
    pthread_mutex_unlock(statemutex); 

    /* set the signal handler */ 
    signal(SIGALRM, signal_handler); 

    /* create the interval timer */ 
    itimerval itval; 
    itval.it_interval.tv_sec = 0; 
    itval.it_interval.tv_usec = 100000; 
    itval.it_value.tv_sec = 0; 
    itval.it_value.tv_usec = 100000; 
    setitimer(ITIMER_REAL,&itval,NULL);	
}
