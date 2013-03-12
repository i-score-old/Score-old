/*
 * A Time Event interface
 * Copyright © 2013, Théo de la Hogue
 *
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#include "TimeEvent.h"

#define thisTTClass		TimeEvent

/****************************************************************************************************/

TimeEvent::TimeEvent(TTValue& arguments) :
TTObjectBase(arguments),
mDate(0),
mCallback(NULL)
{
    TT_ASSERT("Correct number of args to create Event", arguments.size() == 1);
    
    if (arguments.size() >= 1)
		mCallback = TTCallbackPtr((TTObjectBasePtr)arguments[0]);

   	addAttributeWithSetter(Date, kTypeUInt32);
    
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
    
    // cache some attributes for high speed notification feedbacks
    this->findAttribute(TTSymbol("date"), &dateAttribute);
}

TimeEvent::~TimeEvent()
{
    TTValue v;
    
    if (mCallback) {
        TTObjectBaseRelease(TTObjectBaseHandle(&mCallback));
        mCallback = NULL;
    }
}

TTErr TimeEvent::getParameterNames(TTValue& value)
{
	TTValue		attributeNames;
	TTSymbol	attributeName;
	
	// filter all default attributes (Name, Version, Author, ...)
	this->getAttributeNames(attributeNames);
	
	value.clear();
	for (TTUInt8 i = 0; i < attributeNames.size(); i++) {
		attributeName = attributeNames[0];
		
		if (attributeName == TTSymbol("date"))
			continue;
		
		value.append(attributeName);
	}
	
	return kTTErrNone;
}

TTErr TimeEvent::setDate(const TTValue& value)
{
    // set the internal active value
    mDate = value[0];
        
    // notify each attribute observers
    dateAttribute->sendNotification(kTTSym_notify, mDate);             // we use kTTSym_notify because we know that observers are TTCallback
    
    return kTTErrNone;
}

TTErr TimeEvent::WriteAsXml(const TTValue& inputValue, TTValue& outputValue)
{
	TTXmlHandlerPtr	aXmlHandler = NULL;
	
	aXmlHandler = TTXmlHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// TODO : write the time event attributes
	
	return kTTErrGeneric;
}

TTErr TimeEvent::ReadFromXml(const TTValue& inputValue, TTValue& outputValue)
{
	TTXmlHandlerPtr	aXmlHandler = NULL;
	
	aXmlHandler = TTXmlHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// TODO : parse the time event attributes
	
	return kTTErrGeneric;
}

TTErr TimeEvent::WriteAsText(const TTValue& inputValue, TTValue& outputValue)
{
	TTTextHandlerPtr	aTextHandler;
	
	aTextHandler = TTTextHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// TODO : write the time event attributes
	
	return kTTErrGeneric;
}

TTErr TimeEvent::ReadFromText(const TTValue& inputValue, TTValue& outputValue)
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





/***************************************************************************
 
 TimeEventLib
 
 ***************************************************************************/

void TimeEventLib::getTimeEventNames(TTValue& timeEventNames)
{
	timeEventNames.clear();
	//timeProcessNames.append(TTSymbol("??"));
}

