/** @file
 *
 * @ingroup scoreLibrary
 *
 * @brief a class to define an event
 *
 * @see TTTimeProcess
 *
 * @authors Théo de la Hogue & Clément Bossut
 *
 * @copyright Copyright © 2013, Théo de la Hogue & Clément Bossut @n
 * This code is licensed under the terms of the "CeCILL-C" @n
 * http://www.cecill.info
 */

#include "TTTimeEvent.h"
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlreader.h>

#define thisTTClass         TTTimeEvent
#define thisTTClassName     "TimeEvent"
#define thisTTClassTags     "time, event"

#if 0
#pragma mark -
#pragma mark Constructor / Destructor
#endif

TT_BASE_OBJECT_CONSTRUCTOR,
mName(kTTSymEmpty),
mDate(0),
mStatus(kTTSym_eventWaiting),
mMute(NO),
mState(kTTSym_Script),
mMinReachedProcessesCount(0),
mEndedProcessesCount(0),
mDisposedProcessesCount(0),
mRequestWait(NO),
mRequestHappen(NO),
mRequestDispose(NO),
mPushing(NO)
{
    TTValue none;
    
    TT_ASSERT("Correct number of args to create TTTimeEvent", arguments.size() == 1 || arguments.size() == 2);
    
    if (arguments.size() >= 1)
        mDate = arguments[0];

    if (arguments.size() == 2)
        mContainer = arguments[1];
    
    addAttribute(Container, kTypeObject);

    addAttribute(Name, kTypeSymbol);
   	addAttributeWithSetter(Date, kTypeUInt32);
    addAttribute(Mute, kTypeBoolean);
    addAttribute(State, kTypeObject);
    addAttributeWithSetter(Condition, kTypeObject);
    addAttributeWithSetter(Status, kTypeSymbol);
    
    addAttribute(AttachedProcesses, kTypeLocalValue);
    addAttributeProperty(AttachedProcesses, readOnly, YES);
    addAttributeProperty(AttachedProcesses, hidden, YES);
    
    addMessage(Wait);
    addMessage(Happen);
    addMessage(Dispose);
    addMessage(StatusUpdate);
    
    addMessageWithArguments(StateAddressGetValue);
    addMessageWithArguments(StateAddressSetValue);
    addMessageWithArguments(StateAddressClear);
    addMessage(StatePush);
    
    addMessageWithArguments(ProcessAttach);
    addMessageWithArguments(ProcessDetach);
    addMessageWithArguments(ProcessDurationMinReached);
    addMessageWithArguments(ProcessEnded);
    addMessageWithArguments(ProcessDisposed);
    
	// needed to be handled by a TTXmlHandler
	addMessageWithArguments(WriteAsXml);
	addMessageProperty(WriteAsXml, hidden, YES);
	addMessageWithArguments(ReadFromXml);
	addMessageProperty(ReadFromXml, hidden, YES);
    
    // generate a random name
    mName = mName.random();
}

TTTimeEvent::~TTTimeEvent()
{
    ;
}

#if 0
#pragma mark -
#pragma mark Accessors
#endif

TTErr TTTimeEvent::setDate(const TTValue& value)
{
    TT_ASSERT("TTTimeEvent::setDate : inputValue is correct", inputValue.size() == 1 && (inputValue[0].type() == kTypeInt32 || inputValue[0].type() == kTypeUInt32));
    
    TTUInt32 newDate = value[0];
    
    // filter repetitions
    if (newDate != mDate) {
        
        // set the internal date value
        mDate = newDate;
        
        // notify each date attribute observers
        sendNotification(kTTSym_EventDateChanged, TTObject(this));
    }
    
    return kTTErrNone;
}

TTErr TTTimeEvent::setCondition(const TTValue& value)
{
    TT_ASSERT("TTTimeEvent::setCondition : inputValue is correct", inputValue.size() == 1 && inputValue[0].type() == kTypeObject);
    
    TTObject newCondition = value[0];
    
    // filter repetitions
    if (newCondition != mCondition)
    {
        mCondition = newCondition;
        
        // notify each condition attribute observers
        TTValue v(TTObject(this), mCondition);
        sendNotification("EventConditionChanged", v);
    }
    
    return kTTErrNone;
}

TTErr TTTimeEvent::setStatus(const TTValue& value)
{
    if (mContainer.valid())
    {
        TTBoolean running;
        mContainer.get("running", running);
        if (running)
        {
            TTLogError("TTTimeEvent::setStatus %s : couldn't set status using direct accessor when the container is running\n", mName.c_str());
            return kTTErrGeneric;
        }
    }
    
    mStatus = value;
    
    // reset requests
    mRequestWait = NO;
    mRequestHappen = NO;
    mRequestDispose = NO;
    
    // reset counts
    mMinReachedProcessesCount = 0;
    mEndedProcessesCount = 0;
    mDisposedProcessesCount = 0;
    
    return kTTErrNone;
}

#if 0
#pragma mark -
#pragma mark Status Management
#endif

TTErr TTTimeEvent::Wait()
{
    // the wait request is sent so many times to reset a score that we filter repetitions here in this case
    if (mStatus == kTTSym_eventWaiting)
        return kTTErrNone;
    
    if (mRequestWait)
    {
        TTLogError("TTTimeEvent::Wait %s : this resquest is already registered\n", mName.c_str());
        return kTTErrGeneric;
    }
    
    mRequestWait = YES;
    
    // if the event have no container, update the status our self
    if (!mContainer.valid())
        return StatusUpdate();
    
    return  kTTErrNone;
}

TTErr TTTimeEvent::Happen()
{
    if (mStatus != kTTSym_eventWaiting && mStatus != kTTSym_eventPending)
    {
        TTLogError("TTTimeEvent::Happen %s : is not waiting or pending (%s)\n", mName.c_str(), mStatus.c_str());
        return kTTErrGeneric;
    }
    
    if (mRequestHappen)
    {
        TTLogError("TTTimeEvent::Happen %s : this resquest is already registered\n", mName.c_str());
        return kTTErrGeneric;
    }
    
    mRequestHappen = YES;
    
    // if the event have no container, update the status our self
    if (!mContainer.valid())
        return StatusUpdate();
    
    return kTTErrNone;
}

TTErr TTTimeEvent::Dispose()
{
    // if the event is already happened or disposed : do nothing
    if (mStatus == kTTSym_eventHappened)
    {
        TTLogError("TTTimeEvent::Dispose %s : is already happened\n", mName.c_str());
        return kTTErrGeneric;
    }
    
    if (mStatus == kTTSym_eventDisposed)
    {
        TTLogError("TTTimeEvent::Dispose %s : is already disposed\n", mName.c_str());
        return kTTErrGeneric;
    }
    
    if (mRequestDispose)
    {
        TTLogError("TTTimeEvent::Dispose %s : this resquest is already registered\n", mName.c_str());
        return kTTErrGeneric;
    }
    
    mRequestDispose = YES;
    
    // if the event have no container, update the status our self
    if (!mContainer.valid())
        return StatusUpdate();
    
    return kTTErrNone;
}

TTErr TTTimeEvent::StatusUpdate()
{
    // if there is a request to make the event to wait
    if (mRequestWait)
    {
        mRequestWait = NO;
        return applyStatus(kTTSym_eventWaiting);
    }
    
    // if there is a request to make the event happen
    if (mRequestHappen)
    {
        mRequestHappen = NO;

        if (StatePush())
            TTLogError("TTTimeEvent::StatusUpdate %s : StatePush error\n", mName.c_str());
        
        return applyStatus(kTTSym_eventHappened);
    }
    
    // if there is a request to dispose the event
    if (mRequestDispose)
    {
        mRequestDispose = NO;
        return applyStatus(kTTSym_eventDisposed);
    }
    
    // any event with attached processes
    if (mAttachedProcesses.size() != 0)
    {
        // a conditioned event becomes pending when all attached processes have reached their minimal duration bound
        if (mCondition.valid() &&
            mStatus == kTTSym_eventWaiting &&
            mMinReachedProcessesCount == mAttachedProcesses.size())
        {
            return applyStatus(kTTSym_eventPending);
        }
        
        // when all attached processes are ended or disposed
        if (mEndedProcessesCount + mDisposedProcessesCount == mAttachedProcesses.size())
        {
            // a conditioned pending event forces its condition to apply its default case
            if (mCondition.valid() &&
                mStatus == kTTSym_eventPending)
            {
                return mCondition.send("Default");
            }
            // a non conditioned waiting event happens
            else if (!mCondition.valid() &&
                     mStatus == kTTSym_eventWaiting)
            {
                if (StatePush())
                    TTLogError("TTTimeEvent::StatusUpdate %s : StatePush error\n", mName.c_str());
                
                return applyStatus(kTTSym_eventHappened);
            }
        }
        
        // an event is disposed when all attached processes are disposed
        if (mStatus == kTTSym_eventWaiting &&
            mDisposedProcessesCount == mAttachedProcesses.size())
        {
            return applyStatus(kTTSym_eventDisposed);
        }
    }
    
    // waiting event with no attached process
    else if (mStatus == kTTSym_eventWaiting)
    {
        // set conditionned event as pending
        if (mCondition.valid())
        {
            return applyStatus(kTTSym_eventPending);
        }
        // or make none conditioned event to happen at its date
        else if (mContainer.valid())
        {
            TTValue v;
            mContainer.get("date", v);
            TTUInt32 containerDate = v[0];
            
            if (mDate <= containerDate)
            {
                if (StatePush())
                    TTLogError("TTTimeEvent::StatusUpdate %s : StatePush error\n", mName.c_str());
                
                return applyStatus(kTTSym_eventHappened);
            }
        }
    }
    
    return kTTErrGeneric;
}

TTErr TTTimeEvent::applyStatus(const TTValue& value)
{
    TT_ASSERT("TTTimeEvent::applyStatus : inputValue is correct", inputValue.size() == 1 && inputValue[0].type() == kTypeSymbol);
    
    TTSymbol lastStatus = mStatus;
    
    // set status
    mStatus = value[0];
    
    // log error if conflicted request are detected
    if (mRequestWait + mRequestHappen + mRequestDispose > 1)
        TTLogError("TTTimeEvent::applyStatus %s : at %d requests have been received\n", mName.c_str(), mRequestWait + mRequestHappen + mRequestDispose);
    
    // reset requests
    mRequestWait = NO;
    mRequestHappen = NO;
    mRequestDispose = NO;
    
    // reset counts if the event is not pending
    if (mStatus != kTTSym_eventPending)
    {
        mMinReachedProcessesCount = 0;
        mEndedProcessesCount = 0;
        mDisposedProcessesCount = 0;
    }
    
    // filter repetitions
    if (lastStatus == mStatus)
    {
        // log error only for non waiting status repetition
        //if (mStatus != kTTSym_eventWaiting)
            TTLogError("TTTimeEvent::applyStatus %s : new status equals last status (%s)\n", mName.c_str(), mStatus.c_str());
        
        return kTTErrGeneric;
    }
    
    // notify observers
    TTValue v = TTObject(this);
    v.append(mStatus);
    v.append(lastStatus);
#ifdef TTSCORE_DEBUG
    TTLogMessage("TTTimeEvent::applyStatus %s : %s -> %s\n", mName.c_str(), lastStatus.c_str(), mStatus.c_str());
#endif
    return sendNotification(kTTSym_EventStatusChanged, v);
}

#if 0
#pragma mark -
#pragma mark State Management
#endif

TTErr TTTimeEvent::StateAddressGetValue(const TTValue& inputValue, TTValue& outputValue)
{
    TTValue         v;
    TTAddress       address;
    TTListPtr       lines;
    TTDictionaryBasePtr aLine;
    TTErr           err;
    
    if (inputValue.size() == 1) {
        
        if (inputValue[0].type() == kTypeSymbol) {
            
            address = inputValue[0];
            
            // get the lines of the state
            mState.get(kTTSym_lines, v);
            lines = TTListPtr(TTPtr(v[0]));
            
            // find the line at address
            err = lines->find(&TTScriptFindAddress, (TTPtr)&address, v);
            
            if (err)
                return err;
            
            aLine = TTDictionaryBasePtr((TTPtr)v[0]);
            
            // get the value
            aLine->getValue(outputValue);
            
            return  kTTErrNone;
        }
    }
    
    return kTTErrGeneric;
}

TTErr TTTimeEvent::StateAddressSetValue(const TTValue& inputValue, TTValue& outputValue)
{
    TTValue         v, command;
    TTAddress       address;
    TTValuePtr      aValue;
    TTListPtr       flattenedLines;
    TTDictionaryBasePtr aLine;
    TTErr           err;
    
    if (inputValue.size() == 2) {
        
        if (inputValue[0].type() == kTypeSymbol && inputValue[1].type() == kTypePointer) {
            
            address = inputValue[0];
            aValue = TTValuePtr(TTPtr(inputValue[1]));
            
            // get the lines of the state
            mState.get("flattenedLines", v);
            flattenedLines = TTListPtr(TTPtr(v[0]));
            
            // find the line at address
            err = flattenedLines->find(&TTScriptFindAddress, (TTPtr)&address, v);
            
            // if the line doesn't exist : append it to the state
            if (err) {
                
                command = *aValue;
                command.prepend(address);
                
                mState.send("AppendCommand", command, v);
            }
            else {
            
                aLine = TTDictionaryBasePtr((TTPtr)v[0]);
            
                // set the value
                aLine->setValue(*aValue);
            }
            
            return  kTTErrNone;
        }
    }
    
    return kTTErrGeneric;
}

TTErr TTTimeEvent::StateAddressClear(const TTValue& inputValue, TTValue& outputValue)
{
    TTValue none;

    if (inputValue.size() == 1) {
        
        if (inputValue[0].type() == kTypeSymbol) {
            
            // remove the lines of the state
            return mState.send("RemoveCommand", inputValue, none);
        }
    }
    
    return kTTErrGeneric;
}

TTErr TTTimeEvent::StatePush()
{
    if (mMute)
        return kTTErrNone;
    
    if (!mPushing)
    {
        mPushing = YES;

        // recall the state
        TTErr err = mState.send(kTTSym_Run);
        
        mPushing = NO;
        
        return err;
    }
    
    return kTTErrGeneric;
}

#if 0
#pragma mark -
#pragma mark Process Management
#endif

TTErr TTTimeEvent::ProcessAttach(const TTValue& inputValue, TTValue& outputValue)
{
    if (inputValue.size() == 1) {
        
        if (inputValue[0].type() == kTypeObject) {
            
            TTObject thisObject(this);
            TTObject aTimeProcess = inputValue[0];
            
            if (aTimeProcess.valid())
            {
                // check if the time process is not already attached
                TTValue     newAttachedProcesses;
                TTBoolean   found = false;
                
                for (TTElementIter it = mAttachedProcesses.begin() ; it != mAttachedProcesses.end() ; it++)
                {
                    TTObject attachedProcess = TTElement(*it);
                    if (attachedProcess == aTimeProcess)
                        found = true;
                    
                    newAttachedProcesses.append(attachedProcess);
                }
                
                if (!found) {
                    
                    // start process observation
                    aTimeProcess.registerObserverForNotifications(thisObject);
                    
                    // update the attached processes
                    newAttachedProcesses.append(aTimeProcess);
                    mAttachedProcesses = newAttachedProcesses;
                    
                    return kTTErrNone;
                }
            }
        }
    }
    
    return kTTErrGeneric;
}

TTErr TTTimeEvent::ProcessDetach(const TTValue& inputValue, TTValue& outputValue)
{
    if (inputValue.size() == 1)
    {
        if (inputValue[0].type() == kTypeObject)
        {
            TTObject thisObject(this);
            TTObject aTimeProcess = inputValue[0];
            
            if (aTimeProcess.valid())
            {
                // stop process observation
                aTimeProcess.unregisterObserverForNotifications(thisObject);
                
                // update the attached processes
                TTValue newAttachedProcesses;
 
                for (TTElementIter it = mAttachedProcesses.begin() ; it != mAttachedProcesses.end() ; it++)
                {
                    TTObject attachedProcess = TTElement(*it);
                    if (attachedProcess != aTimeProcess)
                        newAttachedProcesses.append(attachedProcess);
                }
                
                mAttachedProcesses = newAttachedProcesses;
                
                return kTTErrNone;
            }
        }
    }
    
    return kTTErrGeneric;
}

#if 0
#pragma mark -
#pragma mark Notifications
#endif

TTErr TTTimeEvent::ProcessDurationMinReached(const TTValue& inputValue, TTValue& outputValue)
{
    TT_ASSERT("TTTimeEvent::ProcessDurationMinReached : inputValue is correct", inputValue.size() == 1 && inputValue[0].type() == kTypeObject);
    
    // update count
    mMinReachedProcessesCount++;
#ifdef TTSCORE_DEBUG
    TTLogMessage("TTTimeEvent::ProcessDurationMinReached %s : attached = %d, minReached = %d, ended = %d, disposed = %d\n", mName.c_str(), mAttachedProcesses.size(), mMinReachedProcessesCount, mEndedProcessesCount, mDisposedProcessesCount);
#endif
    return kTTErrNone;
}

TTErr TTTimeEvent::ProcessEnded(const TTValue& inputValue, TTValue& outputValue)
{
    TT_ASSERT("TTTimeEvent::ProcessEnded : inputValue is correct", inputValue.size() == 1 && inputValue[0].type() == kTypeObject);
    
    // update count
    mEndedProcessesCount++;
#ifdef TTSCORE_DEBUG
    TTLogMessage("TTTimeEvent::ProcessEnded %s : attached = %d, minReached = %d, ended = %d, disposed = %d\n", mName.c_str(), mAttachedProcesses.size(), mMinReachedProcessesCount, mEndedProcessesCount, mDisposedProcessesCount);
#endif
    return kTTErrNone;
}

TTErr TTTimeEvent::ProcessDisposed(const TTValue& inputValue, TTValue& outputValue)
{
    TT_ASSERT("TTTimeEvent::ProcessDisposed : inputValue is correct", inputValue.size() == 1 && inputValue[0].type() == kTypeObject);
    
    // update count
    mDisposedProcessesCount++;
#ifdef TTSCORE_DEBUG
    TTLogMessage("TTTimeEvent::ProcessDisposed %s : attached = %d, minReached = %d, ended = %d, disposed = %d\n", mName.c_str(), mAttachedProcesses.size(), mMinReachedProcessesCount, mEndedProcessesCount, mDisposedProcessesCount);
#endif
    return kTTErrNone;
}

TTErr TTTimeEvent::WriteAsXml(const TTValue& inputValue, TTValue& outputValue)
{
    TTObject o = inputValue[0];
	TTXmlHandlerPtr aXmlHandler = (TTXmlHandlerPtr)o.instance();
    if (!aXmlHandler)
		return kTTErrGeneric;
    
    TTValue     v;
    TTString    s;
	
    // Write the date
    v = mDate;
    v.toString();
    s = TTString(v[0]);
    xmlTextWriterWriteAttribute((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "date", BAD_CAST s.data());
    
    // Write the mute
    v = mMute;
    v.toString();
    s = TTString(v[0]);
    xmlTextWriterWriteAttribute((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "mute", BAD_CAST s.data());
    
    // Write the name of the condition object
    if (mCondition.valid()) {
        
        mCondition.get(kTTSym_name, v);
        v.toString();
        s = TTString(v[0]);
        xmlTextWriterWriteAttribute((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "condition", BAD_CAST s.data());
    }
    
    // Write the state
    aXmlHandler->setAttributeValue(kTTSym_object, mState);
    aXmlHandler->sendMessage(kTTSym_Write);
    
	return kTTErrNone;
}

TTErr TTTimeEvent::ReadFromXml(const TTValue& inputValue, TTValue& outputValue)
{
    TTObject o = inputValue[0];
	TTXmlHandlerPtr aXmlHandler = (TTXmlHandlerPtr)o.instance();
    if (!aXmlHandler)
		return kTTErrGeneric;
    
    TTValue v;
	
    // Event node
    if (aXmlHandler->mXmlNodeName == kTTSym_event) {
        
        // get the date
        if (!aXmlHandler->getXmlAttribute(kTTSym_date, v, NO)) {
            
            if (v.size() == 1) {
                
                if (v[0].type() == kTypeUInt32) {
                    
                    this->setDate(v);
                }
            }
        }
        
        // get the mute
        if (!aXmlHandler->getXmlAttribute(kTTSym_mute, v, NO)) {
            
            if (v.size() == 1) {
                
                if (v[0].type() == kTypeInt32) {
                    
                    this->mMute = v[0];
                }
            }
        }
        
        // get the condition object name
        if (!aXmlHandler->getXmlAttribute(kTTSym_condition, v, YES)) {
            
            if (v.size() == 1) {
                
                if (v[0].type() == kTypeSymbol) {
                    
                    // do nothing
                    // the time condition will be registered after so we can't link the time event to an unexisting object
                }
            }
        }
    }
    
    // Command node
    if (aXmlHandler->mXmlNodeName == kTTSym_command) {
        
        // Pass the xml handler to the current state to fill his data structure
        aXmlHandler->setAttributeValue(kTTSym_object, mState);
        return aXmlHandler->sendMessage(kTTSym_Read);
    }
	
	return kTTErrNone;
}

#if 0
#pragma mark -
#pragma mark Some Methods
#endif

TTBoolean TTSCORE_EXPORT TTTimeEventCompareDate(TTValue& v1, TTValue& v2)
{
    TTObject timeEvent1 = v1[0];
    TTObject timeEvent2 = v2[0];
    
    return TTTimeEventPtr(timeEvent1.instance())->mDate < TTTimeEventPtr(timeEvent2.instance())->mDate;
}
