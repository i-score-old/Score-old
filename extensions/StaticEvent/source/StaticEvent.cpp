/*
 * StaticEvent time event
 * Copyright © 2013, Théo de la Hogue, Clément Bossut
 *
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

/*!
 * \class StaticEvent
 *
 *  StaticEvent time event class handle a callback to use it at a planned date
 *
 */

#include "StaticEvent.h"

#define thisTTClass                 StaticEvent
#define thisTTClassName             "StaticEvent"
#define thisTTClassTags             "time, event, static"

#define thisTimeEventVersion		"0.1"
#define thisTimeEventAuthor        "Theo de la Hogue"

extern "C" TT_EXTENSION_EXPORT TTErr TTLoadJamomaExtension_StaticEvent(void)
{
	TTFoundationInit();
	StaticEvent::registerClass();
	return kTTErrNone;
}

TIME_EVENT_CONSTRUCTOR
{
    TIME_EVENT_INITIALIZE
    
	TT_ASSERT("Correct number of args to create StaticEvent", arguments.size() == 0);
	
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
}

StaticEvent::~StaticEvent()
{
    ;
}

TTErr StaticEvent::getParameterNames(TTValue& value)
{
    value.clear();
	//value.append(TTSymbol("aParameterName"));
	
	return kTTErrNone;
}

TTErr StaticEvent::Trigger(const TTValue& inputValue, TTValue& outputValue)
{
    // append the triggered value to the trigger list
    mTriggerList.append(inputValue);
    
    return kTTErrNone;
}

TTErr StaticEvent::Notify()
{
    // if there is no triggered value : don't notify any subscriber
    if (mTriggerList.isEmpty())
        return kTTErrGeneric;
    
    TTObjectBasePtr aCallback;
    
    // notify all subscriber using the first triggered value
    for (mSubscriberList.begin(); mSubscriberList.end(); mSubscriberList.next()) {
        
        aCallback = mSubscriberList.current()[0];
        
        aCallback->sendMessage(kTTSym_notify, mTriggerList.getHead(), kTTValNONE);
    }
    
    // clear the trigger list
    mTriggerList.clear();
    
    return kTTErrNone;
}


TTErr StaticEvent::WriteAsXml(const TTValue& inputValue, TTValue& outputValue)
{
	TTXmlHandlerPtr	aXmlHandler = NULL;
	
	aXmlHandler = TTXmlHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// TODO : write the time event attributes
	
	return kTTErrGeneric;
}

TTErr StaticEvent::ReadFromXml(const TTValue& inputValue, TTValue& outputValue)
{
	TTXmlHandlerPtr	aXmlHandler = NULL;
	
	aXmlHandler = TTXmlHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// TODO : parse the time event attributes
	
	return kTTErrGeneric;
}

TTErr StaticEvent::WriteAsText(const TTValue& inputValue, TTValue& outputValue)
{
	TTTextHandlerPtr	aTextHandler;
	
	aTextHandler = TTTextHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// TODO : write the time event attributes
	
	return kTTErrGeneric;
}

TTErr StaticEvent::ReadFromText(const TTValue& inputValue, TTValue& outputValue)
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
