/*
 * A Time Process interface
 * Copyright © 2013, Théo de la Hogue
 *
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#include "TimeProcess.h"

#define thisTTClass		TimeProcess

/****************************************************************************************************/

TimeProcess::TimeProcess(TTValue& arguments) :
TTObjectBase(arguments),
mActive(YES),
mRunning(NO),
mStart(0),
mEnd(0),
mStartReceiver(NULL),
mEndReceiver(NULL),
mStartCallback(NULL),
mEndCallback(NULL),
mProgressionCallback(NULL),
mProgressionBaton(NULL),
mScheduler(NULL)
{
    TT_ASSERT("Correct number of args to create TimeProcess", arguments.size() == 4);
    
    if (arguments.size() >= 1)
		mStartCallback = TTCallbackPtr((TTObjectBasePtr)arguments[0]);
	
	if (arguments.size() >= 2)
		mEndCallback = TTCallbackPtr((TTObjectBasePtr)arguments[1]);
    
    if (arguments.size() >= 3)
        mProgressionCallback = TimeProcessProgressionCallback((TTPtr)arguments[2]);
    
    if (arguments.size() >= 4)
        mProgressionBaton = arguments[3];

    addAttribute(Active, kTypeBoolean);
    
	addAttributeWithSetter(Start, kTypeUInt32);
    addAttributeWithSetter(End, kTypeUInt32);
    
    addAttribute(StartReceiver, kTypeObject);
    addAttributeProperty(StartReceiver, hidden, YES);
    
    addAttribute(EndReceiver, kTypeObject);
    addAttributeProperty(EndReceiver, hidden, YES);
    
    addAttribute(Scheduler, kTypeObject);
    addAttributeProperty(Scheduler, hidden, YES);
	
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
	
    // Prepare callback argument to be notified of :
    //      - a contrained change of the start date (is it needed ?)
    //      - a contrained change of the end date (is it needed ?)
    //      - the progression
    
    //args.append((TTPtr)&TimeProcessChangeStartCallback);
    //args.append((TTPtr)&TimeProcessChangeEndCallback);
    args.append((TTPtr)&TimeProcessSchedulerCallback);
    args.append((TTPtr)this);   // we have to store this as a pointer
    
    err = TTObjectBaseInstantiate(TTSymbol("EcoMachine"), TTObjectBaseHandle(&mScheduler), args);
    
	if (err) {
        mScheduler = NULL;
		logError("TimeProcess failed to load the EcoMachine Scheduler");
    }
}

TimeProcess::~TimeProcess()
{    
    if (mStartReceiver) {
        TTObjectBaseRelease(TTObjectBaseHandle(&mStartReceiver));
        mStartReceiver = NULL;
    }
    
    if (mEndReceiver) {
        TTObjectBaseRelease(TTObjectBaseHandle(&mEndReceiver));
        mEndReceiver = NULL;
    }
    
    if (mScheduler) {
        TTObjectBaseRelease(TTObjectBaseHandle(&mScheduler));
        mScheduler = NULL;
    }
}

TTErr TimeProcess::getParameterNames(TTValue& value)
{
	TTValue		attributeNames;
	TTSymbol	attributeName;
	
	// filter all default attributes (Name, Version, Author, ...)
	this->getAttributeNames(attributeNames);
	
	value.clear();
	for (TTUInt8 i = 0; i < attributeNames.size(); i++) {
		attributeName = attributeNames[0];
		
		if (attributeName == TTSymbol("name")           ||
			attributeName == TTSymbol("version")        ||
			attributeName == TTSymbol("author")         ||
            attributeName == TTSymbol("active")         ||
            attributeName == TTSymbol("running")        ||
            attributeName == TTSymbol("progression")    ||
            attributeName == TTSymbol("start")          ||
            attributeName == TTSymbol("end"))
			continue;
		
		value.append(attributeName);
	}
	
	return kTTErrNone;
}

TTErr TimeProcess::setStart(const TTValue& value)
{
    TTValue retunedValue;
    TTErr   err;
    
    // Try to change the scheduler start date and get the effective start date back
    err = mScheduler->sendMessage(TTSymbol("changeStart"), value, retunedValue);
    
    if (!err)
        mStart = retunedValue[0];
    
    return err;
}

TTErr TimeProcess::setEnd(const TTValue& value)
{
    TTValue retunedValue;
    TTErr   err;
    
    // Try to change the scheduler end date and get the effective end date back
    err = mScheduler->sendMessage(TTSymbol("changeEnd"), value, retunedValue);
    
    if (!err)
        mEnd = retunedValue[0];
    
    return err;
}

TTErr TimeProcess::TriggerStartAdd(const TTValue& inputValue, TTValue& outputValue)
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
        
        // but we need the value back to test it (using the TimeProcessStartReceiverCallback function)
        returnValueCallback = NULL;
        TTObjectBaseInstantiate(TTSymbol("callback"), &returnValueCallback, kTTValNONE);
        returnValueBaton = new TTValue(TTObjectBasePtr(this));
        returnValueCallback->setAttributeValue(kTTSym_baton, TTPtr(returnValueBaton));
        returnValueCallback->setAttributeValue(kTTSym_function, TTPtr(&TimeProcessStartReceiverCallback));
        args.append(returnValueCallback);
        
        mStartReceiver = NULL;
        err = TTObjectBaseInstantiate(kTTSym_Receiver, TTObjectBaseHandle(&mStartReceiver), args);
        
        if (!err) {
            
            // Make the receiver binds on the given address
            mStartReceiver->setAttributeValue(kTTSym_address, triggerAddress);
            
            // TODO : tell to scheduler there is a trigger point here (?)
            // (it means append a trigger point to the storyline)
            
        }
    }
	
	return err;
}

TTErr TimeProcess::TriggerStartRemove()
{
    if (mStartReceiver) {
        
        TTObjectBaseRelease(TTObjectBaseHandle(&mStartReceiver));
        mStartReceiver = NULL;
        
        // TODO : tell to scheduler there is no more trigger point here (?)
        // (it means remove a trigger point to the storyline)
    }
    
    return kTTErrNone;
}

TTErr TimeProcess::TriggerEndAdd(const TTValue& inputValue, TTValue& outputValue)
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
        
        // but we need the value back to test it (using the TimeProcessStartReceiverCallback function)
        returnValueCallback = NULL;
        TTObjectBaseInstantiate(TTSymbol("callback"), &returnValueCallback, kTTValNONE);
        returnValueBaton = new TTValue(TTObjectBasePtr(this));
        returnValueCallback->setAttributeValue(kTTSym_baton, TTPtr(returnValueBaton));
        returnValueCallback->setAttributeValue(kTTSym_function, TTPtr(&TimeProcessEndReceiverCallback));
        args.append(returnValueCallback);
        
        mEndReceiver = NULL;
        err = TTObjectBaseInstantiate(kTTSym_Receiver, TTObjectBaseHandle(&mEndReceiver), args);
        
        if (!err) {
            
            // Make the receiver binds on the given address
            mEndReceiver->setAttributeValue(kTTSym_address, triggerAddress);
            
            // TODO : tell to scheduler there is a trigger point here (?)
            // (it means append a trigger point to the storyline)
        }
    }
	
	return err;
}

TTErr TimeProcess::TriggerEndRemove()
{
    // remove former receiver
    if (mEndReceiver) {
        
        TTObjectBaseRelease(TTObjectBaseHandle(&mEndReceiver));
        mEndReceiver = NULL;
        
        // TODO : tell to scheduler there is no more trigger point here (?)
        // (it means remove a trigger point to the storyline)
    }
    
    return kTTErrNone;
}

TTErr TimeProcess::WriteAsXml(const TTValue& inputValue, TTValue& outputValue)
{
	TTXmlHandlerPtr	aXmlHandler = NULL;
	
	aXmlHandler = TTXmlHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// TODO : write the time process attributes, the cue start and end content, start and end receiver, ...
	
	return kTTErrGeneric;
}

TTErr TimeProcess::ReadFromXml(const TTValue& inputValue, TTValue& outputValue)
{
	TTXmlHandlerPtr	aXmlHandler = NULL;
	
	aXmlHandler = TTXmlHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// TODO : parse the time process attributes, the cue start and end content, start and end receiver, ...
	
	return kTTErrGeneric;
}

TTErr TimeProcess::WriteAsText(const TTValue& inputValue, TTValue& outputValue)
{
	TTTextHandlerPtr	aTextHandler;
	
	aTextHandler = TTTextHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// TODO : write the time process attributes, the cue start and end content, start and end receiver, ...
	
	return kTTErrGeneric;
}

TTErr TimeProcess::ReadFromText(const TTValue& inputValue, TTValue& outputValue)
{
	TTTextHandlerPtr aTextHandler;
	TTValue	v;
	
	aTextHandler = TTTextHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
    // TODO : parse the time process attributes, the cue start and end content, start and end receiver, ...
	
	return kTTErrGeneric;
}

#if 0
#pragma mark -
#pragma mark Some Methods
#endif

TTErr TimeProcessStartReceiverCallback(TTPtr baton, TTValue& data)
{
    TimeProcessPtr  aTimeProcess;
	TTValuePtr      b;
	
	// unpack baton (a TimeProcess)
	b = (TTValuePtr)baton;
	aTimeProcess = TimeProcessPtr((TTObjectBasePtr)(*b)[0]);
	
    // if the time process active, launch the scheduler
	if (aTimeProcess->mActive) {
        
        // TODO : add a conditionnal expression depending on the received data
        // like if (data > 3.)
        if (YES)
            return aTimeProcess->mScheduler->sendMessage(TTSymbol("Go"), aTimeProcess->mEnd - aTimeProcess->mStart, kTTValNONE);
    }
    
    return kTTErrGeneric;
}

TTErr TimeProcessEndReceiverCallback(TTPtr baton, TTValue& data)
{
    TimeProcessPtr  aTimeProcess;
	TTValuePtr      b;
	
	// unpack baton (a TTMapper)
	b = (TTValuePtr)baton;
	aTimeProcess = TimeProcessPtr((TTObjectBasePtr)(*b)[0]);
	
    // if the time process active, stop the scheduler
	if (aTimeProcess->mActive) {
        
        // TODO : add a conditionnal expression depending on the data
        // like if (data > 3.)
        if (YES)
            return aTimeProcess->mScheduler->sendMessage(TTSymbol("Stop"));
    }
    
    return kTTErrGeneric;
}

void TimeProcessSchedulerCallback(TTPtr object, TTFloat64 progression)
{
	TimeProcessPtr	aTimeProcess = (TimeProcessPtr)object;
    
    aTimeProcess->mProgression = progression;
    
    // Case 0 :
    // notify the time process owner that the time process starts
    // then recall the start state
    if (progression == 0.) {
        
        // close start receiver listening
        aTimeProcess->mStartReceiver->setAttributeValue(kTTSym_active, NO);
        
        // notify owner that the start appends
        aTimeProcess->mStartCallback->notify(TTObjectBasePtr(aTimeProcess), kTTValNONE);
        
        // use the specific start process method of the time process
        aTimeProcess->ProcessStart();
        return;
    }
    
    // Case 1 :
    // recall the start state
    // then notify the time process owner that the time process ends
    else if (progression == 1.) {
        
        // close end receiver listening
        aTimeProcess->mEndReceiver->setAttributeValue(kTTSym_active, NO);
        
        // use the specific process end method of the time process
        aTimeProcess->ProcessEnd();
        
        // notify owner that the end appends
        aTimeProcess->mEndCallback->notify(TTObjectBasePtr(aTimeProcess), kTTValNONE);
        return;
    }

    // Case ]0 :: 1[ :
    // use the specific process method
    aTimeProcess->Process();
}