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

/****************************************************************************************************/

TT_BASE_OBJECT_CONSTRUCTOR,
mName(kTTSymEmpty),
mDate(0),
mStatus(kTTSym_eventWaiting),
mMute(NO),
mState(kTTSym_Script),
mStartedProcessesCount(0),
mEndedProcessesCount(0),
mDisposedProcessesCount(0),
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
    
    addMessage(Trigger);
    addMessage(Happen);
    addMessage(Dispose);
    addMessageWithArguments(StateAddressGetValue);
    addMessageWithArguments(StateAddressSetValue);
    addMessageWithArguments(StateAddressClear);
    addMessage(StatePush);
    
    addMessageWithArguments(ProcessAttach);
    addMessageWithArguments(ProcessDetach);
    addMessageWithArguments(ProcessStarted);
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
    TT_ASSERT("TTTimeEvent::setStatus : inputValue is correct", inputValue.size() == 1 && inputValue[0].type() == kTypeSymbol);
    
    TTSymbol    lastStatus = mStatus;
    TTValue     v = TTObject(this);
    
    // set status
    mStatus = value[0];
    
    // filter repetitions
    if (lastStatus == mStatus)
    {
        // log error only for non waiting status repetition
        if (mStatus != kTTSym_eventWaiting)
            TTLogError("TTTimeEvent::setStatus %s : new status equals last status (%s)\n", mName.c_str(), mStatus.c_str());
        
        return kTTErrGeneric;
    }
    
    // reset counts if the event is not pending
    if (mStatus != kTTSym_eventPending)
    {
        mStartedProcessesCount = 0;
        mEndedProcessesCount = 0;
        mDisposedProcessesCount = 0;
    }
    
    // is the container running ? (the notification is sent if there is no valid container)
    TTBoolean running = YES;
    if (mContainer.valid())
        mContainer.get(kTTSym_running, running);
    
    if (running)
    {
        // notify each attribute observers
        v.append(mStatus);
        v.append(lastStatus);
#ifdef TTSCORE_DEBUG
        TTLogMessage("TTTimeEvent::setStatus %s : %s -> %s\n", mName.c_str(), lastStatus.c_str(), mStatus.c_str());
#endif
        return sendNotification(kTTSym_EventStatusChanged, v);
    }
#ifdef TTSCORE_DEBUG
    TTLogMessage("TTTimeEvent::setStatus %s : don't notify %s status because the container is not running\n", mName.c_str(), mStatus.c_str());
#endif
    return kTTErrNone;
}

TTErr TTTimeEvent::Trigger()
{
    // this method is deprecated : use Happen
    return Happen();
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
    
    return setStatus(kTTSym_eventDisposed);
}

TTErr TTTimeEvent::Happen()
{
    TTErr err;
    
    if (mStatus != kTTSym_eventWaiting && mStatus != kTTSym_eventPending)
    {
        TTLogError("TTTimeEvent::Happen %s : is not waiting or pending (%s)\n", mName.c_str(), mStatus.c_str());
        return kTTErrGeneric;
    }
    
    // if the event is not muted
    if (!mMute)
    {
        // push the state
        err = StatePush();
    }
    else
        err = kTTErrNone;
    
    setStatus(kTTSym_eventHappened);
    return err;
}

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
    if (!mPushing) {

        mPushing = YES;

        // recall the state
        TTErr err = mState.send(kTTSym_Run);
        
        mPushing = NO;
        
        return err;
    }
    
    return kTTErrGeneric;
}

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
                
                for (TTUInt32 i = 0; i < mAttachedProcesses.size(); i++)
                {
                    TTObject attachedProcess = mAttachedProcesses[i];
                    if (attachedProcess == aTimeProcess)
                        found = true;
                    
                    newAttachedProcesses.append(mAttachedProcesses[i]);
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
 
                for (TTUInt32 i = 0; i < mAttachedProcesses.size(); i++)
                {
                    TTObject attachedProcess = mAttachedProcesses[i];
                    if (attachedProcess != aTimeProcess)
                        newAttachedProcesses.append(mAttachedProcesses[i]);
                }
                
                mAttachedProcesses = newAttachedProcesses;
                
                return kTTErrNone;
            }
        }
    }
    
    return kTTErrGeneric;
}

TTErr TTTimeEvent::ProcessStarted(const TTValue& inputValue, TTValue& outputValue)
{
    TT_ASSERT("TTTimeProcess::ProcessStarted : inputValue is correct", inputValue.size() == 1 && inputValue[0].type() == kTypeObject);
    
    TTObject aTimeProcess = inputValue[0];
    
    // update count
    mStartedProcessesCount++;
#ifdef TTSCORE_DEBUG
    TTLogMessage("TTTimeEvent::ProcessStarted %s : attached = %d, started = %d, ended = %d, disposed = %d\n", mName.c_str(), mAttachedProcesses.size(), mStartedProcessesCount, mEndedProcessesCount, mDisposedProcessesCount);
#endif
    // a conditioned event becomes pending when all attached processes are started
    if (mCondition.valid() &&
        mStatus == kTTSym_eventWaiting &&
        mStartedProcessesCount == mAttachedProcesses.size())
    {
        return setStatus(kTTSym_eventPending);
    }
    
    return kTTErrNone;
}

TTErr TTTimeEvent::ProcessEnded(const TTValue& inputValue, TTValue& outputValue)
{
    TT_ASSERT("TTTimeProcess::ProcessEnded : inputValue is correct", inputValue.size() == 1 && inputValue[0].type() == kTypeObject);
    
    TTObject aTimeProcess = inputValue[0];
    TTObject startEvent;
    TTSymbol startEventStatus;
    
    // update count
    mEndedProcessesCount++;
#ifdef TTSCORE_DEBUG
    TTLogMessage("TTTimeEvent::ProcessEnded %s : attached = %d, started = %d, ended = %d, disposed = %d\n", mName.c_str(), mAttachedProcesses.size(), mStartedProcessesCount, mEndedProcessesCount, mDisposedProcessesCount);
#endif
    // when all attached processes are ended or disposed
    if (mEndedProcessesCount + mDisposedProcessesCount == mAttachedProcesses.size())
    {
        // a conditioned pending event forces its condition to apply its default case
        if (mCondition.valid() &&
            mStatus == kTTSym_eventPending)
        {
            // théo : the use of Trigger is temporary because there is no method to apply default case
            // théo : commenting out the line below because the TimeProcess::Play method does'nt set the scheduler correctly for now
            //return mCondition.send(kTTSym_Trigger);
        }
        // a non conditioned waiting event happens
        else if (!mCondition.valid() &&
                 mStatus == kTTSym_eventWaiting)
        {
            return Happen();
        }
    }

    return kTTErrNone;
}

TTErr TTTimeEvent::ProcessDisposed(const TTValue& inputValue, TTValue& outputValue)
{
    TT_ASSERT("TTTimeProcess::ProcessDisposed : inputValue is correct", inputValue.size() == 1 && inputValue[0].type() == kTypeObject);
    
    TTObject aTimeProcess = inputValue[0];
    
    // update count
    mDisposedProcessesCount++;
#ifdef TTSCORE_DEBUG
    TTLogMessage("TTTimeEvent::ProcessDisposed %s : attached = %d, started = %d, ended = %d, disposed = %d\n", mName.c_str(), mAttachedProcesses.size(), mStartedProcessesCount, mEndedProcessesCount, mDisposedProcessesCount);
#endif
    // an event is disposed when all attached processes are disposed
    if (mStatus == kTTSym_eventWaiting &&
        mDisposedProcessesCount == mAttachedProcesses.size())
    {
        return Dispose();
    }
    
    // a non conditioned event happens when all attached processes are ended or disposed
    if (!mCondition.valid() &&
        mStatus == kTTSym_eventWaiting &&
        mEndedProcessesCount + mDisposedProcessesCount == mAttachedProcesses.size())
    {
        return Happen();
    }
    
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
