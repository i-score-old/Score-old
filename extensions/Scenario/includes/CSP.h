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
//#include "Interval.h"                         // THEO : je ne suis pas sure que tu as besoin d'inclure Interval.h puisque un interval est un TimeProcess "pure" (il n'y a pas de membres ou methodes speciales dont tu as besoin)
#include "solver.hpp"

class CSP
{
    
public :
    
    CSP(TimeProcessPtr pScenario);              // THEO : on utilise TimeProcessPtr et on vérifie nous même que le nom de la classe est bien TTsymbol("Scenario")
    
    ~CSP();
    
    TTErr addBox(TimeProcessPtr pBox);
    
    TTErr removeBox(TimeProcessPtr pBox);
    
    TTErr addEvent(TimeEventPtr pEvent);
    
    TTErr removeEvent(TimeEventPtr pEvent);
    
    TTErr addRelation(TimeProcessPtr pRel);      // THEO : on utilise TimeProcessPtr et on vérifie nous même que le nom de la classe est bien TTsymbol("Interval")
    
    TTErr removeRelation(TimeProcessPtr pRel);   // THEO : on utilise TimeProcessPtr et on vérifie nous même que le nom de la classe est bien TTsymbol("Interval")
    
private :
    
    Solver solver;
    
    TimeProcessPtr      pScenario;               // THEO : on utilise TimeProcessPtr et on vérifie nous même que le nom de la classe est bien TTsymbol("Scenario")
    
    TimeProcessMapPtr   mTimeProcessMap;
    
    TimeEventMapPtr     mTimeEventMap;           // THEO : pas sure que ce soit utile mais je l'ai mis pour te montrer que c'est aussi possible
};

#endif // __CSP_H__
