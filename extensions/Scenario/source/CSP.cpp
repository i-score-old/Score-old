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


CSP::CSP(void(*aCSPReportFunction)(void*, CSPValue))
    :mCallback(aCSPReportFunction)
{
    
}

CSP::~CSP()
{
    CSPElementMapIterator it;
    int *IDs;
    
    // clear variable map
    for (it = mVariablesMap.begin() ; it != mVariablesMap.end() ; it++) {
        
        IDs = it->second;
        
        // remove position variable
        mSolver.removeIntVar(IDs[0]);
        
        // remove length variable
        mSolver.removeIntVar(IDs[1]);
        
        delete IDs;
    }
    
    // clear process constraint map
    for (it = mProcessConstraintsMap.begin() ; it != mProcessConstraintsMap.end() ; it++) {
        
        IDs = it->second;
        
        // remove process constraint
        mSolver.removeConstraint(IDs[0]);
        
        delete IDs;
    }
    
    // clear interval constraint map
    for (it = mIntervalConstraintsMap.begin() ; it != mIntervalConstraintsMap.end() ; it++) {
        
        IDs = it->second;
        
        // remove interval constraint
        mSolver.removeConstraint(IDs[0]);
        
        delete IDs;
    }
}

CSPError CSP::addProcess(void *pStartObject, void *pEndObject, CSPValue start, CSPValue end, CSPValue max, CSPValue minBound, CSPValue maxBound)
{
    // TODO : Editor créait une relation/interval/WTF de hiérarchie, peut-être ce doit être fait par le scenario
    
    if (start >= end || max == 0 || end > max)
        return CSPErrorGeneric;

    // add solver variables
    int startID = mSolver.addIntVar(1, max, start, BEGIN_VAR_TYPE);
    int startLengthID = mSolver.addIntVar(10, max, (end - start), LENGTH_VAR_TYPE);
    
    int endID = mSolver.addIntVar(1, max, end, BEGIN_VAR_TYPE);
    int endLengthID = mSolver.addIntVar(0, max, 0, LENGTH_VAR_TYPE);
    
    // add FINISHES allen relation between the start and the end
    // (see in : CSPold addBox, addAllenRelation and addConstraint)
    int variableIDs[4] = {startID, startLengthID, endID, endLengthID};
    int coefs[4] = {1,1,-1,-1};
    
    int *constraintID = new int[1];
    constraintID[0] = mSolver.addConstraint(variableIDs, coefs, 4, EQ_RELATION, 0, false);
    
    // store the variable IDs related to each object
    int *startObjectIDs = new int[2];
    startObjectIDs[0] = startID;
    startObjectIDs[1] = startLengthID;
    mVariablesMap.emplace(pStartObject, startObjectIDs);
    
    int *endObjectIDs = new int[2];
    endObjectIDs[0] = endID;
    endObjectIDs[1] = endLengthID;
    mVariablesMap.emplace(pEndObject, endObjectIDs);
    
    // store the process constraint ID twice (one for each object)
    mProcessConstraintsMap.emplace(pStartObject, constraintID);
    mProcessConstraintsMap.emplace(pEndObject, constraintID);
    
    // NOTE :
    // mSolver.addIntVar(min, max, val, weight)
    // Here, we have in fact no min and max.
    
    // mins are arbitrary values taken from the original CSP.cpp
    // max is the length of the parent Scenario
    // val is explicit
    // weights are "types" extracted from CSPTypes.hpp
    
    // The endLengthID is a weird value, but the mSolver seem to need it
    
    return CSPErrorNone;
}

CSPError CSP::removeProcess(void *pStartObject, void *pEndObject)
{
    CSPElementMapIterator it;
    int *IDs;
    
    // get variable IDs back for startObject
    it = mVariablesMap.find(pStartObject);
    IDs = it->second;
    
    // remove variables relative to startObject
    mSolver.removeIntVar(IDs[0]);
    mSolver.removeIntVar(IDs[1]);
    
    delete IDs;
    
    // get variable IDs back for startObject
    it = mVariablesMap.find(pEndObject);
    IDs = it->second;
    
    // remove variables relative to startObject
    mSolver.removeIntVar(IDs[0]);
    mSolver.removeIntVar(IDs[1]);
    
    delete IDs;
    
    // get constraint ID back using startObject (or endObject it doesn't matter)
    it = mProcessConstraintsMap.find(pStartObject);
    IDs = it->second;
    
    // remove constraint relative to both objects
    mSolver.removeConstraint(IDs[0]);
    
    delete IDs;
    
    // finally remove all from the maps
    mVariablesMap.erase(pStartObject);
    mVariablesMap.erase(pEndObject);
    
    mProcessConstraintsMap.erase(pStartObject);
    mProcessConstraintsMap.erase(pEndObject);
    
    return CSPErrorNone;
}

CSPError CSP::moveProcess(void *pStartObject, void *pEndObject, CSPValue newStart, CSPValue newEnd)
{
    CSPElementMapIterator   it;
    int                     *startIDs, *endIDs;
    CSPValue                deltaMax;
    
    // get IDs back for startObject
    it = mVariablesMap.find(pStartObject);
    startIDs = it->second;
    
    // get IDs back for endObject
    it = mVariablesMap.find(pEndObject);
    endIDs = it->second;
    
    // edit variableIDs to constrain : { startID, endID, startLengthID }
    // note : the endLengthID variable is useless here
    int variableIDs[3] = {startIDs[0], endIDs[0], startIDs[1]};
    
    // edit new position to constrain
    CSPValue position[3] = {newStart, newEnd, newEnd-newStart};
    
    // what is the maximal modification ?
    CSPValue deltaStart = abs(mSolver.getVariableValue(startIDs[0]) - int(newStart));
    CSPValue deltaEnd = abs(mSolver.getVariableValue(endIDs[0]) - int(newEnd));
    
    if (deltaStart < deltaEnd)
        deltaMax = deltaEnd;
    else
        deltaMax = deltaStart;
    
    // compute a solution
    if ( mSolver.suggestValues(variableIDs, position, 3, deltaMax) ) {
    
        // then update each variable // TODO : comment vérifier que ça a vraiment changé ?
        for (it = mVariablesMap.begin() ; it != mVariablesMap.end() ; it++) {
        
            // return solved position variable back
            mCallback(it->first, mSolver.getVariableValue(it->second[0]));
            
        }
        
        return CSPErrorNone;
    }
    
    return CSPErrorGeneric;
}

CSPError CSP::addInterval(void *pStartObject, void *pEndObject, CSPValue start, CSPValue end, CSPValue minBound, CSPValue maxBound)
{
    CSPElementMapIterator it;
    int startID, endID;
    int *constraintID;
    
    // get IDs back for startObject
    it = mVariablesMap.find(pStartObject);
    startID = it->second[0];
    
    // get IDs back for endObject
    it = mVariablesMap.find(pEndObject);
    endID = it->second[0];
    
    // add ANTPOST_ANTERIORITY relation
    // (see in : CSPold addAntPostRelation and addConstraint)
    int IDs[2] = { start > end ? startID : endID, end >= start ? startID : endID };     // TODO : est-ce vraiment nécessaire si on évite le backward relation
    int coefs[2] = {1,-1};
    
    if (minBound && maxBound) { // TODO : ça devient rigide si on donne un minbound et un maxBound égaux
        
        constraintID = new int[2];
        constraintID[0] = mSolver.addConstraint(IDs, coefs, 2, GQ_RELATION, minBound, false);
        constraintID[1] = mSolver.addConstraint(IDs, coefs, 2, LQ_RELATION, maxBound, false);
    }
    else {
        
        constraintID = new int[1];
        constraintID[0] = mSolver.addConstraint(IDs, coefs, 2, GQ_RELATION, 0, false);
    }
    
    // TODO : must call the mSolver if the variables aren't in the right order (backward relation), then update the results of the mSolver
    // THEO : in Scenario, I've commented out a part of code that move processes in case of backward relation ... maybe it can replace the TODO ?
    
    // store the process constraint ID twice (one for each object)
    mIntervalConstraintsMap.emplace(pStartObject, constraintID);
    mIntervalConstraintsMap.emplace(pEndObject, constraintID);
    
    return CSPErrorNone;
}

CSPError CSP::removeInterval(void *pStartObject, void *pEndObject)
{
    CSPElementMapIterator it;
    int *IDs;
    
    // don't remove variables ! (see in : removeProcess)
    
    // get constraint ID back using startObject (or endObject it doesn't matter)
    it = mIntervalConstraintsMap.find(pStartObject);
    IDs = it->second;
    
    // remove constraint relative to both objects
    mSolver.removeConstraint(IDs[0]);
    
    delete IDs;
    
    // finally remove all from the map
    mIntervalConstraintsMap.erase(pStartObject);
    mIntervalConstraintsMap.erase(pEndObject);
    
    return CSPErrorNone;
}

CSPError CSP::moveInterval(void *pStartObject, void *pEndObject, CSPValue newStart, CSPValue newEnd)
{
    CSPElementMapIterator   it;
    int                     *startIDs, *endIDs;
    
    // get IDs back for startObject
    it = mVariablesMap.find(pStartObject);
    startIDs = it->second;
    
    // get IDs back for endObject
    it = mVariablesMap.find(pEndObject);
    endIDs = it->second;
    
    // ?
    
    return CSPErrorNone;
}