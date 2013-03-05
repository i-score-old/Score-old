/*
 * A TimeBox Object
 * Copyright © 2013, Théo de la Hogue
 *
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#include "TTTimeBox.h"

#define thisTTClass			TTTimeBox
#define thisTTClassName		"TimeBox"
#define thisTTClassTags		"time, box"

TT_SCORE_CONSTRUCTOR,
mActive(YES),
mStart(0),
mEnd(0),
mStartCue(NULL),
mEndCue(NULL),
mStartReceiver(NULL),
mEndReceiver(NULL),
mStartCallback(NULL),
mEndCallback(NULL),
mNamespace(NULL),
mScheduler(NULL)
{
	TT_ASSERT("Correct number of args to create TTTimeBox", arguments.size() == 2);
    
    if (arguments.size() >= 1)
		mStartCallback = TTCallbackPtr((TTObjectBasePtr)arguments[0]);
	
	if (arguments.size() >= 2)
		mEndCallback = TTCallbackPtr((TTObjectBasePtr)arguments[1]);
	
    addAttribute(Active, kTypeBoolean);
    
	addAttributeWithSetter(Start, kTypeUInt32);
    addAttributeWithSetter(End, kTypeUInt32);
    
    addAttribute(StartCue, kTypeObject);
    addAttribute(EndCue, kTypeObject);
    
    addAttribute(StartReceiver, kTypeObject);
    addAttribute(EndReceiver, kTypeObject);
    
    addAttribute(Namespace, kTypePointer);
    addAttribute(Scheduler, kTypeObject);
	
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
    
    // Creation of a scheduler based on the EcoMachine scheduler plugin
    TTValue args;
    TTErr   err;
	
    // prepare callback argument to be notifed of the progression (we have to store this as a pointer)
    args.append((TTPtr)&TTTimeBoxSchedulerCallback);
    args.append((TTPtr)this);
    
    err = TTObjectBaseInstantiate(TTSymbol("EcoMachine"), TTObjectBaseHandle(&mScheduler), args);
    
	if (err) {
        mScheduler = NULL;
		logError("TTTimeBox failed to load the EcoMachine Scheduler");
    }
}

TTTimeBox::~TTTimeBox()
{
    if (mStartCue) {
        TTObjectBaseRelease(TTObjectBaseHandle(&mStartCue));
        mStartCue = NULL;
    }
    
    if (mEndCue) {
        TTObjectBaseRelease(TTObjectBaseHandle(&mEndCue));
        mEndCue = NULL;
    }
    
    if (mStartReceiver) {
        TTObjectBaseRelease(TTObjectBaseHandle(&mStartReceiver));
        mStartReceiver = NULL;
    }
    
    if (mEndReceiver) {
        TTObjectBaseRelease(TTObjectBaseHandle(&mEndReceiver));
        mEndReceiver = NULL;
    }
    
    if (mNamespace) {
        delete mNamespace;
        mNamespace = NULL;
    }
    
    if (mScheduler) {
        TTObjectBaseRelease(TTObjectBaseHandle(&mScheduler));
        mScheduler = NULL;
    }
}

TTErr TTTimeBox::setStart(const TTValue& value)
{
    TTValue retunedValue;
    TTErr   err;
    
    // Try to change the scheduler start date and get the effective start date back
    err = mScheduler->sendMessage(TTSymbol("changeStart"), value, retunedValue);
    
    if (!err)
        mStart = retunedValue[0];
    
    return err;
}

TTErr TTTimeBox::setEnd(const TTValue& value)
{
    TTValue retunedValue;
    TTErr   err;
    
    // Try to change the scheduler end date and get the effective end date back
    err = mScheduler->sendMessage(TTSymbol("changeEnd"), value, retunedValue);
    
    if (!err)
        mEnd = retunedValue[0];
    
    return err;
}

TTErr TTTimeBox::TriggerStartAdd(const TTValue& inputValue, TTValue& outputValue)
{
    TTValue			args;
    TTAddress       triggerAddress;
	TTObjectBasePtr	returnValueCallback;
	TTValuePtr		returnValueBaton;
    TTErr           err;
    
    if (inputValue.size() == 1) {
        
        triggerAddress = inputValue[0];
        
        // Remove former receiver
        TriggerStartRemove();
        
        // Prepare arguments for TTReceiver creation
        
        // we don't need the address back
        args.append(NULL);
        
        // but we need the value back to test it (using the TTTimeBoxStartReceiverCallback function)
        returnValueCallback = NULL;
        TTObjectBaseInstantiate(TTSymbol("callback"), &returnValueCallback, kTTValNONE);
        returnValueBaton = new TTValue(TTObjectBasePtr(this));
        returnValueCallback->setAttributeValue(kTTSym_baton, TTPtr(returnValueBaton));
        returnValueCallback->setAttributeValue(kTTSym_function, TTPtr(&TTTimeBoxStartReceiverCallback));
        args.append(returnValueCallback);
        
        mStartReceiver = NULL;
        err = TTObjectBaseInstantiate(kTTSym_Receiver, TTObjectBaseHandle(&mStartReceiver), args);
        
        if (!err) {
            
            // Make the receiver binds on the given address
            mStartReceiver->setAttributeValue(kTTSym_address, triggerAddress);
            
            // TODO : tell to scheduler there is a trigger point here (?)
            
        }
    }
	
	return err;
}

TTErr TTTimeBox::TriggerStartRemove()
{
    if (mStartReceiver) {
        
        TTObjectBaseRelease(TTObjectBaseHandle(&mStartReceiver));
        mStartReceiver = NULL;
        
        // TODO : tell to scheduler there is no more trigger point here (?)
    }
    
    return kTTErrNone;
}

TTErr TTTimeBox::TriggerEndAdd(const TTValue& inputValue, TTValue& outputValue)
{
    TTValue			args;
    TTAddress       triggerAddress;
	TTObjectBasePtr	returnValueCallback;
	TTValuePtr		returnValueBaton;
    TTErr           err;
    
    if (inputValue.size() == 1) {
        
        triggerAddress = inputValue[0];
        
        // Remove former receiver
        TriggerEndRemove();
        
        // Prepare arguments for TTReceiver creation
        
        // we don't need the address back
        args.append(NULL);
        
        // but we need the value back to test it (using the TTTimeBoxStartReceiverCallback function)
        returnValueCallback = NULL;
        TTObjectBaseInstantiate(TTSymbol("callback"), &returnValueCallback, kTTValNONE);
        returnValueBaton = new TTValue(TTObjectBasePtr(this));
        returnValueCallback->setAttributeValue(kTTSym_baton, TTPtr(returnValueBaton));
        returnValueCallback->setAttributeValue(kTTSym_function, TTPtr(&TTTimeBoxEndReceiverCallback));
        args.append(returnValueCallback);
        
        mEndReceiver = NULL;
        err = TTObjectBaseInstantiate(kTTSym_Receiver, TTObjectBaseHandle(&mEndReceiver), args);
        
        if (!err) {
            
            // Make the receiver binds on the given address
            mEndReceiver->setAttributeValue(kTTSym_address, triggerAddress);
            
            // TODO : tell to scheduler there is a trigger point here (?)
            
        }
    }
	
	return err;
}

TTErr TTTimeBox::TriggerEndRemove()
{
    // remove former receiver
    if (mEndReceiver) {
        
        TTObjectBaseRelease(TTObjectBaseHandle(&mEndReceiver));
        mEndReceiver = NULL;
        
        // TODO : tell to scheduler there is no more trigger point here (?)
    }
    
    return kTTErrNone;
}

TTErr TTTimeBox::WriteAsXml(const TTValue& inputValue, TTValue& outputValue)
{
	TTXmlHandlerPtr	aXmlHandler = NULL;
	
	aXmlHandler = TTXmlHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// TODO : write the time box attributes, the cue start and end content, start and end receiver, ...
	
	return kTTErrGeneric;
}

TTErr TTTimeBox::ReadFromXml(const TTValue& inputValue, TTValue& outputValue)
{
	TTXmlHandlerPtr	aXmlHandler = NULL;
	
	aXmlHandler = TTXmlHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// TODO : parse the time box attributes, the cue start and end content, start and end receiver, ...
	
	return kTTErrGeneric;
}

TTErr TTTimeBox::WriteAsText(const TTValue& inputValue, TTValue& outputValue)
{
	TTTextHandlerPtr	aTextHandler;
	
	aTextHandler = TTTextHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// TODO : write the time box attributes, the cue start and end content, start and end receiver, ...
	
	return kTTErrGeneric;
}

TTErr TTTimeBox::ReadFromText(const TTValue& inputValue, TTValue& outputValue)
{
	TTTextHandlerPtr aTextHandler;
	TTValue	v;
	
	aTextHandler = TTTextHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
    // TODO : parse the time box attributes, the cue start and end content, start and end receiver, ...
	
	return kTTErrGeneric;
}

#if 0
#pragma mark -
#pragma mark Some Methods
#endif

TTErr TTTimeBoxStartReceiverCallback(TTPtr baton, TTValue& data)
{
    TTTimeBoxPtr    aTimeBox;
	TTValuePtr      b;
	
	// unpack baton (a TTMapper)
	b = (TTValuePtr)baton;
	aTimeBox = TTTimeBoxPtr((TTObjectBasePtr)(*b)[0]);
	
    // if the time box active, launch the scheduler
	if (aTimeBox->mActive) {
        
        // TODO : add a conditionnal expression depending on the received data
        // like if (data > 3.)
        if (YES)
            return aTimeBox->mScheduler->sendMessage(TTSymbol("Go"), aTimeBox->mEnd - aTimeBox->mStart, kTTValNONE);
    }
    
    return kTTErrGeneric;
}

TTErr TTTimeBoxEndReceiverCallback(TTPtr baton, TTValue& data)
{
    TTTimeBoxPtr    aTimeBox;
	TTValuePtr      b;
	
	// unpack baton (a TTMapper)
	b = (TTValuePtr)baton;
	aTimeBox = TTTimeBoxPtr((TTObjectBasePtr)(*b)[0]);
	
    // if the time box active, stop the scheduler
	if (aTimeBox->mActive) {
        
        // TODO : add a conditionnal expression depending on the data
        // like if (data > 3.)
        if (YES)
            return aTimeBox->mScheduler->sendMessage(TTSymbol("Stop"));
    }
    
    return kTTErrGeneric;
}

void TTTimeBoxSchedulerCallback(TTPtr object, TTFloat64 progression)
{
	TTTimeBoxPtr	aTimeBox = (TTTimeBoxPtr)object;
    
    // Case 0 :
    // notify the time box owner that the time box starts
    // then recall the start state
    if (progression == 0.) {
        
        aTimeBox->mStartCallback->notify(TTObjectBasePtr(aTimeBox), kTTValNONE);
        aTimeBox->mStartCue->sendMessage(TTSymbol("Recall"), kTTValNONE, kTTValNONE);
        return;
    }
    
    // Case 1 :
    // recall the start state
    // then notify the time box owner that the time box ends
    else if (progression == 1.) {
        
        aTimeBox->mEndCue->sendMessage(TTSymbol("Recall"), kTTValNONE, kTTValNONE);
        aTimeBox->mEndCallback->notify(TTObjectBasePtr(aTimeBox), kTTValNONE);
        return;
    }

    // Case ]0 :: 1[ :
    // process the interpolation between the start state and the end state
    TTCueInterpolate(TTCuePtr(aTimeBox->mStartCue), TTCuePtr(aTimeBox->mEndCue), progression);
}