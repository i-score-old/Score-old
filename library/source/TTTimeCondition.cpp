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

#define thisTTClass         TTTimeCondition
#define thisTTClassName     "TimeCondition"
#define thisTTClassTags     "time, condition"

/****************************************************************************************************/

TT_BASE_OBJECT_CONSTRUCTOR,
mContainer(NULL),
mReady(YES)
{
    TT_ASSERT("Correct number of args to create TTTimeCondition", arguments.size() == 1);
    
    if (arguments.size() == 1)
        mContainer = arguments[0];

    addAttribute(Name, kTypeSymbol);
    addAttributeWithSetter(Ready, kTypeBoolean);
    
    registerAttribute(TTSymbol("cases"), kTypeLocalValue, NULL, (TTGetterMethod)& TTTimeCondition::getCases, NULL);
    registerAttribute(TTSymbol("events"), kTypeLocalValue, NULL, (TTGetterMethod)& TTTimeCondition::getEvents, NULL);

    addMessageWithArguments(EventAdd);
    addMessageWithArguments(EventRemove);
    addMessageWithArguments(EventExpression);
    addMessageWithArguments(CaseFind);
    addMessageWithArguments(CaseTest);
    
	// needed to be handled by a TTXmlHandler
	addMessageWithArguments(WriteAsXml);
	addMessageProperty(WriteAsXml, hidden, YES);
	addMessageWithArguments(ReadFromXml);
	addMessageProperty(ReadFromXml, hidden, YES);
	
    // cache some messages and attributes for high speed notification feedbacks
    this->findAttribute(kTTSym_ready, &readyAttribute);
    
    // generate a random name
    mName = mName.random();
}

TTTimeCondition::~TTTimeCondition()
{
    TTValue         v, keys;
    TTSymbol        key;
    TTObjectBasePtr aReceiver;
    
    // destroy all receivers;
    mReceivers.getKeys(keys);
    for (TTUInt8 i = 0; i < keys.size(); i++) {
        
        key = keys[i];
        mReceivers.lookup(key, v);
        
        aReceiver = v[0];
        TTObjectBaseRelease(&aReceiver);
    }
}

TTErr TTTimeCondition::setReady(const TTValue& value)
{
    // set the ready value
    mReady = value[0];
    
    // notify each attribute observers
    readyAttribute->sendNotification(kTTSym_notify, mReady);             // we use kTTSym_notify because we know that observers are TTCallback
    
    return kTTErrNone;
}

TTErr TTTimeCondition::getCases(TTValue& value)
{
    value.clear();

    for (TTCaseMapIterator it = mCases.begin() ; it != mCases.end() ; it++) {
        value.append(it->second);
    }
    
    return kTTErrNone;
}

TTErr TTTimeCondition::getEvents(TTValue& value)
{
    value.clear();

    for (TTCaseMapIterator it = mCases.begin() ; it != mCases.end() ; it++) {
        value.append((TTObjectBasePtr)it->first);
    }

    return kTTErrNone;
}

TTErr TTTimeCondition::EventAdd(const TTValue& inputValue, TTValue& outputValue)
{
    TTTimeEventPtr event = TTTimeEventPtr(TTObjectBasePtr(inputValue[0]));

    // insert the event contained in inputValue with a blank expression which evaluates to true
    mCases.insert({{event,Expression()}});

    return kTTErrNone;
}

TTErr TTTimeCondition::EventRemove(const TTValue& inputValue, TTValue& outputValue)
{
    TTTimeEventPtr event = TTTimeEventPtr(TTObjectBasePtr(inputValue[0]));

    TTCaseMapIterator it = mCases.find(event);
    Expression exp = it->second;
    TTAddress addr = exp.getAddress();
    mCases.erase(it);

    if (addr != kTTAdrsEmpty) {
        cleanReceiver(addr);
    }

    return kTTErrNone;
}

TTErr TTTimeCondition::EventExpression(const TTValue& inputValue, TTValue& outputValue)
{
    TTTimeEventPtr event = TTTimeEventPtr(TTObjectBasePtr(inputValue[0]));

    TTCaseMapIterator it = mCases.find(event);
    Expression old_exp = it->second;
    TTAddress old_addr = old_exp.getAddress();
    Expression new_exp;
    ExpressionParseFromValue(inputValue[1], new_exp);
    TTAddress new_addr = new_exp.getAddress();
    mCases[it->first] = new_exp;

    if (old_addr != new_addr && old_addr != kTTAdrsEmpty && new_addr != kTTAdrsEmpty) {
        cleanReceiver(old_addr);
        addReceiver(new_addr);
    }

    return kTTErrNone;
}

void TTTimeCondition::cleanReceiver(TTAddress addr) {
    bool found = false;

    for (TTCaseMapIterator it = mCases.begin() ; !found && it != mCases.end() ; it++) {
        Expression exp = it->second;
        if (exp.getAddress() == addr) {
            found = true;
        }
    }

    if (!found) { // ... remove the receiver for this address
        TTValue v;
        TTObjectBasePtr aReceiver;
        if (!mReceivers.lookup(addr, v)) {

            aReceiver = v[0];
            TTObjectBaseRelease(&aReceiver);

            mReceivers.remove(addr);
        }
    }
}

void TTTimeCondition::addReceiver(TTAddress addr) {
    TTValue v;
    TTObjectBasePtr aReceiver;
    TTObjectBasePtr aReceiverCallback;
    TTValuePtr aReceiverBaton;

    // if there is no receiver for the expression address
    if (mReceivers.lookup(addr, v)) {

        // No callback for the address
        v = TTValue((TTObjectBasePtr)NULL);

        // Create a receiver callback to get the expression address value back
        aReceiverCallback = NULL;
        TTObjectBaseInstantiate(TTSymbol("callback"), &aReceiverCallback, kTTValNONE);

        aReceiverBaton = new TTValue(TTObjectBasePtr(this));
        aReceiverBaton->append(addr);

        aReceiverCallback->setAttributeValue(kTTSym_baton, TTPtr(aReceiverBaton));
        aReceiverCallback->setAttributeValue(kTTSym_function, TTPtr(&TTTimeConditionReceiverReturnValueCallback));

        v.append(aReceiverCallback);
        TTObjectBaseInstantiate(kTTSym_Receiver, TTObjectBaseHandle(&aReceiver), v);

        // set the address of the receiver
        aReceiver->setAttributeValue(kTTSym_address, addr);

        v = TTObjectBasePtr(aReceiver);
        mReceivers.append(addr, v);
    }
}

TTErr TTTimeCondition::CaseFind(const TTValue& inputValue, TTValue& outputValue)
{
    TTTimeEventPtr event = TTTimeEventPtr(TTObjectBasePtr(inputValue[0]));

    TTValue     v, keys;
    TTSymbol    key;
    TTErr       err = kTTErrValueNotFound;
    
    // look for a case binding on the same event
    TTCaseMapIterator it = mCases.find(event);

    if (it != mCases.end()) {
        outputValue = it->second;
        err = kTTErrNone;
    }
    
    return err;
}

TTErr TTTimeCondition::CaseTest(const TTValue& inputValue, TTValue& outputValue)
{
    Expression      expression;
    TTObjectBasePtr aReceiver;
    TTValue         v;
    
    // parse the input value
    ExpressionParseFromValue(inputValue, expression);
    
    // get the receiver for the expression address
    if (!mReceivers.lookup(expression.getAddress(), v)) {
     
        aReceiver = v[0];
        
        // ask the value at this address
        return aReceiver->sendMessage(kTTSym_Get);
    }
    
    return kTTErrGeneric;
}

TTErr TTTimeCondition::WriteAsXml(const TTValue& inputValue, TTValue& outputValue)
{
    /*
	TTXmlHandlerPtr	aXmlHandler = NULL;
    TTObjectBasePtr event;
    TTValue         v, keys;
    TTSymbol        key, name;
	
	aXmlHandler = TTXmlHandlerPtr((TTObjectBasePtr)inputValue[0]);
    
    // Write the name
    xmlTextWriterWriteAttribute((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "name", BAD_CAST mName.c_str());
    
    // Write each case
    mCases.getKeys(keys);
    for (TTUInt8 i = 0; i < keys.size(); i++) {
        
        // Start a case node
        xmlTextWriterStartElement((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "case");
        
        key = keys[i];
        mCases.lookup(key, v);
        
        // Write the expression
        xmlTextWriterWriteAttribute((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "expression", BAD_CAST key.c_str());
        
        // Write the event name
        if (v.size() > 0) {
            
            event = v[0];
            event->getAttributeValue(kTTSym_name, v);
            name = v[0];
            xmlTextWriterWriteAttribute((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "event", BAD_CAST name.c_str());
        }
        
        // Close the case node
        xmlTextWriterEndElement((xmlTextWriterPtr)aXmlHandler->mWriter);
    }
    */
	return kTTErrNone;
}

TTErr TTTimeCondition::ReadFromXml(const TTValue& inputValue, TTValue& outputValue)
{
    /*
	TTXmlHandlerPtr	aXmlHandler = NULL;
    TTValue         expressionValue, v, out;
    TTTimeEventPtr  aTimeEvent;
	
	aXmlHandler = TTXmlHandlerPtr((TTObjectBasePtr)inputValue[0]);
    
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
    }
    
    // Case node
    if (aXmlHandler->mXmlNodeName == kTTSym_case) {
        
        // get the expression
        if (!aXmlHandler->getXmlAttribute(kTTSym_expression, expressionValue, YES)) {
        
            CaseAdd(expressionValue, out);
        }
        
        // get the event
        if (!aXmlHandler->getXmlAttribute(kTTSym_event, v, YES)) {
            
            // Find the event using his name from our container
            if (!mContainer->sendMessage(TTSymbol("TimeEventFind"),v, out)) {
            
                expressionValue.append(out[0]);
                CaseLinkEvent(expressionValue, out);
            }
        }
        
        return kTTErrNone;
    }
	*/
	return kTTErrNone;
}

#if 0
#pragma mark -
#pragma mark Some Methods
#endif

TTErr TTTimeConditionReceiverReturnValueCallback(TTPtr baton, TTValue& data)
{
    TTValuePtr          b;
    TTTimeConditionPtr  aTimeCondition;
    TTAddress           anAddress;
    Expression          expression;
//    TTObjectBasePtr     event;
//    TTValue             v, keys;
//    TTBoolean           found = NO;
	
	// unpack baton (condition, address)
	b = (TTValuePtr)baton;
	aTimeCondition = TTTimeConditionPtr(TTObjectBasePtr((*b)[0]));
    anAddress = (*b)[1];

    for (TTCaseMapIterator it = aTimeCondition->mCases.begin() ; it != aTimeCondition->mCases.end() ; it++) {
        expression = it->second;
        if (anAddress == expression.getAddress() && expression.evaluate(data)) {
            it->first->sendMessage(kTTSym_Trigger);
        } else {
            it->first->sendMessage(kTTSym_Dispose);
        }
    }

    aTimeCondition->mReady = NO;
    aTimeCondition->readyAttribute->sendNotification(kTTSym_notify, aTimeCondition->mReady);

    return kTTErrNone;
}
