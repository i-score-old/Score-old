/*
 * A CSP container
 * Copyright © 2013, Théo de la Hogue, Clément Bossut
 *
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

/*!
 * \class CSP
 *
 *  The CSP class is a container for the constraint satisfaction problem which ask solutions to the solver
 *
 */

#ifndef __CSP_H__
#define __CSP_H__

#include "CSPAsker.h"

#include "solver.hpp"

using namespace std;

class CSP
{
    
public :
    
    CSP(CSPAsker *pa);
    
    ~CSP();
    
    int addProcess(void *pProcess, int start, int end); // by default, rigid, move to change
    
    int removeProcess(void *pProcess);
    
    int moveProcess(); // TODO : remember to check min < max when supple
    
    int addInterval(void *pInterval); // by default, rigid, move to change
    
    int removeInterval(void *pInterval);
    
    int moveInterval(); // TODO : remember to check min < max when supple
    
private :
    
    Solver solver;
    
    CSPAsker *pAsker;
    
    unordered_map < void *, int * > varsMap; // unordered because wo don't have to iterate on the elements
};

#endif // __CSP_H__
