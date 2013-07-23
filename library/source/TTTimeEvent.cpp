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
mName(kTTSymEmpty),
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

    addAttribute(Name, kTypeSymbol);
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
    
    // create a script
    TTObjectBaseInstantiate(kTTSym_Script, TTObjectBaseHandle(&mState), kTTValNONE);
    
    // generate a random name
    mName = mName.random();
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
    TTValue         v;
    TTString        s;
	
	aXmlHandler = TTXmlHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
    xmlTextWriterStartElement((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "event");
    
    // Write the name
    xmlTextWriterWriteAttribute((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "name", BAD_CAST mName.c_str());
    
    // Write the date
    v = mDate;
    v.toString();
    s = TTString(v[0]);
    xmlTextWriterWriteAttribute((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "date", BAD_CAST s.data());
    
    // Write if it is interactive
    v = mInteractive;
    v.toString();
    s = TTString(v[0]);
    xmlTextWriterWriteAttribute((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "interactive", BAD_CAST s.data());
    
    // Write the state
    v = TTObjectBasePtr(mState);
    aXmlHandler->setAttributeValue(kTTSym_object, v);
    aXmlHandler->sendMessage(TTSymbol("Write"));
    
    xmlTextWriterEndElement((xmlTextWriterPtr)aXmlHandler->mWriter);
	
	return kTTErrNone;
}

TTErr TTTimeEvent::ReadFromXml(const TTValue& inputValue, TTValue& outputValue)
{
	TTXmlHandlerPtr	aXmlHandler = NULL;
    TTValue         v;
	
	aXmlHandler = TTXmlHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// get the name
    if (xmlTextReaderMoveToAttribute((xmlTextReaderPtr)aXmlHandler->mReader, (const xmlChar*)("name")) == 1) {
        
        aXmlHandler->fromXmlChar(xmlTextReaderValue((xmlTextReaderPtr)aXmlHandler->mReader), v);
        
        if (v.size() == 1) {
            
            if (v[0].type() == kTypeSymbol) {
                
                mName = v[0];
            }
        }
    }
    
    // get the date
    if (xmlTextReaderMoveToAttribute((xmlTextReaderPtr)aXmlHandler->mReader, (const xmlChar*)("date")) == 1) {
        
        aXmlHandler->fromXmlChar(xmlTextReaderValue((xmlTextReaderPtr)aXmlHandler->mReader), v);
        
        if (v.size() == 1) {
            
            if (v[0].type() == kTypeUInt32) {
                
                this->setDate(v);
            }
        }
    }
    
    // get the interactive state
    if (xmlTextReaderMoveToAttribute((xmlTextReaderPtr)aXmlHandler->mReader, (const xmlChar*)("interactive")) == 1) {
        
        aXmlHandler->fromXmlChar(xmlTextReaderValue((xmlTextReaderPtr)aXmlHandler->mReader), v);
        
        if (v.size() == 1) {
            
            if (v[0].type() == kTypeBoolean) {
                
                this->setInteractive(v);
            }
        }
    }
	
	return kTTErrNone;
}