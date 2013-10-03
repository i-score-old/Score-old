/** @file
 *
 * @ingroup scoreExtension
 *
 * @brief Scenario time process class is a container class to manage other time processes instances in the time
 *
 * @see TimePluginLib, TTTimeProcess
 *
 * @authors Théo de la Hogue & Clément Bossut
 *
 * @copyright Copyright © 2013, Théo de la Hogue & Clément Bossut @n
 * This code is licensed under the terms of the "CeCILL-C" @n
 * http://www.cecill.info
 */

#include "Scenario.h"

#define thisTTClass                 Scenario
#define thisTTClassName             "Scenario"
#define thisTTClassTags             "time, process, container, scenario"

extern "C" TT_EXTENSION_EXPORT TTErr TTLoadJamomaExtension_Scenario(void)
{
	TTFoundationInit();
	Scenario::registerClass();
	return kTTErrNone;
}

TIME_CONTAINER_CONSTRUCTOR,
mNamespace(NULL),
mViewZoom(TTValue(1., 1.)),
mViewPosition(TTValue(0, 0)),
mEditionSolver(NULL),
mExecutionGraph(NULL)
{
    TIME_PLUGIN_INITIALIZE
    
	TT_ASSERT("Correct number of args to create Scenario", arguments.size() == 0 || arguments.size() == 2);
    
    addAttributeWithSetter(ViewZoom, kTypeLocalValue);
    addAttributeWithSetter(ViewPosition, kTypeLocalValue);
    
    addMessage(Compile);
    
    // Create the edition solver
    mEditionSolver = new Solver();
    
    // Create extended int
    plusInfinity = ExtendedInt(PLUS_INFINITY, 0);
    minusInfinity = ExtendedInt(MINUS_INFINITY, 0);
    integer0 = ExtendedInt(INTEGER, 0);
    
    // it is possible to pass 2 events for the root scenario (which don't need a container by definition)
    if (arguments.size() == 2) {
        
        if (arguments[0].type() == kTypeObject && arguments[1].type() == kTypeObject) {
            
            this->setStartEvent(TTTimeEventPtr(TTObjectBasePtr(arguments[0])));
            this->setEndEvent(TTTimeEventPtr(TTObjectBasePtr(arguments[1])));
        }
    }
}

Scenario::~Scenario()
{
    if (mNamespace) {
        delete mNamespace;
        mNamespace = NULL;
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

TTErr Scenario::setViewZoom(const TTValue& value)
{
    mViewZoom = value;
    
    return kTTErrNone;
}

TTErr Scenario::setViewPosition(const TTValue& value)
{
    mViewPosition = value;
    
    return kTTErrNone;
}

TTErr Scenario::Compile()
{
    // compile the mExecutionGraph to prepare scenario execution
    // TODO : pass a time offset
    compileGraph(0);
    
    return kTTErrNone;
}

TTErr Scenario::ProcessStart()
{
    // start the execution graph
    mExecutionGraph->start();
    
    // TODO : deal with the timeOffset
    // problem : if we offset the scheduler time it will never call ProcessStart method (called when progression == 0)
    // idea : add the offset afterward ? for any time process or is this specific to a scenario ?
    //mExecutionGraph->addTime(mExecutionGraph->getTimeOffset() * 1000);
    
    return kTTErrNone;
}

TTErr Scenario::ProcessEnd()
{
    return kTTErrNone;
}

TTErr Scenario::Process(const TTValue& inputValue, TTValue& outputValue)
{
    TTFloat64   progression, realTime;
    TTValue     v;
    
    if (inputValue.size() == 2) {
        
        if (inputValue[0].type() == kTypeFloat64 && inputValue[0].type() == kTypeFloat64) {
            
            progression = inputValue[0];
            realTime = inputValue[1];
            
            // update the mExecutionGraph to process the scenario
            if (mExecutionGraph->makeOneStep(realTime))
                return kTTErrNone;
            
            else
                // stop the scenario
                return this->sendMessage(kTTSym_Stop);
            
        }
    }
    
    return kTTErrGeneric;
}

TTErr Scenario::WriteAsXml(const TTValue& inputValue, TTValue& outputValue)
{
	TTXmlHandlerPtr     aXmlHandler = NULL;
    TTTimeProcessPtr    aTimeProcess;
    TTTimeEventPtr      aTimeEvent;
    TTTimeConditionPtr  aTimeCondition;
	
	aXmlHandler = TTXmlHandlerPtr((TTObjectBasePtr)inputValue[0]);
    
    // if the scenario is not handled by a upper scenario
    if (mContainer == NULL) {
        
        // Start a Scenario node
        xmlTextWriterStartElement((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "Scenario");
        
        writeTimeProcessAsXml(aXmlHandler, this);
        
        TTValue     v;
        TTString    s;
        
        // Write the end date (e.g. root scenario duration)
        this->getAttributeValue(TTSymbol("endDate"), v);
        v.toString();
        s = TTString(v[0]);
        xmlTextWriterWriteAttribute((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "endDate", BAD_CAST s.data());
        
        // Write the view zoom
        v = mViewZoom;
        v.toString();
        s = TTString(v[0]);
        xmlTextWriterWriteAttribute((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "viewZoom", BAD_CAST s.data());
        
        // Write the view position
        v = mViewPosition;
        v.toString();
        s = TTString(v[0]);
        xmlTextWriterWriteAttribute((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "viewPosition", BAD_CAST s.data());
    }
    
    // Write all the time events
    for (mTimeEventList.begin(); mTimeEventList.end(); mTimeEventList.next()) {
        
        aTimeEvent = TTTimeEventPtr(TTObjectBasePtr(mTimeEventList.current()[0]));
        
        writeTimeEventAsXml(aXmlHandler, aTimeEvent);
    }
    
    // Write all the time processes
    for (mTimeProcessList.begin(); mTimeProcessList.end(); mTimeProcessList.next()) {
        
        aTimeProcess = TTTimeProcessPtr(TTObjectBasePtr(mTimeProcessList.current()[0]));
        
        writeTimeProcessAsXml(aXmlHandler, aTimeProcess);
    }
    
    // Write all the time conditions
    for (mTimeConditionList.begin(); mTimeConditionList.end(); mTimeConditionList.next()) {
        
        aTimeCondition = TTTimeConditionPtr(TTObjectBasePtr(mTimeConditionList.current()[0]));
        
        writeTimeConditionAsXml(aXmlHandler, aTimeCondition);
    }
    
    // if the scenario is not handled by a upper scenario
    if (mContainer == NULL) {
        
        // Close the event node
        xmlTextWriterEndElement((xmlTextWriterPtr)aXmlHandler->mWriter);
    }
	
	return kTTErrNone;
}

TTErr Scenario::ReadFromXml(const TTValue& inputValue, TTValue& outputValue)
{
    TTXmlHandlerPtr         aXmlHandler = NULL;
    SolverObjectMapIterator itSolver;
    GraphObjectMapIterator  itGraph;
    TTValue                 v;
	
	aXmlHandler = TTXmlHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// Switch on the name of the XML node
	
    // if the scenario is not handled by a upper scenario (root Scenario case)
    if (mContainer == NULL) {
        
        // Starts scenario reading
        if (aXmlHandler->mXmlNodeName == kTTSym_start) {
            
            mCurrentTimeEvent = NULL;
            mCurrentTimeProcess = NULL;
            mCurrentTimeCondition = NULL;
            
            // clear all data structures
            mTimeEventList.clear();
            mTimeProcessList.clear();
            
            for (itSolver = mVariablesMap.begin() ; itSolver != mVariablesMap.end() ; itSolver++)
                delete (SolverVariablePtr)itSolver->second;
            
            mVariablesMap.clear();
            
            for (itSolver = mConstraintsMap.begin() ; itSolver != mConstraintsMap.end() ; itSolver++)
                delete (SolverConstraintPtr)itSolver->second;
            
            mConstraintsMap.clear();
            
            for (itSolver = mRelationsMap.begin() ; itSolver != mRelationsMap.end() ; itSolver++)
                delete (SolverRelationPtr)itSolver->second;
            
            mRelationsMap.clear();
            
            delete mEditionSolver;
            mEditionSolver = new Solver();
            
            clearGraph();
            
            return kTTErrNone;
        }
        
        // Ends scenario reading
        if (aXmlHandler->mXmlNodeName == kTTSym_stop) {
            
            return kTTErrNone;
        }
        
        // Scenario node (root Scenario case)
        if (aXmlHandler->mXmlNodeName == TTSymbol("Scenario")) {
            
            // Get the scenario name
            if (!aXmlHandler->getXmlAttribute(kTTSym_name, v, YES)) {
                
                if (v.size() == 1) {
                    
                    if (v[0].type() == kTypeSymbol) {
                        
                        mName = v[0];
                    }
                }
            }
            
            // Get the scenario end date (e.g. root scenario duration)
            if (!aXmlHandler->getXmlAttribute(kTTSym_endDate, v, NO)) {
                
                if (v.size() == 1) {
                    
                    if (v[0].type() == kTypeUInt32) {
                        
                         this->setAttributeValue(TTSymbol("endDate"), v);
                    }
                }
            }
            
            // Get the scenario color
            if (!aXmlHandler->getXmlAttribute(kTTSym_color, v, NO)) {
                
                if (v.size() == 3) {
                    
                    if (v[0].type() == kTypeInt32 && v[1].type() == kTypeInt32 && v[2].type() == kTypeInt32) {
                        
                        mColor = v;
                    }
                }
            }
        }
    }
    
    // Scenario node (sub and root Scenario case)
    if (aXmlHandler->mXmlNodeName == TTSymbol("Scenario")) {
        
        // Get the scenario viewZoom
        if (!aXmlHandler->getXmlAttribute(kTTSym_viewZoom, v, NO)) {
            
            if (v.size() == 2) {
                
                if (v[0].type() == kTypeFloat64 && v[1].type() == kTypeFloat64) {
                    
                    mViewZoom = v;
                }
            }
        }
        
        // Get the scenario viewPosition
        if (!aXmlHandler->getXmlAttribute(kTTSym_viewPosition, v, NO)) {
            
            if (v.size() == 2) {
                
                if (v[0].type() == kTypeFloat64 && v[1].type() == kTypeFloat64) {
                    
                    mViewPosition = v;
                }
            }
        }
        
        return kTTErrNone;
    }

    
    // Event node
    if (aXmlHandler->mXmlNodeName == kTTSym_event) {
        
        if (aXmlHandler->mXmlNodeStart) {
            
            mCurrentTimeEvent = readTimeEventFromXml(aXmlHandler);
            
            if (aXmlHandler->mXmlNodeIsEmpty)
                mCurrentTimeEvent = NULL;
        }
        else
            
            mCurrentTimeEvent = NULL;
        
        return kTTErrNone;
    }
    
    // If there is a current time event
    if (mCurrentTimeEvent) {
        
        // Pass the xml handler to the current condition to fill his data structure
        v = TTObjectBasePtr(mCurrentTimeEvent);
        aXmlHandler->setAttributeValue(kTTSym_object, v);
        return aXmlHandler->sendMessage(kTTSym_Read);
    }
    
    // Condition node
    if (aXmlHandler->mXmlNodeName == kTTSym_condition) {
        
        if (aXmlHandler->mXmlNodeStart) {
            
            mCurrentTimeCondition = readTimeConditionFromXml(aXmlHandler);
        
            if (aXmlHandler->mXmlNodeIsEmpty)
                mCurrentTimeCondition = NULL;
        }
        else
            
            mCurrentTimeCondition = NULL;
        
        return kTTErrNone;
    }
    
    // If there is a current time condition
    if (mCurrentTimeCondition) {
        
        // Pass the xml handler to the current condition to fill his data structure
        v = TTObjectBasePtr(mCurrentTimeCondition);
        aXmlHandler->setAttributeValue(kTTSym_object, v);
        return aXmlHandler->sendMessage(kTTSym_Read);
    }
    
    // Process node : the name of the node is the name of the process type
    if (!mCurrentTimeProcess) {
        
        if (aXmlHandler->mXmlNodeStart)
            mCurrentTimeProcess = readTimeProcessFromXml(aXmlHandler);
        
        if (aXmlHandler->mXmlNodeIsEmpty)
            mCurrentTimeProcess = NULL;
    }
    else if (mCurrentTimeProcess->getName() == aXmlHandler->mXmlNodeName) {
        
        if (!aXmlHandler->mXmlNodeStart)
            mCurrentTimeProcess = NULL;
    }
    
    // If there is a current time process
    if (mCurrentTimeProcess) {
        
        // Pass the xml handler to the current process to fill his data structure
        v = TTObjectBasePtr(mCurrentTimeProcess);
        aXmlHandler->setAttributeValue(kTTSym_object, v);
        return aXmlHandler->sendMessage(kTTSym_Read);
    }
    
    return kTTErrNone;
}

TTErr Scenario::TimeEventCreate(const TTValue& inputValue, TTValue& outputValue)
{
    TTTimeEventPtr  aTimeEvent = NULL;
    TTValue         args, aCacheElement, scenarioDuration;
    
    if (inputValue.size() == 1) {
        
        if (inputValue[0].type() == kTypeUInt32) {
            
            // prepare argument (date, container)
            args = TTValue(inputValue[0]);
            args.append(TTObjectBasePtr(this));
            
            // create the time event
            if(TTObjectBaseInstantiate(kTTSym_TimeEvent, TTObjectBaseHandle(&aTimeEvent), args))
                return kTTErrGeneric;
            
            // create all observers
            makeTimeEventCacheElement(aTimeEvent, aCacheElement);
            
            // store time event object and observers
            mTimeEventList.append(aCacheElement);
            
            // get scenario duration
            this->getAttributeValue(kTTSym_duration, scenarioDuration);
            
            // add variable to the solver
            SolverVariablePtr variable = new SolverVariable(mEditionSolver, aTimeEvent, TTUInt32(scenarioDuration[0]));
            
            // store the variable relative to the time event
            mVariablesMap.emplace(TTObjectBasePtr(aTimeEvent), variable);
            
            // return the time event
            outputValue = TTObjectBasePtr(aTimeEvent);
            
            return kTTErrNone;
        }
    }
    
    return kTTErrGeneric;
}

TTErr Scenario::TimeEventRelease(const TTValue& inputValue, TTValue& outputValue)
{
    TTTimeEventPtr          aTimeEvent;
    TTValue                 v, aCacheElement;
    TTBoolean               found;
    SolverVariablePtr       variable;
    SolverObjectMapIterator it;
    
    if (inputValue.size() == 1) {
        
        if (inputValue[0].type() == kTypeObject) {
            
            aTimeEvent = TTTimeEventPtr((TTObjectBasePtr)inputValue[0]);
            
            // try to find the time event
            mTimeEventList.find(&TTTimeContainerFindTimeEvent, (TTPtr)aTimeEvent, aCacheElement);
            
            // couldn't find the same time event in the scenario
            if (aCacheElement == kTTValNONE)
                return kTTErrValueNotFound;
            
            else {
                
                // if the time event is used by a time process it can't be released
                mTimeProcessList.find(&TTTimeContainerFindTimeProcessWithTimeEvent, (TTPtr)aTimeEvent, v);
                
                if (v == kTTValNONE) {
                    
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
                    
                    // release the time event
                    TTObjectBaseRelease(TTObjectBaseHandle(&aTimeEvent));
                    
                    return kTTErrNone;
                }
            }
        }
    }
    
    return kTTErrGeneric;
}

TTErr Scenario::TimeEventMove(const TTValue& inputValue, TTValue& outputValue)
{
    TTTimeEventPtr          aTimeEvent;
    SolverVariablePtr       variable;
    SolverObjectMapIterator it;
    SolverError             sErr;
    
    if (inputValue.size() == 2) {
        
        if (inputValue[0].type() == kTypeObject && inputValue[1].type() == kTypeUInt32 ) {
            
            aTimeEvent = TTTimeEventPtr((TTObjectBasePtr)inputValue[0]);
            
            // retreive solver variable relative to the time event
            it = mVariablesMap.find(aTimeEvent);
            variable = SolverVariablePtr(it->second);
            
            // move all constraints relative to the variable
            for (it = mConstraintsMap.begin() ; it != mConstraintsMap.end() ; it++) {
                
                if (SolverConstraintPtr(it->second)->startVariable == variable)
                    sErr = SolverConstraintPtr(it->second)->move(inputValue[1], SolverConstraintPtr(it->second)->endVariable->get());
                
                if (SolverConstraintPtr(it->second)->endVariable == variable)
                    sErr = SolverConstraintPtr(it->second)->move(SolverConstraintPtr(it->second)->startVariable->get(), inputValue[1]);
                
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

TTErr Scenario::TimeEventCondition(const TTValue& inputValue, TTValue& outputValue)
{
    TTTimeEventPtr          aTimeEvent;
    TTTimeConditionPtr      aTimeCondition;
    TTTimeProcessPtr        aTimeProcess;
    TTValue                 v, aCacheElement;
    SolverObjectMapIterator it;
    
    if (inputValue.size() == 2) {
        
        if (inputValue[0].type() == kTypeObject && inputValue[1].type() == kTypeObject) {
            
            aTimeEvent = TTTimeEventPtr((TTObjectBasePtr)inputValue[0]);
            aTimeCondition = TTTimeConditionPtr((TTObjectBasePtr)inputValue[1]);
            
            // try to find the time event
            mTimeEventList.find(&TTTimeContainerFindTimeEvent, (TTPtr)aTimeEvent, aCacheElement);
            
            // couldn't find the former time event in the scenario
            if (aCacheElement == kTTValNONE)
                return kTTErrValueNotFound;
            
            // try to find the time condition
            mTimeConditionList.find(&TTTimeContainerFindTimeCondition, (TTPtr)aTimeCondition, aCacheElement);
            
            // couldn't find the former time condition in the scenario
            if (aCacheElement == kTTValNONE)
                return kTTErrValueNotFound;
            
            // for each time process
            for (mTimeProcessList.begin(); mTimeProcessList.end(); mTimeProcessList.next()) {
                
                aTimeProcess = TTTimeProcessPtr(TTObjectBasePtr(mTimeProcessList.current()[0]));
                
                // if the time event is the time process end event
                if (aTimeEvent == getTimeProcessEndEvent(aTimeProcess)) {
                    
                    // a time process with a conditioned end event cannot be rigid
                    v = TTBoolean(aTimeCondition == NULL);
                    aTimeProcess->setAttributeValue(kTTSym_rigid, v);
                }
            }
            
            // théo : maybe there will be other stuff to do considering there is a condition with several case now ? 
            
            return kTTErrNone;
        }
    }
    
    return kTTErrGeneric;
}

TTErr Scenario::TimeEventTrigger(const TTValue& inputValue, TTValue& outputValue)
{
    TTTimeEventPtr aTimeEvent;
    
    if (inputValue.size() == 1) {
        
        if (inputValue[0].type() == kTypeObject) {
            
            aTimeEvent = TTTimeEventPtr((TTObjectBasePtr)inputValue[0]);
            
            // cf ECOMachine::receiveNetworkMessage(std::string netMessage)
            
            if (mExecutionGraph) {
                
                // if the excecution graph is running
                if (mExecutionGraph->getUpdateFactor() != 0) {
                    
                    // append the event to the event queue to process its triggering
                    TTLogMessage("Scenario::TimeEventTrigger : %p\n", TTPtr(aTimeEvent));
                    mExecutionGraph->putAnEvent(TTPtr(aTimeEvent));
                    
                    return kTTErrNone;
                }
            }
        }
    }
    
    return kTTErrGeneric;
}

TTErr Scenario::TimeEventReplace(const TTValue& inputValue, TTValue& outputValue)
{
    TTTimeEventPtr          aFormerTimeEvent, aNewTimeEvent;
    TTTimeProcessPtr        aTimeProcess;
    TTValue                 v, aCacheElement;
    SolverObjectMapIterator it;
    
    if (inputValue.size() == 2) {
        
        if (inputValue[0].type() == kTypeObject && inputValue[1].type() == kTypeObject) {
            
            aFormerTimeEvent = TTTimeEventPtr((TTObjectBasePtr)inputValue[0]);
            aNewTimeEvent = TTTimeEventPtr((TTObjectBasePtr)inputValue[1]);
            
            // try to find the former time event
            mTimeEventList.find(&TTTimeContainerFindTimeEvent, (TTPtr)aFormerTimeEvent, aCacheElement);
            
            // couldn't find the former time event in the scenario
            if (aCacheElement == kTTValNONE)
                return kTTErrValueNotFound;
            
            else {
                
                // remove the former time event object and observers
                mTimeEventList.remove(aCacheElement);
                
                // delete all observers on the former time event
                deleteTimeEventCacheElement(aCacheElement);
                
                // create all observers on the new time event
                makeTimeEventCacheElement(aNewTimeEvent, aCacheElement);
                
                // store the new time event object and observers
                mTimeEventList.append(aCacheElement);
            }
            
            // replace the former time event in all time process which binds on it
            for (mTimeProcessList.begin(); mTimeProcessList.end(); mTimeProcessList.next()) {
                
                aTimeProcess = TTTimeProcessPtr(TTObjectBasePtr(mTimeProcessList.current()[0]));
                
                if (getTimeProcessStartEvent(aTimeProcess) == aFormerTimeEvent) {
                    
                    setTimeProcessStartEvent(aTimeProcess, aNewTimeEvent);
                    continue;
                }
                
                if (getTimeProcessEndEvent(aTimeProcess) == aFormerTimeEvent) {
                    
                    setTimeProcessEndEvent(aTimeProcess, aNewTimeEvent);
                    
                    // a time process with a conditioned end event cannot be rigid
                    v = TTBoolean(getTimeEventCondition(aNewTimeEvent) == NULL);
                    aTimeProcess->setAttributeValue(kTTSym_rigid, v);
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

TTErr Scenario::TimeProcessCreate(const TTValue& inputValue, TTValue& outputValue)
{
    TTTimeProcessPtr        aTimeProcess = NULL;
    TTValue                 args, aCacheElement;
    TTValue                 duration, scenarioDuration;
    SolverVariablePtr       startVariable, endVariable;
    SolverObjectMapIterator it;
    
    if (inputValue.size() == 3) {
        
        if (inputValue[0].type() == kTypeSymbol && inputValue[1].type() == kTypeObject && inputValue[2].type() == kTypeObject) {
            
            // prepare argument (container)
            args = TTObjectBasePtr(this);
            
            // create a time process of the given type
            if (TTObjectBaseInstantiate(inputValue[0], TTObjectBaseHandle(&aTimeProcess), args))
                return kTTErrGeneric;
            
            // set the start and end events
            setTimeProcessStartEvent(aTimeProcess, TTTimeEventPtr(TTObjectBasePtr(inputValue[1])));
            setTimeProcessEndEvent(aTimeProcess, TTTimeEventPtr(TTObjectBasePtr(inputValue[2])));
            
            // check time process duration
            if (!aTimeProcess->getAttributeValue(kTTSym_duration, duration)) {
                
                // create all observers
                makeTimeProcessCacheElement(aTimeProcess, aCacheElement);
                
                // store time process object and observers
                mTimeProcessList.append(aCacheElement);
                
                // get scenario duration
                this->getAttributeValue(kTTSym_duration, scenarioDuration);
                
                // retreive solver variable relative to each event
                it = mVariablesMap.find(getTimeProcessStartEvent(aTimeProcess));
                startVariable = SolverVariablePtr(it->second);
                
                it = mVariablesMap.find(getTimeProcessEndEvent(aTimeProcess));
                endVariable = SolverVariablePtr(it->second);
                
                // update the Solver depending on the type of the time process
                if (aTimeProcess->getName() == TTSymbol("Interval")) {
                    
                    // add a relation between the 2 variables to the solver
                    SolverRelationPtr relation = new SolverRelation(mEditionSolver, startVariable, endVariable, getTimeProcessDurationMin(aTimeProcess), getTimeProcessDurationMax(aTimeProcess));
                    
                    // store the relation relative to this time process
                    mRelationsMap.emplace(aTimeProcess, relation);
                    
                }
                else {
                    
                    // limit the start variable to the process duration
                    // this avoid time crushing when a time process moves while it is connected to other process
                    startVariable->limit(duration, duration);
                    
                    // add a constraint between the 2 variables to the solver
                    SolverConstraintPtr constraint = new SolverConstraint(mEditionSolver, startVariable, endVariable, getTimeProcessDurationMin(aTimeProcess), getTimeProcessDurationMax(aTimeProcess), TTUInt32(scenarioDuration[0]));
                    
                    // store the constraint relative to this time process
                    mConstraintsMap.emplace(aTimeProcess, constraint);
                    
                }
                
                // return the time process
                outputValue = TTObjectBasePtr(aTimeProcess);
                
                return kTTErrNone;
            }
            
            // in case of bad duration
            else {
                
                // release the time process
                TTObjectBaseRelease(TTObjectBaseHandle(&aTimeProcess));
            }
        }
    }
    
    return kTTErrGeneric;
}

TTErr Scenario::TimeProcessRelease(const TTValue& inputValue, TTValue& outputValue)
{
    TTTimeProcessPtr        aTimeProcess;
    TTValue                 aCacheElement;
    SolverObjectMapIterator it;
    
    if (inputValue.size() == 1) {
        
        if (inputValue[0].type() == kTypeObject) {
            
            aTimeProcess = TTTimeProcessPtr((TTObjectBasePtr)inputValue[0]);
            
            // try to find the time process
            mTimeProcessList.find(&TTTimeContainerFindTimeProcess, (TTPtr)aTimeProcess, aCacheElement);
            
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
                
                // fill outputValue with start and event
                outputValue.resize(2);
                outputValue[0] = getTimeProcessStartEvent(aTimeProcess);
                outputValue[1] = getTimeProcessEndEvent(aTimeProcess);
                
                // release the time process
                TTObjectBaseRelease(TTObjectBaseHandle(&aTimeProcess));
                
                return kTTErrNone;
            }
        }
    }
    
    return kTTErrGeneric;
}

TTErr Scenario::TimeProcessMove(const TTValue& inputValue, TTValue& outputValue)
{
    TTTimeProcessPtr        aTimeProcess;
    TTValue                 v;
    TTValue                 duration, scenarioDuration;
    TTSymbol                timeProcessType;
    SolverObjectMapIterator it;
    SolverError             sErr;
    
    if (inputValue.size() == 3) {
        
        if (inputValue[0].type() == kTypeObject && inputValue[1].type() == kTypeUInt32 && inputValue[2].type() == kTypeUInt32) {
            
            aTimeProcess = TTTimeProcessPtr((TTObjectBasePtr)inputValue[0]);
            
            // get time process duration
            aTimeProcess->getAttributeValue(kTTSym_duration, duration);
            
            // get scenario duration
            this->getAttributeValue(kTTSym_duration, scenarioDuration);
            
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
                
                // extend the limit of the start variable
                constraint->startVariable->limit(0, TTUInt32(scenarioDuration[0]));
                
                sErr = constraint->move(inputValue[1], inputValue[2]);
                
                // set the start variable limit back
                // this avoid time crushing when a time process moves while it is connected to other process
                constraint->startVariable->limit(TTUInt32(duration[0]), TTUInt32(duration[0]));
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
    TTTimeProcessPtr        aTimeProcess;
    TTValue                 durationMin, durationMax;
    TTSymbol                timeProcessType;
    SolverObjectMapIterator it;
    SolverError             sErr;
    
    if (inputValue.size() == 3) {
        
        if (inputValue[0].type() == kTypeObject && inputValue[1].type() == kTypeUInt32 && inputValue[2].type() == kTypeUInt32) {
            
            aTimeProcess = TTTimeProcessPtr((TTObjectBasePtr)inputValue[0]);
            
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

/*
 TTErr Scenario::TimeProcessActiveChange(const TTValue& inputValue, TTValue& outputValue)
 {
 TTTimeProcessPtr    aTimeProcess;
 TTBoolean           active;
 
 if (inputValue.size() == 2) {
 
 if (inputValue[0].type() == kTypeObject && inputValue[1].type() == kTypeBoolean) {
 
 // get time process where the change comes from
 aTimeProcess = TTTimeProcessPtr((TTObjectBasePtr)inputValue[0]);
 
 // get new active value
 active = inputValue[1];
 
 // TODO : warn Solver (or mExecutionGraph ?) that this time process active state have changed
 // TODO : update all Solver (or mExecutionGraph ?) consequences by setting time processes attributes that are affected by the consequence
 }
 }
 
 return kTTErrGeneric;
 }
 */

TTErr Scenario::TimeConditionCreate(const TTValue& inputValue, TTValue& outputValue)
{
    TTTimeConditionPtr  aTimeCondition = NULL;
    TTValue             args, aCacheElement, out;
    
    // prepare argument (container)
    args = TTValue(TTObjectBasePtr(this));
    
    // create the time condition
    if(TTObjectBaseInstantiate(TTSymbol("TimeCondition"), TTObjectBaseHandle(&aTimeCondition), args))
        return kTTErrGeneric;
    
    // create all observers
    makeTimeConditionCacheElement(aTimeCondition, aCacheElement);
    
    // store time condition object and observers
    mTimeConditionList.append(aCacheElement);
    
    // add a first case if
    
    // TODO : how conditions are constrained by the Solver ?
    
    // return the time condition
    outputValue = TTObjectBasePtr(aTimeCondition);
    
    // optionnal : for each given symbol, add a case to the condition
    for (TTUInt8 i = 0; i < inputValue.size(); i++)
        if (inputValue[i].type() == kTypeSymbol)
            aTimeCondition->sendMessage(TTSymbol("CaseAdd"), inputValue[i], out);
    
    return kTTErrNone;
}

TTErr Scenario::TimeConditionRelease(const TTValue& inputValue, TTValue& outputValue)
{
    TTTimeConditionPtr      aTimeCondition;
    TTValue                 aCacheElement;
    
    if (inputValue.size() == 1) {
        
        if (inputValue[0].type() == kTypeObject) {
            
            aTimeCondition = TTTimeConditionPtr((TTObjectBasePtr)inputValue[0]);
            
            // try to find the time condition
            mTimeConditionList.find(&TTTimeContainerFindTimeCondition, (TTPtr)aTimeCondition, aCacheElement);
            
            // couldn't find the same time condition in the scenario
            if (aCacheElement == kTTValNONE)
                return kTTErrValueNotFound;
            
            else {
                
                // remove time condition object and observers
                mTimeConditionList.remove(aCacheElement);
                
                // delete all observers
                deleteTimeConditionCacheElement(aCacheElement);
                
                // release the time condition
                TTObjectBaseRelease(TTObjectBaseHandle(&aTimeCondition));
                
                return kTTErrNone;
            }
        }
    }
    
    return kTTErrGeneric;
}

void Scenario::makeTimeProcessCacheElement(TTTimeProcessPtr aTimeProcess, TTValue& newCacheElement)
{
    newCacheElement.clear();
    
	// 0 : cache time process object
	newCacheElement.append((TTObjectBasePtr)aTimeProcess);
}

void Scenario::deleteTimeProcessCacheElement(const TTValue& oldCacheElement)
{
    ;
}

void Scenario::makeTimeEventCacheElement(TTTimeEventPtr aTimeEvent, TTValue& newCacheElement)
{
    newCacheElement.clear();
    
	// 0 : cache time event object
	newCacheElement.append((TTObjectBasePtr)aTimeEvent);
}

void Scenario::deleteTimeEventCacheElement(const TTValue& oldCacheElement)
{
    ;
}

void Scenario::makeTimeConditionCacheElement(TTTimeConditionPtr aTimeCondition, TTValue& newCacheElement)
{
    newCacheElement.clear();
    
	// 0 : cache time condition object
	newCacheElement.append((TTObjectBasePtr)aTimeCondition);
}

void Scenario::deleteTimeConditionCacheElement(const TTValue& oldCacheElement)
{
    ;
}

#if 0
#pragma mark -
#pragma mark Some Methods
#endif

void ScenarioGraphTimeEventCallBack(TTPtr arg)
{
    // cf ECOMachine : crossAControlPointCallBack function
    
    TTTimeEventPtr aTimeEvent = (TTTimeEventPtr) arg;
    
	aTimeEvent->sendMessage(kTTSym_Happen);
}


void ScenarioGraphIsEventReadyCallBack(TTPtr arg, TTBoolean isReady)
{
	TTTimeEventPtr aTimeEvent = (TTTimeEventPtr) arg;
    
    TTValue v = isReady;
    aTimeEvent->setAttributeValue(kTTSym_ready, v);
}