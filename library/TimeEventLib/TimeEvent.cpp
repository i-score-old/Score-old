/*
 * A Time Event interface
 * Copyright © 2013, Théo de la Hogue, Clément Bossut
 *
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#include "TimeEvent.h"

#define thisTTClass		TimeEvent

/****************************************************************************************************/

TimeEvent::TimeEvent(TTValue& arguments) :
TTObjectBase(arguments),
mScenario(NULL),
mDate(0),
mState(NULL),
mActive(YES)
{
    TT_ASSERT("Correct number of args to create TimeEvent", arguments.size() == 0);
    
    addAttribute(Scenario, kTypeObject);
   	addAttributeWithSetter(Date, kTypeUInt32);
    addAttribute(State, kTypeObject);
    addAttributeWithSetter(Active, kTypeBoolean);
    
    addMessage(Trigger);
    addMessage(Happen);
    addMessageWithArguments(StateAddressGetValue);
    
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
    
    // cache some messages and attributes for high speed notification feedbacks
    this->findMessage(TTSymbol("Happen"), &happenMessage);
    this->findAttribute(TTSymbol("date"), &dateAttribute);
    this->findAttribute(TTSymbol("active"), &activeAttribute);
    
    TTObjectBaseInstantiate(kTTSym_Script, TTObjectBaseHandle(&mState), kTTValNONE);
}

TimeEvent::~TimeEvent()
{
    /* if the time event is managed by a scenario
    if (mScenario) {
        
        v = TTValue((TTObjectBasePtr)this);
        
        // remove the time event from the scenario
        //(even if it can be done by the creator but it is safe to remove our self)
        mScenario->sendMessage(TTSymbol("TimeEventRemove"), v, kTTValNONE);
    }
    */
    
    if (mState) {
        TTObjectBaseRelease(TTObjectBaseHandle(&mState));
        mState = NULL;
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

TTErr TimeEvent::setActive(const TTValue& value)
{
    // set the internal active value
    mActive = value[0];
    
    // notify each attribute observers
    activeAttribute->sendNotification(kTTSym_notify, mActive);             // we use kTTSym_notify because we know that observers are TTCallback
    
    return kTTErrNone;
}

TTErr TimeEvent::StateAddressGetValue(const TTValue& inputValue, TTValue& outputValue)
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

