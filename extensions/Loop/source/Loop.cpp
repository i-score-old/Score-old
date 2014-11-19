/** @file
 *
 * @ingroup scoreExtension
 *
 * @brief Loop class is a time process class to iterate other time processes
 *
 * @details The Loop class allows to ... @n@n
 *
 * @see TimePluginLib, TTTimeProcess, TimeContainer
 *
 * @authors Théo de la Hogue & Clément Bossut
 *
 * @copyright Copyright © 2013, Théo de la Hogue & Clément Bossut @n
 * This code is licensed under the terms of the "CeCILL-C" @n
 * http://www.cecill.info
 */

#include "Loop.h"

#define thisTTClass                 Loop
#define thisTTClassName             "Loop"
#define thisTTClassTags             "time, process, loop"

extern "C" TT_EXTENSION_EXPORT TTErr TTLoadJamomaExtension_Loop(void)
{
	TTFoundationInit();
	Loop::registerClass();
	return kTTErrNone;
}

#if 0
#pragma mark -
#pragma mark Constructor/Destructor
#endif

TIME_CONTAINER_PLUGIN_CONSTRUCTOR,
mNamespace(NULL)
{
    TIME_PLUGIN_INITIALIZE
    
	TT_ASSERT("Correct number of args to create Loop", arguments.size() == 0);
    
    addMessageWithArguments(PatternAttach);
    addMessageWithArguments(PatternDetach);
    
    // needed to be notified by events
    addMessageWithArguments(EventDateChanged);
    addMessageProperty(EventDateChanged, hidden, YES);
    
    TTObject    thisObject(this);
    TTValue     args;
    
    // create pattern start event
    args = TTValue(0, thisObject);
    mPatternStartEvent = TTObject(kTTSym_TimeEvent, args);
    mPatternStartEvent.set("name", TTSymbol("patternStart"));
    mPatternStartEvent.set("container", thisObject);
    
    // create pattern end event
    args = TTValue(1000, thisObject);
    mPatternEndEvent = TTObject(kTTSym_TimeEvent, args);
    mPatternEndEvent.set("name", TTSymbol("patternEnd"));
    mPatternEndEvent.set("container", thisObject);
    
    // create pattern condition
    args = TTValue(thisObject);
    mPatternCondition = TTObject("TimeCondition", args);
    mPatternCondition.set("container", thisObject);
}

Loop::~Loop()
{
    if (mNamespace) {
        delete mNamespace;
        mNamespace = NULL;
    }
}

#if 0
#pragma mark -
#pragma mark TimeContainerPlugin Methods
#endif

TTErr Loop::getParameterNames(TTValue& value)
{
    value.clear();
	//value.append(TTSymbol("aParameterName"));
	
	return kTTErrNone;
}

#if 0
#pragma mark -
#pragma mark TTTimeContainer Methods
#endif

TTErr Loop::getTimeProcesses(TTValue& value)
{
    value.clear();
    
    if (mPatternProcesses.isEmpty())
        return kTTErrGeneric;
    
    for (mPatternProcesses.begin(); mPatternProcesses.end(); mPatternProcesses.next())
        value.append(mPatternProcesses.current()[0]);
    
    return kTTErrNone;
}

TTErr Loop::getTimeEvents(TTValue& value)
{
    value.clear();
    value.append(mPatternStartEvent);
    value.append(mPatternEndEvent);
    
    return kTTErrNone;
}

TTErr Loop::getTimeConditions(TTValue& value)
{
    value.clear();
    value.append(mPatternCondition);
    
    return kTTErrNone;
}

#if 0
#pragma mark -
#pragma mark TTTimeProcess Methods
#endif

TTErr Loop::Compile()
{
    mCompiled = YES;
    
    return kTTErrNone;
}

TTErr Loop::ProcessStart()
{
    // start the loop pattern
    mPatternStartEvent.send(kTTSym_Happen);
    
    return kTTErrNone;
}

TTErr Loop::ProcessEnd()
{
    // needs to be compiled again
    mCompiled = NO;
    
    // stop the loop pattern
    mPatternEndEvent.send(kTTSym_Happen);
   
    return kTTErrNone;
}

TTErr Loop::Process(const TTValue& inputValue, TTValue& outputValue)
{
    TTFloat64 position, date;
    
    if (inputValue.size() == 2)
    {
        if (inputValue[0].type() == kTypeFloat64 && inputValue[1].type() == kTypeFloat64)
        {
            position = inputValue[0];
            date = inputValue[1];
            
            // if the end event pattern happened
            TTSymbol status;
            mPatternEndEvent.get("status", status);
            if (status == kTTSym_eventHappened)
            {
                // restart the loop pattern
                mPatternStartEvent.set("status", kTTSym_eventWaiting);
                mPatternStartEvent.send(kTTSym_Happen);
            }
        }
    }
    
    return kTTErrGeneric;
}

TTErr Loop::ProcessPaused(const TTValue& inputValue, TTValue& outputValue)
{
    TTObject    aTimeProcess;
    TTBoolean   paused;
    
    if (inputValue.size() == 1) {
        
        if (inputValue[0].type() == kTypeBoolean) {
            
            paused = inputValue[0];
            
            
        }
        
        return kTTErrNone;
    }
    
    return kTTErrGeneric;
}

TTErr Loop::Goto(const TTValue& inputValue, TTValue& outputValue)
{
    TTValue         v;
    TTUInt32        duration, timeOffset;
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
            
            // needs to be compiled again
            mCompiled = NO;
            
            return kTTErrNone;
        }
    }
    
    return kTTErrGeneric;
}

TTErr Loop::WriteAsXml(const TTValue& inputValue, TTValue& outputValue)
{
    TTObject o = inputValue[0];
	TTXmlHandlerPtr aXmlHandler = (TTXmlHandlerPtr)o.instance();
    if (!aXmlHandler)
		return kTTErrGeneric;
	
	return kTTErrNone;
}

TTErr Loop::ReadFromXml(const TTValue& inputValue, TTValue& outputValue)
{
    TTObject o = inputValue[0];
	TTXmlHandlerPtr aXmlHandler = (TTXmlHandlerPtr)o.instance();
    if (!aXmlHandler)
		return kTTErrGeneric;

    return kTTErrNone;
}

#if 0
#pragma mark -
#pragma mark Notifications
#endif

TTErr Loop::EventDateChanged(const TTValue& inputValue, TTValue& outputValue)
{
    TT_ASSERT("Loop::EventDateChanged : inputValue is correct", inputValue.size() == 1 && inputValue[0].type() == kTypeObject);
    
    TTObject aTimeEvent = inputValue[0];
    
    if (aTimeEvent != this->getStartEvent() && aTimeEvent != this->getEndEvent())
    {
        TTLogError("Loop::EventDateChanged %s : wrong event\n", mName.c_str());
        return kTTErrGeneric;
    }
    
    // if needed, the compile method should be called again now
    mCompiled = NO;
    
    // get new duration to update the pattern end event date
    TTValue v;
    getAttributeValue(kTTSym_duration, v);
    mPatternEndEvent.set("date", v);
    
    return kTTErrNone;
}

TTErr Loop::EventConditionChanged(const TTValue& inputValue, TTValue& outputValue)
{
    TT_ASSERT("Loop::EventConditionChanged : inputValue is correct", inputValue.size() == 2 && inputValue[0].type() == kTypeObject && inputValue[1].type() == kTypeObject);
    
    TTObject    aTimeEvent = inputValue[0];
    TTObject    aTimeCondition = inputValue[1];
    
    // no rule
    
    return kTTErrNone;
}

#if 0
#pragma mark -
#pragma mark Specific Loop Methods
#endif

TTErr Loop::PatternAttach(const TTValue& inputValue, TTValue& outputValue)
{
    TT_ASSERT("Loop::PatternAttach : expects an object", inputValue.size() == 1 && inputValue[0].type() == kTypeObject);
    
    TTObject aTimeProcess = inputValue[0];
    
    // set loop as time process container
    TTObject thisObject(this);
    aTimeProcess.set("container", thisObject);
    
    // make pattern start and end events as time process start and end events
    setTimeProcessStartEvent(aTimeProcess, mPatternStartEvent);
    setTimeProcessEndEvent(aTimeProcess, mPatternEndEvent);
    
    // cache the time process
    mPatternProcesses.append(aTimeProcess);
    
    // get duration to update the pattern end event date
    TTValue v;
    getAttributeValue(kTTSym_duration, v);
    mPatternEndEvent.set("date", v);
    
    return kTTErrNone;
}

TTErr Loop::PatternDetach(const TTValue& inputValue, TTValue& outputValue)
{
    TT_ASSERT("Loop::PatternDetach : expects an object", inputValue.size() == 1 && inputValue[0].type() == kTypeObject);
    
    TTObject aTimeProcess = inputValue[0];
    
    // reset loop container as time process container
    aTimeProcess.set("container", mContainer);
    
    // make loop start and end events as time process start and end events
    setTimeProcessStartEvent(aTimeProcess, this->getStartEvent());
    setTimeProcessEndEvent(aTimeProcess, this->getEndEvent());
    
    // uncache the time process
    mPatternProcesses.remove(aTimeProcess);
    
    return kTTErrNone;
}