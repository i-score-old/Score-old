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
#include "Interval.h"
#include "solver.hpp"

class CSP
{
    
public :
    
    CSP(ScenarioPtr pScenario);
    
    ~CSP();
    
    TTErr addBox(TimeProcessPtr pBox);
    
    TTErr removeBox(TimeProcessPtr pBox);
    
    TTErr addEvent(TimeEventPtr pEvent);
    
    TTErr removeEvent(TimeEventPtr pEvent);
    
    TTErr addRelation(IntervalPtr pRel);
    
    TTErr removeRelation(IntervalPtr pRel);
    
private :
    
    Solver solver;
    
    ScenarioPtr pScenario;
    
}

#endif // __CSP_H__
