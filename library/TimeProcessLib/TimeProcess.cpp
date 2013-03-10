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
mScenario(NULL),
mActive(YES),
mRunning(NO),
mStart(0),
mEnd(0),
mProgression(0.),
mStartTrigger(NULL),
mEndTrigger(NULL),
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
    
    addAttribute(Scenario, kTypeObject);
    addAttributeProperty(Scenario, hidden, YES);

    addAttributeWithSetter(Active, kTypeBoolean);
    
    addAttribute(Running, kTypeBoolean);
    addAttributeProperty(Running, readOnly, YES);
    
	addAttributeWithSetter(Start, kTypeUInt32);
    addAttributeWithSetter(End, kTypeUInt32);
    
    addAttribute(Progression, kTypeUInt32);
    addAttributeProperty(Progression, readOnly, YES);
    
    addAttribute(StartTrigger, kTypeObject);
    addAttributeProperty(StartTrigger, readOnly, YES);
    addAttributeProperty(StartTrigger, hidden, YES);
    
    addAttribute(EndTrigger, kTypeObject);
    addAttributeProperty(EndTrigger, readOnly, YES);
    addAttributeProperty(EndTrigger, hidden, YES);
    
    addAttribute(Scheduler, kTypeObject);
    addAttributeProperty(Scheduler, readOnly, YES);
    addAttributeProperty(Scheduler, hidden, YES);
    
    addMessageWithArguments(StartTriggerAdd);
    addMessageWithArguments(StartTriggerRemove);
    
    addMessageWithArguments(EndTriggerAdd);
    addMessageWithArguments(EndTriggerRemove);
	
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
    
    // cache some attributes for high speed notification feedbacks
    this->findAttribute(TTSymbol("active"), &activeAttribute);
    this->findAttribute(TTSymbol("running"), &runningAttribute);
    this->findAttribute(TTSymbol("start"), &startAttribute);
    this->findAttribute(TTSymbol("end"), &endAttribute);
    this->findAttribute(TTSymbol("progression"), &progressionAttribute);
    this->findAttribute(TTSymbol("startTrigger"), &startTriggerAttribute);
    this->findAttribute(TTSymbol("endTrigger"), &endTriggerAttribute);
}

TimeProcess::~TimeProcess()
{    
    if (mStartTrigger) {
        TTObjectBaseRelease(TTObjectBaseHandle(&mStartTrigger));
        mStartTrigger = NULL;
    }
    
    if (mEndTrigger) {
        TTObjectBaseRelease(TTObjectBaseHandle(&mEndTrigger));
        mEndTrigger = NULL;
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

TTErr TimeProcess::setActive(const TTValue& value)
{
    // set the internal active value
    mActive = value[0];
        
    // notify each attribute observers (like a parent scenario for example)
    activeAttribute->sendNotification(kTTSym_notify, mActive);             // we use kTTSym_notify because we know that observers are TTCallback
    
    return kTTErrNone;
}

TTErr TimeProcess::setStart(const TTValue& value)
{
    TTValue     v, returnedValue;
    TTUInt32    newStart;
    TTErr       err = kTTErrNone;
    
    if (value.size() == 1) {
        
        if (value[0].type() == kTypeUInt32) {
            
            newStart = value[0];
            
            // if the time process is managed by a scenario
            if (mScenario) {
                
                v = TTValue((TTObjectBasePtr)this);
                v.append(newStart);
                
                // try to change start date inside the scenario and get the effective start date back
                err = mScenario->sendMessage(TTSymbol("TimeProcessStartChange"), v, returnedValue);
                
                if (!err)
                    newStart = returnedValue[0];
                else
                    return err;
            }
            
            // set the internal start value
            mStart = newStart;
            
            // notify each attribute observers
            startAttribute->sendNotification(kTTSym_notify, mStart);             // we use kTTSym_notify because we know that observers are TTCallback
            
            return err;
        }
    }
    
    return kTTErrGeneric;
}

TTErr TimeProcess::setEnd(const TTValue& value)
{
    TTValue     v, returnedValue;
    TTUInt32    newEnd;
    TTErr       err = kTTErrNone;
    
    if (value.size() == 1) {
        
        if (value[0].type() == kTypeUInt32) {
            
            newEnd = value[0];
            
            // if the time process is managed by a scenario
            if (mScenario) {
                
                v = TTValue((TTObjectBasePtr)this);
                v.append(newEnd);
                
                // try to change end date inside the scenario and get the effective end date back
                err = mScenario->sendMessage(TTSymbol("TimeProcessEndChange"), v, returnedValue);
                
                if (!err)
                    newEnd = returnedValue[0];
                else
                    return err;
            }
            
            // set the internal start value
            mEnd = newEnd;
            
            // notify each attribute observers
            endAttribute->sendNotification(kTTSym_notify, mEnd);             // we use kTTSym_notify because we know that observers are TTCallback
            
            return err;
        }
    }
    
    return kTTErrGeneric;
}

TTErr TimeProcess::StartTriggerAdd(const TTValue& inputValue, TTValue& outputValue)
{
    TTValue			v, args;
    TTAddress       triggerAddress;
	TTObjectBasePtr	returnValueCallback;
	TTValuePtr		returnValueBaton;
    TTErr           err;
    
    // can't add trigger on relation
    if (this->getName() == TTSymbol("Relation"))
        return kTTErrGeneric;
    
    if (inputValue.size() == 1) {
        
        triggerAddress = inputValue[0];
        
        // Remove former trigger
        StartTriggerRemove();
        
        // if the time process is managed by a scenario
        if (mScenario) {
            
            v = TTValue((TTObjectBasePtr)this);
            
            // Try to add a start trigger inside the scenario
            err = mScenario->sendMessage(TTSymbol("TimeProcessStartTriggerAdd"), v, kTTValNONE);
            
            if (err)
                return err;
        }
        
        // Prepare arguments for TTReceiver creation
        
        // we don't need the address back
        args.append(NULL);
        
        // but we need the value back to test it (using the TimeProcessStartTriggerCallback function)
        returnValueCallback = NULL;
        TTObjectBaseInstantiate(TTSymbol("callback"), &returnValueCallback, kTTValNONE);
        returnValueBaton = new TTValue(TTObjectBasePtr(this));
        returnValueCallback->setAttributeValue(kTTSym_baton, TTPtr(returnValueBaton));
        returnValueCallback->setAttributeValue(kTTSym_function, TTPtr(&TimeProcessStartTriggerCallback));
        args.append(returnValueCallback);
        
        mStartTrigger = NULL;
        err = TTObjectBaseInstantiate(kTTSym_Receiver, TTObjectBaseHandle(&mStartTrigger), args);
        
        if (!err) {
            
            // make the receiver binds on the given address
            mStartTrigger->setAttributeValue(kTTSym_address, triggerAddress);
            
            // notify each attribute observers
            startTriggerAttribute->sendNotification(kTTSym_notify, mStartTrigger);             // we use kTTSym_notify because we know that observers are TTCallback
            
        }
    }
	
	return err;
}

TTErr TimeProcess::StartTriggerRemove()
{
    TTValue v;
    TTErr   err;
    
    if (mStartTrigger) {
        
        // if the time process is managed by a scenario
        if (mScenario) {
            
            v = TTValue((TTObjectBasePtr)this);
            
            // Try to add a start trigger inside the scenario
            err = mScenario->sendMessage(TTSymbol("TimeProcessStartTriggerRemove"), v, kTTValNONE);
            
            if (err)
                return err;
        }
        
        TTObjectBaseRelease(TTObjectBaseHandle(&mStartTrigger));
        mStartTrigger = NULL;
        
        // notify each attribute observers
        startTriggerAttribute->sendNotification(kTTSym_notify, mStartTrigger);             // we use kTTSym_notify because we know that observers are TTCallback
    }
    
    return kTTErrNone;
}

TTErr TimeProcess::EndTriggerAdd(const TTValue& inputValue, TTValue& outputValue)
{
    TTValue			v, args;
    TTAddress       triggerAddress;
	TTObjectBasePtr	returnValueCallback;
	TTValuePtr		returnValueBaton;
    TTErr           err;
    
    // can't add trigger on relation
    if (this->getName() == TTSymbol("Relation"))
        return kTTErrGeneric;
    
    if (inputValue.size() == 1) {
        
        triggerAddress = inputValue[0];
        
        // Remove former trigger
        EndTriggerRemove();
        
        // if the time process is managed by a scenario
        if (mScenario) {
            
            v = TTValue((TTObjectBasePtr)this);
            
            // Try to add a start trigger inside the scenario
            err = mScenario->sendMessage(TTSymbol("TimeProcessEndTriggerAdd"), v, kTTValNONE);
            
            if (err)
                return err;
        }
        
        // Prepare arguments for TTReceiver creation
        
        // we don't need the address back
        args.append(NULL);
        
        // but we need the value back to test it (using the TimeProcessStartTriggerCallback function)
        returnValueCallback = NULL;
        TTObjectBaseInstantiate(TTSymbol("callback"), &returnValueCallback, kTTValNONE);
        returnValueBaton = new TTValue(TTObjectBasePtr(this));
        returnValueCallback->setAttributeValue(kTTSym_baton, TTPtr(returnValueBaton));
        returnValueCallback->setAttributeValue(kTTSym_function, TTPtr(&TimeProcessEndTriggerCallback));
        args.append(returnValueCallback);
        
        mEndTrigger = NULL;
        err = TTObjectBaseInstantiate(kTTSym_Receiver, TTObjectBaseHandle(&mEndTrigger), args);
        
        if (!err) {
            
            // make the receiver binds on the given address
            mEndTrigger->setAttributeValue(kTTSym_address, triggerAddress);
            
            // notify each attribute observers
            endTriggerAttribute->sendNotification(kTTSym_notify, mEndTrigger);             // we use kTTSym_notify because we know that observers are TTCallback
        }
    }
	
	return err;
}

TTErr TimeProcess::EndTriggerRemove()
{
    TTValue v;
    TTErr   err;
    
    // remove former trigger
    if (mEndTrigger) {
        
        // if the time process is managed by a scenario
        if (mScenario) {
            
            v = TTValue((TTObjectBasePtr)this);
            
            // Try to add a start trigger inside the scenario
            err = mScenario->sendMessage(TTSymbol("TimeProcessEndTriggerRemove"), v, kTTValNONE);
            
            if (err)
                return err;
        }
        
        TTObjectBaseRelease(TTObjectBaseHandle(&mEndTrigger));
        mEndTrigger = NULL;
        
        // notify each attribute observers
        endTriggerAttribute->sendNotification(kTTSym_notify, mEndTrigger);             // we use kTTSym_notify because we know that observers are TTCallback
    }
    
    return kTTErrNone;
}

TTErr TimeProcess::WriteAsXml(const TTValue& inputValue, TTValue& outputValue)
{
	TTXmlHandlerPtr	aXmlHandler = NULL;
	
	aXmlHandler = TTXmlHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// TODO : write the time process attributes, the cue start and end content, start and end trigger, ...
	
	return kTTErrGeneric;
}

TTErr TimeProcess::ReadFromXml(const TTValue& inputValue, TTValue& outputValue)
{
	TTXmlHandlerPtr	aXmlHandler = NULL;
	
	aXmlHandler = TTXmlHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// TODO : parse the time process attributes, the cue start and end content, start and end trigger, ...
	
	return kTTErrGeneric;
}

TTErr TimeProcess::WriteAsText(const TTValue& inputValue, TTValue& outputValue)
{
	TTTextHandlerPtr	aTextHandler;
	
	aTextHandler = TTTextHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// TODO : write the time process attributes, the cue start and end content, start and end trigger, ...
	
	return kTTErrGeneric;
}

TTErr TimeProcess::ReadFromText(const TTValue& inputValue, TTValue& outputValue)
{
	TTTextHandlerPtr aTextHandler;
	TTValue	v;
	
	aTextHandler = TTTextHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
    // TODO : parse the time process attributes, the cue start and end content, start and end trigger, ...
	
	return kTTErrGeneric;
}

#if 0
#pragma mark -
#pragma mark Some Methods
#endif

TTErr TimeProcessStartTriggerCallback(TTPtr baton, TTValue& data)
{
    TimeProcessPtr  aTimeProcess;
	TTValuePtr      b;
	
	// unpack baton (a time process)
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

TTErr TimeProcessEndTriggerCallback(TTPtr baton, TTValue& data)
{
    TimeProcessPtr  aTimeProcess;
	TTValuePtr      b;
	
	// unpack baton (a time process)
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
    
    // set internal progression value
    aTimeProcess->mProgression = progression;
    
    // notify each attribute observers (like a parent scenario for example)
    aTimeProcess->progressionAttribute->sendNotification(kTTSym_notify,  aTimeProcess->mProgression);     // we use kTTSym_notify because we know that observers are TTCallback
    
    // Case 0 :
    // notify the time process owner that the time process starts
    // then recall the start state
    if (progression == 0.) {
        
        // close start trigger listening
        aTimeProcess->mStartTrigger->setAttributeValue(kTTSym_active, NO);
        
        // notify owner that the start appends
        aTimeProcess->mStartCallback->notify(TTObjectBasePtr(aTimeProcess), kTTValNONE);
        
        // set internal running state
        aTimeProcess->mRunning = YES;
        
        // notify each attribute observers (like a parent scenario for example)
        aTimeProcess->runningAttribute->sendNotification(kTTSym_notify, aTimeProcess->mRunning);          // we use kTTSym_notify because we know that observers are TTCallback
        
        // use the specific start process method of the time process
        aTimeProcess->ProcessStart();
        
        return;
    }
    
    // Case 1 :
    // recall the start state
    // then notify the time process owner that the time process ends
    else if (progression == 1.) {
        
        // close end trigger listening
        aTimeProcess->mEndTrigger->setAttributeValue(kTTSym_active, NO);
        
        // use the specific process end method of the time process
        aTimeProcess->ProcessEnd();
        
        // notify owner that the end appends
        aTimeProcess->mEndCallback->notify(TTObjectBasePtr(aTimeProcess), kTTValNONE);
        
        // set internal running state
        aTimeProcess->mRunning = YES;
        
        // notify each attribute observers (like a parent scenario for example)
        aTimeProcess->runningAttribute->sendNotification(kTTSym_notify, aTimeProcess->mRunning);          // we use kTTSym_notify because we know that observers are TTCallback
        
        return;
    }

    // Case ]0 :: 1[ :
    // use the specific process method
    aTimeProcess->Process();
}