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


CSP::CSP(TimeProcessPtr pScenario):
pScenario(NULL)
{
    this->pScenario = pScenario;
    
    mTimeProcessMap = new TimeProcessMap();
}

CSP::~CSP()
{
    // TODO : clear mTimeProcessMap 
}

TTErr CSP::addBox(TimeProcessPtr pBox)
{
    TTValue     boxStart, boxEnd, boxDuration;
    TTValue     scenarioDuration;
    TTValue     boxIDs;
    TTUInt32    beginID, lengthID, endID, endLengthID; // THEO : utiliser si possible les types TT qui garantissent un support multiplateforme
    
    // the box should not be an interval process
    if (pBox->getName() != TTSymbol("Interval")) {
        
        // get scenario duration
        pScenario->getAttributeValue(TTSymbol("duration"), scenarioDuration);
        
        // get time process informations
        pBox->getAttributeValue(TTSymbol("startDate"), boxStart);
        pBox->getAttributeValue(TTSymbol("endDate"), boxEnd);
        pBox->getAttributeValue(TTSymbol("duration"), boxDuration);
        
        // add solver variables
        beginID = solver.addIntVar(1, TTUInt32(scenarioDuration[0]), TTUInt32(boxStart[0]), BEGIN_VAR_TYPE);
        lengthID = solver.addIntVar(10, TTUInt32(scenarioDuration[0]), TTUInt32(boxDuration[0]), LENGTH_VAR_TYPE);
        
        endID = solver.addIntVar(1, TTUInt32(scenarioDuration[0]), TTUInt32(boxEnd[0]), BEGIN_VAR_TYPE);
        endLengthID = solver.addIntVar(0, TTUInt32(scenarioDuration[0]), 0, LENGTH_VAR_TYPE);
        
        // THEO : voici une proposition de mecanisme pour memoriser les IDs mais je ne sais pas si ça va convenir à tout ce qu'il y a faire ...
        
        // store all solver variables IDs into a value : <beginID, lengthID, endID, endLengthID>
        boxIDs.resize(4);
        
        boxIDs[0] = beginID;
        boxIDs[1] = lengthID;
        boxIDs[2] = endID;
        boxIDs[3] = endLengthID;
        
        // store the IDs using the TimeProcessPtr
        mTimeProcessMap->insert(TimeProcessKey(pBox, boxIDs));
        
        
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
    
    return kTTErrGeneric;
}

TTErr CSP::removeBox(TimeProcessPtr pBox)
{
    // TODO : remove the variables from the solver, wherever the IDs are stored (read upward)
    
    // TODO : remove the associated intervals (or whatever is associated)
    
    return kTTErrGeneric;
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

TTErr CSP::addRelation(TimeProcessPtr pRel)
{
    // WARNING : TTEngines used to verify that the relation starts and ends at different points
    
    // check it is an interval process
    if (pRel->getName() == TTSymbol("Interval")) {
    
        
    }
    
    return kTTErrNone;
}

TTErr CSP::removeRelation(TimeProcessPtr pRel)
{

        
    return kTTErrGeneric;
}
