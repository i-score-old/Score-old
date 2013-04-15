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

#include "CSP.h"
#include "CSPTypes.hpp"



CSP::CSP(CSPAsker *pa)
    :pAsker(pa)
{
    
}


CSP::~CSP()
{
    for (iterator it = varsMap.begin() ; it != varsMap.end() ; it++) {
        solver.removeIntVar(*it);
    }
    
    // TODO : suppress associated things (like in removeBox (factoriser le code ?))
}


TTErr CSP::addProcess(void *pProcess, int start, int end) // TODO : Editor créait une relation/interval/WTF de hiérarchie, peut-être ce doit être fait par le scenario
{
    // get max value
    int max = pAsker->getMaxValue();
    
    // add solver variables
    int beginID = solver.addIntVar(1, max, start, BEGIN_VAR_TYPE);
    int lengthID = solver.addIntVar(10, max, (end - start), LENGTH_VAR_TYPE);
    
    int endID = solver.addIntVar(1, max, end, BEGIN_VAR_TYPE);
    int endLengthID = solver.addIntVar(0, max, 0, LENGTH_VAR_TYPE);
    
    // store all solver variables IDs into a tab : {beginID, lengthID, endID, endLengthID}
    int boxIDs[4] = {beginID, lengthID, endID, endLengthID};
    
    // add FINISHES allen relation between the box and it's end (from CSPold addBox, addAllenRelation and addConstraint)
    int coefs[4] = {1,1,-1,-1};
    solver.addConstraint(boxIDs, coefs, 4, EQ_RELATION, 0, false);
    
    // store the IDs using the TimeProcessPtr
    varsMap.emplace(pProcess, boxIDs);
    
    
    // solver.addIntVar(min, max, val, weight)
    // Here, we have in fact no min and max.
    
    // mins are arbitrary values taken from the original CSP.cpp
    // max is the length of the parent Scenario
    // val is explicit
    // weights are "types" extracted from CSPTypes.hpp
    
    // The endLengthID is a weird value, but the solver seem to need it
    // TODO : Verify what I just said
    
    return kTTErrNone;
}


TTErr CSP::removeBox(TimeProcessPtr pBox)
{
    // TODO : remove the variables from the solver, wherever the IDs are stored (read upward)
    
    // TODO : remove the associated intervals (or whatever is associated)
    
    return kTTErrGeneric;
}


TTErr CSP::addRelation(void *pInterval)
{
    // TODO : TTEngines used to check that the relation starts and ends at different points, someone needs to do that
    // TODO : Editor used to check that the relation starts and ends in the same Scenario, someone needs to do that
    // TODO : Editor used to check that the Interval doesn't already exists, someone needs to do that
    
    int beginID1, beginID2;
    
    // TODO : retrieve above IDs
    
    // add ANTPOST_ANTERIORITY relation (from CSPold addAntPostRelation and addConstraint)
    int IDs[2] = {beginID2, beginID1};
    int coefs[2] = {1,-1};
    solver.addConstraint(IDs, coefs, 2, GQ_RELATION, 0, false); // TODO : must call the solver if the variables aren't in the right order (backward relation), then update the results of the solver
    
    return kTTErrNone;
}


TTErr CSP::removeRelation(TimeProcessPtr pRel)
{

        
    return kTTErrGeneric;
}
