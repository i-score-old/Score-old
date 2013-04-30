/*
 * Scenario time Process
 * Copyright © 2013, Théo de la Hogue, Clément Bossut
 *
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

/*!
 * \class Scenario
 *
 *  Scenario time process class is a container class to manage other time processes instances in the time
 *
 */

#include "Scenario.h"

#define thisTTClass                 Scenario
#define thisTTClassName             "Scenario"
#define thisTTClassTags             "time, process, scenario"

#define thisTimeProcessVersion		"0.1"
#define thisTimeProcessAuthor        "Theo de la Hogue"

extern "C" TT_EXTENSION_EXPORT TTErr TTLoadJamomaExtension_Scenario(void)
{
	TTFoundationInit();
	Scenario::registerClass();
	return kTTErrNone;
}

TIME_PROCESS_CONSTRUCTOR,
mNamespace(NULL),
mFirstEvent(NULL),
mLastEvent(NULL),
mEditionSolver(NULL),
mExecutionGraph(NULL)
{
    TIME_PROCESS_INITIALIZE
    
    TTErr err;
    
	TT_ASSERT("Correct number of args to create Scenario", arguments.size() == 0);
    
    addMessageWithArguments(TimeProcessAdd);
    addMessageProperty(TimeProcessAdd, hidden, YES);
    
    addMessageWithArguments(TimeProcessRemove);
    addMessageProperty(TimeProcessRemove, hidden, YES);
    
    addMessageWithArguments(TimeProcessMove);
    addMessageProperty(TimeProcessMove, hidden, YES);
    
    addMessageWithArguments(TimeProcessLimit);
    addMessageProperty(TimeProcessLimit, hidden, YES);
    
    addMessageWithArguments(TimeEventAdd);
    addMessageWithArguments(TimeEventRemove);
    addMessageWithArguments(TimeEventMove);
    addMessageWithArguments(TimeEventReplace);
    
    // all messages below are hidden because they are for internal use
    addMessageWithArguments(TimeProcessActiveChange);
    addMessageProperty(TimeProcessActiveChange, hidden, YES);
	
	// needed to be handled by a TTXmlHandler
	addMessageWithArguments(WriteAsXml);
	addMessageProperty(WriteAsXml, hidden, YES);
	addMessageWithArguments(ReadFromXml);
	addMessageProperty(ReadFromXml, hidden, YES);
	
	// needed to be handled by a TTTextHandler
	addMessageWithArguments(WriteAsText);
	addMessageProperty(WriteAsText, hidden, YES);
	addMessageWithArguments(ReadFromText);
	addMessageProperty(ReadFromText, hidden, YES);
    
    // Creation of the first static event of the scenario (but don't subscribe to it)
    err = TTObjectBaseInstantiate(TTSymbol("StaticEvent"), TTObjectBaseHandle(&mFirstEvent), kTTValNONE);
    
	if (err) {
        mFirstEvent = NULL;
		logError("Scenario failed to load the first static event");
    }
    
    // Creation of the last static event of the scenario and subscribe to it
    err = TTObjectBaseInstantiate(TTSymbol("StaticEvent"), TTObjectBaseHandle(&mLastEvent), kTTValNONE);
    
	if (err) {
        mLastEvent = NULL;
		logError("Scenario failed to load the last static event");
    }
    
    // Subscribe to the last event using the mEndEventCallback (see in ProcessEnd why)
    mLastEvent->sendMessage(TTSymbol("Subscribe"), mEndEventCallback, kTTValNONE);
    
    // Create the edition solver
    mEditionSolver = new Solver();
    
    // Create extended int
    plusInfinity = ExtendedInt(PLUS_INFINITY, 0);
    minusInfinity = ExtendedInt(MINUS_INFINITY, 0);
    integer0 = ExtendedInt(INTEGER, 0);
}

Scenario::~Scenario()
{
    if (mNamespace) {
        delete mNamespace;
        mNamespace = NULL;
    }
    
    if (mFirstEvent) {
        TTObjectBaseRelease(TTObjectBaseHandle(&mFirstEvent));
        mFirstEvent = NULL;
    }
    
    if (mLastEvent) {
        TTObjectBaseRelease(TTObjectBaseHandle(&mLastEvent));
        mLastEvent = NULL;
    }
    
    if (mEditionSolver) {
        delete mEditionSolver;
        mEditionSolver = NULL;
    }
    
    if (mExecutionGraph) {
        delete mExecutionGraph;
        mExecutionGraph = NULL;
    }
}

TTErr Scenario::getParameterNames(TTValue& value)
{
    value.clear();
	//value.append(TTSymbol("aParameterName"));
	
	return kTTErrNone;
}

TTErr Scenario::ProcessStart()
{
    
    // compile the mExecutionGraph to prepare scenario execution
    compileScenario(0);
    
    // Trigger the first event
    mFirstEvent->sendMessage(TTSymbol("Trigger"), kTTValNONE, kTTValNONE);
    
    // Notify the first event subscribers
    mFirstEvent->sendMessage(TTSymbol("Notify"));
    
    return kTTErrNone;
}

TTErr Scenario::ProcessEnd()
{
    // Trigger the end event
    mEndEvent->sendMessage(TTSymbol("Trigger"), kTTValNONE, kTTValNONE);
    
    // Notify the end event subscribers
    mEndEvent->sendMessage(TTSymbol("Notify"));
    
    // TODO : clear mExecutionGraph as we don't need it until the next execution
    
    return kTTErrNone;
}

TTErr Scenario::Process(const TTValue& inputValue, TTValue& outputValue)
{
    TTFloat64 progression;
    
    if (inputValue.size() == 1) {
        
        if (inputValue[0].type() == kTypeFloat64) {
            
            progression = inputValue[0];
            
            // TODO : we need to update the mExecutionGraph to process the scenario
            
            /* ADD : from mExecutionGraph::mainThreadFunction
             
            mExecutionGraph->m_startPlace->produceTokens(1);
            mExecutionGraph->m_endPlace->consumeTokens(mExecutionGraph->m_endPlace->nbOfTokens());
            
            mExecutionGraph->addTime(mExecutionGraph->getTimeOffset() * 1000);
            //mExecutionGraph->makeOneStep();
            
            while (mExecutionGraph->m_endPlace->nbOfTokens() == 0 && !mExecutionGraph->m_mustStop) {
                mExecutionGraph->update();
                
                cout << "mExecutionGraph::mainThreadFunction -- " << mExecutionGraph->m_currentTime << endl;
            }
            
            mExecutionGraph->m_isRunning = false;
            */
            
            return kTTErrNone;
        }
    }
    
    return kTTErrGeneric;
}

TTErr Scenario::TimeProcessAdd(const TTValue& inputValue, TTValue& outputValue)
{
    TimeProcessPtr          aTimeProcess;
    TTValue                 aCacheElement, v;
    TTValue                 startEvent, endEvent;
    TTValue                 duration, durationMin, durationMax, scenarioDuration;
    SolverVariablePtr       startVariable, endVariable;
    SolverObjectMapIterator it;
    
    if (inputValue.size() == 1) {
        
        if (inputValue[0].type() == kTypeObject) {
            
            aTimeProcess = TimeProcessPtr((TTObjectBasePtr)inputValue[0]);
            
            // check time process duration
            if (!aTimeProcess->getAttributeValue(TTSymbol("duration"), duration)) {
                
                // check if the time process is not already registered
                mTimeProcessList.find(&ScenarioFindTimeProcess, (TTPtr)aTimeProcess, aCacheElement);
                
                // if couldn't find the same time process in the scenario
                if (aCacheElement == kTTValNONE) {
                    
                    // create all observers
                    makeTimeProcessCacheElement(aTimeProcess, aCacheElement);
                    
                    // store time process object and observers
                    mTimeProcessList.append(aCacheElement);
                    
                    // add the time process events to the scenario
                    aTimeProcess->getAttributeValue(TTSymbol("startEvent"), startEvent);
                    TimeEventAdd(startEvent, kTTValNONE);
                    
                    aTimeProcess->getAttributeValue(TTSymbol("endEvent"), endEvent);
                    TimeEventAdd(endEvent, kTTValNONE);
                    
                    // get time process duration bounds
                    aTimeProcess->getAttributeValue(TTSymbol("durationMin"), durationMin);
                    aTimeProcess->getAttributeValue(TTSymbol("durationMax"), durationMax);
                    
                    // get scenario duration
                    this->getAttributeValue(TTSymbol("duration"), scenarioDuration);
                    
                    // retreive solver variable relative to each event
                    it = mVariablesMap.find(TTObjectBasePtr(startEvent[0]));
                    startVariable = SolverVariablePtr(it->second);
                    
                    it = mVariablesMap.find(TTObjectBasePtr(endEvent[0]));
                    endVariable = SolverVariablePtr(it->second);
                    
                    // update the Solver depending on the type of the time process
                    if (aTimeProcess->getName() == TTSymbol("Interval")) {
                        
                        // add a relation between the 2 variables to the solver
                        SolverRelationPtr relation = new SolverRelation(mEditionSolver, startVariable, endVariable, TTUInt32(durationMin[0]), TTUInt32(durationMax[0]));
                        
                        // store the relation relative to this time process
                        mRelationsMap.emplace(aTimeProcess, relation);

                    }
                    else {
                        
                        // add a constraint between the 2 variables to the solver
                        SolverConstraintPtr constraint = new SolverConstraint(mEditionSolver, startVariable, endVariable, TTUInt32(durationMin[0]), TTUInt32(durationMax[0]), TTUInt32(scenarioDuration[0]));
                        
                        // store the constraint relative to this time process
                        mConstraintsMap.emplace(aTimeProcess, constraint);
                        
                    }
                }
                
                return kTTErrNone;
            }
        }
    }
    
    return kTTErrGeneric;
}

TTErr Scenario::TimeProcessRemove(const TTValue& inputValue, TTValue& outputValue)
{
    TimeProcessPtr          aTimeProcess;
    TTValue                 aCacheElement;
    TTValue                 startEvent, endEvent;
    SolverObjectMapIterator it;
    
    if (inputValue.size() == 1) {
        
        if (inputValue[0].type() == kTypeObject) {
            
            aTimeProcess = TimeProcessPtr((TTObjectBasePtr)inputValue[0]);
            
            // try to find the time process
            mTimeProcessList.find(&ScenarioFindTimeProcess, (TTPtr)aTimeProcess, aCacheElement);
            
            // couldn't find the same time process in the scenario :
            if (aCacheElement == kTTValNONE)
                return kTTErrValueNotFound;
            
            else {
                
                // remove time process object and observers
                mTimeProcessList.remove(aCacheElement);
                
                // delete all observers
                deleteTimeProcessCacheElement(aCacheElement);
                
                // update the Solver depending on the type of the time process
                if (aTimeProcess->getName() == TTSymbol("Interval")) {
                    
                    // retreive solver relation relative to the time process
                    it = mRelationsMap.find(aTimeProcess);
                    SolverRelationPtr relation = SolverRelationPtr(it->second);
                    
                    mRelationsMap.erase(aTimeProcess);
                    delete relation;
                    
                } else {
 
                    // retreive solver constraint relative to the time process
                    it = mConstraintsMap.find(aTimeProcess);
                    SolverConstraintPtr constraint = SolverConstraintPtr(it->second);
                    
                    mConstraintsMap.erase(aTimeProcess);
                    delete constraint;
                }
                
                // remove the time process events from the scenario
                aTimeProcess->getAttributeValue(TTSymbol("startEvent"), startEvent);
                TimeEventRemove(startEvent, kTTValNONE);
                
                aTimeProcess->getAttributeValue(TTSymbol("endEvent"), endEvent);
                TimeEventRemove(endEvent, kTTValNONE);
                
                return kTTErrNone;
            }
        }
    }
    
    return kTTErrGeneric;
}

TTErr Scenario::TimeProcessMove(const TTValue& inputValue, TTValue& outputValue)
{
    TimeProcessPtr          aTimeProcess;
    TTValue                 v;
    TTValue                 startEvent, endEvent;
    TTSymbol                timeProcessType;
    SolverObjectMapIterator it;
    SolverError             sErr;
    
    if (inputValue.size() == 3) {
        
        if (inputValue[0].type() == kTypeObject && inputValue[1].type() == kTypeUInt32 && inputValue[2].type() == kTypeUInt32) {
            
            aTimeProcess = TimeProcessPtr((TTObjectBasePtr)inputValue[0]);
            
            // get time process events
            aTimeProcess->getAttributeValue(TTSymbol("startEvent"), startEvent);
            aTimeProcess->getAttributeValue(TTSymbol("endEvent"), endEvent);
            
            // update the Solver depending on the type of the time process
            timeProcessType = aTimeProcess->getName();
            
            if (timeProcessType == TTSymbol("Interval")) {
                
                // retreive solver relation relative to the time process
                it = mRelationsMap.find(aTimeProcess);
                SolverRelationPtr relation = SolverRelationPtr(it->second);
                
                sErr = relation->move(inputValue[1], inputValue[2]);

            } else {
                
                // retreive solver constraint relative to the time process
                it = mConstraintsMap.find(aTimeProcess);
                SolverConstraintPtr constraint = SolverConstraintPtr(it->second);
                
                sErr = constraint->move(inputValue[1], inputValue[2]);
            }
            
            if (!sErr) {
                
                // update each solver variable value
                for (it = mVariablesMap.begin() ; it != mVariablesMap.end() ; it++)
                    
                    SolverVariablePtr(it->second)->update();
                
                return kTTErrNone;
            }
        }
    }
    
    return kTTErrGeneric;
}

TTErr Scenario::TimeProcessLimit(const TTValue& inputValue, TTValue& outputValue)
{
    TimeProcessPtr          aTimeProcess;
    TTValue                 durationMin, durationMax;
    TTSymbol                timeProcessType;
    SolverObjectMapIterator it;
    SolverError             sErr;
    
    if (inputValue.size() == 3) {
        
        if (inputValue[0].type() == kTypeObject && inputValue[1].type() == kTypeUInt32 && inputValue[2].type() == kTypeUInt32) {
            
            aTimeProcess = TimeProcessPtr((TTObjectBasePtr)inputValue[0]);
            
            // update the Solver depending on the type of the time process
            timeProcessType = aTimeProcess->getName();
            
            if (timeProcessType == TTSymbol("Interval")) {
                
                // retreive solver relation relative to the time process
                it = mRelationsMap.find(aTimeProcess);
                SolverRelationPtr relation = SolverRelationPtr(it->second);
                
                sErr = relation->limit(inputValue[1], inputValue[2]);
                
            } else {
                
                // retreive solver constraint relative to the time process
                it = mConstraintsMap.find(aTimeProcess);
                SolverConstraintPtr constraint = SolverConstraintPtr(it->second);
                
                sErr = constraint->limit(inputValue[1], inputValue[2]);
            }
            
            if (!sErr) {
                
                // update each solver variable value
                for (it = mVariablesMap.begin() ; it != mVariablesMap.end() ; it++)
                    
                    SolverVariablePtr(it->second)->update();
                
                return kTTErrNone;
            }
        }
    }
    
    return kTTErrGeneric;
}

TTErr Scenario::TimeEventAdd(const TTValue& inputValue, TTValue& outputValue)
{
    TimeEventPtr    aTimeEvent;
    TTValue         aCacheElement, scenarioDuration;
    
    if (inputValue.size() == 1) {
        
        if (inputValue[0].type() == kTypeObject) {
            
            aTimeEvent = TimeEventPtr((TTObjectBasePtr)inputValue[0]);
            
            // check if the time event is not already registered
            mTimeEventList.find(&ScenarioFindTimeEvent, (TTPtr)aTimeEvent, aCacheElement);
            
            // if couldn't find the same time event in the scenario
            if (aCacheElement == kTTValNONE) {
                
                // create all observers
                makeTimeEventCacheElement(aTimeEvent, aCacheElement);
                
                // store time event object and observers
                mTimeEventList.append(aCacheElement);
                
                // get scenario duration
                this->getAttributeValue(TTSymbol("duration"), scenarioDuration);
                
                // add variable to the solver
                SolverVariablePtr variable = new SolverVariable(mEditionSolver, aTimeEvent, 0, TTUInt32(scenarioDuration[0]));
                
                // store the variables relative to those time events
                mVariablesMap.emplace(TTObjectBasePtr(aTimeEvent), variable);
            }
            
            return kTTErrNone;
        }
    }
    
    return kTTErrGeneric;
}

TTErr Scenario::TimeEventRemove(const TTValue& inputValue, TTValue& outputValue)
{
    TimeEventPtr            aTimeEvent;
    TTValue                 aCacheElement;
    TTBoolean               found;
    SolverVariablePtr       variable;
    SolverObjectMapIterator it;
    
    if (inputValue.size() == 1) {
        
        if (inputValue[0].type() == kTypeObject) {
            
            aTimeEvent = TimeEventPtr((TTObjectBasePtr)inputValue[0]);
            
            // try to find the time event
            mTimeEventList.find(&ScenarioFindTimeEvent, (TTPtr)aTimeEvent, aCacheElement);
            
            // couldn't find the same time event in the scenario
            if (aCacheElement == kTTValNONE)
                return kTTErrValueNotFound;
            else {
                
                // remove time event object and observers
                mTimeEventList.remove(aCacheElement);
                
                // delete all observers
                deleteTimeEventCacheElement(aCacheElement);
                
                // retreive solver variable relative to each event
                it = mVariablesMap.find(aTimeEvent);
                variable = SolverVariablePtr(it->second);
                
                // remove variable from the solver
                // if it is still not used in a constraint
                found = NO;
                for (it = mConstraintsMap.begin() ; it != mConstraintsMap.end() ; it++) {
                    
                    found = SolverConstraintPtr(it->second)->startVariable == variable || SolverConstraintPtr(it->second)->endVariable == variable;
                    if (found) break;
                }
                
                if (!found) {
                    
                    // if it is still not used in a relation
                    for (it = mRelationsMap.begin() ; it != mRelationsMap.end() ; it++) {
                        
                        found = SolverRelationPtr(it->second)->startVariable == variable || SolverRelationPtr(it->second)->endVariable == variable;
                        if (found) break;
                    }
                }
                
                if (!found) {
                    mVariablesMap.erase(aTimeEvent);
                    delete variable;
                }
                
                return kTTErrNone;
            }
        }
    }
    
    return kTTErrGeneric;
}

TTErr Scenario::TimeEventMove(const TTValue& inputValue, TTValue& outputValue)
{
    TimeEventPtr            aTimeEvent;
    TTValue                 v;
    TTSymbol                timeEventType;
    SolverVariablePtr       variable;
    SolverObjectMapIterator it;
    SolverError             sErr;
    
    if (inputValue.size() == 2) {
        
        if (inputValue[0].type() == kTypeObject && inputValue[1].type() == kTypeUInt32 ) {
            
            aTimeEvent = TimeEventPtr((TTObjectBasePtr)inputValue[0]);
            
            // retreive solver variable relative to the time event
            it = mVariablesMap.find(aTimeEvent);
            variable = SolverVariablePtr(it->second);
            
            // move all constraints relative to the variable
            for (it = mConstraintsMap.begin() ; it != mConstraintsMap.end() ; it++) {
                
                if (SolverConstraintPtr(it->second)->startVariable == variable)
                    sErr = SolverConstraintPtr(it->second)->move(TTUInt32(inputValue[1]), SolverConstraintPtr(it->second)->endVariable->get());
                    
                if (SolverConstraintPtr(it->second)->endVariable == variable)
                    sErr = SolverConstraintPtr(it->second)->move(SolverConstraintPtr(it->second)->startVariable->get(), TTUInt32(inputValue[1]));
                
                if (sErr)
                    break;
            }
            
            if (!sErr) {
                
                // update each solver variable value
                for (it = mVariablesMap.begin() ; it != mVariablesMap.end() ; it++)
                    
                    SolverVariablePtr(it->second)->update();
                
                return kTTErrNone;
            }
        }
    }
    
    return kTTErrGeneric;
}

TTErr Scenario::TimeEventReplace(const TTValue& inputValue, TTValue& outputValue)
{
    TimeEventPtr            aFormerTimeEvent, aNewTimeEvent, timeEvent;
    TimeProcessPtr          aTimeProcess;
    TTBoolean               isStatic;
    TTValue                 v;
    SolverObjectMapIterator it;
    
    if (inputValue.size() == 2) {
        
        if (inputValue[0].type() == kTypeObject && inputValue[1].type() == kTypeObject) {
            
            aFormerTimeEvent = TimeEventPtr((TTObjectBasePtr)inputValue[0]);
            aNewTimeEvent = TimeEventPtr((TTObjectBasePtr)inputValue[1]);
            
            // is the new event static ?
            isStatic = aNewTimeEvent->getName() == TTSymbol("Static");
                        
            // replace the former time event in all time process which binds on it
            for (mTimeProcessList.begin(); mTimeProcessList.end(); mTimeProcessList.next()) {
                
                aTimeProcess = TimeProcessPtr(TTObjectBasePtr(mTimeProcessList.current()[0]));
                
                aTimeProcess->getAttributeValue(TTSymbol("startEvent"), v);
                timeEvent = TimeEventPtr(TTObjectBasePtr(v[0]));
                
                if (timeEvent == aFormerTimeEvent) {
                    
                    aTimeProcess->sendMessage(TTSymbol("StartEventRelease"));
                    aTimeProcess->setAttributeValue(TTSymbol("startEvent"), TTObjectBasePtr(aNewTimeEvent));
                    continue;
                }
                
                aTimeProcess->getAttributeValue(TTSymbol("endEvent"), v);
                timeEvent = TimeEventPtr(TTObjectBasePtr(v[0]));
                
                if (timeEvent == aFormerTimeEvent) {
                    
                    aTimeProcess->sendMessage(TTSymbol("EndEventRelease"));
                    aTimeProcess->setAttributeValue(TTSymbol("endEvent"), TTObjectBasePtr(aNewTimeEvent));
                    
                    // a time process with a none static end event cannot be rigid
                    v = TTBoolean(isStatic);
                    aTimeProcess->setAttributeValue(TTSymbol("rigid"), v);
                }
            }
            
            // retreive solver variable relative to the time event
            it = mVariablesMap.find(aFormerTimeEvent);
            SolverVariablePtr variable = SolverVariablePtr(it->second);
            
            // replace the time event
            // note : only in the variable as all other solver elements binds on variables
            variable->event = aNewTimeEvent;
            
            // update the variable (this will copy the date into the new time event)
            variable->update();
            
            // update the variables map
            mVariablesMap.erase(aFormerTimeEvent);
            mVariablesMap.emplace(aNewTimeEvent, variable);
            
            return kTTErrNone;
        }
    }
    
    return kTTErrGeneric;
}

TTErr Scenario::TimeProcessActiveChange(const TTValue& inputValue, TTValue& outputValue)
{
    TimeProcessPtr  aTimeProcess;
    TTBoolean       active;
	
    if (inputValue.size() == 2) {
        
        if (inputValue[0].type() == kTypeObject && inputValue[1].type() == kTypeBoolean) {
            
            // get time process where the change comes from
            aTimeProcess = TimeProcessPtr((TTObjectBasePtr)inputValue[0]);
            
            // get new active value
            active = inputValue[1];
            
            // TODO : warn Solver (or mExecutionGraph ?) that this time process active state have changed
            // TODO : update all Solver (or mExecutionGraph ?) consequences by setting time processes attributes that are affected by the consequence
        }
    }
    
    return kTTErrGeneric;
}

TTErr Scenario::WriteAsXml(const TTValue& inputValue, TTValue& outputValue)
{
	TTXmlHandlerPtr	aXmlHandler = NULL;
	
	aXmlHandler = TTXmlHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// TODO : pass the XmlHandlerPtr to all the time processes and their time events of the scenario to write their content into the file
	
	return kTTErrGeneric;
}

TTErr Scenario::ReadFromXml(const TTValue& inputValue, TTValue& outputValue)
{
	TTXmlHandlerPtr	aXmlHandler = NULL;
	
	aXmlHandler = TTXmlHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// TODO : pass the XmlHandlerPtr to all the time processes of the scenario to read their content from the file
	
	return kTTErrGeneric;
}

TTErr Scenario::WriteAsText(const TTValue& inputValue, TTValue& outputValue)
{
	TTTextHandlerPtr	aTextHandler;
	
	aTextHandler = TTTextHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// TODO : pass the TextHandlerPtr to all the time processes of the scenario to write their content into the file
	
	return kTTErrGeneric;
}

TTErr Scenario::ReadFromText(const TTValue& inputValue, TTValue& outputValue)
{
	TTTextHandlerPtr aTextHandler;
	TTValue	v;
	
	aTextHandler = TTTextHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
    // TODO : pass the TextHandlerPtr to all the time processes of the scenario to read their content from the file
	
	return kTTErrGeneric;
}

void Scenario::makeTimeProcessCacheElement(TimeProcessPtr aTimeProcess, TTValue& newCacheElement)
{
    TTValue			v;
    TTAttributePtr	anAttribute;
	TTObjectBasePtr	aScheduler, runningObserver, progressionObserver;
    TTValuePtr		runningBaton, progressionBaton;
    TTErr           err;
	
	// 0 : cache time process object
	newCacheElement.append((TTObjectBasePtr)aTimeProcess);
    
    // get the scheduler
    aTimeProcess->getAttributeValue(TTSymbol("scheduler"), v);
    aScheduler = v[0];
    
    // 1 : create and cache scheduler running attribute observer on this time process
    err = aScheduler->findAttribute(TTSymbol("running"), &anAttribute);
    
    if (!err) {
        
        runningObserver = NULL;
        TTObjectBaseInstantiate(TTSymbol("callback"), &runningObserver, kTTValNONE);
        
        runningBaton = new TTValue(TTObjectBasePtr(this));
        runningBaton->append(TTObjectBasePtr(aTimeProcess));
        
        runningObserver->setAttributeValue(kTTSym_baton, TTPtr(runningBaton));
        runningObserver->setAttributeValue(kTTSym_function, TTPtr(&ScenarioSchedulerRunningAttributeCallback));
        
        anAttribute->registerObserverForNotifications(*runningObserver);
    }
    
    newCacheElement.append(runningObserver);
    
    // 2 : create and cache scheduler progression attribute observer on this time process
    err = aScheduler->findAttribute(TTSymbol("progression"), &anAttribute);
    
    if (!err) {
        
        progressionObserver = NULL;
        TTObjectBaseInstantiate(TTSymbol("callback"), &progressionObserver, kTTValNONE);
        
        progressionBaton = new TTValue(TTObjectBasePtr(this));
        progressionBaton->append(TTObjectBasePtr(aTimeProcess));
        
        progressionObserver->setAttributeValue(kTTSym_baton, TTPtr(progressionBaton));
        progressionObserver->setAttributeValue(kTTSym_function, TTPtr(&ScenarioSchedulerProgressionAttributeCallback));
        
        anAttribute->registerObserverForNotifications(*progressionObserver);
    }
    
    newCacheElement.append(progressionObserver);
}

void Scenario::deleteTimeProcessCacheElement(const TTValue& oldCacheElement)
{
	TTValue			v;
    TimeProcessPtr	aTimeProcess;
    TTObjectBasePtr aScheduler, anObserver;
	TTAttributePtr	anAttribute;
	TTErr			err;
    
    // 0 : get cached time process
    aTimeProcess = TimeProcessPtr((TTObjectBasePtr)oldCacheElement[0]);
    
    // get the scheduler
    aTimeProcess->getAttributeValue(TTSymbol("scheduler"), v);
    aScheduler = v[0];
    
    // 1 : delete scheduler running attribute observer
    anObserver = NULL;
    anObserver = oldCacheElement[1];
    
    anAttribute = NULL;
    err = aScheduler->findAttribute(TTSymbol("running"), &anAttribute);
    
    if (!err) {
        
        err = anAttribute->unregisterObserverForNotifications(*anObserver);
        
        if (!err)
            TTObjectBaseRelease(&anObserver);
    }
    
    // 2 : delete scheduler progression attribute observer
    anObserver = NULL;
    anObserver = oldCacheElement[2];
    
    anAttribute = NULL;
    err = aScheduler->findAttribute(TTSymbol("progression"), &anAttribute);
    
    if (!err) {
        
        err = anAttribute->unregisterObserverForNotifications(*anObserver);
        
        if (!err)
            TTObjectBaseRelease(&anObserver);
    }    
}

void Scenario::makeTimeEventCacheElement(TimeEventPtr aTimeEvent, TTValue& newCacheElement)
{
    TTValue			v;
    TTMessagePtr	aMessage;
	TTObjectBasePtr	triggerObserver;
    TTValuePtr		triggerBaton;
    TTErr           err;
	
	// 0 : cache time process object
	newCacheElement.append((TTObjectBasePtr)aTimeEvent);
    
    // 1 : create and cache event trigger message observer on this time event
    err = aTimeEvent->findMessage(TTSymbol("Trigger"), &aMessage);
    
    if (!err) {
        triggerObserver = NULL;
        TTObjectBaseInstantiate(TTSymbol("callback"), &triggerObserver, kTTValNONE);
        
        triggerBaton = new TTValue(TTObjectBasePtr(this));
        triggerBaton->append(TTObjectBasePtr(aTimeEvent));
        
        triggerObserver->setAttributeValue(kTTSym_baton, TTPtr(triggerBaton));
        triggerObserver->setAttributeValue(kTTSym_function, TTPtr(&ScenarioSchedulerRunningAttributeCallback));
        
        aMessage->registerObserverForNotifications(*triggerObserver);
    }
    
    newCacheElement.append(triggerObserver);
}

void Scenario::deleteTimeEventCacheElement(const TTValue& oldCacheElement)
{
    TTValue			v;
    TimeEventPtr	aTimeEvent;
    TTObjectBasePtr anObserver;
	TTMessagePtr	aMessage;
	TTErr			err;
    
    // 0 : get cached time event
    aTimeEvent = TimeEventPtr((TTObjectBasePtr)oldCacheElement[0]);
    
    // 1 : delete event trigger message observer observer
    anObserver = NULL;
    anObserver = oldCacheElement[1];
    
    aMessage = NULL;
    err = aTimeEvent->findMessage(TTSymbol("Trigger"), &aMessage);
    
    if (!err) {
        
        err = aMessage->unregisterObserverForNotifications(*anObserver);
        
        if (!err)
            TTObjectBaseRelease(&anObserver);
    }
}

#if 0
#pragma mark -
#pragma mark Some Methods
#endif

void ScenarioFindTimeProcess(const TTValue& aValue, TTPtr timeProcessPtrToMatch, TTBoolean& found)
{
	found = (TTObjectBasePtr)aValue[0] == (TTObjectBasePtr)timeProcessPtrToMatch;
}

void ScenarioFindTimeProcessWithTimeEvent(const TTValue& aValue, TTPtr timeEventPtrToMatch, TTBoolean& found)
{
    TimeProcessPtr  timeProcess = TimeProcessPtr(TTObjectBasePtr(aValue[0]));
    TTValue         v;
    
    // ignore interval process
    if (timeProcess->getName() != TTSymbol("Interval")) {
        
        // check start event
        timeProcess->getAttributeValue(TTSymbol("startEvent"), v);
        found = TTObjectBasePtr(v[0]) == TTObjectBasePtr(timeEventPtrToMatch);
        
        if (found)
            return;
        
        // check end event
        timeProcess->getAttributeValue(TTSymbol("endEvent"), v);
        found = TTObjectBasePtr(v[0]) == TTObjectBasePtr(timeEventPtrToMatch);
        
        return;
    }
}

void ScenarioFindTimeEvent(const TTValue& aValue, TTPtr timeEventPtrToMatch, TTBoolean& found)
{
    found = (TTObjectBasePtr)aValue[0] == (TTObjectBasePtr)timeEventPtrToMatch;
}

TTErr ScenarioSchedulerRunningAttributeCallback(TTPtr baton, TTValue& data)
{
    TimeProcessPtr  aScenario, aTimeProcess;
	TTValuePtr      b;
    TTSymbol        timeProcessType;
    TTBoolean       running;
	
	// unpack baton (a scenario and a time process)
	b = (TTValuePtr)baton;
	aScenario = TimeProcessPtr((TTObjectBasePtr)(*b)[0]);
    aTimeProcess = TimeProcessPtr((TTObjectBasePtr)(*b)[1]);
    
    // get new running value
    running = data[0];
    
    timeProcessType = aTimeProcess->getName();
    
    if (timeProcessType == TTSymbol("Automation")) {
        
        // TODO : update scenario's ECOMachine scheduler in Automation case
        return kTTErrGeneric;
    }
    else if (timeProcessType == TTSymbol("Scenario")) {
        
        // TODO : update scenario's ECOMachine scheduler in Scenario case
        return kTTErrGeneric;
    }
    else if (timeProcessType == TTSymbol("Interval")) {
        
        // TODO : update scenario's ECOMachine scheduler in Interval case
        return kTTErrGeneric;
    }
    // else if ...
    
    return kTTErrGeneric;
}

TTErr ScenarioSchedulerProgressionAttributeCallback(TTPtr baton, TTValue& data)
{
    TimeProcessPtr  aScenario, aTimeProcess;
	TTValuePtr      b;
    TTSymbol        timeProcessType;
    TTFloat64       progression;
	
	// unpack baton (a scenario and a time process)
	b = (TTValuePtr)baton;
	aScenario = TimeProcessPtr((TTObjectBasePtr)(*b)[0]);
    aTimeProcess = TimeProcessPtr((TTObjectBasePtr)(*b)[1]);
    
    // get new progression value
    progression = data[0];
    
    // TODO : warn Scheduler that this time process progression have changed (?)
    timeProcessType = aTimeProcess->getName();
    
    if (timeProcessType == TTSymbol("Automation")) {
        
        // TODO : update scenario's ECOMachine scheduler in Automation case
        return kTTErrGeneric;
    }
    else if (timeProcessType == TTSymbol("Scenario")) {
        
        // TODO : update scenario's ECOMachine scheduler in Scenario case
        return kTTErrGeneric;
    }
    else if (timeProcessType == TTSymbol("Interval")) {
        
        // TODO : update scenario's ECOMachine scheduler in Interval case
        return kTTErrGeneric;
    }
    // else if ...
    
    return kTTErrGeneric;
}

void ScenarioGraphTransitionTimeProcessCallBack(void* arg)
{
    // cf ECOProcess : processCallBack function
    
    TimeProcessPtr  aTimeProcess = (TimeProcessPtr) arg;
    TTValue         v;
    TTObjectBasePtr aScheduler;
    TTFloat64       duration;
    
    // get the scheduler object of the time process
    aTimeProcess->getAttributeValue(TTSymbol("scheduler"), v);
    aScheduler = TTObjectBasePtr(v[0]);
    
    // set scheduler duration equal to the time process duration
    aTimeProcess->getAttributeValue(TTSymbol("duration"), v);
    
    duration = TTUInt32(v[0]);
    aScheduler->setAttributeValue(TTSymbol("duration"), duration);
    
    aScheduler->sendMessage(TTSymbol("Go"));
}

void ScenarioGraphTransitionTimeEventCallBack(void* arg)
{
    // cf ECOMachine : crossAControlPointCallBack function

    TimeEventPtr aTimeEvent = (TimeEventPtr) arg;
    
	aTimeEvent->sendMessage(TTSymbol("Notify"));
}
/*
void waitedTriggerPointMessageCallBack(void* arg, bool isWaited, TransitionPtr transition)
{
	ECOMachine* currentECOMachine = (ECOMachine*) arg;
    
	if (currentECOMachine->hasTriggerPointInformations(transition)) {
		TriggerPointInformations currentTriggerPointInformations = currentECOMachine->getTriggerPointInformations(transition);
        
		if (currentTriggerPointInformations.m_waitedTriggerPointMessageAction != NULL) {
			currentTriggerPointInformations.m_waitedTriggerPointMessageAction(currentECOMachine->m_waitedTriggerPointMessageArg,
																			  isWaited,
																			  currentTriggerPointInformations.m_triggerId,
																			  currentTriggerPointInformations.m_boxId,
																			  currentTriggerPointInformations.m_controlPointIndex,
																			  currentTriggerPointInformations.m_waitedString);
            
		}
	}
}
*/
