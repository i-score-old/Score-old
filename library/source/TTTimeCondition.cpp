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
    // TODO : destroy all receivers;
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
    
    // TODO : append the expression to the case table
    
    return kTTErrNone;
}

TTErr TTTimeCondition::CaseRemove(const TTValue& inputValue, TTValue& outputValue)
{
    return kTTErrNone;
}

TTErr TTTimeCondition::CaseTest(const TTValue& inputValue, TTValue& outputValue)
{
    return kTTErrNone;
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
    return kTTErrNone;
}