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

#define thisTTClass         TTTimeProcess
#define thisTTClassName     "TimeProcess"
#define thisTTClassTags     "time, process"

/****************************************************************************************************/

TT_BASE_OBJECT_CONSTRUCTOR,
mContainer(NULL),
mName(kTTSymEmpty),
mDurationMin(0),
mDurationMax(0),
mMute(NO),
mVerticalPosition(0),
mVerticalSize(1),
mScheduler(NULL),
mRunning(NO),
mCompiled(NO),
mStartEvent(NULL),
mEndEvent(NULL)
{
    TT_ASSERT("Correct number of args to create TTTimeProcess", arguments.size() == 1);
    
    TTValue    args;
    TTErr      err;
    
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
    registerAttribute(TTSymbol("progression"), kTypeFloat64, NULL, (TTGetterMethod)& TTTimeProcess::getProgression);
    registerAttribute(TTSymbol("realTime"), kTypeFloat64, NULL, (TTGetterMethod)& TTTimeProcess::getRealTime);
    
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
    
    // Creation of a scheduler based on the System scheduler plugin
    // Prepare callback argument to be notified of :
    //      - the progression
    args = TTValue((TTPtr)&TTTimeProcessSchedulerCallback);
    args.append((TTPtr)this);   // we have to store this as a pointer for Scheduler
    
    err = TTObjectBaseInstantiate(TTSymbol("System"), TTObjectBaseHandle(&mScheduler), args);
    
	if (err) {
        mScheduler = NULL;
		logError("TimeProcess failed to load the System Scheduler");
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
    // Don't release start event here because it can be used by another time process
    setStartEvent(NULL);
    
    // Don't release end event here because it can be used by another time process
    setEndEvent(NULL);
    
    // Release scheduler
    if (mScheduler) {
        TTObjectBaseRelease(TTObjectBaseHandle(&mScheduler));
        mScheduler = NULL;
    }
}

TTErr TTTimeProcess::getRigid(TTValue& value)
{
    value = mDurationMin && mDurationMin && mDurationMin == mDurationMax;
    
    return kTTErrNone;
}

TTErr TTTimeProcess::setRigid(const TTValue& value)
{
    TTValue v, none, duration;
    
    if (value.size()) {
        
        if (value[0].type() == kTypeBoolean) {
            
            v = TTObjectBasePtr(this);
            
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
            if (mContainer)
                return mContainer->sendMessage(TTSymbol("TimeProcessLimit"), v, none);
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
            if (mContainer) {
                
                TTValue none, v = TTObjectBasePtr(this);
                v.append(mDurationMin);
                v.append(mDurationMax);
                return mContainer->sendMessage(TTSymbol("TimeProcessLimit"), v, none);
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
            if (mContainer) {
                
                TTValue none, v = TTObjectBasePtr(this);
                v.append(mDurationMin);
                v.append(mDurationMax);
                return mContainer->sendMessage(TTSymbol("TimeProcessLimit"), v, none);
            }
        }
    }
    
    return kTTErrGeneric;
}

TTErr TTTimeProcess::getStartDate(TTValue& value)
{
    value = TTTimeEventPtr(mStartEvent)->mDate;
    return kTTErrNone;
}

TTErr TTTimeProcess::setStartDate(const TTValue& value)
{
    if (value.size()) {
            
        if (value[0].type() == kTypeUInt32) {
                
            // tell to the container the process date changes
            if (mContainer) {
                
                TTValue none, v = TTObjectBasePtr(this);
                v.append(TTUInt32(value[0]));
                v.append(mStartDate);
                return mContainer->sendMessage(TTSymbol("TimeProcessMove"), v, none);
            }
            
            // or set the start event date directly
            else
                mStartEvent->setAttributeValue(kTTSym_date, value);
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
            if (mContainer) {
                
                TTValue none, v = TTObjectBasePtr(this);
                v.append(mStartDate);
                v.append(TTUInt32(value[0]));
                return mContainer->sendMessage(TTSymbol("TimeProcessMove"), v, none);
            }
            
            // or set the end event date directly
            else
                mEndEvent->setAttributeValue(kTTSym_date, value);
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
    if (mScheduler)
        return mScheduler->getAttributeValue(kTTSym_speed, value);
    
    return kTTErrGeneric;
}

TTErr TTTimeProcess::setSpeed(const TTValue& value)
{
    if (mScheduler)
        return mScheduler->setAttributeValue(kTTSym_speed, value);
    
    return kTTErrGeneric;
}

TTErr TTTimeProcess::getProgression(TTValue& value)
{
    if (mScheduler)
        return mScheduler->getAttributeValue("progression", value);
    
    return kTTErrGeneric;
}

TTErr TTTimeProcess::getRealTime(TTValue& value)
{
    if (mScheduler)
        return mScheduler->getAttributeValue("realTime", value);
    
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
                if (mContainer) {
                    
                    TTValue none, v = TTObjectBasePtr(this);
                    v.append(TTUInt32(inputValue[0]));
                    v.append(TTUInt32(inputValue[1]));
                    return mContainer->sendMessage(TTSymbol("TimeProcessMove"), v, none);
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
            if (mContainer) {
                
                TTValue none, v = TTObjectBasePtr(this);
                v.append(mDurationMin);
                v.append(mDurationMax);
                return mContainer->sendMessage(TTSymbol("TimeProcessLimit"), v, none);
            }
        }
    }
    
    return kTTErrGeneric;
}

TTErr TTTimeProcess::Start()
{
    return mStartEvent->sendMessage(kTTSym_Happen);
}

TTErr TTTimeProcess::End()
{
    return mEndEvent->sendMessage(kTTSym_Happen);
}

TTErr TTTimeProcess::Play()
{
    TTValue    v;
    TTUInt32   start, end;
    
    // set the running state of the process
    mRunning = YES;
    
    // launch the scheduler
    mStartEvent->getAttributeValue(kTTSym_date, v);
    start = v[0];
    
    mEndEvent->getAttributeValue(kTTSym_date, v);
    end = v[0];
    
    if (end > start) {
        
        v = TTFloat64(end - start);
        mScheduler->setAttributeValue(kTTSym_duration, v);
        
        mScheduler->sendMessage(kTTSym_Go);
        
        return kTTErrNone;
    }
    
    return kTTErrGeneric;
}

TTErr TTTimeProcess::Stop()
{
    // set the running state of the process
    mRunning = NO;

    return mScheduler->sendMessage(kTTSym_Stop);
}

TTErr TTTimeProcess::Pause()
{
    TTValue none;
    
    mScheduler->sendMessage(kTTSym_Pause);
    
    return ProcessPaused(TTBoolean(YES), none);
}

TTErr TTTimeProcess::Resume()
{
    TTValue none;
    
    mScheduler->sendMessage(kTTSym_Resume);
    
    return ProcessPaused(TTBoolean(NO), none);
}

#if 0
#pragma mark -
#pragma mark Notifications
#endif

TTErr TTTimeProcess::EventDateChanged(const TTValue& inputValue, TTValue& outputValue)
{
    TT_ASSERT("TTTimeProcess::EventDateChanged : inputValue is correct", inputValue.size() == 1 && inputValue[0].type() == kTypeObject);
    
    TTObjectBasePtr aTimeEvent = inputValue[0];
    
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
    
    TTTimeEventPtr          aTimeEvent = TTTimeEventPtr(TTObjectBasePtr(inputValue[0]));
    TTSymbol                newStatus = inputValue[1], oldStatus = inputValue[2];
    TTValue                 v;
    
    TT_ASSERT("TTTimeProcess::EventStatusChanged : status effectively changed", newStatus != oldStatus);
    
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
            
            // note : don't set start event ready attribute to NO : it is to the container to take this decision
            
            // use the specific start process method of the time process
            if (!ProcessStart()) {

                // notify start message observers
                sendNotification(kTTSym_ProcessStarted, TTObjectBasePtr(this));
                
                // play the process
                return Play();
            }
        }
        else if (aTimeEvent == mEndEvent) {
            
            // if the time process is muted
            if (mMute)
                return kTTErrNone;
            
            // stop the process
            Stop();
            
            // note : don't set end event ready attribute to NO : it is to the container to take this decision
            
            // use the specific process end method of the time process
            if (!ProcessEnd()) {
                
                // notify observers
                sendNotification(kTTSym_ProcessEnded, TTObjectBasePtr(this));
                
                return kTTErrNone;
            }
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

#if 0
#pragma mark -
#pragma mark Start and End events accessors
#endif

TTTimeEventPtr TTTimeProcess::getStartEvent()
{
    return (TTTimeEventPtr)mStartEvent;
}

TTTimeEventPtr TTTimeProcess::getEndEvent()
{
    return (TTTimeEventPtr)mEndEvent;
}

TTErr TTTimeProcess::setStartEvent(TTTimeEventPtr aTimeEvent)
{
    // Stop start event observation
    if (mStartEvent)
        mStartEvent->unregisterObserverForNotifications(*this);
    
    // Replace the start event by the new one
    mStartEvent = aTimeEvent;
    
    // Observe start event
    if (mStartEvent)
        mStartEvent->registerObserverForNotifications(*this);
    
    return kTTErrNone;
}

TTErr TTTimeProcess::setEndEvent(TTTimeEventPtr aTimeEvent)
{
    // Stop end event observation
    if (mEndEvent)
        mEndEvent->unregisterObserverForNotifications(*this);
    
    // Replace the end event by the new one
    mEndEvent = aTimeEvent;
    
    // Observe end event
    if (mEndEvent)
        mEndEvent->registerObserverForNotifications(*this);
    
    return kTTErrNone;
}

#if 0
#pragma mark -
#pragma mark Scheduler callback
#endif

void TTTimeProcessSchedulerCallback(TTPtr object, TTFloat64 progression, TTFloat64 realTime)
{
	TTTimeProcessPtr	aTimeProcess = (TTTimeProcessPtr)object;
    TTValue             none;
    
    // use the specific process method
    if (aTimeProcess->mRunning)
       aTimeProcess->Process(TTValue(progression, realTime), none);
    //else
    //    std::cout << "TTTimeProcessSchedulerCallback : avoid last scheduler tick" << std::endl;
}