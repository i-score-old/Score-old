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


CSP::CSP()
{
    mTimeProcessMap = new TimeProcessMap();
}

CSP::~CSP()
{
    // clear the solver
    TimeProcessMapIterator it;
    
    for (it = mTimeProcessMap->begin(); it != mTimeProcessMap->end(); ++it)
        removeBox(it->first);
    
    delete mTimeProcessMap;
}

TTErr CSP::addBox(TimeProcessPtr pBox, TTUInt32 boxStart, TTUInt32 boxEnd, TTUInt32 boxDuration, TTUInt32 scenarioDuration)
{
    TTValuePtr  boxIDs = new TTValue();
    TTUInt32    beginID, lengthID, endID, endLengthID;
    
    // check dates and durations
    if (boxStart > boxEnd           ||
        boxDuration == 0            ||
        scenarioDuration == 0       ||
        boxEnd > scenarioDuration)
        return kTTErrGeneric;
    
    // add solver variables
    beginID = mSolver.addIntVar(1, scenarioDuration, boxStart, BEGIN_VAR_TYPE);
    lengthID = mSolver.addIntVar(10, scenarioDuration, boxDuration, LENGTH_VAR_TYPE);
    
    endID = mSolver.addIntVar(1, scenarioDuration, boxEnd, BEGIN_VAR_TYPE);
    endLengthID = mSolver.addIntVar(0, scenarioDuration, 0, LENGTH_VAR_TYPE);
    
    // store all solver variables IDs into a value : <beginID, lengthID, endID, endLengthID>
    boxIDs->append(beginID);
    boxIDs->append(lengthID);
    boxIDs->append(endID);
    boxIDs->append(endLengthID);
    
    // store the IDs using the TimeProcessPtr
    mTimeProcessMap->insert(TimeProcessKey(pBox, boxIDs));
    
    // NOTE :
    // solver.addIntVar(min, max, val, weight)
    // Here, we have in fact no min and max.
    
    // mins are arbitrary values taken from the original CSP.cpp
    // max is the length of the parent Scenario
    // val is explicit
    // weights are "types" extracted from CSPTypes.hpp
    
    // The endLengthID is a weird value, but the solver seem to need it
    // TODO : Verify what I just said
    
    // TODO : What to do with these IDs ? Keep'em here or give'em to their associated TimeProcess
    
    return kTTErrNone;
}

TTErr CSP::removeBox(TimeProcessPtr pBox)
{
    // get all solver variables IDs back
    TimeProcessMapIterator  it = mTimeProcessMap->find(pBox);
    TTValue                 boxIDs = *(it->second);
    
    // remove the variables from the solver
    mSolver.removeIntVar(boxIDs[0]);
    mSolver.removeIntVar(boxIDs[1]);
    mSolver.removeIntVar(boxIDs[2]);
    mSolver.removeIntVar(boxIDs[3]);
    
    // remove the variables IDs
    mTimeProcessMap->erase(pBox);
    
    // TODO : remove the associated intervals (or whatever is associated)
    // THEO : IMHO this should be done by the Scenario because it knows which interval are related
    
    return kTTErrNone;
}

TTErr CSP::addEvent(TimeEventPtr pEvent)
{
    // TODO : no sé que hacer, must start condition implementation first I think
    
    return kTTErrGeneric;
}

TTErr CSP::removeEvent(TimeEventPtr pEvent)
{
    // TODO : same thought as above
    
    return kTTErrGeneric;
}

TTErr CSP::addRelation(TimeProcessPtr pRel, )
{
    TTValuePtr  relIDs = new TTValue();
    TTUInt32    beginID, lengthID, endID, endLengthID;
    
    // check dates and durations
    if (relStart > relEnd           ||
        relDuration == 0            ||
        scenarioDuration == 0       ||
        relEnd > scenarioDuration)
        return kTTErrGeneric;
    
    /* add solver constrain
    switch(type) {
        case ANTPOST_ANTERIORITY :
            varsIDs->push_back(ent2->beginID());
            varsCoeffs->push_back(1);
            varsIDs->push_back(ent1->beginID());
            varsCoeffs->push_back(-1);
            break;
            
        case ANTPOST_POSTERIORITY :
            varsIDs->push_back(ent1->beginID());
            varsCoeffs->push_back(1);
            varsIDs->push_back(ent2->beginID());
            varsCoeffs->push_back(-1);
            break;
    }
    
    if (minBound != NO_BOUND)
        addConstraint(varsIDs, varsCoeffs, GQ_RELATION, minBound, mustCallSolver);
    else
        addConstraint(varsIDs, varsCoeffs, GQ_RELATION, 0, mustCallSolver);
    
    if (maxBound != NO_BOUND)
        addConstraint(varsIDs, varsCoeffs, LQ_RELATION, maxBound, mustCallSolver);
    
    
    int *iDs = new int[varsIDs->size()],  *coeffs = new int[varsIDs->size()];
	for (unsigned int i=0; i<varsIDs->size(); i++) {
		iDs[i] = varsIDs->at(i);
		coeffs[i] = varsCoeffs->at(i);
	}
    
	CSPLinearConstraint *newConstraint = NULL;
	int newID = _solver->addConstraint(iDs, coeffs, varsIDs->size(), (int)type, value, mustCallSolver);
	if (newID >= 0)
		newConstraint = new CSPLinearConstraint(newID, varsIDs, varsCoeffs, type, value);
    
	delete[] iDs;
	delete[] coeffs;
    
	return newConstraint;
    
    // store all solver variables IDs into a value : <beginID, lengthID, endID, endLengthID>
    relIDs->append(beginID);
    relIDs->append(lengthID);
    relIDs->append(endID);
    relIDs->append(endLengthID);
    
    // store the IDs using the TimeProcessPtr
    mTimeProcessMap->insert(TimeProcessKey(prel, relIDs));
    */
    
    return kTTErrNone;
}

TTErr CSP::removeRelation(TimeProcessPtr pRel)
{

        
    return kTTErrGeneric;
}
