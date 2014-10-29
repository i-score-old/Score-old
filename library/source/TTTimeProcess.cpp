/** @file
 *
 * @ingroup scoreLibrary
 *
 * @brief a class to define a process
 *
 * @see TTTimeEvent
 *
 * @authors Théo de la Hogue & Clément Bossut
 *
 * @copyright Copyright © 2013, Théo de la Hogue & Clément Bossut @n
 * This code is licensed under the terms of the "CeCILL-C" @n
 * http://www.cecill.info
 */

#include "TTTimeProcess.h"
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlreader.h>

#define thisTTClass         TTTimeProcess
#define thisTTClassName     "TimeProcess"
#define thisTTClassTags     "time, process"

/****************************************************************************************************/

TT_BASE_OBJECT_CONSTRUCTOR,
mName(kTTSymEmpty),
mDurationMin(0),
mDurationMax(0),
mMute(NO),
mVerticalPosition(0),
mVerticalSize(1),
mRunning(NO),
mCompiled(NO),
mExternalTick(NO)
{
    TT_ASSERT("Correct number of args to create TTTimeProcess", arguments.size() == 1);
    
    if (arguments.size() == 1)
        mContainer = arguments[0];
    
    addAttribute(Container, kTypeObject);
    
    // the rigid state handles the DurationMin and DurationMax attribute
    registerAttribute(kTTSym_rigid, kTypeBoolean, NULL, (TTGetterMethod)& TTTimeProcess::getRigid, (TTSetterMethod)& TTTimeProcess::setRigid);
    
    addAttribute(Name, kTypeSymbol);
    
    addAttributeWithSetter(DurationMin, kTypeUInt32);
    addAttributeWithSetter(DurationMax, kTypeUInt32);
    
    addAttribute(Mute, kTypeBoolean);
    
    addAttribute(Running, kTypeBoolean);
    addAttributeProperty(Running, readOnly, YES);
    
    addAttribute(Compiled, kTypeBoolean);
    addAttributeProperty(Compiled, readOnly, YES);
    
    addAttribute(ExternalTick, kTypeBoolean);
    
    addAttributeWithSetter(Color, kTypeLocalValue);
    addAttribute(VerticalPosition, kTypeUInt32);
    addAttribute(VerticalSize, kTypeUInt32);
    
    addAttribute(Scheduler, kTypeObject);
    addAttributeProperty(Scheduler, readOnly, YES);
    addAttributeProperty(Scheduler, hidden, YES);
    
    addAttribute(StartEvent, kTypeObject);
    addAttributeProperty(StartEvent, readOnly, YES);
    addAttributeProperty(StartEvent, hidden, YES);
    
    addAttribute(EndEvent, kTypeObject);
    addAttributeProperty(EndEvent, readOnly, YES);
    addAttributeProperty(EndEvent, hidden, YES);
    
    // the attributes below are not related to any TTTimeProcess member
    // but we need to declare them as attribute to ease the use of the class
    registerAttribute(TTSymbol("startDate"), kTypeUInt32, NULL, (TTGetterMethod)& TTTimeProcess::getStartDate, (TTSetterMethod)& TTTimeProcess::setStartDate);
    registerAttribute(TTSymbol("endDate"), kTypeUInt32, NULL, (TTGetterMethod)& TTTimeProcess::getEndDate, (TTSetterMethod)& TTTimeProcess::setEndDate);
    registerAttribute(TTSymbol("startCondition"), kTypeBoolean, NULL, (TTGetterMethod)& TTTimeProcess::getStartCondition, (TTSetterMethod)& TTTimeProcess::setStartCondition);
    registerAttribute(TTSymbol("endCondition"), kTypeBoolean, NULL, (TTGetterMethod)& TTTimeProcess::getEndCondition, (TTSetterMethod)& TTTimeProcess::setEndCondition);
    registerAttribute(kTTSym_duration, kTypeUInt32, NULL, (TTGetterMethod)& TTTimeProcess::getDuration);
    registerAttribute(kTTSym_speed, kTypeFloat64, NULL, (TTGetterMethod)& TTTimeProcess::getSpeed, (TTSetterMethod)& TTTimeProcess::setSpeed);
    registerAttribute(TTSymbol("position"), kTypeFloat64, NULL, (TTGetterMethod)& TTTimeProcess::getPosition);
    registerAttribute(TTSymbol("date"), kTypeFloat64, NULL, (TTGetterMethod)& TTTimeProcess::getDate);
    
    addMessage(Compile);
    addMessageProperty(Compile, hidden, YES);
    
    addMessage(ProcessStart);
    addMessageProperty(ProcessStart, hidden, YES);
    
    addMessage(ProcessEnd);
    addMessageProperty(ProcessEnd, hidden, YES);
    
    addMessageWithArguments(Process);
    addMessageProperty(Process, hidden, YES);
    
    addMessageWithArguments(ProcessPaused);
    addMessageProperty(ProcessPaused, hidden, YES);
    
    addMessageWithArguments(Goto);
    addMessageProperty(Goto, hidden, YES);
    
    addMessageWithArguments(Move);
    addMessageWithArguments(Limit);
    
    addMessage(Start);
    addMessage(End);
    addMessage(Play);
    addMessage(Stop);
    addMessage(Pause);
    addMessage(Resume);
    addMessage(Tick);
    
	// needed to be handled by a TTXmlHandler
	addMessageWithArguments(WriteAsXml);
	addMessageProperty(WriteAsXml, hidden, YES);
	addMessageWithArguments(ReadFromXml);
	addMessageProperty(ReadFromXml, hidden, YES);
    
    // needed to be notified by events
    addMessageWithArguments(EventDateChanged);
    addMessageProperty(EventDateChanged, hidden, YES);
    addMessageWithArguments(EventStatusChanged);
    addMessageProperty(EventStatusChanged, hidden, YES);
    
    // needed to be notified by the scheduler
    addMessageWithArguments(SchedulerRunningChanged);
    addMessageProperty(SchedulerRunningChanged, hidden, YES);
    
    // Creation of a scheduler based on the System scheduler plugin
    // Prepare callback argument to be notified of :
    //      - the position
    TTValue args = TTValue((TTPtr)&TTTimeProcessSchedulerCallback);
    args.append((TTPtr)this);   // we have to store this as a pointer for Scheduler
    
    mScheduler = TTObject("system", args);
    
	if (!mScheduler.valid()) {
		logError("TimeProcess failed to load the System Scheduler");
    }
    else {
    
        // observe the scheduler
		TTObject thisObject(this);
        mScheduler.registerObserverForNotifications(thisObject);
    }
    
    // generate a random name
    mName = mName.random();
    
    // set default color to white
    mColor.append(255);
    mColor.append(255);
    mColor.append(255);
}

TTTimeProcess::~TTTimeProcess()
{
    ;
}

TTErr TTTimeProcess::getRigid(TTValue& value)
{
    value = mDurationMin && mDurationMax && mDurationMin == mDurationMax;
    
    return kTTErrNone;
}

TTErr TTTimeProcess::setRigid(const TTValue& value)
{
    TTValue v, none, duration;
    
    if (value.size()) {
        
        if (value[0].type() == kTypeBoolean) {
            
            v = TTObject(this);
            
            // rigid means Limit(duration, duration)
            if (TTBoolean(value[0])) {
                
                if (!getDuration(duration)) {
                    
                    v.append(duration[0]);
                    v.append(duration[0]);
                }
            }
            // non rigid means Limit(durationMin, durationMax)
            else {
                v.append(mDurationMin);
                v.append(mDurationMax);
            }
            
            // if the time process is handled by a scenario
            if (mContainer.valid())
                return mContainer.send("TimeProcessLimit", v, none);
        }
    }
    
    return kTTErrGeneric;
}

TTErr TTTimeProcess::setDurationMin(const TTValue& value)
{
    if (value.size()) {
        
        if (value[0].type() == kTypeUInt32) {
            
            // set minimal duration
            mDurationMin = TTUInt32(value[0]);
            
            // tell to the container the limit changes
            if (mContainer.valid()) {
                
                TTValue none, v = TTObject(this);
                v.append(mDurationMin);
                v.append(mDurationMax);
                return mContainer.send("TimeProcessLimit", v, none);
            }
        }
    }
    
    return kTTErrGeneric;
}

TTErr TTTimeProcess::setDurationMax(const TTValue& value)
{
    if (value.size()) {
        
        if (value[0].type() == kTypeUInt32) {
            
            // set maximal duration
            mDurationMax = TTUInt32(value[0]);
            
            // tell to the container the limit changes
            if (mContainer.valid()) {
                
                TTValue none, v = TTObject(this);
                v.append(mDurationMin);
                v.append(mDurationMax);
                return mContainer.send("TimeProcessLimit", v, none);
            }
        }
    }
    
    return kTTErrGeneric;
}

TTErr TTTimeProcess::getStartDate(TTValue& value)
{
    value = TTTimeEventPtr(mStartEvent.instance())->mDate;
    return kTTErrNone;
}

TTErr TTTimeProcess::setStartDate(const TTValue& value)
{
    if (value.size()) {
            
        if (value[0].type() == kTypeUInt32) {
                
            // tell to the container the process date changes
            if (mContainer.valid()) {
                
                TTValue none, v = TTObject(this);
                v.append(TTUInt32(value[0]));
                v.append(mStartDate);
                return mContainer.send("TimeProcessMove", v, none);
            }
            
            // or set the start event date directly
            else
                mStartEvent.set(kTTSym_date, value);
        }
    }
    
    return kTTErrGeneric;
}

TTErr TTTimeProcess::getStartCondition(TTValue& value)
{
    value = mStartCondition;
    return kTTErrNone;
}

TTErr TTTimeProcess::setStartCondition(const TTValue& value)
{
    if (value.size() == 1) {
        
        if (value[0].type() == kTypeObject) {
            
            mStartCondition = value[0];
        }
    }
    
    return kTTErrGeneric;
}

TTErr TTTimeProcess::getEndDate(TTValue& value)
{
    value = mEndDate;
    return kTTErrNone;
}

TTErr TTTimeProcess::setEndDate(const TTValue& value)
{
    if (value.size()) {
            
        if (value[0].type() == kTypeUInt32) {
                
            // tell to the container the process date changes
            if (mContainer.valid()) {
                
                TTValue none, v = TTObject(this);
                v.append(mStartDate);
                v.append(TTUInt32(value[0]));
                return mContainer.send("TimeProcessMove", v, none);
            }
            
            // or set the end event date directly
            else
                mEndEvent.set(kTTSym_date, value);
        }
    }
    
    return kTTErrGeneric;
}

TTErr TTTimeProcess::getEndCondition(TTValue& value)
{
    value = mEndCondition;
    return kTTErrNone;
}

TTErr TTTimeProcess::setEndCondition(const TTValue& value)
{
    if (value.size() == 1) {
        
        if (value[0].type() == kTypeBoolean) {
            
            mEndCondition = value[0];
        }
    }
    
    return kTTErrGeneric;
}

TTErr TTTimeProcess::getDuration(TTValue& value)
{
    // the end must be after the start
    if (mEndDate >= mStartDate) {
            
        value = TTValue( TTUInt32( abs(mDuration) ) );
        
        return kTTErrNone;
    }
    
    return kTTErrGeneric;
}

TTErr TTTimeProcess::setColor(const TTValue& value)
{
    mColor = value;
    
    return kTTErrNone;
}

TTErr TTTimeProcess::getSpeed(TTValue& value)
{
    if (mScheduler.valid())
        return mScheduler.get(kTTSym_speed, value);
    
    return kTTErrGeneric;
}

TTErr TTTimeProcess::setSpeed(const TTValue& value)
{
    if (mScheduler.valid())
        return mScheduler.set(kTTSym_speed, value);
    
    return kTTErrGeneric;
}

TTErr TTTimeProcess::getPosition(TTValue& value)
{
    if (mScheduler.valid())
        return mScheduler.get("position", value);
    
    return kTTErrGeneric;
}

TTErr TTTimeProcess::getDate(TTValue& value)
{
    if (mScheduler.valid())
        return mScheduler.get("date", value);
    
    return kTTErrGeneric;
}

TTErr TTTimeProcess::getIntermediateEvents(TTValue& value)
{
    mIntermediateEvents.assignToValue(value);
    
    return kTTErrNone;
}

TTErr TTTimeProcess::Move(const TTValue& inputValue, TTValue& outputValue)
{
    if (inputValue.size() == 2) {
        
        if (inputValue[0].type() == kTypeUInt32 && inputValue[1].type() == kTypeUInt32) {
            
            if (TTUInt32(inputValue[0]) <= TTUInt32(inputValue[1])) {
                
                // if the time process is handled by a container
                if (mContainer.valid()) {
                    
                    TTValue none, v = TTObject(this);
                    v.append(TTUInt32(inputValue[0]));
                    v.append(TTUInt32(inputValue[1]));
                    return mContainer.send("TimeProcessMove", v, none);
                }
            }
        }
    }
    
    return kTTErrGeneric;
}

TTErr TTTimeProcess::Limit(const TTValue& inputValue, TTValue& outputValue)
{
    if (inputValue.size() == 2) {
        
        if (inputValue[0].type() == kTypeUInt32 && inputValue[1].type() == kTypeUInt32) {
            
            // set minimal and maximal duration
            mDurationMin = TTUInt32(inputValue[0]);
            mDurationMax = TTUInt32(inputValue[1]);
            
            // if the time process is handled by a scenario
            if (mContainer.valid()) {
                
                TTValue none, v = TTObject(this);
                v.append(mDurationMin);
                v.append(mDurationMax);
                return mContainer.send("TimeProcessLimit", v, none);
            }
        }
    }
    
    return kTTErrGeneric;
}

TTErr TTTimeProcess::Start()
{
    return mStartEvent.send(kTTSym_Trigger);
}

TTErr TTTimeProcess::End()
{
    return mEndEvent.send(kTTSym_Trigger);
}

TTErr TTTimeProcess::Play()
{
    TTValue    v;
    TTUInt32   start, end;
    
    // set the running state of the process
    mRunning = YES;
    
    // launch the scheduler
    mStartEvent.get(kTTSym_date, v);
    start = v[0];
    
    mEndEvent.get(kTTSym_date, v);
    end = v[0];
    
    if (end > start) {
        
        v = TTFloat64(end - start);
        mScheduler.set(kTTSym_duration, v);
        
        mScheduler.set("externalTick", mExternalTick);
        
        return mScheduler.send(kTTSym_Go);
    }
    
    return kTTErrGeneric;
}

TTErr TTTimeProcess::Stop()
{
    // set the running state of the process
    mRunning = NO;

    return mScheduler.send(kTTSym_Stop);
}

TTErr TTTimeProcess::Pause()
{
    TTValue none;
    
    mScheduler.send(kTTSym_Pause);
    
    return ProcessPaused(TTBoolean(YES), none);
}

TTErr TTTimeProcess::Resume()
{
    TTValue none;
    
    mScheduler.send(kTTSym_Resume);
    
    return ProcessPaused(TTBoolean(NO), none);
}

TTErr TTTimeProcess::Tick()
{
    if (mExternalTick && mRunning)
        return mScheduler.send(kTTSym_Tick);
    else
        return kTTErrGeneric;
}

#if 0
#pragma mark -
#pragma mark Notifications
#endif

TTErr TTTimeProcess::EventDateChanged(const TTValue& inputValue, TTValue& outputValue)
{
    TT_ASSERT("TTTimeProcess::EventDateChanged : inputValue is correct", inputValue.size() == 1 && inputValue[0].type() == kTypeObject);
    
    TTObject aTimeEvent = inputValue[0];
    
    if (aTimeEvent == mStartEvent) {
        
        // if needed, the compile method should be called again now
        mCompiled = NO;
        
        return kTTErrNone;
    }
    else if (aTimeEvent == mEndEvent) {
        
        // if needed, the compile method should be called again now
        mCompiled = NO;
        
        return kTTErrNone;
    }
    
    TTLogError("TTTimeProcess::EventDateChanged : wrong event\n");
    return kTTErrGeneric;
}

TTErr TTTimeProcess::EventStatusChanged(const TTValue& inputValue, TTValue& outputValue)
{
    TT_ASSERT("TTTimeProcess::EventStatusChanged : inputValue is correct", inputValue.size() == 3 && inputValue[0].type() == kTypeObject);
    
    TTObject    aTimeEvent = inputValue[0];
    TTSymbol    newStatus = inputValue[1];
    TTSymbol    oldStatus = inputValue[2];
    TTValue     v;
    
    TTBoolean   statusChanged = newStatus != oldStatus;
    TT_ASSERT("TTTimeProcess::EventStatusChanged : status effectively changed", statusChanged);
    
    // event wainting case :
    if (newStatus == kTTSym_eventWaiting) {
        return kTTErrNone;
    }
    // event pending case :
    else if (newStatus == kTTSym_eventPending) {
        return kTTErrNone;
    }
    // event happened case :
    else if (newStatus == kTTSym_eventHappened) {
        
        if (aTimeEvent == mStartEvent) {
            
            // if the time process is muted
            if (mMute)
                return kTTErrNone;
            
            // use the specific compiled method of the time process
            if (!mCompiled)
                Compile();
            
            // use the specific start process method of the time process
            if (!ProcessStart()) {

                // notify start message observers
				TTObject thisObject(this);
                sendNotification(kTTSym_ProcessStarted, thisObject);
                
                // play the process
                return Play();
            }
            
            TTLogError("TTTimeProcess::EventStatusChanged : ProccessStart failed\n");
            return kTTErrGeneric;
        }
        else if (aTimeEvent == mEndEvent) {
            
            // if the time process is muted
            if (mMute)
                return kTTErrNone;
            
            // stop the process
            return Stop();
            
            // the kTTSym_ProcessEnded notification is sent in TTTimeProcess::SchedulerRunningChanged
        }
        
        TTLogError("TTTimeProcess::EventStatusChanged : wrong event happened\n");
        return kTTErrGeneric;
    }
    // event disposed case :
    else if (newStatus == kTTSym_eventDisposed) {
        return kTTErrNone;
    }
    
    TTLogError("TTTimeProcess::EventStatusChanged : wrong status\n");
    return kTTErrGeneric;
}

TTErr TTTimeProcess::SchedulerRunningChanged(const TTValue& inputValue, TTValue& outputValue)
{
    TT_ASSERT("TTTimeProcess::SchedulerRunningChanged : inputValue is correct", inputValue.size() == 1 && inputValue[0].type() == kTypeBoolean);
    
    TTBoolean running = inputValue[0];
    
    if (running) {
        
        // the kTTSym_ProcessStarted is sent in TTTimeProcess::EventStatusChanged
        return kTTErrNone;
    }
    else {
        
        // use the specific process end method of the time process
        if (!ProcessEnd()) {
            
            // notify observers
			TTObject thisObject(this);
            sendNotification(kTTSym_ProcessEnded, thisObject);
            
            return kTTErrNone;
        }
        
        return kTTErrGeneric;
    }
}

#if 0
#pragma mark -
#pragma mark Start and End events accessors
#endif

TTObject& TTTimeProcess::getStartEvent()
{
    return mStartEvent;
}

TTObject& TTTimeProcess::getEndEvent()
{
    return mEndEvent;
}

TTErr TTTimeProcess::setStartEvent(TTObject& aTimeEvent)
{
    TTObject thisObject(this);
    
    // Stop start event observation
    if (mStartEvent.valid())
        mStartEvent.unregisterObserverForNotifications(thisObject);
    
    // Replace the start event by the new one
    mStartEvent = aTimeEvent;
    
    // Observe start event
    if (mStartEvent.valid())
        mStartEvent.registerObserverForNotifications(thisObject);
    
    return kTTErrNone;
}

TTErr TTTimeProcess::setEndEvent(TTObject&  aTimeEvent)
{
    TTObject thisObject(this);
    
    // Stop end event observation
    if (mEndEvent.valid())
        mEndEvent.unregisterObserverForNotifications(thisObject);
    
    // Replace the end event by the new one
    mEndEvent = aTimeEvent;
    
    // Observe end event
    if (mEndEvent.valid())
        mEndEvent.registerObserverForNotifications(thisObject);
    
    return kTTErrNone;
}

#if 0
#pragma mark -
#pragma mark Scheduler callback
#endif

void TTTimeProcessSchedulerCallback(TTPtr object, TTFloat64 position, TTFloat64 date)
{
	TTTimeProcessPtr	aTimeProcess = (TTTimeProcessPtr)object;
    TTValue             none;
    
    // use the specific process method
    if (aTimeProcess->mRunning) {
        
        aTimeProcess->Process(TTValue(position, date), none);
        
        // the notifications below are useful for network observation purpose for exemple
        // TODO : shouldn't we limit the sending of those observation to not overcrowed the network ?
        
        // notify position observers
        TTAttributePtr	positionAttribute;
        aTimeProcess->findAttribute("position", &positionAttribute);
        positionAttribute->sendNotification(kTTSym_notify, position);
        
        // notify date observers
        TTAttributePtr	dateAttribute;
        aTimeProcess->findAttribute("date", &dateAttribute);
        dateAttribute->sendNotification(kTTSym_notify, date);
    }
}
