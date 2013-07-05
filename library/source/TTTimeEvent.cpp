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

#define thisTTClass         TTTimeEvent
#define thisTTClassName     "TimeEvent"
#define thisTTClassTags     "time, event"

/****************************************************************************************************/

TT_BASE_OBJECT_CONSTRUCTOR,
mContainer(NULL),
mDate(0),
mState(NULL),
mInteractive(NO),
mReady(YES)
{
    TT_ASSERT("Correct number of args to create TTTimeEvent", arguments.size() == 1 || arguments.size() == 2);
    
    if (arguments.size() >= 1)
        mDate = arguments[0];

    if (arguments.size() == 2)
        mContainer = arguments[1];

   	addAttributeWithSetter(Date, kTypeUInt32);
    addAttribute(State, kTypeObject);
    addAttributeWithSetter(Interactive, kTypeBoolean);
    addAttributeWithSetter(Ready, kTypeBoolean);
    
    addMessage(Trigger);
    addMessage(Happen);
    addMessageWithArguments(StateAddressGetValue);
    
	// needed to be handled by a TTXmlHandler
	addMessageWithArguments(WriteAsXml);
	addMessageProperty(WriteAsXml, hidden, YES);
	addMessageWithArguments(ReadFromXml);
	addMessageProperty(ReadFromXml, hidden, YES);
	
    // cache some messages and attributes for high speed notification feedbacks
    this->findAttribute(TTSymbol("date"), &dateAttribute);
    this->findAttribute(TTSymbol("ready"), &readyAttribute);
    this->findMessage(TTSymbol("Happen"), &happenMessage);
    
    TTObjectBaseInstantiate(kTTSym_Script, TTObjectBaseHandle(&mState), kTTValNONE);
    
    /* TODO : move this in i-score by creating a collection of receiver which will call the Trigger method of a time event

    // Create a TTReceiver to listen any address
    TTValue			args;
	TTObjectBasePtr	returnValueCallback;
	TTValuePtr		returnValueBaton;
    
    // we don't need the address back
    args.append(NULL);
    
    // but we need the value back to trigger it (using the InteractiveEventReceiverCallback function)
    returnValueCallback = NULL;
    TTObjectBaseInstantiate(TTSymbol("callback"), &returnValueCallback, kTTValNONE);
    returnValueBaton = new TTValue(TTObjectBasePtr(this));
    returnValueCallback->setAttributeValue(kTTSym_baton, TTPtr(returnValueBaton));
    returnValueCallback->setAttributeValue(kTTSym_function, TTPtr(&InteractiveEventReceiverCallback));
    args.append(returnValueCallback);
    
    mReceiver = NULL;
    TTObjectBaseInstantiate(kTTSym_Receiver, TTObjectBaseHandle(&mReceiver), args);
    
    */
}

TTTimeEvent::~TTTimeEvent()
{
    if (mState) {
        TTObjectBaseRelease(TTObjectBaseHandle(&mState));
        mState = NULL;
    }
}

TTErr TTTimeEvent::setDate(const TTValue& value)
{
    TTUInt32 newDate = value[0];
    
    // filter repetitions
    if (newDate != mDate) {
        
        // set the internal active value
        mDate = newDate;
        
        // notify each attribute observers
        dateAttribute->sendNotification(kTTSym_notify, mDate);             // we use kTTSym_notify because we know that observers are TTCallback
    }
    
    return kTTErrNone;
}

TTErr TTTimeEvent::setInteractive(const TTValue& value)
{
    // set the interactive value
    mInteractive = value[0];
    
    // tell the container the event is becoming (or not) interactive
    if (mContainer) {
        
        TTValue v = TTObjectBasePtr(this);
        v.append(mInteractive);
        return mContainer->sendMessage(TTSymbol("TimeEventInteractive"), v, kTTValNONE);
    }

    return kTTErrNone;
}

TTErr TTTimeEvent::setReady(const TTValue& value)
{
    // set the ready value
    mReady = value[0];
    
    // notify each attribute observers
    readyAttribute->sendNotification(kTTSym_notify, mReady);             // we use kTTSym_notify because we know that observers are TTCallback
    
    return kTTErrNone;
}

TTErr TTTimeEvent::Trigger()
{
    // if not ready : do nothing
    if (!mReady)
        return kTTErrGeneric;
    
    // if not interactive : do nothing
    if (!mInteractive)
        return kTTErrGeneric;
    
    // use container to make the event happen
    if (mContainer) {
        
        TTValue v = TTObjectBasePtr(this);
        return mContainer->sendMessage(TTSymbol("TimeEventTrigger"), v, kTTValNONE);
    }
    
    // otherwise make it happens now
    else
        return Happen();
}

TTErr TTTimeEvent::Happen()
{
    // recall the state
    TTErr err = mState->sendMessage(TTSymbol("Run"));
    
    // notify observers
    happenMessage->sendNotification(kTTSym_notify, kTTValNONE);	// we use kTTSym_notify because we know that observers are TTCallback
    
    return err;
}

TTErr TTTimeEvent::StateAddressGetValue(const TTValue& inputValue, TTValue& outputValue)
{
    TTValue         v;
    TTAddress       address;
    TTListPtr       lines;
    TTDictionaryPtr aLine;
    TTErr           err;
    
    if (inputValue.size() == 1) {
        
        if (inputValue[0].type() == kTypeSymbol) {
            
            address = inputValue[0];
            
            // get the lines of the state
            mState->getAttributeValue(TTSymbol("lines"), v);
            lines = TTListPtr(TTPtr(v[0]));
            
            // find the line at address
            err = lines->find(&TTScriptFindAddress, (TTPtr)&address, v);
            
            if (err)
                return err;
            
            aLine = TTDictionaryPtr((TTPtr)v[0]);
            
            // get the start value
            aLine->getValue(outputValue);
            
            return  kTTErrNone;
        }
    }
    
    return kTTErrGeneric;
}

TTErr TTTimeEvent::WriteAsXml(const TTValue& inputValue, TTValue& outputValue)
{
	TTXmlHandlerPtr	aXmlHandler = NULL;
	
	aXmlHandler = TTXmlHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// TODO : write the time event attributes
	
	return kTTErrGeneric;
}

TTErr TTTimeEvent::ReadFromXml(const TTValue& inputValue, TTValue& outputValue)
{
	TTXmlHandlerPtr	aXmlHandler = NULL;
	
	aXmlHandler = TTXmlHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// TODO : parse the time event attributes
	
	return kTTErrGeneric;
}