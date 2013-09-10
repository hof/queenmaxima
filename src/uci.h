/* 
 * File:   uci.h
 * Author: Hermen Reitsma
 *
 * Created on 4 september 2013, 19:34
 */

#ifndef UCI_H
#define	UCI_H

#include <sstream>
#include <fstream>
#include <cstdlib>
#include <iostream>
#include <istream>
#include <string.h>

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdio.h> 

typedef std::istringstream TInputParser;

bool uci(std::string cmd);

bool handleUci();
bool handleIsReady();
bool handleIsReady();
bool handlePonderHit();
bool handleStop();
bool handleNewGame();
bool handleGo(TInputParser &parser);
bool handlePosition(TInputParser &parser);
bool handleForward(TInputParser &parser);
bool handleSetOption(TInputParser &parser);
bool handleTestrun(TInputParser &parser);
bool handlePonderHit(TInputParser &parser);

static inline void output(std::string str) {
    std::ofstream myfile;
    myfile.open("uci.log", std::ios::app);
    myfile << "< " << str << std::endl;
    myfile.close();
    std::cout << str << std::endl;
}




#endif	/* UCI_H */

