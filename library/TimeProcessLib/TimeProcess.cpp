/*
 * A Time Process interface
 * Copyright © 2013, Théo de la Hogue
 *
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#include "TimeProcess.h"

#define thisTTClass		TimeProcess

/****************************************************************************************************/

TimeProcess::TimeProcess(TTValue& arguments) :
TTObjectBase(arguments),
mScenario(NULL),
mActive(YES),
mStartEvent(NULL),
mEndEvent(NULL),
mScheduler(NULL)
{
    TTValue args;
    TTErr   err;
    
    TT_ASSERT("Correct number of args to create TimeProcess", arguments.size() == 0);
    
    addAttribute(Scenario, kTypeObject);
    addAttributeProperty(Scenario, hidden, YES);

    addAttributeWithSetter(Active, kTypeBoolean);
    
    addAttributeWithSetter(StartEvent, kTypeObject);
    addAttributeProperty(StartEvent, hidden, YES);
    
    addAttributeWithSetter(EndEvent, kTypeObject);
    addAttributeProperty(EndEvent, hidden, YES);
    
    addAttribute(Scheduler, kTypeObject);
    addAttributeProperty(Scheduler, readOnly, YES);
    addAttributeProperty(Scheduler, hidden, YES);
	
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
    
    // Creation of a static time event for the start
    args = TTValue((TTPtr)&TimeProcessStartEventCallback);
    args.append((TTPtr)this);      // we have to store this as a pointer for TimeEvent
    
    err = TTObjectBaseInstantiate(TTSymbol("StaticEvent"), TTObjectBaseHandle(&mStartEvent), args);
    
	if (err) {
        mStartEvent = NULL;
		logError("TimeProcess failed to load a static start event");
    }
    
    // Creation of a static time event for the end
    args = TTValue((TTPtr)&TimeProcessEndEventCallback);
    args.append((TTPtr)this);      // we have to store this as a pointer for TimeEvent
    
    err = TTObjectBaseInstantiate(TTSymbol("StaticEvent"), TTObjectBaseHandle(&mEndEvent), args);
    
	if (err) {
        mStartEvent = NULL;
		logError("TimeProcess failed to load a static end event");
    }
    
    // Creation of a scheduler based on the EcoMachine scheduler plugin
    // Prepare callback argument to be notified of :
    //      - the progression
    args = TTValue((TTPtr)&TimeProcessSchedulerCallback);
    args.append((TTPtr)this);   // we have to store this as a pointer for Scheduler
    
    err = TTObjectBaseInstantiate(TTSymbol("EcoMachine"), TTObjectBaseHandle(&mScheduler), args);
    
	if (err) {
        mScheduler = NULL;
		logError("TimeProcess failed to load the EcoMachine Scheduler");
    }
    
    // Cache some attributes for high speed notification feedbacks
    this->findAttribute(TTSymbol("active"), &activeAttribute);
}

TimeProcess::~TimeProcess()
{
    TTValue v;
    
    // if the time process is managed by a scenario
    if (mScenario) {
        
        v = TTValue((TTObjectBasePtr)this);
        
        // remove the time process from the scenario (even if it can be done by the creator but it is safe to remove our self)
        mScenario->sendMessage(TTSymbol("TimeProcessRemove"), v, kTTValNONE);
        
        // remove the start event
        mScenario->sendMessage(TTSymbol("TimeEventRemove"), mStartEvent, kTTValNONE);
        
        // remove the end event
        mScenario->sendMessage(TTSymbol("TimeEventRemove"), mEndEvent, kTTValNONE);
    }
    
    if (mStartEvent) {
        TTObjectBaseRelease(TTObjectBaseHandle(&mStartEvent));
        mStartEvent = NULL;
    }
    
    if (mEndEvent) {
        TTObjectBaseRelease(TTObjectBaseHandle(&mEndEvent));
        mEndEvent = NULL;
    }
    
    if (mScheduler) {
        TTObjectBaseRelease(TTObjectBaseHandle(&mScheduler));
        mScheduler = NULL;
    }
}

TTErr TimeProcess::getParameterNames(TTValue& value)
{
	TTValue		attributeNames;
	TTSymbol	attributeName;
	
	// filter all default attributes (Name, Version, Author, ...)
	this->getAttributeNames(attributeNames);
	
	value.clear();
	for (TTUInt8 i = 0; i < attributeNames.size(); i++) {
		attributeName = attributeNames[0];
		
		if (attributeName == TTSymbol("name")           ||
			attributeName == TTSymbol("version")        ||
			attributeName == TTSymbol("author")         ||
            attributeName == TTSymbol("active")         ||
            attributeName == TTSymbol("running")        ||
            attributeName == TTSymbol("progression")    ||
            attributeName == TTSymbol("start")          ||
            attributeName == TTSymbol("end"))
			continue;
		
		value.append(attributeName);
	}
	
	return kTTErrNone;
}

TTErr TimeProcess::setActive(const TTValue& value)
{
    // set the internal active value
    mActive = value[0];
        
    // notify each attribute observers
    activeAttribute->sendNotification(kTTSym_notify, mActive);             // we use kTTSym_notify because we know that observers are TTCallback
    
    return kTTErrNone;
}

TTErr TimeProcess::setStartEvent(const TTValue& value)
{
    if (value.size() == 1) {
        
        if (value[0].type() == kTypeObject) {
            
            // if the time process is managed by a scenario
            if (mScenario) {
                
                // remove the old start event
                mScenario->sendMessage(TTSymbol("TimeEventRemove"), mStartEvent, kTTValNONE);
                
                // delete the start end event
                TTObjectBaseRelease(TTObjectBaseHandle(&mStartEvent));
                
                mStartEvent = value[0];
                // TODO : lui donner TimeProcessStartEventCallback
                
                // add the new start event
                return mScenario->sendMessage(TTSymbol("TimeEventAdd"), value, kTTValNONE);
            }
        }
    }
    
    return kTTErrGeneric;
}

TTErr TimeProcess::setEndEvent(const TTValue& value)
{
    if (value.size() == 1) {
        
        if (value[0].type() == kTypeObject) {
            
            // if the time process is managed by a scenario
            if (mScenario) {
                
                // remove the old end event
                mScenario->sendMessage(TTSymbol("TimeEventRemove"), mEndEvent, kTTValNONE);
                
                // delete the old end event
                TTObjectBaseRelease(TTObjectBaseHandle(&mEndEvent));
                
                mEndEvent = value[0];
                // TODO : lui donner TimeProcessEndEventCallback
                
                // add the new start event
                return mScenario->sendMessage(TTSymbol("TimeEventAdd"), value, kTTValNONE);
            }
        }
    }
    
    return kTTErrGeneric;
}

#if 0
#pragma mark -
#pragma mark Some Methods
#endif

TTErr TimeProcessStartEventCallback(TTPtr object)
{
    TimeProcessPtr aTimeProcess = (TimeProcessPtr)object;
    TTUInt32       start, end, duration;
    TTValue        v;
    
    // if the time process active, launch the scheduler
    // note : the ProcessStart method is called inside TimeProcessSchedulerCallback
	if (aTimeProcess->mActive) {
        
        aTimeProcess->mStartEvent->getAttributeValue(TTSymbol("date"), v);
        start = v[0];
        
        aTimeProcess->mEndEvent->getAttributeValue(TTSymbol("date"), v);
        end = v[0];
        
        if (end > start) {
            
            duration = end - start;
            
            return aTimeProcess->mScheduler->sendMessage(TTSymbol("Go"), duration, kTTValNONE);
        }
    }
    
    return kTTErrGeneric;
}

TTErr TimeProcessEndEventCallback(TTPtr object)
{
    TimeProcessPtr aTimeProcess = (TimeProcessPtr)object;
    
    // if the time process active, stop the scheduler
    // note : the ProcessStart method is called inside TimeProcessSchedulerCallback
	if (aTimeProcess->mActive)
            return aTimeProcess->mScheduler->sendMessage(TTSymbol("Stop"));
    
    return kTTErrGeneric;
}

void TimeProcessSchedulerCallback(TTPtr object, TTFloat64 progression)
{
	TimeProcessPtr	aTimeProcess = (TimeProcessPtr)object;
    
    // Case 0 : recall the start state
    if (progression == 0.) {
        
        // close start event listening
        aTimeProcess->mStartEvent->setAttributeValue(kTTSym_active, NO);
        
        // use the specific start process method of the time process
        aTimeProcess->ProcessStart();
        
        return;
    }
    
    // Case 1 : recall the end state
    else if (progression == 1.) {
        
        // close end trigger listening
        aTimeProcess->mEndEvent->setAttributeValue(kTTSym_active, NO);
        
        // use the specific process end method of the time process
        aTimeProcess->ProcessEnd();
        
        return;
    }

    // Case ]0 :: 1[ : use the specific process method
    aTimeProcess->Process(progression, kTTValNONE);
}

/***************************************************************************
 
 TimeProcessLib
 
 ***************************************************************************/

void TimeProcessLib::getTimeProcessNames(TTValue& timeProcessNames)
{
	timeProcessNames.clear();
	timeProcessNames.append(TTSymbol("Automation"));
    timeProcessNames.append(TTSymbol("Scenario"));
    timeProcessNames.append(TTSymbol("Relation"));
}

