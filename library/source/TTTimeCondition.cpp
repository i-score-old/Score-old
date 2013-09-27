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
    
    addMessageWithArguments(CaseAdd);
    addMessageWithArguments(CaseRemove);
    addMessageWithArguments(CaseLinkEvent);
    addMessageWithArguments(CaseUnlinkEvent);
    addMessageWithArguments(CaseTest);
    
	// needed to be handled by a TTXmlHandler
	addMessageWithArguments(WriteAsXml);
	addMessageProperty(WriteAsXml, hidden, YES);
	addMessageWithArguments(ReadFromXml);
	addMessageProperty(ReadFromXml, hidden, YES);
	
    // cache some messages and attributes for high speed notification feedbacks
    this->findAttribute(TTSymbol("ready"), &readyAttribute);
    
    // generate a random name
    mName = mName.random();
}

TTTimeCondition::~TTTimeCondition()
{
    TTValue         v, keys;
    TTSymbol        key;
    TTObjectBasePtr aReceiver;
    
    // TODO : destroy all receivers;
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
    mCases.getKeys(value);
    
    return kTTErrNone;
}

TTErr TTTimeCondition::CaseAdd(const TTValue& inputValue, TTValue& outputValue)
{
    Expression      expression;
    TTObjectBasePtr aReceiver = NULL;
    TTObjectBasePtr aReceiverCallback = NULL;
    TTValuePtr      aReceiverBaton;
    TTValue         v;
    
    // parse the input value
    ExpressionParseFromValue(inputValue, expression);
    
    // if there is no receiver for the expression address
    if (mReceivers.lookup(expression.getAddress(), v)) {
        
        // Create a receiver callback to get the expression address value back
        aReceiverCallback = NULL;
        TTObjectBaseInstantiate(TTSymbol("callback"), &aReceiverCallback, kTTValNONE);
        
        aReceiverBaton = new TTValue(TTObjectBasePtr(this));
        aReceiverBaton->append(expression.getAddress());
        
        aReceiver->setAttributeValue(kTTSym_baton, TTPtr(aReceiverBaton));
        aReceiver->setAttributeValue(kTTSym_function, TTPtr(&TTTimeConditionReceiverReturnValueCallback));
        
        v = TTValue((TTPtr)aReceiverCallback);
        TTObjectBaseInstantiate(TTSymbol("Receiver"), TTObjectBaseHandle(&aReceiver), v);
        
        v = TTObjectBasePtr(aReceiver);
        mReceivers.append(expression.getAddress(), v);
    }
    
    // if the expression don't exist yet
    if (mCases.lookup(expression, v)) {
        
        // append the expression to the case table (with no related event)
        v.clear();
        return mCases.append(expression, v);
    }
    
    return kTTErrGeneric;
}

TTErr TTTimeCondition::CaseRemove(const TTValue& inputValue, TTValue& outputValue)
{
    Expression      expression, e;
    TTObjectBasePtr event;
    TTObjectBasePtr aReceiver;
    TTValue         v, args, out, keys;
    TTBoolean       found = NO;
    TTErr           err;

    // parse the input value
    ExpressionParseFromValue(inputValue, expression);
    
    // if the expression don't exist yet
    if (!mCases.lookup(expression, v)) {
        
        // unlink the event relative to this expression
        args = expression;
        args.append(v[0]);
        CaseUnlinkEvent(args, out);
        
        // remove the case
        err = mCases.remove(expression);

        // if there is no other expression whith the same address ...
        mCases.getKeys(keys);
        for (TTUInt8 i = 0; i < keys.size(); i++) {
            
            ExpressionParseFromValue(keys[i], e);
            
            if (e.getAddress() == expression.getAddress()) {
                
                found = YES;
                break;
            }
        }
        
        // ... remove the receiver for this address
        if (!found) {
            
            if (!mReceivers.lookup(expression.getAddress(), v)) {
                
                aReceiver = v[0];
                TTObjectBaseRelease(&aReceiver);
                
                mReceivers.remove(expression.getAddress());
            }
        }
        
        return err;
    }
    
    return kTTErrGeneric;
}

TTErr TTTimeCondition::CaseLinkEvent(const TTValue& inputValue, TTValue& outputValue)
{
    Expression      expression;
    TTObjectBasePtr event;
    TTValue         v;
    TTErr           err;
    
    if (inputValue.size() == 2) {
        
        if (inputValue[0] == kTypeSymbol && inputValue[1] == kTypeObject) {
            
            ExpressionParseFromValue(inputValue[0], expression);
            event = inputValue[1];
            
            // if the expression exist
            if (!mCases.lookup(expression, v)) {
                
                // remove the case
                mCases.remove(expression);
                
                // then append the same case with the given event
                err = mCases.append(expression, event);
                
                // tell the event it is conditioned
                event->setAttributeValue(TTSymbol("condition"), TTObjectBasePtr(this));
                
                // TODO : observe the event ready attribute value
                
                return err;
            }
        }
    }
    
    return kTTErrGeneric;
}

TTErr TTTimeCondition::CaseUnlinkEvent(const TTValue& inputValue, TTValue& outputValue)
{
    Expression      expression;
    TTObjectBasePtr event;
    TTValue         v;
    TTErr           err;
    
    if (inputValue.size() == 2) {
        
        if (inputValue[0] == kTypeSymbol && inputValue[1] == kTypeObject) {
            
            ExpressionParseFromValue(inputValue[0], expression);
            event = inputValue[1];
            
            // if the expression exist
            if (!mCases.lookup(expression, v)) {
                
                // remove the case
                mCases.remove(expression);
                
                // then append an expression without event
                err = mCases.append(expression, kTTValNONE);
                
                // tell the event it is not conditioned anymore
                event->setAttributeValue(TTSymbol("condition"), TTObjectBasePtr(NULL));
                
                // TODO : stop observation of the event ready attribute value
                
                return err;
            }
        }
    }
    
    return kTTErrGeneric;
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
	TTXmlHandlerPtr	aXmlHandler = NULL;
    TTValue         v;
    TTString        s;
	
	aXmlHandler = TTXmlHandlerPtr((TTObjectBasePtr)inputValue[0]);
    
	return kTTErrNone;
}

TTErr TTTimeCondition::ReadFromXml(const TTValue& inputValue, TTValue& outputValue)
{
	TTXmlHandlerPtr	aXmlHandler = NULL;
    TTValue         v;
	
	aXmlHandler = TTXmlHandlerPtr((TTObjectBasePtr)inputValue[0]);
    
    // Condition node
    if (aXmlHandler->mXmlNodeName == TTSymbol("Condition")) {
        
        return kTTErrNone;
    }
	
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
    TTObjectBasePtr     event;
    TTValue             v, keys;
    TTBoolean           found = NO;
	
	// unpack baton (condition, address)
	b = (TTValuePtr)baton;
	aTimeCondition = TTTimeConditionPtr(TTObjectBasePtr((*b)[0]));
    anAddress = (*b)[1];
	
	// for all expressions which have the same address
    aTimeCondition->mCases.getKeys(keys);
    
    for (TTUInt8 i = 0; i < keys.size(); i++) {
        
        ExpressionParseFromValue(keys[i], expression);
        
        if (expression.getAddress() == anAddress) {
            
            // test the expression with the data
            if (expression.evaluate(data)) {
                
                // get the event
                aTimeCondition->mCases.lookup(expression, v);
                event = v[0];
                
                // trigger the event
                event->sendMessage(TTSymbol("Trigger"));
                
                // at least one event has been triggered
                found = YES;
            }
        }
    }
    
    // if at least one event has been triggered,
    // the time condition is not ready anymore
    if (found) {
        
        aTimeCondition->mReady = NO;
        
        // notify each attribute observers
        aTimeCondition->readyAttribute->sendNotification(kTTSym_notify, aTimeCondition->mReady);             // we use kTTSym_notify because we know that observers are TTCallback
    }
    return kTTErrNone;
}