/*
 * InteractiveEvent time event
 * Copyright © 2013, Théo de la Hogue, Clément Bossut
 *
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

/*!
 * \class InteractiveEvent
 *
 *  InteractiveEvent time event class handle a callback to use it when it receive a value from an address
 *
 */

#include "InteractiveEvent.h"

#define thisTTClass                 InteractiveEvent
#define thisTTClassName             "InteractiveEvent"
#define thisTTClassTags             "time, event, interactive"

extern "C" TT_EXTENSION_EXPORT TTErr TTLoadJamomaExtension_InteractiveEvent(void)
{
	TTFoundationInit();
	InteractiveEvent::registerClass();
	return kTTErrNone;
}

TIME_EVENT_CONSTRUCTOR,
mAddress(kTTAdrsEmpty),
mReceiver(NULL)
{
    TIME_EVENT_INITIALIZE
    
	TT_ASSERT("Correct number of args to create InteractiveEvent", arguments.size() == 0);
    
    addAttributeWithSetter(Address, kTypeSymbol);
    
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
}

InteractiveEvent::~InteractiveEvent()
{
    if (mReceiver) {
        TTObjectBaseRelease(TTObjectBaseHandle(&mReceiver));
        mReceiver = NULL;
    }
}

TTErr InteractiveEvent::getParameterNames(TTValue& value)
{
    value.clear();
	value.append(TTSymbol("address"));
	
	return kTTErrNone;
}

TTErr InteractiveEvent::Trigger()
{
    // inside a scenario
    if (mScenario)
        
        // use scenario to make the event happen
        mScenario->sendMessage(TTSymbol("TimeEventTrigger"), TTObjectBasePtr(this), kTTValNONE);
    
    else
        
        // otherwise make it happens now
        Happen();
    
    return kTTErrNone;
}

TTErr InteractiveEvent::Happen()
{    
    // recall the state
    mState->sendMessage(TTSymbol("Run"));
    
    // notify observers
    happenMessage->sendNotification(kTTSym_notify, kTTValNONE);	// we use kTTSym_notify because we know that observers are TTCallback
    
    return kTTErrNone;
}

TTErr InteractiveEvent::setAddress(const TTValue& value)
{
    if (value.size() == 1) {
        if (value[0].type() == kTypeSymbol) {
            
            mAddress = value[0];
            
            // if the receiver exist, set the address to listen
            if (mReceiver)
                return mReceiver->setAttributeValue(kTTSym_address, mAddress);
            
        }
    }
    
    return kTTErrGeneric;
}

TTErr InteractiveEvent::WriteAsXml(const TTValue& inputValue, TTValue& outputValue)
{
	TTXmlHandlerPtr	aXmlHandler = NULL;
	
	aXmlHandler = TTXmlHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// TODO : write the time event attributes
	
	return kTTErrGeneric;
}

TTErr InteractiveEvent::ReadFromXml(const TTValue& inputValue, TTValue& outputValue)
{
	TTXmlHandlerPtr	aXmlHandler = NULL;
	
	aXmlHandler = TTXmlHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// TODO : parse the time event attributes
	
	return kTTErrGeneric;
}

TTErr InteractiveEvent::WriteAsText(const TTValue& inputValue, TTValue& outputValue)
{
	TTTextHandlerPtr	aTextHandler;
	
	aTextHandler = TTTextHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// TODO : write the time event attributes
	
	return kTTErrGeneric;
}

TTErr InteractiveEvent::ReadFromText(const TTValue& inputValue, TTValue& outputValue)
{
	TTTextHandlerPtr aTextHandler;
	TTValue	v;
	
	aTextHandler = TTTextHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
    // TODO : parse the time event attributes
	
	return kTTErrGeneric;
}

#if 0
#pragma mark -
#pragma mark Some Methods
#endif

TTErr InteractiveEventReceiverCallback(TTPtr baton, TTValue& data)
{
    TimeEventPtr    aTimeEvent;
	TTValuePtr      b;
	
	// unpack baton (a scenario and a time process)
	b = (TTValuePtr)baton;
	aTimeEvent = TimeEventPtr((TTObjectBasePtr)(*b)[0]);
    
    // trigger the event
    return aTimeEvent->Trigger();
}
