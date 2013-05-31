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

TTErr StaticEvent::Trigger()
{
    // do nothing : a static event can't be triggered
    return kTTErrGeneric;
}

TTErr StaticEvent::Happen()
{
    // recall the state
    mState->sendMessage(TTSymbol("Run"));
    
    // notify observers
    happenMessage->sendNotification(kTTSym_notify, kTTValNONE);	// we use kTTSym_notify because we know that observers are TTCallback
    
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
