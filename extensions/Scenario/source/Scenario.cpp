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

TIME_CONTAINER_PLUGIN_CONSTRUCTOR,
mNamespace(NULL),
mViewZoom(TTValue(1., 1.)),
mViewPosition(TTValue(0, 0)),
#ifndef NO_EDITION_SOLVER
mEditionSolver(NULL),
#endif
#ifndef NO_EXECUTION_GRAPH
mExecutionGraph(NULL),
#endif
mLoading(NO),
mAttributeLoaded(NO)
{
    TIME_PLUGIN_INITIALIZE
    
	TT_ASSERT("Correct number of args to create Scenario", arguments.size() == 0 || arguments.size() == 2);
    
    addAttributeWithSetter(ViewZoom, kTypeLocalValue);
    addAttributeWithSetter(ViewPosition, kTypeLocalValue);
    
#ifndef NO_EXECUTION_GRAPH
    addMessage(Compile);
#endif
#ifndef NO_EDITION_SOLVER
    // Create the edition solver
    mEditionSolver = new Solver();
#endif
#ifndef NO_EXECUTION_GRAPH
    // Create extended int
    plusInfinity = ExtendedInt(PLUS_INFINITY, 0);
    minusInfinity = ExtendedInt(MINUS_INFINITY, 0);
    integer0 = ExtendedInt(INTEGER, 0);
#endif
    
    // it is possible to pass 2 events for the root scenario (which don't need a container by definition)
    if (arguments.size() == 2) {
        
        if (arguments[0].type() == kTypeObject && arguments[1].type() == kTypeObject) {
            
            TTObject start = arguments[0];
            TTObject end = arguments[1];
            
            this->setStartEvent(start);
            this->setEndEvent(end);
        }
    }
    // else create 2 time events with the end at 1 hour (in millisecond)
    else {
        
        TTObject start("TimeEvent");
        TTObject end("TimeEvent", 36000000);
        
        this->setStartEvent(start);
        this->setEndEvent(end);
    }
    
    mScheduler.set("granularity", TTFloat64(1.));
}

Scenario::~Scenario()
{
    if (mNamespace) {
        delete mNamespace;
        mNamespace = NULL;
    }
#ifndef NO_EDITION_SOLVER
    if (mEditionSolver) {
        delete mEditionSolver;
        mEditionSolver = NULL;
    }
#endif
#ifndef NO_EXECUTION_GRAPH
    if (mExecutionGraph) {
        delete mExecutionGraph;
        mExecutionGraph = NULL;
    }
#endif
    
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
    TTValue     v;
    TTUInt32    timeOffset;
    TTBoolean   compiled;
    TTObject    aTimeEvent;
    TTObject    aTimeProcess;
    
    // don't compile empty scenario
    if (mTimeEventList.isEmpty() && mTimeProcessList.isEmpty() && mTimeConditionList.isEmpty())
        return kTTErrGeneric;
    
    // get scheduler time offset
    mScheduler.get(kTTSym_offset, v);
    timeOffset = TTFloat64(v[0]);
    
    // set all time events to a waiting status
    for (mTimeEventList.begin(); mTimeEventList.end(); mTimeEventList.next()) {
        
        aTimeEvent = mTimeEventList.current()[0];
        
        aTimeEvent.set(kTTSym_status, kTTSym_eventWaiting);
    }
    
#ifndef NO_EXECUTION_GRAPH
    // compile the mExecutionGraph to prepare scenario execution from the scheduler time offset
    compileGraph(timeOffset);
#endif
    
    // compile all time processes if they need to be compiled and propagate the externalTick attribute
    for (mTimeProcessList.begin(); mTimeProcessList.end(); mTimeProcessList.next()) {
        
        aTimeProcess = mTimeProcessList.current()[0];
        
        aTimeProcess.get(kTTSym_compiled, v);
        compiled = v[0];
        
        if (!compiled)
            aTimeProcess.send(kTTSym_Compile);
        
        aTimeProcess.set("externalTick", mExternalTick);
    }
    
    mCompiled = YES;
    
    return kTTErrNone;
}

TTErr Scenario::ProcessStart()
{
#ifndef NO_EXECUTION_GRAPH
    
    // the execution graph needs to be compiled before
    if (!mCompiled)
        return kTTErrNone;

    // start the execution graph
    mExecutionGraph->start();

#else
    
    TTLogMessage("Scenario::ProcessStart : without execution graph\n");
    
    // go to the first time event (as they are sorted by date)
    mTimeEventList.begin();
#endif
    return kTTErrNone;
}

TTErr Scenario::ProcessEnd()
{
    TTObject aTimeProcess;

    // When a Scenario ends : stop all the time processes
    for (mTimeProcessList.begin(); mTimeProcessList.end(); mTimeProcessList.next()) {
        
        aTimeProcess = mTimeProcessList.current()[0];
        
        aTimeProcess.send(kTTSym_Stop);
    }
    
    // needs to be compiled again
    mCompiled = NO;
   
    return kTTErrNone;
}

TTErr Scenario::Process(const TTValue& inputValue, TTValue& outputValue)
{
    TTFloat64       position, date;
    TTObject		aTimeCondition, aTimeProcess;
    TTValue         v;
    
    if (inputValue.size() == 2) {
        
        if (inputValue[0].type() == kTypeFloat64 && inputValue[1].type() == kTypeFloat64) {
            
            position = inputValue[0];
            date = inputValue[1];
            
            // enable or disable conditions
            for (mTimeConditionList.begin(); mTimeConditionList.end(); mTimeConditionList.next()) {
                
                aTimeCondition = mTimeConditionList.current()[0];
                
                // if a condition is ready we activate it
                aTimeCondition.get(kTTSym_ready, v);
                aTimeCondition.set(kTTSym_active, v);
            }
            
            // propagate the tick to all the time process
            if (mExternalTick) {
                
                for (mTimeProcessList.begin(); mTimeProcessList.end(); mTimeProcessList.next()) {
                
                    aTimeProcess = mTimeProcessList.current()[0];
                
                    aTimeProcess.send(kTTSym_Tick);
                }
            }
            
#ifndef NO_EXECUTION_GRAPH
            // the execution graph needs to be compiled before
            if (!mCompiled)
                return kTTErrGeneric;
            
            // update the mExecutionGraph to process the scenario
            if (mExecutionGraph->makeOneStep(date))
                return kTTErrNone;
            
            // the root Scenario ends itself
            else if (mContainer == NULL)
                return getEndEvent().send(kTTSym_Happen);
#else
            TTValue     v;
            TTUInt32    eventDate;
            
            // if there is more event to process
            if (mTimeEventList.end()) {
                
                // get the current time event (as they are sorted by date)
                TTObject aTimeEvent = mTimeEventList.current()[0];
                aTimeEvent.get(kTTSym_date, v);
                eventDate = v[0];
                
                // if the event date is lower than the current date
                if (eventDate < date) {
                    
                    // make the event to happen
                    aTimeEvent.send(kTTSym_Happen);
                    
                    // try to process the next event
                    mTimeEventList.next();
                    return Process(inputValue, outputValue);
                }
            }
            else
                // Make the end happen
                return getEndEvent()->send(kTTSym_Happen);
#endif
            
        }
    }
    
    return kTTErrGeneric;
}

TTErr Scenario::ProcessPaused(const TTValue& inputValue, TTValue& outputValue)
{
    TTObject    aTimeProcess;
    TTBoolean   paused;
    
    if (inputValue.size() == 1) {
        
        if (inputValue[0].type() == kTypeBoolean) {
            
            paused = inputValue[0];
            
            for (mTimeProcessList.begin(); mTimeProcessList.end(); mTimeProcessList.next()) {
                
                aTimeProcess = mTimeProcessList.current()[0];
                
                if (paused)
                    aTimeProcess.send(kTTSym_Pause);
                else
                    aTimeProcess.send(kTTSym_Resume);
            }
        }
        
        return kTTErrNone;
    }
    
    return kTTErrGeneric;
}

TTErr Scenario::Goto(const TTValue& inputValue, TTValue& outputValue)
{
    TTObject		aTimeEvent, aTimeProcess, state;
    TTValue         v, none;
    TTUInt32        duration, timeOffset, date;
    TTBoolean       muteRecall = NO;
    
    if (inputValue.size() >= 1) {
        
        if (inputValue[0].type() == kTypeUInt32 || inputValue[0].type() == kTypeInt32) {
            
            this->getAttributeValue(kTTSym_duration, v);
            
            // TODO : TTTimeProcess should extend Scheduler class ?
            duration = v[0];
            mScheduler.set(kTTSym_duration, TTFloat64(duration));
            
            timeOffset = inputValue[0];
            mScheduler.set(kTTSym_offset, TTFloat64(timeOffset));
            
            // is the recall of the state is muted ?
            if (inputValue.size() == 2) {
                
                if (inputValue[1].type() == kTypeBoolean) {
                    
                    muteRecall = inputValue[1];
                }
            }
            
            if (!muteRecall && !mMute) {
                
                // create a temporary state to compile all the event states before the time offset
                state = TTObject(kTTSym_Script);
                
                // add the state of the scenario start
                TTScriptMerge(getTimeEventState(getStartEvent()), state);;
                
                // add the state of each event before the time offset (expect those which are muted)
                for (mTimeEventList.begin(); mTimeEventList.end(); mTimeEventList.next()) {
                    
                    aTimeEvent = mTimeEventList.current()[0];
                    aTimeEvent.get(kTTSym_date, v);
                    date = v[0];
                    
                    aTimeEvent.get(kTTSym_mute, v);
                    TTBoolean mute = v[0];
                    
                    if (!mute) {
                        
                        // merge the event state into the temporary state
                        if (date < timeOffset)
                            TTScriptMerge(getTimeEventState( getStartEvent()), state);
                    }
                }
                
                // run the temporary state
                state.send(kTTSym_Run);
            }
            
            // prepare the timeOffset of each time process scheduler
            for (mTimeProcessList.begin(); mTimeProcessList.end(); mTimeProcessList.next()) {
                
                aTimeProcess = mTimeProcessList.current()[0];
                
                TTObject  startEvent = getTimeProcessStartEvent(aTimeProcess);
                TTObject  endEvent = getTimeProcessEndEvent(aTimeProcess);
                
                // if the date to start is in the middle of a time process
                if (getTimeEventDate(startEvent) < timeOffset && getTimeEventDate(endEvent) > timeOffset) {
                    
                    // go to time offset
                    v = timeOffset - getTimeEventDate(startEvent);
                }
                
                else if (getTimeEventDate(startEvent) >= timeOffset)
                    v = TTUInt32(0.);
                
                else if (getTimeEventDate(endEvent) <= timeOffset)
                    v = TTUInt32(1.);
                
                v.append(muteRecall);
                
                aTimeProcess.send(kTTSym_Goto, v, none);
            }
            
            // needs to be compiled again
            mCompiled = NO;
            
            return kTTErrNone;
        }
    }
    
    return kTTErrGeneric;
}

TTErr Scenario::WriteAsXml(const TTValue& inputValue, TTValue& outputValue)
{
    TTObject o = inputValue[0];
	TTXmlHandlerPtr aXmlHandler = (TTXmlHandlerPtr)o.instance();
    if (!aXmlHandler)
		return kTTErrGeneric;
    
    TTObject aTimeProcess;
    TTObject aTimeEvent;
    TTObject aTimeCondition;
    
    // if the scenario is not handled by a upper scenario
    if (!mContainer.valid()) {
        
        TTValue     v;
        TTString    s;
        TTObject    thisObject(this);
        
        // Start a Scenario node
        xmlTextWriterStartElement((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "Scenario");
        
        writeTimeProcessAsXml(aXmlHandler, thisObject);
        
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
        
        // Write the start event
        {
            xmlTextWriterStartElement((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "startEvent");
            
            // Write the name
            xmlTextWriterWriteAttribute((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "name", BAD_CAST kTTSym_start.c_str());
            
            // Pass the xml handler to the event to fill his attribute
            aXmlHandler->setAttributeValue(kTTSym_object, getStartEvent());
            aXmlHandler->sendMessage(kTTSym_Write);
            
            // Close the event node
            xmlTextWriterEndElement((xmlTextWriterPtr)aXmlHandler->mWriter);
        }
        
        // Write the end event
        {
            xmlTextWriterStartElement((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "endEvent");
            
            // Write the name
            xmlTextWriterWriteAttribute((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "name", BAD_CAST kTTSym_end.c_str());
            
            // Pass the xml handler to the event to fill his attribute
            aXmlHandler->setAttributeValue(kTTSym_object, getEndEvent());
            aXmlHandler->sendMessage(kTTSym_Write);
            
            // Close the event node
            xmlTextWriterEndElement((xmlTextWriterPtr)aXmlHandler->mWriter);
        }
    }
    
    // Write all the time events
    for (mTimeEventList.begin(); mTimeEventList.end(); mTimeEventList.next()) {
        
        aTimeEvent = mTimeEventList.current()[0];
        
        writeTimeEventAsXml(aXmlHandler, aTimeEvent);
    }
    
    // Write all the time processes
    for (mTimeProcessList.begin(); mTimeProcessList.end(); mTimeProcessList.next()) {
        
        aTimeProcess = mTimeProcessList.current()[0];
        
        writeTimeProcessAsXml(aXmlHandler, aTimeProcess);
    }
    
    // Write all the time conditions
    for (mTimeConditionList.begin(); mTimeConditionList.end(); mTimeConditionList.next()) {
        
        aTimeCondition = mTimeConditionList.current()[0];
        
        writeTimeConditionAsXml(aXmlHandler, aTimeCondition);
    }
    
    // if the scenario is not handled by a upper scenario
    if (!mContainer.valid()) {
        
        // Close the event node
        xmlTextWriterEndElement((xmlTextWriterPtr)aXmlHandler->mWriter);
    }
	
	return kTTErrNone;
}

TTErr Scenario::ReadFromXml(const TTValue& inputValue, TTValue& outputValue)
{
    TTObject o = inputValue[0];
	TTXmlHandlerPtr aXmlHandler = (TTXmlHandlerPtr)o.instance();
    if (!aXmlHandler)
		return kTTErrGeneric;
#ifndef NO_EDITION_SOLVER
    SolverObjectMapIterator itSolver;
#endif
    TTValue                 v;
    
    // Filtering Score plugin
    // théo : this is ugly but it is due to the bad design of XmlHandler
    if (aXmlHandler->mXmlNodeName != TTSymbol("xmlHandlerReadingStarts") &&
        aXmlHandler->mXmlNodeName != TTSymbol("xmlHandlerReadingEnds") &&
        aXmlHandler->mXmlNodeName != TTSymbol("Scenario") &&
        aXmlHandler->mXmlNodeName != TTSymbol("Automation") &&
        aXmlHandler->mXmlNodeName != TTSymbol("indexedCurves") &&
        aXmlHandler->mXmlNodeName != TTSymbol("curve") &&
        aXmlHandler->mXmlNodeName != TTSymbol("Interval") &&
        aXmlHandler->mXmlNodeName != TTSymbol("event") &&
        aXmlHandler->mXmlNodeName != TTSymbol("command") &&
        aXmlHandler->mXmlNodeName != TTSymbol("startEvent") &&
        aXmlHandler->mXmlNodeName != TTSymbol("endEvent") &&
        aXmlHandler->mXmlNodeName != TTSymbol("condition")&&
        aXmlHandler->mXmlNodeName != TTSymbol("case")) {
        return kTTErrNone;
    }
    
    // DEBUG
    //TTLogMessage("%s reading %s\n", mName.c_str(), aXmlHandler->mXmlNodeName.c_str());
    
    // When reading a sub scenario
    if (mCurrentScenario.valid()) {
        
        // Scenario node :
        if (aXmlHandler->mXmlNodeName == TTSymbol("Scenario")) {
            
            TTSymbol subName;
            
            // get sub scenario name
            if (!aXmlHandler->getXmlAttribute(kTTSym_name, v, YES)) {
                
                if (v.size() == 1) {
                    
                    if (v[0].type() == kTypeSymbol) {
                        
                        subName = v[0];
                    }
                }
            }
            
            if (subName == ScenarioPtr(mCurrentScenario.instance())->mName) {
                
                // if this is the end of a scenario node : forget the sub scenario
                if (!aXmlHandler->mXmlNodeStart) {
                    
                    // DEBUG
                    //TTLogMessage("%s forgets %s sub scenario (end node)\n", mName.c_str(), ScenarioPtr(mCurrentScenario.instance())->mName.c_str());
                    
                    mCurrentScenario = TTObject();
                    mCurrentTimeProcess = TTObject();
                    return kTTErrNone;
                }
                
                // if this is an empty scenario node : read the node and then forget the sub scenario
                if (aXmlHandler->mXmlNodeIsEmpty) {
                    
                    mCurrentScenario.send("ReadFromXml", inputValue, outputValue);
                    
                    // DEBUG
                    //TTLogMessage("%s forgets %s sub scenario (empty node)\n", mName.c_str(), ScenarioPtr(mCurrentScenario.instance())->mName.c_str());
                    
                    mCurrentScenario = TTObject();
                    mCurrentTimeProcess = TTObject();
                    return kTTErrNone;
                }
            }
        }

        // any other case
        return mCurrentScenario.send("ReadFromXml", inputValue, outputValue);
    }
	
	// Switch on the name of the XML node
	
    // Starts scenario reading
    if (aXmlHandler->mXmlNodeName == kTTSym_xmlHandlerReadingStarts) {
        
        mLoading = YES;
        mAttributeLoaded = NO;
        
        mCurrentTimeEvent = TTObject();

        mCurrentTimeProcess = TTObject();
        mCurrentTimeCondition = TTObject();
        
        // clear all data structures
        mTimeEventList.clear();
        mTimeProcessList.clear();
#ifndef NO_EDITION_SOLVER
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
#endif
#ifndef NO_EXECUTION_GRAPH
        clearGraph();
#endif
        
        return kTTErrNone;
    }
    
    // Ends scenario reading
    if (aXmlHandler->mXmlNodeName == kTTSym_xmlHandlerReadingEnds) {
        
        mLoading = NO;
        mCompiled = NO;
        
        return kTTErrNone;
    }
    
    // Scenario node : read attribute only for upper scenario
    if (aXmlHandler->mXmlNodeName == TTSymbol("Scenario") && !mAttributeLoaded) {
        
        // Get the scenario name
        if (!aXmlHandler->getXmlAttribute(kTTSym_name, v, YES)) {
            
            if (v.size() == 1) {
                
                if (v[0].type() == kTypeSymbol) {
                    
                    mName = v[0];
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
        
        mAttributeLoaded = YES;
        return kTTErrNone;
    }
    
    // Start Event node (root Scenario only)
    if (aXmlHandler->mXmlNodeName == TTSymbol("startEvent")) {
        
        if (aXmlHandler->mXmlNodeStart) {
            
            // Get the date
            if (!aXmlHandler->getXmlAttribute(kTTSym_date, v, NO))
                getStartEvent().set(kTTSym_date, v);
            
            // Get the name
            if (!aXmlHandler->getXmlAttribute(kTTSym_name, v, YES))
                getStartEvent().set(kTTSym_name, v);
            
            if (!aXmlHandler->mXmlNodeIsEmpty)
                mCurrentTimeEvent = getStartEvent();
            
        }
        else
            mCurrentTimeEvent = NULL;
    }
    
    // End Event node (root Scenario only)
    if (aXmlHandler->mXmlNodeName == TTSymbol("endEvent")) {
        
        if (aXmlHandler->mXmlNodeStart) {
            
            // Get the date
            if (!aXmlHandler->getXmlAttribute(kTTSym_date, v, NO))
                getEndEvent().set(kTTSym_date, v);
            
            // Get the name
            if (!aXmlHandler->getXmlAttribute(kTTSym_name, v, YES))
                getEndEvent().set(kTTSym_name, v);
            
            if (!aXmlHandler->mXmlNodeIsEmpty)
                mCurrentTimeEvent = getEndEvent();
            
        }
        else
            mCurrentTimeEvent = NULL;
    }
    
    // Event node
    if (aXmlHandler->mXmlNodeName == kTTSym_event) {
        
        if (aXmlHandler->mXmlNodeStart) {
            
            readTimeEventFromXml(aXmlHandler, mCurrentTimeEvent);
            
            if (aXmlHandler->mXmlNodeIsEmpty)
                mCurrentTimeEvent = TTObject();
        }
        else
            
            mCurrentTimeEvent = TTObject();
        
        return kTTErrNone;
    }
    
    // If there is a current time event
    if (mCurrentTimeEvent.valid()) {
        
        // Pass the xml handler to the current condition to fill his data structure
        aXmlHandler->setAttributeValue(kTTSym_object, mCurrentTimeEvent);
        return aXmlHandler->sendMessage(kTTSym_Read);
    }
    
    // Condition node
    if (aXmlHandler->mXmlNodeName == kTTSym_condition) {
        
        if (aXmlHandler->mXmlNodeStart) {
            
            readTimeConditionFromXml(aXmlHandler, mCurrentTimeCondition);
        
            if (aXmlHandler->mXmlNodeIsEmpty)
                mCurrentTimeCondition = TTObject();
        }
        else
            
            mCurrentTimeCondition = TTObject();
        
        return kTTErrNone;
    }
    
    // If there is a current time condition
    if (mCurrentTimeCondition.valid()) {
        
        // Pass the xml handler to the current condition to fill his data structure
        aXmlHandler->setAttributeValue(kTTSym_object, mCurrentTimeCondition);
        return aXmlHandler->sendMessage(kTTSym_Read);
    }
    
    // Process node : the name of the node is the name of the process type
    if (!mCurrentTimeProcess.valid()) {
        
        if (aXmlHandler->mXmlNodeStart) {

            readTimeProcessFromXml(aXmlHandler, mCurrentTimeProcess);
            
            if (aXmlHandler->mXmlNodeIsEmpty)
                mCurrentTimeProcess = TTObject();
        }
    }
    else if (mCurrentTimeProcess.name() == aXmlHandler->mXmlNodeName) {
        
        if (!aXmlHandler->mXmlNodeStart)
            mCurrentTimeProcess = TTObject();
    }
    
    // If there is a current time process
    if (mCurrentTimeProcess.valid()) {
        
        // if the current time process is a sub scenario : don't forget it
        if (mCurrentTimeProcess.name() == TTSymbol("Scenario") &&
            !aXmlHandler->mXmlNodeIsEmpty) {
            
            mCurrentScenario = mCurrentTimeProcess;
            
            // DEBUG
            //TTLogMessage("%s set %s as sub scenario \n", mName.c_str(), ScenarioPtr(mCurrentScenario.instance())->mName.c_str());
        }
        
        // Pass the xml handler to the current process to fill his data structure
        aXmlHandler->setAttributeValue(kTTSym_object, mCurrentTimeProcess);
        return aXmlHandler->sendMessage(kTTSym_Read);
    }

    return kTTErrNone;
}

TTErr Scenario::TimeEventCreate(const TTValue& inputValue, TTValue& outputValue)
{
    TTObject    aTimeEvent, thisObject(this);
    TTValue     args, aCacheElement, scenarioDuration;
    
    if (inputValue.size() == 1) {
        
        if (inputValue[0].type() == kTypeUInt32) {
            
            // an event cannot be created beyond the duration of its container
            thisObject.get(kTTSym_duration, scenarioDuration);
            
            if (TTUInt32(inputValue[0]) > TTUInt32(scenarioDuration[0])) {
                
                TTLogError("Scenario::TimeEventCreate : event created beyond the duration of its container\n");
                return kTTErrGeneric;
            }
            
            // prepare argument (date, container)
            args = TTValue(inputValue[0], thisObject);
            
            aTimeEvent = TTObject(kTTSym_TimeEvent, args);
            
            // create all observers
            makeTimeEventCacheElement(aTimeEvent, aCacheElement);
            
            // store time event object and observers
            mTimeEventList.append(aCacheElement);
            mTimeEventList.sort(&TTTimeEventCompareDate);
#ifndef NO_EDITION_SOLVER
            // add variable to the solver
            SolverVariablePtr variable = new SolverVariable(mEditionSolver, aTimeEvent, TTUInt32(scenarioDuration[0]));
            
            // store the variable relative to the time event
            mVariablesMap.emplace(aTimeEvent.instance(), variable);
#endif
            // return the time event
            outputValue = aTimeEvent;
            
            // needs to be compiled again
            mCompiled = NO;
            
            return kTTErrNone;
        }
    }
    
    return kTTErrGeneric;
}

TTErr Scenario::TimeEventRelease(const TTValue& inputValue, TTValue& outputValue)
{
    TTObject                aTimeEvent;
    TTValue                 v, aCacheElement;
#ifndef NO_EDITION_SOLVER
    TTBoolean               found;
    SolverVariablePtr       variable;
    SolverObjectMapIterator it;
#endif
    
    if (inputValue.size() == 1) {
        
        if (inputValue[0].type() == kTypeObject) {
            
            aTimeEvent = inputValue[0];
            
            // try to find the time event
            mTimeEventList.find(&TTTimeContainerFindTimeEvent, (TTPtr)aTimeEvent.instance(), aCacheElement);
            
            // couldn't find the same time event in the scenario
            if (aCacheElement.size() == 0)
                return kTTErrValueNotFound;
            
            else {
                
                // if the time event is used by a time process it can't be released
                mTimeProcessList.find(&TTTimeContainerFindTimeProcessWithTimeEvent, (TTPtr)aTimeEvent.instance(), v);
                
                if (v.size() == 0) {
                    
                    // remove time event object and observers
                    mTimeEventList.remove(aCacheElement);
                    
                    // delete all observers
                    deleteTimeEventCacheElement(aCacheElement);
#ifndef NO_EDITION_SOLVER
                    // retreive solver variable relative to each event
                    it = mVariablesMap.find(aTimeEvent.instance());
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
                        mVariablesMap.erase(aTimeEvent.instance());
                        delete variable;
                    }
#endif
                    // needs to be compiled again
                    mCompiled = NO;
                    
                    return kTTErrNone;
                }
            }
        }
    }
    
    return kTTErrGeneric;
}

TTErr Scenario::TimeEventMove(const TTValue& inputValue, TTValue& outputValue)
{
    TTObject                aTimeEvent, thisObject(this);
#ifndef NO_EDITION_SOLVER
    SolverVariablePtr       variable;
    SolverObjectMapIterator it;
    SolverError             sErr;
#endif
    TTValue                 scenarioDuration;
    
    // can't move an event during a load
    if (mLoading)
        return kTTErrGeneric;
    
    if (inputValue.size() == 2) {
        
        if (inputValue[0].type() == kTypeObject && inputValue[1].type() == kTypeUInt32 ) {
            
            aTimeEvent = inputValue[0];
            
            // an event cannot be moved beyond the duration of its container
            thisObject.get(kTTSym_duration, scenarioDuration);
            
            if (TTUInt32(inputValue[1]) > TTUInt32(scenarioDuration[0])) {
                
                TTLogError("Scenario::TimeEventMove : event moved beyond the duration of its container\n");
                return kTTErrGeneric;
            }
#ifndef NO_EDITION_SOLVER
            // retreive solver variable relative to the time event
            it = mVariablesMap.find(aTimeEvent.instance());
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
                
                // needs to be compiled again
                mCompiled = NO;
                
                return kTTErrNone;
            }
#endif
        }
    }
    
    return kTTErrGeneric;
}

TTErr Scenario::TimeEventCondition(const TTValue& inputValue, TTValue& outputValue)
{
    TTObject    aTimeEvent;
    TTObject    aTimeCondition;
    TTObject    aTimeProcess;
    TTValue     v, aCacheElement;
#ifndef NO_EDITION_SOLVER
    SolverObjectMapIterator it;
#endif
    if (inputValue.size() == 2) {
        
        if (inputValue[0].type() == kTypeObject && inputValue[1].type() == kTypeObject) {
            
            aTimeEvent = inputValue[0];
            aTimeCondition = inputValue[1];
            
            // try to find the time event
            mTimeEventList.find(&TTTimeContainerFindTimeEvent, (TTPtr)aTimeEvent.instance(), aCacheElement);
            
            // couldn't find the former time event in the scenario
            if (aCacheElement.size() == 0)
                return kTTErrValueNotFound;
            
            // try to find the time condition
            mTimeConditionList.find(&TTTimeContainerFindTimeCondition, (TTPtr)aTimeCondition.instance(), aCacheElement);
            
            // couldn't find the former time condition in the scenario
            if (aCacheElement.size() == 0)
                return kTTErrValueNotFound;
            
            // for each time process
            for (mTimeProcessList.begin(); mTimeProcessList.end(); mTimeProcessList.next()) {
                
                aTimeProcess = mTimeProcessList.current()[0];
                
                // if the time event is the time process end event
                if (aTimeEvent == getTimeProcessEndEvent(aTimeProcess)) {
                    
                    // a time process with a conditioned end event cannot be rigid
                    v = TTBoolean(!aTimeCondition.valid());
                    aTimeProcess.set(kTTSym_rigid, v);
                }
            }
            
            // théo : maybe there will be other stuff to do considering there is a condition with several case now ?
            
            // needs to be compiled again
            mCompiled = NO;
            
            return kTTErrNone;
        }
    }
    
    return kTTErrGeneric;
}

TTErr Scenario::TimeEventTrigger(const TTValue& inputValue, TTValue& outputValue)
{
    TTObject aTimeEvent;
    
    if (inputValue.size() == 1) {
        
        if (inputValue[0].type() == kTypeObject) {
            
            aTimeEvent = inputValue[0];
            
#ifndef NO_EXECUTION_GRAPH
            if (mExecutionGraph) {
                
                // if the excecution graph is running
                if (mExecutionGraph->getUpdateFactor() != 0) {
                    
                    // append the event to the event queue to process its triggering
                    TTLogMessage("Scenario::TimeEventTrigger : %p\n", TTPtr(aTimeEvent.instance()));
                    mExecutionGraph->putAnEvent(TTPtr(aTimeEvent.instance()));
                    
                    return kTTErrNone;
                }
            }
#else
            return kTTErrNone;
#endif
            
        }
    }
    
    return kTTErrGeneric;
}

TTErr Scenario::TimeEventDispose(const TTValue &inputValue, TTValue &outputValue)
{
    TTObject aTimeEvent;

    if (inputValue.size() == 1) {

        if (inputValue[0].type() == kTypeObject) {

            aTimeEvent = inputValue[0];

#ifndef NO_EXECUTION_GRAPH
            if (mExecutionGraph) {

                // if the execution graph is running
                if (mExecutionGraph->getUpdateFactor() != 0) {

                    // put the associated transition in the list of transitions to deactivate
                    TTLogMessage("Scenario::TimeEventDispose : %p\n", TTPtr(aTimeEvent.instance()));
                    mExecutionGraph->deactivateTransition(TransitionPtr(mTransitionsMap[aTimeEvent.instance()]));

                    return kTTErrNone;
                }
            }
#else
            return kTTErrNone;
#endif
            
        }
    }

    return kTTErrGeneric;
}

TTErr Scenario::TimeEventReplace(const TTValue& inputValue, TTValue& outputValue)
{
    TTObject    aFormerTimeEvent, aNewTimeEvent;
    TTObject    aTimeProcess;
    TTValue     v, aCacheElement;
#ifndef NO_EDITION_SOLVER
    SolverObjectMapIterator it;
#endif
    if (inputValue.size() == 2) {
        
        if (inputValue[0].type() == kTypeObject && inputValue[1].type() == kTypeObject) {
            
            aFormerTimeEvent = inputValue[0];
            aNewTimeEvent = inputValue[1];
            
            // try to find the former time event
            mTimeEventList.find(&TTTimeContainerFindTimeEvent, (TTPtr)aFormerTimeEvent.instance(), aCacheElement);
            
            // couldn't find the former time event in the scenario
            if (aCacheElement.size() == 0)
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
                mTimeEventList.sort(&TTTimeEventCompareDate);
            }
            
            // replace the former time event in all time process which binds on it
            for (mTimeProcessList.begin(); mTimeProcessList.end(); mTimeProcessList.next()) {
                
                aTimeProcess = mTimeProcessList.current()[0];
                
                if (getTimeProcessStartEvent(aTimeProcess) == aFormerTimeEvent) {
                    
                    setTimeProcessStartEvent(aTimeProcess, aNewTimeEvent);
                    continue;
                }
                
                if (getTimeProcessEndEvent(aTimeProcess) == aFormerTimeEvent) {
                    
                    setTimeProcessEndEvent(aTimeProcess, aNewTimeEvent);
                    
                    // a time process with a conditioned end event cannot be rigid
                    v = TTBoolean(!getTimeEventCondition(aNewTimeEvent).valid());
                    aTimeProcess.set(kTTSym_rigid, v);
                }
            }
#ifndef NO_EDITION_SOLVER
            // retreive solver variable relative to the time event
            it = mVariablesMap.find(aFormerTimeEvent.instance());
            SolverVariablePtr variable = SolverVariablePtr(it->second);
            
            // replace the time event
            // note : only in the variable as all other solver elements binds on variables
            variable->event = aNewTimeEvent;
            
            // update the variable (this will copy the date into the new time event)
            variable->update();
            
            // update the variables map
            mVariablesMap.erase(aFormerTimeEvent.instance());
            mVariablesMap.emplace(aNewTimeEvent.instance(), variable);
#endif
            // needs to be compiled again
            mCompiled = NO;
            
            return kTTErrNone;
        }
    }
    
    return kTTErrGeneric;
}

TTErr Scenario::TimeProcessCreate(const TTValue& inputValue, TTValue& outputValue)
{
    TTObject    startEvent, endEvent;
    TTObject    aTimeProcess;
    TTValue     args, aCacheElement;
    TTValue     duration, scenarioDuration;
#ifndef NO_EDITION_SOLVER
    SolverVariablePtr       startVariable, endVariable;
    SolverObjectMapIterator it;
#endif
    if (inputValue.size() == 3) {
        
        if (inputValue[0].type() == kTypeSymbol && inputValue[1].type() == kTypeObject && inputValue[2].type() == kTypeObject) {
            
            // prepare argument (container)
            args = TTObject(this);
            
            // create a time process of the given type
            aTimeProcess = TTObject(inputValue[0], args);
            if (!aTimeProcess.valid())
                return kTTErrGeneric;
            
            // check start and end events are differents
            startEvent = inputValue[1];
            endEvent = inputValue[2];
            
            if (startEvent == endEvent)
                return kTTErrGeneric;
            
            // set the start and end events
            setTimeProcessStartEvent(aTimeProcess, startEvent);
            setTimeProcessEndEvent(aTimeProcess, endEvent);
            
            // check time process duration
            if (!aTimeProcess.get(kTTSym_duration, duration)) {
                
                // create all observers
                makeTimeProcessCacheElement(aTimeProcess, aCacheElement);
                
                // store time process object and observers
                mTimeProcessList.append(aCacheElement);
#ifndef NO_EDITION_SOLVER
                // get scenario duration
                this->getAttributeValue(kTTSym_duration, scenarioDuration);

                // retreive solver variable relative to each event
                it = mVariablesMap.find(getTimeProcessStartEvent(aTimeProcess).instance());
                startVariable = SolverVariablePtr(it->second);
                
                it = mVariablesMap.find(getTimeProcessEndEvent(aTimeProcess).instance());
                endVariable = SolverVariablePtr(it->second);
                
                // update the Solver depending on the type of the time process
                if (aTimeProcess.name() == TTSymbol("Interval")) {
                    
                    // add a relation between the 2 variables to the solver
                    SolverRelationPtr relation = new SolverRelation(mEditionSolver, startVariable, endVariable, getTimeProcessDurationMin(aTimeProcess), getTimeProcessDurationMax(aTimeProcess));
                    
                    // store the relation relative to this time process
                    mRelationsMap.emplace(aTimeProcess.instance(), relation);

                }
                else {
                    
                    // limit the start variable to the process duration
                    // this avoid time crushing when a time process moves while it is connected to other process
                    startVariable->limit(duration, duration);
                    
                    // add a constraint between the 2 variables to the solver
                    SolverConstraintPtr constraint = new SolverConstraint(mEditionSolver, startVariable, endVariable, getTimeProcessDurationMin(aTimeProcess), getTimeProcessDurationMax(aTimeProcess), TTUInt32(scenarioDuration[0]));
                    
                    // store the constraint relative to this time process
                    mConstraintsMap.emplace(aTimeProcess.instance(), constraint);
                    
                }
#endif
                // return the time process
                outputValue = aTimeProcess;
                
                // needs to be compiled again
                mCompiled = NO;
                
                return kTTErrNone;
            }
        }
    }
    
    return kTTErrGeneric;
}

TTErr Scenario::TimeProcessRelease(const TTValue& inputValue, TTValue& outputValue)
{
    TTObject    aTimeProcess;
    TTValue     aCacheElement;
#ifndef NO_EDITION_SOLVER
    SolverObjectMapIterator it;
#endif
    if (inputValue.size() == 1) {
        
        if (inputValue[0].type() == kTypeObject) {
            
            aTimeProcess = inputValue[0];
            
            // try to find the time process
            mTimeProcessList.find(&TTTimeContainerFindTimeProcess, (TTPtr)aTimeProcess.instance(), aCacheElement);
            
            // couldn't find the same time process in the scenario :
            if (aCacheElement.size() == 0)
                return kTTErrValueNotFound;
            
            else {
                
                // remove time process object and observers
                mTimeProcessList.remove(aCacheElement);
                
                // delete all observers
                deleteTimeProcessCacheElement(aCacheElement);
#ifndef NO_EDITION_SOLVER
                // update the Solver depending on the type of the time process
                if (aTimeProcess.name() == TTSymbol("Interval")) {
                    
                    // retreive solver relation relative to the time process
                    it = mRelationsMap.find(aTimeProcess.instance());
                    SolverRelationPtr relation = SolverRelationPtr(it->second);
                    
                    mRelationsMap.erase(aTimeProcess.instance());
                    delete relation;
                    
                } else {
                    
                    // retreive solver constraint relative to the time process
                    it = mConstraintsMap.find(aTimeProcess.instance());
                    SolverConstraintPtr constraint = SolverConstraintPtr(it->second);
                    
                    mConstraintsMap.erase(aTimeProcess.instance());
                    delete constraint;
                }
#endif
                // fill outputValue with start and event
                outputValue.resize(2);
                outputValue[0] = getTimeProcessStartEvent(aTimeProcess);
                outputValue[1] = getTimeProcessEndEvent(aTimeProcess);
                
                // needs to be compiled again
                mCompiled = NO;
                
                return kTTErrNone;
            }
        }
    }
    
    return kTTErrGeneric;
}

TTErr Scenario::TimeProcessMove(const TTValue& inputValue, TTValue& outputValue)
{
    TTObject    aTimeProcess, thisObject(this);
    TTValue     v;
    TTValue     duration, scenarioDuration;
    TTSymbol    timeProcessType;
#ifndef NO_EDITION_SOLVER
    SolverObjectMapIterator it;
    SolverError             sErr;
#endif
    // can't move a process during a load
    if (mLoading)
        return kTTErrGeneric;
    
    if (inputValue.size() == 3) {
        
        if (inputValue[0].type() == kTypeObject && inputValue[1].type() == kTypeUInt32 && inputValue[2].type() == kTypeUInt32) {
            
            aTimeProcess = inputValue[0];
            
            // get time process duration
            aTimeProcess.get(kTTSym_duration, duration);
            
            // get scenario duration
            thisObject.get(kTTSym_duration, scenarioDuration);
            
            // a process cannot be moved beyond the duration of its container
            if (TTUInt32(inputValue[1]) > TTUInt32(scenarioDuration[0]) || TTUInt32(inputValue[2]) > TTUInt32(scenarioDuration[0])) {
                
                TTLogError("Scenario::TimeProcessMove : process moved beyond the duration of its container\n");
                return kTTErrGeneric;
            }
#ifndef NO_EDITION_SOLVER
            // update the Solver depending on the type of the time process
            timeProcessType = aTimeProcess.name();
            
            if (timeProcessType == TTSymbol("Interval")) {
                
                // retreive solver relation relative to the time process
                it = mRelationsMap.find(aTimeProcess.instance());
                SolverRelationPtr relation = SolverRelationPtr(it->second);
                
                sErr = relation->move(inputValue[1], inputValue[2]);
                
            } else {
                
                // retreive solver constraint relative to the time process
                it = mConstraintsMap.find(aTimeProcess.instance());
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

                // needs to be compiled again
                mCompiled = NO;
                
                return kTTErrNone;
            }
#else
            return kTTErrNone;
#endif
        }
    }
    
    return kTTErrGeneric;
}

TTErr Scenario::TimeProcessLimit(const TTValue& inputValue, TTValue& outputValue)
{
    TTObject    aTimeProcess;
    TTValue     durationMin, durationMax;
    TTSymbol    timeProcessType;
#ifndef NO_EDITION_SOLVER
    SolverObjectMapIterator it;
    SolverError             sErr;
#endif
    if (inputValue.size() == 3) {
        
        if (inputValue[0].type() == kTypeObject && inputValue[1].type() == kTypeUInt32 && inputValue[2].type() == kTypeUInt32) {
            
            aTimeProcess =inputValue[0];
#ifndef NO_EDITION_SOLVER
            // update the Solver depending on the type of the time process
            timeProcessType = aTimeProcess.name();
            
            if (timeProcessType == TTSymbol("Interval")) {
                
                // retreive solver relation relative to the time process
                it = mRelationsMap.find(aTimeProcess.instance());
                SolverRelationPtr relation = SolverRelationPtr(it->second);
                
                sErr = relation->limit(inputValue[1], inputValue[2]);
                
            } else {
                
                // retreive solver constraint relative to the time process
                it = mConstraintsMap.find(aTimeProcess.instance());
                SolverConstraintPtr constraint = SolverConstraintPtr(it->second);
                
                sErr = constraint->limit(inputValue[1], inputValue[2]);
            }
            
            if (!sErr && !mLoading) {
                
                // update each solver variable value
                for (it = mVariablesMap.begin() ; it != mVariablesMap.end() ; it++)
                    
                    SolverVariablePtr(it->second)->update();
                
                // needs to be compiled again
                mCompiled = NO;
                
                return kTTErrNone;
            }
#else
            return kTTErrNone;
#endif
        }
    }
    
    return kTTErrGeneric;
}

TTErr Scenario::TimeConditionCreate(const TTValue& inputValue, TTValue& outputValue)
{
    TTObject    aTimeCondition;
    TTValue     args, aCacheElement;
    
    // prepare argument (container)
    args = TTObject(this);
    
    // create the time condition
    aTimeCondition = TTObject("TimeCondition", args);
    if (!aTimeCondition.valid())
        return kTTErrGeneric;
    
    // create all observers
    makeTimeConditionCacheElement(aTimeCondition, aCacheElement);
    
    // store time condition object and observers
    mTimeConditionList.append(aCacheElement);
    
    // add a first case if
    
    // TODO : how conditions are constrained by the Solver ?
    
    // return the time condition
    outputValue = aTimeCondition;
    
    // needs to be compiled again
    mCompiled = NO;
    
    return kTTErrNone;
}

TTErr Scenario::TimeConditionRelease(const TTValue& inputValue, TTValue& outputValue)
{
    TTObject    aTimeCondition;
    TTValue     aCacheElement;
    
    if (inputValue.size() == 1) {
        
        if (inputValue[0].type() == kTypeObject) {
            
            aTimeCondition = inputValue[0];
            
            // try to find the time condition
            mTimeConditionList.find(&TTTimeContainerFindTimeCondition, (TTPtr)aTimeCondition.instance(), aCacheElement);
            
            // couldn't find the same time condition in the scenario
            if (aCacheElement.size() == 0)
                return kTTErrValueNotFound;
            
            else {
                
                // remove time condition object and observers
                mTimeConditionList.remove(aCacheElement);
                
                // delete all observers
                deleteTimeConditionCacheElement(aCacheElement);
                
                // needs to be compiled again
                mCompiled = NO;
                
                return kTTErrNone;
            }
        }
    }
    
    return kTTErrGeneric;
}

void Scenario::makeTimeProcessCacheElement(TTObject& aTimeProcess, TTValue& newCacheElement)
{
    newCacheElement.clear();
    
	// 0 : cache time process object
	newCacheElement.append(aTimeProcess);
}

void Scenario::deleteTimeProcessCacheElement(const TTValue& oldCacheElement)
{
    ;
}

void Scenario::makeTimeEventCacheElement(TTObject& aTimeEvent, TTValue& newCacheElement)
{
    newCacheElement.clear();
    
	// 0 : cache time event object
	newCacheElement.append(aTimeEvent);
}

void Scenario::deleteTimeEventCacheElement(const TTValue& oldCacheElement)
{
    ;
}

void Scenario::makeTimeConditionCacheElement(TTObject& aTimeCondition, TTValue& newCacheElement)
{
    newCacheElement.clear();
    
	// 0 : cache time condition object
	newCacheElement.append(aTimeCondition);
}

void Scenario::deleteTimeConditionCacheElement(const TTValue& oldCacheElement)
{
    ;
}

#if 0
#pragma mark -
#pragma mark Some Methods
#endif

#ifndef NO_EXECUTION_GRAPH
void ScenarioGraphTimeEventCallBack(TTPtr arg, TTBoolean active)
{
    // cf ECOMachine : crossAControlPointCallBack function
    
    TTObject aTimeEvent = (TTTimeEventPtr) arg;
    
    if (active)
        aTimeEvent.send(kTTSym_Happen);
    
    // this propagates the disposition to the next events that are connected to a first disposed event
    else
        aTimeEvent.send(kTTSym_Dispose);
}


void ScenarioGraphIsEventReadyCallBack(TTPtr arg, TTBoolean isReady)
{
	TTTimeEventPtr aTimeEvent = (TTTimeEventPtr) arg;
    
    if (isReady)
        aTimeEvent->setAttributeValue(kTTSym_status, kTTSym_eventPending);
}
#endif
