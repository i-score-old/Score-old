/*
 * A Time Process interface
 * Copyright © 2013, Théo de la Hogue, Clément Bossut
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
mStartEventCallback(NULL),
mEndEvent(NULL),
mEndEventCallback(NULL),
mScheduler(NULL)
{
    TT_ASSERT("Correct number of args to create TimeProcess", arguments.size() == 0);
    
    TTValue    args;
    TTErr      err;
    TTValuePtr startEventBaton, endEventBaton;
    
    addAttribute(Scenario, kTypeObject);
    addAttributeProperty(Scenario, hidden, YES);
    
    // the rigid state handles the DurationMin and DurationMax attribute
    registerAttribute(TTSymbol("rigid"), kTypeBoolean, NULL, (TTGetterMethod)& TimeProcess::getRigid, (TTSetterMethod)& TimeProcess::setRigid);
    
    addAttributeWithSetter(DurationMin, kTypeUInt32);
    addAttributeWithSetter(DurationMax, kTypeUInt32);
    
    addAttributeWithSetter(Active, kTypeBoolean);
    
    addAttributeWithSetter(StartEvent, kTypeObject);
    addAttributeProperty(StartEvent, hidden, YES);
    
    addAttributeWithGetter(IntermediateEvents, kTypeLocalValue);
    addAttributeProperty(IntermediateEvents, readOnly, YES);
    addAttributeProperty(IntermediateEvents, hidden, YES);
    
    addAttributeWithSetter(EndEvent, kTypeObject);
    addAttributeProperty(EndEvent, hidden, YES);
    
    addAttribute(Scheduler, kTypeObject);
    addAttributeProperty(Scheduler, readOnly, YES);
    addAttributeProperty(Scheduler, hidden, YES);
    
    // the attributes below are not related to any TimeProcess member
    // but we need to declare them as attribute to ease the use of the class
    registerAttribute(TTSymbol("startDate"), kTypeUInt32, NULL, (TTGetterMethod)& TimeProcess::getStartDate, (TTSetterMethod)& TimeProcess::setStartDate);
    registerAttribute(TTSymbol("endDate"), kTypeUInt32, NULL, (TTGetterMethod)& TimeProcess::getEndDate, (TTSetterMethod)& TimeProcess::setEndDate);
    registerAttribute(TTSymbol("duration"), kTypeUInt32, NULL, (TTGetterMethod)& TimeProcess::getDuration);
    
    addMessage(ProcessStart);
    addMessage(ProcessEnd);
    addMessageWithArguments(Process);
    
    addMessageWithArguments(CreateStartEvent);
    addMessage(ReleaseStartEvent);
    
    addMessageWithArguments(CreateEndEvent);
    addMessage(ReleaseEndEvent);
    
    addMessageWithArguments(Limit);
    
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
    
    // Create a start event callback to be notified and start the process execution
    mStartEventCallback = NULL;
    TTObjectBaseInstantiate(TTSymbol("callback"), &mStartEventCallback, kTTValNONE);
    
    startEventBaton = new TTValue(TTObjectBasePtr(this));
    
    mStartEventCallback->setAttributeValue(kTTSym_baton, TTPtr(startEventBaton));
    mStartEventCallback->setAttributeValue(kTTSym_function, TTPtr(&TimeProcessStartEventCallback));
    
    // Create a end event callback to be notified and end the process execution
    mEndEventCallback = NULL;
    TTObjectBaseInstantiate(TTSymbol("callback"), &mEndEventCallback, kTTValNONE);
    
    endEventBaton = new TTValue(TTObjectBasePtr(this));
    
    mEndEventCallback->setAttributeValue(kTTSym_baton, TTPtr(endEventBaton));
    mEndEventCallback->setAttributeValue(kTTSym_function, TTPtr(&TimeProcessEndEventCallback));
    
    // Creation of a scheduler based on the System scheduler plugin
    // Prepare callback argument to be notified of :
    //      - the progression
    args = TTValue((TTPtr)&TimeProcessSchedulerCallback);
    args.append((TTPtr)this);   // we have to store this as a pointer for Scheduler
    
    err = TTObjectBaseInstantiate(TTSymbol("System"), TTObjectBaseHandle(&mScheduler), args);
    
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
    }
    
    if (mStartEventCallback) {
        delete (TTValuePtr)TTCallbackPtr(mStartEventCallback)->getBaton();
        TTObjectBaseRelease(TTObjectBaseHandle(&mStartEventCallback));
        mStartEventCallback = NULL;
    }
    
    if (mEndEventCallback) {
        delete (TTValuePtr)TTCallbackPtr(mEndEventCallback)->getBaton();
        TTObjectBaseRelease(TTObjectBaseHandle(&mEndEventCallback));
        mEndEventCallback = NULL;
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

TTErr TimeProcess::getRigid(TTValue& value)
{
    value = mDurationMin && mDurationMin && mDurationMin == mDurationMax;
    
    return kTTErrNone;
}

TTErr TimeProcess::setRigid(const TTValue& value)
{
    TTValue v;
    
    if (value.size()) {
        
        if (value[0].type() == kTypeBoolean) {
            
            getDuration(v);
            
            if (TTBoolean(v[0]))
                v.append(TTUInt32(v[0]));   // Limit(duration, duration)
            else
                v.prepend(TTUInt32(0));     // Limit(0, duration)
            
            return Limit(v, kTTValNONE);
        }
    }
    
    return kTTErrGeneric;
}

TTErr TimeProcess::setDurationMin(const TTValue& value)
{
    TTValue v;
    
    if (value.size()) {
        
        if (value[0].type() == kTypeUInt32) {
            
            if (TTUInt32(value[0]) <= mDurationMax) {
                
                // set minimal duration
                mDurationMin = TTUInt32(value[0]);
            
                // if the time process is handled by a scenario
                if (mScenario) {
                
                    v = TTValue(TTObjectBasePtr(this));
                    v.append(mDurationMin);
                    v.append(mDurationMax);
                
                    return mScenario->sendMessage(TTSymbol("TimeProcessLimit"), v, kTTValNONE);
                }
                else
                    return kTTErrNone;
            }
        }
    }
    
    return kTTErrGeneric;
}

TTErr TimeProcess::setDurationMax(const TTValue& value)
{
    TTValue v;
    
    if (value.size()) {
        
        if (value[0].type() == kTypeUInt32) {
            
            if (TTUInt32(value[0]) >= mDurationMin) {
                
                // set maximal duration
                mDurationMax = TTUInt32(value[0]);
                
                // if the time process is handled by a scenario
                if (mScenario) {
                    
                    v = TTValue(TTObjectBasePtr(this));
                    v.append(mDurationMin);
                    v.append(mDurationMax);
                    
                    return mScenario->sendMessage(TTSymbol("TimeProcessLimit"), v, kTTValNONE);
                }
                else
                    return kTTErrNone;
            }
        }
    }
    
    return kTTErrGeneric;
}

TTErr TimeProcess::getStartDate(TTValue& value)
{
    if (mStartEvent)
        return mStartEvent->getAttributeValue(TTSymbol("date"), value);
    
    return kTTErrGeneric;
}

TTErr TimeProcess::setStartDate(const TTValue& value)
{
    TTValue v, endDate;
    
    if (mStartEvent) {
        
        if (value.size()) {
            
            if (value[0].type() == kTypeUInt32) {
                
                // if the time process is handled by a scenario
                if (mScenario) {
                    
                    v = TTValue(TTObjectBasePtr(this));
                    v.append(TTUInt32(value[0]));
                    
                    mEndEvent->getAttributeValue(TTSymbol("date"), endDate);
                    v.append(TTUInt32(endDate[0]));
                    
                    return mScenario->sendMessage(TTSymbol("TimeProcessMove"), v, kTTValNONE);
                }
                else
                    return mStartEvent->setAttributeValue(TTSymbol("date"), TTUInt32(value[0]));
                
            }
        }
    }
    
    return kTTErrGeneric;
}

TTErr TimeProcess::getEndDate(TTValue& value)
{
    if (mEndEvent)
        return mEndEvent->getAttributeValue(TTSymbol("date"), value);
    
    return kTTErrGeneric;
}

TTErr TimeProcess::setEndDate(const TTValue& value)
{
    TTValue v, startDate;
    
    if (mEndEvent) {
        
        if (value.size()) {
            
            if (value[0].type() == kTypeUInt32) {
                
                // if the time process is handled by a scenario
                if (mScenario) {
                    
                    v = TTValue(TTObjectBasePtr(this));
                    
                    mStartEvent->getAttributeValue(TTSymbol("date"), startDate);
                    v.append(TTUInt32(startDate[0]));
                    
                    v.append(TTUInt32(value[0]));
                    
                    return mScenario->sendMessage(TTSymbol("TimeProcessMove"), v, kTTValNONE);
                }
                else
                    return mEndEvent->setAttributeValue(TTSymbol("date"), TTUInt32(value[0]));
                
            }
        }
    }
    
    return kTTErrGeneric;
}

TTErr TimeProcess::getDuration(TTValue& value)
{
    TTValue     start, end;
    TTUInt32    duration;
    
    if (mStartEvent && mEndEvent) {
        
        mStartEvent->getAttributeValue(TTSymbol("date"), start);
        mEndEvent->getAttributeValue(TTSymbol("date"), end);
        
        duration = TTUInt32(end[0]) - TTUInt32(start[0]);
        value = TTValue(duration);
        
        return kTTErrNone;
    }
    
    return kTTErrGeneric;
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
            
            // unsubscribe to the old start event
            if (mStartEvent)
                mStartEvent->sendMessage(TTSymbol("Unsubscribe"), mStartEventCallback, kTTValNONE);
            
            mStartEvent = value[0];
            
            // subscribe to the new start event
            if (mStartEvent)
                return mStartEvent->sendMessage(TTSymbol("Subscribe"), mStartEventCallback, kTTValNONE);
            else
                return kTTErrNone;
        }
    }
    
    return kTTErrGeneric;
}

TTErr TimeProcess::getIntermediateEvents(TTValue& value)
{
    mIntermediateEvents.assignToValue(value);
    
    return kTTErrNone;
}

TTErr TimeProcess::setEndEvent(const TTValue& value)
{
    if (value.size() == 1) {
        
        if (value[0].type() == kTypeObject) {
            
            // unsubscribe to the old end event
            if (mEndEvent)
                mEndEvent->sendMessage(TTSymbol("Unsubscribe"), mEndEventCallback, kTTValNONE);
            
            mEndEvent = value[0];
            
            // subscribe to the new end event
            if (mEndEvent)
                return mEndEvent->sendMessage(TTSymbol("Subscribe"), mEndEventCallback, kTTValNONE);
            else
                return kTTErrNone;
        }
    }
    
    return kTTErrGeneric;
}

TTErr TimeProcess::CreateStartEvent(const TTValue& value)
{
    TimeEventPtr timeEvent;
    TTErr err;
    
    // the start event needs to be released before
    if (mStartEvent)
        return kTTErrGeneric;
    
    if (value.size() >= 1) {
        
        if (value[0].type() == kTypeSymbol) {
            
            // Create a time event for the start
            timeEvent = NULL;
            err = TTObjectBaseInstantiate(value[0], TTObjectBaseHandle(&timeEvent), kTTValNONE);
            
            if (!err) {
                
                setStartEvent(TTObjectBasePtr(timeEvent));
                
                if (value.size() == 2) {
                    
                    if (value[1].type() == kTypeUInt32) {
                        
                        // Set the start date
                        timeEvent->setAttributeValue(TTSymbol("date"), TTUInt32(value[1]));
                    }
                }
            }
            
            return err;
        }
    }
    
    return kTTErrGeneric;
}

TTErr TimeProcess::ReleaseStartEvent()
{
    if (!mStartEvent)
        return kTTErrGeneric;
    
    setStartEvent(TTObjectBasePtr(NULL));
    
    TTObjectBaseRelease(TTObjectBaseHandle(&mStartEvent));
    mStartEvent = NULL;
    
    return kTTErrNone;
}

TTErr TimeProcess::CreateEndEvent(const TTValue& value)
{
    TimeEventPtr timeEvent;
    TTErr err;
    
    // the end event needs to be released before
    if (mEndEvent)
        return kTTErrGeneric;
    
    if (value.size() >= 1) {
        
        if (value[0].type() == kTypeSymbol) {
            
            // Create a time event for the end
            timeEvent = NULL;
            err = TTObjectBaseInstantiate(value[0], TTObjectBaseHandle(&timeEvent), kTTValNONE);
            
            if (!err) {
                
                setEndEvent(TTObjectBasePtr(timeEvent));
                
                if (value.size() == 2) {
                    
                    if (value[1].type() == kTypeUInt32) {
                        
                        // Set the end date
                        timeEvent->setAttributeValue(TTSymbol("date"), TTUInt32(value[1]));
                    }
                }
            }
            
            return err;
        }
    }
    
    return kTTErrGeneric;
}

TTErr TimeProcess::ReleaseEndEvent()
{
    if (!mEndEvent)
        return kTTErrGeneric;
    
    setEndEvent(TTObjectBasePtr(NULL));
    
    TTObjectBaseRelease(TTObjectBaseHandle(&mEndEvent));
    mEndEvent = NULL;
    
    return kTTErrNone;
}

TTErr TimeProcess::Limit(const TTValue& inputValue, TTValue& outputValue)
{
    TTValue v;
    
    if (inputValue.size() == 2) {
        
        if (inputValue[0].type() == kTypeUInt32 && inputValue[1].type() == kTypeUInt32) {
            
            if (TTUInt32(inputValue[0]) <= TTUInt32(inputValue[1])) {
                
                // set minimal and maximal duration
                mDurationMin = TTUInt32(inputValue[0]);
                mDurationMax = TTUInt32(inputValue[1]);
                
                // if the time process is handled by a scenario
                if (mScenario) {
                    
                    v = TTValue(TTObjectBasePtr(this));
                    v.append(mDurationMin);
                    v.append(mDurationMax);
                    
                    return mScenario->sendMessage(TTSymbol("TimeProcessLimit"), v, kTTValNONE);
                }
                else
                    return kTTErrNone;
            }
        }
    }
    
    return kTTErrGeneric;
}

#if 0
#pragma mark -
#pragma mark Some Methods
#endif

TTErr TimeProcessStartEventCallback(TTPtr baton, TTValue& data)
{
    TimeProcessPtr  aTimeProcess;
    TTValuePtr      b;
    TTValue         v;
    TTUInt32        start, end;
    
	// unpack baton (a time process)
	b = (TTValuePtr)baton;
	aTimeProcess = TimeProcessPtr((TTObjectBasePtr)(*b)[0]);
    
    // if the time process active, launch the scheduler
    // note : the ProcessStart method is called inside TimeProcessSchedulerCallback
	if (aTimeProcess->mActive) {
        
        aTimeProcess->mStartEvent->getAttributeValue(TTSymbol("date"), v);
        start = v[0];
        
        aTimeProcess->mEndEvent->getAttributeValue(TTSymbol("date"), v);
        end = v[0];
        
        if (end > start) {
            
            v = TTValue(end - start);
            
            aTimeProcess->mScheduler->setAttributeValue(TTSymbol("duration"), v);
            return aTimeProcess->mScheduler->sendMessage(TTSymbol("Go"));
        }
    }
    
    return kTTErrGeneric;
}

TTErr TimeProcessEndEventCallback(TTPtr baton, TTValue& data)
{
    TimeProcessPtr  aTimeProcess;
    TTValuePtr      b;
    
	// unpack baton (a time process)
	b = (TTValuePtr)baton;
	aTimeProcess = TimeProcessPtr((TTObjectBasePtr)(*b)[0]);
    
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
    timeProcessNames.append(TTSymbol("Interval"));
}

