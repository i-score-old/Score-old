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

#include "TimeProcess.h"
#include "TimeEvent.h"

#include "solver.hpp"

class CSP
{
    
public :
    
    CSP();
    
    ~CSP();
    
    TTErr addBox(TimeProcessPtr pBox, TTUInt32 boxStart, TTUInt32 boxEnd, TTUInt32 boxDuration, TTUInt32 scenarioDuration);
    
    TTErr removeBox(TimeProcessPtr pBox);
    
    TTErr addEvent(TimeEventPtr pEvent);
    
    TTErr removeEvent(TimeEventPtr pEvent);
    
    TTErr addRelation(TimeProcessPtr pRel);
    
    TTErr removeRelation(TimeProcessPtr pRel);
    
private :
    
    Solver              mSolver;
    
    TimeProcessMapPtr   mTimeProcessMap;
    
    TimeEventMapPtr     mTimeEventMap;           // THEO : pas sure que ce soit utile mais je l'ai mis pour te montrer que c'est aussi possible
};

#endif // __CSP_H__
