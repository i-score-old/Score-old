/*
 * Scenario Solver Features
 * Copyright © 2013, Théo de la Hogue, Clément Bossut
 *
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#include "ScenarioSolver.h"

SolverVariable::SolverVariable(SolverPtr aSolver, TTTimeEventPtr anEvent, SolverValue max):
event(anEvent), solver(aSolver)
{
    TTValue v;
    
    event->getAttributeValue(TTSymbol("date"), v);
    
    // add a variable for date's event in solver
    dateID = solver->addIntVar(1, max, TTUInt32(v[0]), DATE_VARIABLE);
    
    // add a variable to set the date only limited by the max value (see in limit method)
    rangeID = solver->addIntVar(0, max, 0, RANGE_VARIABLE);
}

SolverVariable::~SolverVariable()
{
    // remove variable for date's event from solver
    solver->removeIntVar(dateID);
    
    // remove variable for date's event from solver
    solver->removeIntVar(rangeID);
}

SolverValue SolverVariable::get()
{
    return solver->getVariableValue(dateID);
}

void SolverVariable::limit(SolverValue min, SolverValue max)
{
    TTUInt32 value = solver->getVariableValue(dateID);
    
    if (value < min)
        value = min;
    
    if (value > max)
        value = max;
    
    solver->setIntVar(rangeID, min, max, value, RANGE_VARIABLE);
}

void SolverVariable::update()
{
    TTUInt32 value = solver->getVariableValue(dateID);
    
    event->setAttributeValue(TTSymbol("date"), value);
}

SolverConstraint::SolverConstraint(SolverPtr aSolver, SolverVariablePtr variableA, SolverVariablePtr variableB, SolverValue durationMin, SolverValue durationMax, SolverValue max):
solver(aSolver)
{
    TTValue   vA, vB, v;
    TTUInt32  startDate, endDate;
    
    // we need to order the variables in time
    variableA->event->getAttributeValue(TTSymbol("date"), vA);
    variableB->event->getAttributeValue(TTSymbol("date"), vB);
    
    if (vA < vB) {
        
        startVariable = variableA;
        endVariable = variableB;
        
        startDate = vA[0];
        endDate = vB[0];
    }
    else {
        
        startVariable = variableB;
        endVariable = variableA;
        
        startDate = vB[0];
        endDate = vA[0];
    }
    
    if (max == 0 || endDate > max)
        return;
    
    // add FINISHES allen relation between the startVariable and the endVariable in the solver
    // (see in : CSPold addBox, addAllenRelation and addConstraint)
    int IDs[4] = {startVariable->dateID, startVariable->rangeID, endVariable->dateID, endVariable->rangeID};
    int coefs[4] = {1,1,-1,-1};
    
    ID = solver->addConstraint(IDs, coefs, 4, EQ_RELATION, 0);
}

SolverConstraint::~SolverConstraint()
{
    // remove FINISHES allen relation between the startVariable and the endVariable from the solver
    solver->removeConstraint(ID);
}

SolverError SolverConstraint::move(SolverValue newStart, SolverValue newEnd)
{
    SolverValue deltaMax;
    
    // edit IDs to constrain
    // note : the endVariable.rangeID is useless here
    int IDs[3] = {startVariable->dateID, endVariable->dateID, startVariable->rangeID};
    
    // edit new dates to constrain
    SolverValue dates[3] = {newStart, newEnd, newEnd-newStart};
    
    // what is the maximal modification ?
    SolverValue deltaStart = abs(solver->getVariableValue(startVariable->dateID) - int(newStart));
    SolverValue deltaEnd = abs(solver->getVariableValue(endVariable->dateID) - int(newEnd));
    
    if (deltaStart < deltaEnd)
        deltaMax = deltaEnd;
    else
        deltaMax = deltaStart;
    
    // compute a solution (then each variable needs to be updated)
    if ( solver->suggestValues(IDs, dates, 3, deltaMax) )
        return SolverErrorNone;
    else
        return SolverErrorGeneric;
}

SolverError SolverConstraint::limit(SolverValue newDurationMin, SolverValue newDurationMax)
{
    return SolverErrorGeneric; // TODO
}

SolverRelation::SolverRelation(SolverPtr aSolver, SolverVariablePtr variableA, SolverVariablePtr variableB, SolverValue durationMin, SolverValue durationMax):
solver(aSolver), minBoundID(0), maxBoundID(0)
{
    TTValue     vA, vB, v;
    TTBoolean   ordered;
    
    // check duration bounds
    if (durationMin > durationMax)
        return;
    
    // we need to know the time order of the variables
    variableA->event->getAttributeValue(TTSymbol("date"), vA);
    variableB->event->getAttributeValue(TTSymbol("date"), vB);
    
    ordered = vA < vB;
    
    startVariable = ordered ? variableA : variableB;
    endVariable = ordered ? variableB : variableA;
    
    // add ANTPOST_ANTERIORITY relation
    // (see in : CSPold addAntPostRelation and addConstraint)
    int IDs[2] = {endVariable->dateID, startVariable->dateID};
    int coefs[2] = {1,-1};
    
    minBoundID = solver->addConstraint(IDs, coefs, 2, GQ_RELATION, durationMin);
    
    if (durationMax)
        maxBoundID = solver->addConstraint(IDs, coefs, 2, LQ_RELATION, durationMax);
    
    // update the solver in case of reversed relation
    if (!ordered)
        solver->updateVariablesValues();
}

SolverRelation::~SolverRelation()
{
    if (minBoundID)
        solver->removeConstraint(minBoundID);
    
    if (maxBoundID)
        solver->removeConstraint(maxBoundID);
}

SolverError SolverRelation::move(SolverValue newStart, SolverValue newEnd)
{
    SolverValue deltaMax;
    
    // edit IDs to constrain
    // note : the endVariable.rangeID is useless here
    int IDs[3] = {startVariable->dateID, endVariable->dateID, startVariable->rangeID};
    
    // edit new dates to constrain
    SolverValue dates[3] = {newStart, newEnd, newEnd-newStart};
    
    // what is the maximal modification ?
    SolverValue deltaStart = abs(solver->getVariableValue(startVariable->dateID) - int(newStart));
    SolverValue deltaEnd = abs(solver->getVariableValue(endVariable->dateID) - int(newEnd));
    
    if (deltaStart < deltaEnd)
        deltaMax = deltaEnd;
    else
        deltaMax = deltaStart;
    
    // compute a solution (then each variable needs to be updated)
    if ( solver->suggestValues(IDs, dates, 3, deltaMax) )
        return SolverErrorNone;
    else
        return SolverErrorGeneric;
}

SolverError SolverRelation::limit(SolverValue newDurationMin, SolverValue newDurationMax)
{
    // remove former constraints
    if (minBoundID) {
        solver->removeConstraint(minBoundID);
        minBoundID = 0;
    }
    
    if (maxBoundID) {
        solver->removeConstraint(maxBoundID);
        maxBoundID = 0;
    }
    
    // add new ANTPOST_ANTERIORITY relation
    // (see in : CSPold addAntPostRelation and addConstraint)
    int IDs[2] = {endVariable->dateID, startVariable->dateID};
    int coefs[2] = {1,-1};
    
    minBoundID = solver->addConstraint(IDs, coefs, 2, GQ_RELATION, newDurationMin);
    
    if (newDurationMax)
        maxBoundID = solver->addConstraint(IDs, coefs, 2, LQ_RELATION, newDurationMax);
    
    // update the solver in any case
    solver->updateVariablesValues();
    
    return SolverErrorNone;
}