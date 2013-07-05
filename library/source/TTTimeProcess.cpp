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
mDurationMin(0),
mDurationMax(0),
mActive(YES),
mStartEvent(NULL),
mEndEvent(NULL),
mScheduler(NULL),
mStartEventCallback(NULL),
mEndEventCallback(NULL)
{
    TT_ASSERT("Correct number of args to create TTTimeProcess", arguments.size() == 2 || arguments.size() == 3);
    
    TTValue         args;
    TTErr           err;
    TTMessagePtr    aMessage = NULL;
    TTValuePtr      startEventBaton, endEventBaton;
    
    if (arguments.size() >= 2) {
        mStartEvent = arguments[0];
        mEndEvent = arguments[1];
    }
    
    if (arguments.size() == 3)
        mContainer = arguments[2];
    
    // the rigid state handles the DurationMin and DurationMax attribute
    registerAttribute(TTSymbol("rigid"), kTypeBoolean, NULL, (TTGetterMethod)& TTTimeProcess::getRigid, (TTSetterMethod)& TTTimeProcess::setRigid);
    
    addAttributeWithSetter(DurationMin, kTypeUInt32);
    addAttributeWithSetter(DurationMax, kTypeUInt32);
    
    addAttributeWithSetter(Active, kTypeBoolean);
    
    addAttribute(StartEvent, kTypeObject);
    addAttributeProperty(StartEvent, readOnly, YES);
    addAttributeProperty(StartEvent, hidden, YES);
    
    addAttributeWithGetter(IntermediateEvents, kTypeLocalValue);
    addAttributeProperty(IntermediateEvents, readOnly, YES);
    addAttributeProperty(IntermediateEvents, hidden, YES);
    
    addAttribute(EndEvent, kTypeObject);
    addAttributeProperty(EndEvent, readOnly, YES);
    addAttributeProperty(EndEvent, hidden, YES);
    
    addAttribute(Scheduler, kTypeObject);
    addAttributeProperty(Scheduler, readOnly, YES);
    addAttributeProperty(Scheduler, hidden, YES);
    
    // the attributes below are not related to any TTTimeProcess member
    // but we need to declare them as attribute to ease the use of the class
    registerAttribute(TTSymbol("startDate"), kTypeUInt32, NULL, (TTGetterMethod)& TTTimeProcess::getStartDate, (TTSetterMethod)& TTTimeProcess::setStartDate);
    registerAttribute(TTSymbol("endDate"), kTypeUInt32, NULL, (TTGetterMethod)& TTTimeProcess::getEndDate, (TTSetterMethod)& TTTimeProcess::setEndDate);
    registerAttribute(TTSymbol("startInteractive"), kTypeBoolean, NULL, (TTGetterMethod)& TTTimeProcess::getStartInteractive, (TTSetterMethod)& TTTimeProcess::setStartInteractive);
    registerAttribute(TTSymbol("endInteractive"), kTypeBoolean, NULL, (TTGetterMethod)& TTTimeProcess::getEndInteractive, (TTSetterMethod)& TTTimeProcess::setEndInteractive);
    registerAttribute(TTSymbol("duration"), kTypeUInt32, NULL, (TTGetterMethod)& TTTimeProcess::getDuration);
    
    addMessage(ProcessStart);
    addMessage(ProcessEnd);
    addMessageWithArguments(Process);
    
    addMessageWithArguments(Move);
    addMessageWithArguments(Limit);
    
    addMessage(Play);
    addMessage(Stop);
    addMessage(Pause);
    addMessage(Resume);
    
    addMessage(ReleaseEvents);
    
	// needed to be handled by a TTXmlHandler
	addMessageWithArguments(WriteAsXml);
	addMessageProperty(WriteAsXml, hidden, YES);
	addMessageWithArguments(ReadFromXml);
	addMessageProperty(ReadFromXml, hidden, YES);
    
    // Create a start event callback to be notified and start the process execution
    mStartEventCallback = NULL;
    TTObjectBaseInstantiate(TTSymbol("callback"), &mStartEventCallback, kTTValNONE);
    
    startEventBaton = new TTValue(TTObjectBasePtr(this));
    
    mStartEventCallback->setAttributeValue(kTTSym_baton, TTPtr(startEventBaton));
    mStartEventCallback->setAttributeValue(kTTSym_function, TTPtr(&TTTimeProcessStartEventHappenCallback));
    
    // Observe start event happening
    err = mStartEvent->findMessage(TTSymbol("Happen"), &aMessage);
    
    if(!err)
        aMessage->registerObserverForNotifications(*mStartEventCallback);
    
    // Create a end event callback to be notified and end the process execution
    mEndEventCallback = NULL;
    TTObjectBaseInstantiate(TTSymbol("callback"), &mEndEventCallback, kTTValNONE);
    
    endEventBaton = new TTValue(TTObjectBasePtr(this));
    
    mEndEventCallback->setAttributeValue(kTTSym_baton, TTPtr(endEventBaton));
    mEndEventCallback->setAttributeValue(kTTSym_function, TTPtr(&TTTimeProcessEndEventHappenCallback));
    
    // Observe end event happening
    err = mEndEvent->findMessage(TTSymbol("Happen"), &aMessage);
    
    if(!err)
        aMessage->registerObserverForNotifications(*mEndEventCallback);
    
    // Creation of a scheduler based on the System scheduler plugin
    // Prepare callback argument to be notified of :
    //      - the progression
    args = TTValue((TTPtr)&TTTimeProcessSchedulerCallback);
    args.append((TTPtr)this);   // we have to store this as a pointer for Scheduler
    
    err = TTObjectBaseInstantiate(TTSymbol("System"), TTObjectBaseHandle(&mScheduler), args);
    
	if (err) {
        mScheduler = NULL;
		logError("TimeProcess failed to load the EcoMachine Scheduler");
    }
    
    // Cache some attributes for high speed notification feedbacks
    this->findAttribute(TTSymbol("active"), &activeAttribute);
}

TTTimeProcess::~TTTimeProcess()
{
    TTMessagePtr    aMessage = NULL;
    TTErr           err;
    
    // Stop start event happening observation
    if (mStartEvent) {
        
        err = mStartEvent->findMessage(TTSymbol("Happen"), &aMessage);
    
        if(!err) {
        
            aMessage->unregisterObserverForNotifications(*mStartEventCallback);
            mStartEvent = NULL;
        }
    }
    
    // Don't release start event here because it can be used by another time process
    
    // Release start event callback
    if (mStartEventCallback) {
        delete (TTValuePtr)TTCallbackPtr(mStartEventCallback)->getBaton();
        TTObjectBaseRelease(TTObjectBaseHandle(&mStartEventCallback));
        mStartEventCallback = NULL;
    }
    
    // Stop end event happening observation
    if (mEndEvent) {
        
        err = mEndEvent->findMessage(TTSymbol("Happen"), &aMessage);
        
        if(!err) {
            
            aMessage->unregisterObserverForNotifications(*mEndEventCallback);
            mEndEvent = NULL;
        }
    }
    
    // Don't release end event here because it can be used by another time process
    
    // Release end event callback
    if (mEndEventCallback) {
        delete (TTValuePtr)TTCallbackPtr(mEndEventCallback)->getBaton();
        TTObjectBaseRelease(TTObjectBaseHandle(&mEndEventCallback));
        mEndEventCallback = NULL;
    }
    
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
    TTValue v;
    
    if (value.size()) {
        
        if (value[0].type() == kTypeBoolean) {
            
            if (!getDuration(v)) {
            
                if (TTBoolean(value[0]))
                    v.append(TTUInt32(v[0]));   // Limit(duration, duration)
                else
                    v.prepend(TTUInt32(0));     // Limit(0, duration)
            
                return Limit(v, kTTValNONE);
            }
        }
    }
    
    return kTTErrGeneric;
}

TTErr TTTimeProcess::setDurationMin(const TTValue& value)
{
    if (value.size()) {
        
        if (value[0].type() == kTypeUInt32) {
            
            if (TTUInt32(value[0]) <= mDurationMax) {
                
                // set minimal duration
                mDurationMin = TTUInt32(value[0]);
            
                // tell to the container the limit changes
                if (mContainer) {
                    
                    TTValue v = TTObjectBasePtr(this);
                    v.append(mDurationMin);
                    v.append(mDurationMax);
                    return mContainer->sendMessage(TTSymbol("TimeProcessLimit"), v, kTTValNONE);
                }
            }
        }
    }
    
    return kTTErrGeneric;
}

TTErr TTTimeProcess::setDurationMax(const TTValue& value)
{
    if (value.size()) {
        
        if (value[0].type() == kTypeUInt32) {
            
            if (TTUInt32(value[0]) >= mDurationMin) {
                
                // set maximal duration
                mDurationMax = TTUInt32(value[0]);
                
                // tell to the container the limit changes
                if (mContainer) {
                    
                    TTValue v = TTObjectBasePtr(this);
                    v.append(mDurationMin);
                    v.append(mDurationMax);
                    return mContainer->sendMessage(TTSymbol("TimeProcessLimit"), v, kTTValNONE);
                }
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
                
                TTValue v = TTObjectBasePtr(this);
                v.append(TTUInt32(value[0]));
                v.append(mStartDate);
                return mContainer->sendMessage(TTSymbol("TimeProcessMove"), v, kTTValNONE);
            }
        }
    }
    
    return kTTErrGeneric;
}

TTErr TTTimeProcess::getStartInteractive(TTValue& value)
{
    value = mStartInteractive;
    return kTTErrNone;
}

TTErr TTTimeProcess::setStartInteractive(const TTValue& value)
{
    if (value.size() == 1) {
        
        if (value[0].type() == kTypeBoolean) {
            
            mStartInteractive = value[0];
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
                
                TTValue v = TTObjectBasePtr(this);
                v.append(mStartDate);
                v.append(TTUInt32(value[0]));
                return mContainer->sendMessage(TTSymbol("TimeProcessMove"), v, kTTValNONE);
            }
        }
    }
    
    return kTTErrGeneric;
}

TTErr TTTimeProcess::getEndInteractive(TTValue& value)
{
    value = mEndInteractive;
    return kTTErrNone;
}

TTErr TTTimeProcess::setEndInteractive(const TTValue& value)
{
    if (value.size() == 1) {
        
        if (value[0].type() == kTypeBoolean) {
            
            mEndInteractive = value[0];
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

TTErr TTTimeProcess::setActive(const TTValue& value)
{
    // set the internal active value
    mActive = value[0];
    
    // notify each attribute observers
    activeAttribute->sendNotification(kTTSym_notify, mActive);             // we use kTTSym_notify because we know that observers are TTCallback
    
    return kTTErrNone;
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
                    
                    TTValue v = TTObjectBasePtr(this);
                    v.append(TTUInt32(inputValue[0]));
                    v.append(TTUInt32(inputValue[1]));
                    return mContainer->sendMessage(TTSymbol("TimeProcessMove"), v, kTTValNONE);
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
            
            if (TTUInt32(inputValue[0]) <= TTUInt32(inputValue[1])) {
                
                // set minimal and maximal duration
                mDurationMin = TTUInt32(inputValue[0]);
                mDurationMax = TTUInt32(inputValue[1]);
                
                // if the time process is handled by a scenario
                if (mContainer) {
                    
                    TTValue v = TTObjectBasePtr(this);
                    v.append(mDurationMin);
                    v.append(mDurationMax);
                    return mContainer->sendMessage(TTSymbol("TimeProcessLimit"), v, kTTValNONE);
                }
            }
        }
    }
    
    return kTTErrGeneric;
}

TTErr TTTimeProcess::Play()
{    
    return mStartEvent->sendMessage(TTSymbol("Happen"));
}

TTErr TTTimeProcess::Stop()
{
    return mEndEvent->sendMessage(TTSymbol("Happen"));
}

TTErr TTTimeProcess::Pause()
{
    return mScheduler->sendMessage(TTSymbol("Pause"));
}

TTErr TTTimeProcess::Resume()
{
    return mScheduler->sendMessage(TTSymbol("Resume"));
}

TTErr TTTimeProcess::ReleaseEvents()
{
    TTMessagePtr    aMessage = NULL;
    TTErr           err;
    
    // Stop start event happening observation
    err = mStartEvent->findMessage(TTSymbol("Happen"), &aMessage);
    
    if(!err)
        aMessage->unregisterObserverForNotifications(*mStartEventCallback);
    
    // Release start event
    TTObjectBaseRelease(TTObjectBaseHandle(&mStartEvent));
    mStartEvent = NULL;
    
    // Stop end event happening observation
    err = mEndEvent->findMessage(TTSymbol("Happen"), &aMessage);
    
    if(!err)
        aMessage->unregisterObserverForNotifications(*mEndEventCallback);
    
    // Release end event
    TTObjectBaseRelease(TTObjectBaseHandle(&mEndEvent));
    mEndEvent = NULL;
    
    return kTTErrNone;
}

#if 0
#pragma mark -
#pragma mark Some Methods
#endif

TTErr TTTimeProcessStartEventHappenCallback(TTPtr baton, TTValue& data)
{
    TTTimeProcessPtr    aTimeProcess;
    TTValuePtr          b;
    TTValue             v;
    TTUInt32            start, end;
    
	// unpack baton (a time process)
	b = (TTValuePtr)baton;
	aTimeProcess = TTTimeProcessPtr((TTObjectBasePtr)(*b)[0]);
    
    // if the time process active
	if (aTimeProcess->mActive) {
        
        // close start event listening
        aTimeProcess->mStartEvent->setAttributeValue(kTTSym_active, NO);
        
        // use the specific start process method of the time process
        aTimeProcess->ProcessStart();
        
        // launch the scheduler
        aTimeProcess->mStartEvent->getAttributeValue(TTSymbol("date"), v);
        start = v[0];
        
        aTimeProcess->mEndEvent->getAttributeValue(TTSymbol("date"), v);
        end = v[0];
        
        if (end > start) {
            
            v = TTFloat64(end - start);
            
            aTimeProcess->mScheduler->setAttributeValue(TTSymbol("duration"), v);
            aTimeProcess->mScheduler->sendMessage(TTSymbol("Go"));
            
            return kTTErrNone;
        }
    }
    
    return kTTErrGeneric;
}

TTErr TTTimeProcessEndEventHappenCallback(TTPtr baton, TTValue& data)
{
    TTTimeProcessPtr    aTimeProcess;
    TTValuePtr          b;
    
	// unpack baton (a time process)
	b = (TTValuePtr)baton;
	aTimeProcess = TTTimeProcessPtr((TTObjectBasePtr)(*b)[0]);
    
    // if the time process active, stop the scheduler
    // note : the ProcessStart method is called inside TTTimeProcessSchedulerCallback
	if (aTimeProcess->mActive) {
        
        aTimeProcess->mScheduler->sendMessage(TTSymbol("Stop"));
        
        // close end trigger listening
        aTimeProcess->mEndEvent->setAttributeValue(kTTSym_active, NO);
        
        // use the specific process end method of the time process
        aTimeProcess->ProcessEnd();
        
        return kTTErrNone;
    }
    
    return kTTErrGeneric;
}

void TTTimeProcessSchedulerCallback(TTPtr object, TTFloat64 progression)
{
	TTTimeProcessPtr	aTimeProcess = (TTTimeProcessPtr)object;
    
    // use the specific process method
    aTimeProcess->Process(progression, kTTValNONE);
}