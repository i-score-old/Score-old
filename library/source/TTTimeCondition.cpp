/** @file
 *
 * @ingroup scoreLibrary
 *
 * @brief a class to define a condition and a set of different cases
 *
 * @see TTTimeCondition
 *
 * @authors Théo de la Hogue & Clément Bossut
 *
 * @copyright Copyright © 2013, Théo de la Hogue & Clément Bossut @n
 * This code is licensed under the terms of the "CeCILL-C" @n
 * http://www.cecill.info
 */

#include "TTTimeCondition.h"
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlreader.h>

#define thisTTClass         TTTimeCondition
#define thisTTClassName     "TimeCondition"
#define thisTTClassTags     "time, condition"

/****************************************************************************************************/

TT_BASE_OBJECT_CONSTRUCTOR,
mActive(NO),
mReady(NO),
mPendingCounter(0)
{
    TT_ASSERT("Correct number of args to create TTTimeCondition", arguments.size() == 1);
    
    if (arguments.size() == 1)
        mContainer = arguments[0];
    
    addAttribute(Name, kTypeSymbol);
    
    addAttributeWithSetter(Active, kTypeBoolean);
    
    addAttribute(Ready, kTypeBoolean);
    addAttributeProperty(Ready, readOnly, YES);
    
    addAttributeWithGetterAndSetter(DisposeExpression, kTypeSymbol);
    
    registerAttribute(TTSymbol("expressions"), kTypeLocalValue, NULL, (TTGetterMethod)& TTTimeCondition::getExpressions, NULL);
    registerAttribute(TTSymbol("events"), kTypeLocalValue, NULL, (TTGetterMethod)& TTTimeCondition::getEvents, NULL);
    
    addMessageWithArguments(EventAdd);
    addMessageWithArguments(EventRemove);
    addMessageWithArguments(EventExpression);
    addMessageWithArguments(EventDefault);
    addMessageWithArguments(ExpressionFind);
    addMessageWithArguments(DefaultFind);
    addMessageWithArguments(ExpressionTest);
    
	// needed to be handled by a TTXmlHandler
	addMessageWithArguments(WriteAsXml);
	addMessageProperty(WriteAsXml, hidden, YES);
	addMessageWithArguments(ReadFromXml);
	addMessageProperty(ReadFromXml, hidden, YES);
    
    // needed to be notified by events
    addMessageWithArguments(EventDateChanged);
    addMessageWithArguments(EventStatusChanged);
	
    // generate a random name
    mName = mName.random();
}

TTTimeCondition::~TTTimeCondition()
{
    TTObject empty;
    
    // disable condition
    mActive = NO;
    
    // update each event condition
    for (TTCaseMapIterator it = mCases.begin() ; it != mCases.end() ; it++) {
        if (TTObjectBasePtr(it->first)->valid)
            TTObjectBasePtr(it->first)->setAttributeValue(kTTSym_condition, empty);
    }
    
    // destroy all receivers;
    mReceivers.clear();
}

TTErr TTTimeCondition::setActive(const TTValue& value)
{
    TTBoolean newActive = value[0];
    
    // filter repetitions
    if (newActive != mActive) {
        
        // if the condition is ready to be active
        if (newActive && mReady) {
            
            mActive = YES;
            
            // create the receivers
            TTCaseMapIterator it;
            
            // for each trigger case
            for(it = mCases.begin() ; it != mCases.end() ; it++)
                addReceiver(it->second.trigger.getAddress());
            
            // for dispose case
            addReceiver(mDispose.getAddress());
            
            return kTTErrNone;
        }
        
        if (!newActive) {
            
            mActive = NO;
            
            mReceivers.clear();
            
            return kTTErrNone;
        }
    }
    
    return kTTErrGeneric;
}

TTErr TTTimeCondition::getExpressions(TTValue& value)
{
    value.clear();
    
    // for each event, append the associated expressions to the result
    for (TTCaseMapIterator it = mCases.begin() ; it != mCases.end() ; it++) {
        value.append(it->second.trigger);
    }
    
    // append the dispose expression
    value.append(mDispose);
    
    return kTTErrNone;
}

TTErr TTTimeCondition::getEvents(TTValue& value)
{
    value.clear();
    
    // for each case, append the event to the result
    for (TTCaseMapIterator it = mCases.begin() ; it != mCases.end() ; it++) {
        value.append((TTObjectBasePtr)it->first); // cast to TTObjectBasePtr to associate the type kTypeObject and not kTypePointer
    }
    
    return kTTErrNone;
}

TTErr TTTimeCondition::EventAdd(const TTValue& inputValue, TTValue& outputValue)
{
    TTObject        event, thisObject(this);
    Comportment     aComportment;
    TTValue         v;
    
    switch (inputValue.size()) {/* TODO : don't know how to pass Comportment as TTValue
                                 
                                 // if we have two arguments
                                 case 2 :
                                 
                                 // if the second argument isn't a symbol
                                 if (inputValue[1].type() != kTypeSymbol)
                                 
                                 // return an error TODO : should warn the user
                                 return kTTErrInvalidType;
                                 
                                 // if it's a symbol : convert it to an expression
                                 ExpressionParseFromValue(inputValue[1], anExpression);
                                 
                                 // add receivers for the address if needed
                                 if (anExpression.getAddress() != kTTAdrsEmpty)
                                 addReceiver(anExpression.getAddress());
                                 */
            // if we have one or two arguments
        case 1 :
            
            // if the first argument isn't an object
            if (inputValue[0].type() != kTypeObject)
                
                // return en error TODO : should warn the user
                return kTTErrInvalidType;
            
            // if it's an object : convert it to an event
            event = inputValue[0];
            
            // insert the event with an expression
            mCases.insert({{event.instance(), aComportment}});
            
            // increment the pending counter
            mPendingCounter++;
            
            // set the event to waiting
            event.set(kTTSym_status, kTTSym_eventWaiting); // CB TODO : Why ?
            
            // tell the event it is conditioned
            event.set(kTTSym_condition, thisObject);
            
            // observe the event
            event.registerObserverForNotifications(thisObject);
            
            // return no error
            return kTTErrNone;
            
            // if there is less than 1 or more than 2 arguments
        default :
            
            // return an error TODO : should warn the user
            return kTTErrWrongNumValues;
    }
    
    // never evaluated
    return kTTErrGeneric;
}

TTErr TTTimeCondition::EventRemove(const TTValue& inputValue, TTValue& outputValue)
{
    TTObject            event = inputValue[0];
    TTCaseMapIterator   it = mCases.find(event.instance());
    
    // if the event exists
    if (it != mCases.end()) {
        
        TTValue     v;
        TTObject    thisObject(this);
        
        // remove the case
        mCases.erase(it);
        
        // decrement the unready counter
        mPendingCounter--;
        
        // tell the event it is not conditioned anymore
        v = TTObject();
        event.set(kTTSym_condition, v);
        
        // don't observe the event anymore
        event.unregisterObserverForNotifications(thisObject);
        
        return kTTErrNone;
    }
    
    return kTTErrValueNotFound;
}

TTErr TTTimeCondition::EventExpression(const TTValue& inputValue, TTValue& outputValue)
{
    TTObject            event = inputValue[0];
    TTCaseMapIterator   it = mCases.find(event.instance());
    
    // if the event exists
    if (it != mCases.end()) {
        
        // replace the old expression by the new one
        Expression newExpression;
        
        ExpressionParseFromValue(inputValue[1], newExpression);
        
        mCases[it->first].trigger = newExpression;
        
        return kTTErrNone;
    }
    
    return kTTErrValueNotFound;
}

TTErr TTTimeCondition::EventDefault(const TTValue &inputValue, TTValue &outputValue)
{
    TTObject            event = inputValue[0];
    TTCaseMapIterator   it = mCases.find(event.instance());
    
    // if the event exists
    if (it != mCases.end()) {
        
        // change its default comportment
        mCases[it->first].dflt = TTBoolean(inputValue[1]);
        
        return kTTErrNone;
    }
    
    return kTTErrValueNotFound;
}

TTErr TTTimeCondition::ExpressionFind(const TTValue& inputValue, TTValue& outputValue)
{
    TTObject            event = inputValue[0];
    TTCaseMapIterator   it = mCases.find(event.instance());
    
    // if the event exists
    if (it != mCases.end()) {
        
        outputValue = it->second.trigger;
        return kTTErrNone;
    }
    
    return kTTErrValueNotFound;
}

TTErr TTTimeCondition::DefaultFind(const TTValue& inputValue, TTValue& outputValue)
{
    TTObject            event = inputValue[0];
    TTCaseMapIterator   it = mCases.find(event.instance());
    
    // if the event exists
    if (it != mCases.end()) {
        
        outputValue = it->second.dflt;
        return kTTErrNone;
    }
    
    return kTTErrValueNotFound;
}

TTErr TTTimeCondition::getDisposeExpression(TTValue &value)
{
    value.clear();
    
    value.append(mDispose);
    
    return kTTErrNone;
}

TTErr TTTimeCondition::setDisposeExpression(const TTValue &value)
{
    ExpressionParseFromValue(value, mDispose);
    
    return kTTErrNone;
}

TTErr TTTimeCondition::ExpressionTest(const TTValue& inputValue, TTValue& outputValue)
{
    Expression      anExpression;
    TTObject        aReceiver;
    TTValue         v;
    
    // parse the input value
    ExpressionParseFromValue(inputValue, anExpression);
    
    // get the receiver for the expression address
    if (!mReceivers.lookup(anExpression.getAddress(), v)) {
        
        aReceiver = v[0];
        
        // ask the value at this address
        return aReceiver.send(kTTSym_Get);
    }
    
    return kTTErrGeneric;
}

TTErr TTTimeCondition::WriteAsXml(const TTValue& inputValue, TTValue& outputValue)
{
	TTObject o = inputValue[0];
	TTXmlHandlerPtr aXmlHandler = (TTXmlHandlerPtr)o.instance();
    if (!aXmlHandler)
		return kTTErrGeneric;
    
    TTObjectBasePtr event;
    TTValue         v, keys;
    TTSymbol        key, name;
    TTCaseMapIterator it;
	
    // Write the name
    xmlTextWriterWriteAttribute((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "name", BAD_CAST mName.c_str());
    
    // Write the dispose expression
    xmlTextWriterWriteAttribute((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "dispose", BAD_CAST mDispose.c_str());
    
    // Write each case
    for (it = mCases.begin(); it != mCases.end(); it++) {
        
        Comportment aComportment = it->second;
        
        event = it->first;
        
        // Start a case node
        xmlTextWriterStartElement((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "case");
        
        // Write the event name
        event->getAttributeValue(kTTSym_name, v);
        name = v[0];
        xmlTextWriterWriteAttribute((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "event", BAD_CAST name.c_str());
        
        // Write the comportment
        xmlTextWriterWriteAttribute((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "trigger", BAD_CAST aComportment.trigger.c_str());
        xmlTextWriterWriteAttribute((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "default", BAD_CAST (aComportment.dflt ? "1" : "0"));
        
        // Close the case node
        xmlTextWriterEndElement((xmlTextWriterPtr)aXmlHandler->mWriter);
    }
    
	return kTTErrNone;
}

TTErr TTTimeCondition::ReadFromXml(const TTValue& inputValue, TTValue& outputValue)
{
	TTObject o = inputValue[0];
	TTXmlHandlerPtr aXmlHandler = (TTXmlHandlerPtr)o.instance();
    if (!aXmlHandler)
		return kTTErrGeneric;
    
    TTValue v, out;
    
    // Condition node
    if (aXmlHandler->mXmlNodeName == kTTSym_condition) {
        
        // Get the name
        if (!aXmlHandler->getXmlAttribute(kTTSym_name, v, YES)) {
            
            if (v.size() == 1) {
                
                if (v[0].type() == kTypeSymbol) {
                    
                    mName = v[0];
                }
            }
        }
        
        // Get the dispose expression
        if (!aXmlHandler->getXmlAttribute(TTSymbol("dispose"), v, YES)) {
            
            if (v.size() == 1) {
                
                if (v[0].type() == kTypeSymbol) {
                    
                    ExpressionParseFromValue(v, mDispose);
                }
            }
        }
    }
    
    // Case node
    if (aXmlHandler->mXmlNodeName == kTTSym_case) {
        
        // get the event
        if (!aXmlHandler->getXmlAttribute(kTTSym_event, v, YES)) {
            
            // Find the event using his name from our container
            if (!mContainer.send("TimeEventFind", v, out)) {
                
                EventAdd(out, v); // TODO : better using the second argument
                
                // get the expressions
                if (!aXmlHandler->getXmlAttribute(TTSymbol("trigger"), v, YES)) {
                    out.append(v[0]);
                    EventExpression(out, v);
                    out.pop_back();
                }
                if (!aXmlHandler->getXmlAttribute(TTSymbol("default"), v, NO)) {
                    out.append(v[0] == 1);
                    EventDefault(out, v);
                }
            }
        }
    }
	
	return kTTErrNone;
}

TTErr TTTimeCondition::EventDateChanged(const TTValue& inputValue, TTValue& outputValue)
{
    TT_ASSERT("TTTimeCondition::EventDateChanged : inputValue is correct", inputValue.size() == 1 && inputValue[0].type() == kTypeObject);
    
    TTObject            event = inputValue[0];
    TTCaseMapIterator   it = mCases.find(event.instance());
    TTUInt32            date;
    TTValue             v;
    
    // if the event exists
    if (it != mCases.end()) {
        
        // get the date
        event.get(kTTSym_date, v);
        date = v[0];
        
        return kTTErrNone;
    }
    
    TTLogError("TTTimeCondition::EventDateChanged : wrong event\n");
    return kTTErrGeneric;
}

TTErr TTTimeCondition::EventStatusChanged(const TTValue& inputValue, TTValue& outputValue)
{
    TT_ASSERT("TTTimeCondition::EventStatusChanged : inputValue is correct", inputValue.size() == 3 && inputValue[0].type() == kTypeObject);
    
    TTObject                event = inputValue[0];
    TTCaseMapIterator       it = mCases.find(event.instance());
    TTSymbol                newStatus = inputValue[1], oldStatus = inputValue[2];
    TTValue                 v;
    
    TT_ASSERT("TTTimeCondition::EventStatusChanged : status effectively changed", newStatus != oldStatus);
    
    // if the event exists
    if (it != mCases.end()) {
        
        if (newStatus == kTTSym_eventPending && --mPendingCounter == 0) {
            setReady(YES);
        } else if (oldStatus == kTTSym_eventPending && mPendingCounter++ == 0 && mReady == YES) {
            setReady(NO);
            applyDefaults();
        }
        
        return kTTErrNone;
    }
    
    TTLogError("TTTimeCondition::EventStatusChanged : wrong event\n");
    return kTTErrGeneric;
}

TTErr TTTimeCondition::setReady(TTBoolean newReady)
{
    // filter repetitions
    if (newReady != mReady) {
        
        // set the ready value
        mReady = newReady;
        
        // notify each observers
        sendNotification(kTTSym_ConditionReadyChanged, mReady);
    }
    
    return kTTErrGeneric;
}

void TTTimeCondition::addReceiver(TTAddress anAddress)
{
    TTObject    aReceiver, aReceiverCallback;
    TTValue     v, baton;
    
    // if there is no receiver for the expression address
    if (anAddress != kTTAdrsEmpty && mReceivers.lookup(anAddress, v)) {
        
        // no callback to get the received address back
        v = TTObject();
        
        // a callback to get the received value back
        aReceiverCallback = TTObject("callback");
        
        baton = TTValue(TTObject(this), anAddress);
        aReceiverCallback.set(kTTSym_baton, baton);
        aReceiverCallback.set(kTTSym_function, TTPtr(&TTTimeConditionReceiverReturnValueCallback));
        
        v.append(aReceiverCallback);
        
        aReceiver = TTObject(kTTSym_Receiver, v);
        
        // set the address of the receiver
        aReceiver.set(kTTSym_address, anAddress);
        
        mReceivers.append(anAddress, aReceiver);
    }
}

void TTTimeCondition::applyDefaults()
{
    TTValue v;
    TTSymbol status;
    
    for (TTCaseMapIterator it = mCases.begin() ; it != mCases.end() ; it++)
    {
        it->first->getAttributeValue(kTTSym_status, v);
        status = v[0];
        
        if (status != kTTSym_eventDisposed && status != kTTSym_eventHappened)
            it->first->sendMessage(it->second.dflt?kTTSym_Happen:kTTSym_Dispose);
    }
}

#if 0
#pragma mark -
#pragma mark Some Methods
#endif

TTErr TTTimeConditionReceiverReturnValueCallback(const TTValue& baton, const TTValue& data)
{
    TTObject            o;
    TTTimeConditionPtr  aTimeCondition;
    TTAddress           anAddress;
    Expression          triggerExp;
    TTList              timeEventToTrigger;
    TTList              timeEventToDispose;
    TTValue             v;
	
	// unpack baton (condition, address)
	o = baton[0];
	aTimeCondition = (TTTimeConditionPtr)o.instance();
    
    // only if the condition is ready
    if (!aTimeCondition->mReady)
        return kTTErrNone;
    
    anAddress = baton[1];
    
    // if the dispose expression is true
    if (anAddress == aTimeCondition->mDispose.getAddress() && aTimeCondition->mDispose.evaluate(data)) {
        
        aTimeCondition->setReady(NO);
        
        // dispose every event
        aTimeCondition->getEvents(v);
        for (TTElementIter it = v.begin() ; it != v.end() ; it++) {
            TTObject event = TTElement(*it);
            event.send(kTTSym_Dispose);
        }
    }
    // if didn't dispose
    else {
        
        // for each event's expressions matching the incoming address
        for (TTCaseMapIterator it = aTimeCondition->mCases.begin(); it != aTimeCondition->mCases.end(); it++) {
            
            triggerExp = it->second.trigger;
            
            // if the test of the expression passes
            if (anAddress == triggerExp.getAddress() && triggerExp.evaluate(data)) {
                
                // append the event to the trigger list
                timeEventToTrigger.append(TTObjectBasePtr(it->first));
            } else {
                
                // append the event to the dispose list
                timeEventToDispose.append(TTObjectBasePtr(it->first));
            }
        }
        
        // if at least one event is in the trigger list
        if (!timeEventToTrigger.isEmpty()) {
            
            aTimeCondition->setReady(NO);
            
            // trigger all events of the trigger list
            for (timeEventToTrigger.begin(); timeEventToTrigger.end(); timeEventToTrigger.next()) {
                o = timeEventToTrigger.current()[0];
                o.send(kTTSym_Trigger);
            }
            
            // dispose all the other events
            for (timeEventToDispose.begin(); timeEventToDispose.end(); timeEventToDispose.next()) {
                o = timeEventToTrigger.current()[0];
                o.send(kTTSym_Dispose);
            }
        }
    }
    
    return kTTErrNone;
}
