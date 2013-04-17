/*
 * Scenario time Process
 * Copyright © 2013, Théo de la Hogue, Clément Bossut
 *
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

/*!
 * \class Scenario
 *
 *  Scenario time process class is a container class to manage other time processes instances in the time
 *
 */

#include "Scenario.h"

#define thisTTClass                 Scenario
#define thisTTClassName             "Scenario"
#define thisTTClassTags             "time, process, scenario"

#define thisTimeProcessVersion		"0.1"
#define thisTimeProcessAuthor        "Theo de la Hogue"

extern "C" TT_EXTENSION_EXPORT TTErr TTLoadJamomaExtension_Scenario(void)
{
	TTFoundationInit();
	Scenario::registerClass();
	return kTTErrNone;
}

TIME_PROCESS_CONSTRUCTOR,
mNamespace(NULL),
mFirstEvent(NULL),
mLastEvent(NULL),
mEditionSolver(NULL),
mExecutionGraph(NULL)
{
    TIME_PROCESS_INITIALIZE
    
    TTErr err;
    
	TT_ASSERT("Correct number of args to create Scenario", arguments.size() == 0);
    
    addMessageWithArguments(TimeProcessAdd);
    addMessageWithArguments(TimeProcessRemove);
    
     addMessageWithArguments(TimeEventAdd);
     addMessageWithArguments(TimeEventRemove);
    
    // all messages below are hidden because they are for internal use
    addMessageWithArguments(TimeProcessActiveChange);
    addMessageProperty(TimeProcessActiveChange, hidden, YES);
    
    addMessageWithArguments(TimeEventDateChange);
    addMessageProperty(TimeEventDateChange, hidden, YES);
	
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
    
    // Creation of a static time event for the start and subscribe to it
    err = TTObjectBaseInstantiate(TTSymbol("StaticEvent"), TTObjectBaseHandle(&mStartEvent), kTTValNONE);
    
	if (err) {
        mStartEvent = NULL;
		logError("Scenario failed to load a static start event");
    }
    
    // Subscribe to the start event using the mStartEventCallback
    mStartEvent->sendMessage(TTSymbol("Subscribe"), mStartEventCallback, kTTValNONE);
    
    // Creation of a static time event for the end (but don't subscribe to it)
    err = TTObjectBaseInstantiate(TTSymbol("StaticEvent"), TTObjectBaseHandle(&mEndEvent), kTTValNONE);
    
	if (err) {
        mEndEvent = NULL;
		logError("Scenario failed to load a static end event");
    }
    
    // Creation of the first static event of the scenario (but don't subscribe to it)
    err = TTObjectBaseInstantiate(TTSymbol("StaticEvent"), TTObjectBaseHandle(&mFirstEvent), kTTValNONE);
    
	if (err) {
        mFirstEvent = NULL;
		logError("Scenario failed to load the first static event");
    }
    
    // Creation of the last static event of the scenario and subscribe to it
    err = TTObjectBaseInstantiate(TTSymbol("StaticEvent"), TTObjectBaseHandle(&mLastEvent), kTTValNONE);
    
	if (err) {
        mLastEvent = NULL;
		logError("Scenario failed to load the last static event");
    }
    
    // Subscribe to the last event using the mEndEventCallback (see in ProcessEnd why)
    mLastEvent->sendMessage(TTSymbol("Subscribe"), mEndEventCallback, kTTValNONE);
    
    // Create the edition solver
    mEditionSolver = new CSP(this);
    
    // Create the execution graph
    mExecutionGraph = new PetriNet();
}

Scenario::~Scenario()
{
    if (mNamespace) {
        delete mNamespace;
        mNamespace = NULL;
    }
    
    if (mFirstEvent) {
        TTObjectBaseRelease(TTObjectBaseHandle(&mFirstEvent));
        mFirstEvent = NULL;
    }
    
    if (mLastEvent) {
        TTObjectBaseRelease(TTObjectBaseHandle(&mLastEvent));
        mLastEvent = NULL;
    }
    
    if (mEditionSolver) {
        delete mEditionSolver;
        mEditionSolver = NULL;
    }
    
    if (mExecutionGraph) {
        delete mExecutionGraph;
        mExecutionGraph = NULL;
    }
}

TTErr Scenario::getParameterNames(TTValue& value)
{
    value.clear();
	//value.append(TTSymbol("aParameterName"));
	
	return kTTErrNone;
}

TTErr Scenario::ProcessStart()
{
    
    // TODO : compile the PetriNet to prepare scenario execution
    // cf : ECOMachine::compilePetriNet
    
    // Trigger the first event
    mFirstEvent->sendMessage(TTSymbol("Trigger"), kTTValNONE, kTTValNONE);
    
    // Notify the first event subscribers
    mFirstEvent->sendMessage(TTSymbol("Notify"));
    
    return kTTErrNone;
}

TTErr Scenario::ProcessEnd()
{
    // Trigger the end event
    mEndEvent->sendMessage(TTSymbol("Trigger"), kTTValNONE, kTTValNONE);
    
    // Notify the end event subscribers
    mEndEvent->sendMessage(TTSymbol("Notify"));
    
    // TODO : clear PetriNet as we don't need it until the next execution
    
    return kTTErrNone;
}

TTErr Scenario::Process(const TTValue& inputValue, TTValue& outputValue)
{
    TTFloat64 progression;
    
    if (inputValue.size() == 1) {
        
        if (inputValue[0].type() == kTypeFloat64) {
            
            progression = inputValue[0];
            
            // TODO : we need to update the PetriNet to process the scenario
            
            /* ADD : from PetriNet::mainThreadFunction
             
            petriNet->m_startPlace->produceTokens(1);
            petriNet->m_endPlace->consumeTokens(petriNet->m_endPlace->nbOfTokens());
            
            petriNet->addTime(petriNet->getTimeOffset() * 1000);
            //petriNet->makeOneStep();
            
            while (petriNet->m_endPlace->nbOfTokens() == 0 && !petriNet->m_mustStop) {
                petriNet->update();
                
                cout << "PetriNet::mainThreadFunction -- " << petriNet->m_currentTime << endl;
            }
            
            petriNet->m_isRunning = false;
            */
            
            return kTTErrNone;
        }
    }
    
    return kTTErrGeneric;
}

TTErr Scenario::TimeProcessAdd(const TTValue& inputValue, TTValue& outputValue)
{
    TimeProcessPtr  aTimeProcess;
    TTValue         aCacheElement, v;
    TTSymbol        timeProcessType;
    
    if (inputValue.size() == 1) {
        
        if (inputValue[0].type() == kTypeObject) {
            
            aTimeProcess = TimeProcessPtr((TTObjectBasePtr)inputValue[0]);
            
            // set this scenario as parent scenario for the time process
            v = TTValue((TTObjectBasePtr)this);
            aTimeProcess->setAttributeValue(TTSymbol("scenario"), v);
            
            // create all observers
            makeTimeProcessCacheElement(aTimeProcess, aCacheElement);
            
            // store time process object and observers
            mTimeProcessList.append(aCacheElement);

            // update the CSP depending on the type of the time process
            timeProcessType = aTimeProcess->getName();
            
            if (timeProcessType == TTSymbol("Interval")) {
                
                return mEditionSolver->addRelation(aTimeProcess);
                
            } else {
                
                TTValue boxStart, boxEnd, boxDuration, scenarioDuration;
                
                // get scenario duration
                this->getAttributeValue(TTSymbol("duration"), scenarioDuration);
                
                // get time process informations
                aTimeProcess->getAttributeValue(TTSymbol("startDate"), boxStart);
                aTimeProcess->getAttributeValue(TTSymbol("endDate"), boxEnd);
                aTimeProcess->getAttributeValue(TTSymbol("duration"), boxDuration);
                
                // add a constrain to the solver
                return mEditionSolver->addBox(aTimeProcess, boxStart[0], boxEnd[0], boxDuration[0], scenarioDuration[0]);
            }

        }
    }
    
    return kTTErrGeneric;
}

TTErr Scenario::TimeProcessRemove(const TTValue& inputValue, TTValue& outputValue)
{
    TimeProcessPtr  aTimeProcess;
    TTValue         aCacheElement;
    TTSymbol        timeProcessType;
    
    if (inputValue.size() == 1) {
        
        if (inputValue[0].type() == kTypeObject) {
            
            aTimeProcess = TimeProcessPtr((TTObjectBasePtr)inputValue[0]);
            
            // set no scenario as parent scenario for the time process
            aTimeProcess->setAttributeValue(TTSymbol("scenario"), (TTObjectBasePtr)NULL);
            
            // try to find the time process
            mTimeProcessList.find(&ScenarioFindTimeProcess, (TTPtr)aTimeProcess, aCacheElement);
            
            // couldn't find the same time in the scenario :
            if (aCacheElement == kTTValNONE)
                return kTTErrValueNotFound;
            
            else {
                
                // remove time process object and observers
                mTimeProcessList.remove(aCacheElement);
                
                // delete all observers
                deleteTimeProcessCacheElement(aCacheElement);
                
                // ? : where are removed the start and end time events of the time process
            
                // update the CSP depending on the type of the time process
                timeProcessType = aTimeProcess->getName();
                
                if (timeProcessType == TTSymbol("Interval")) {
                    
                    return mEditionSolver->removeRelation(aTimeProcess);
                    
                } else {
 
                    return mEditionSolver->removeBox(aTimeProcess);
                }
            }
        }
    }
    
    return kTTErrGeneric;
}

TTErr Scenario::TimeEventAdd(const TTValue& inputValue, TTValue& outputValue)
{
    TimeEventPtr    aTimeEvent;
    TTValue         aCacheElement;
    TTSymbol        timeEventType;
    
    if (inputValue.size() == 1) {
        
        if (inputValue[0].type() == kTypeObject) {
            
            aTimeEvent = TimeEventPtr((TTObjectBasePtr)inputValue[0]);
            
            // set this scenario as parent scenario for the time event
            //aTimeEvent->setAttributeValue(TTSymbol("scenario"), (TTObjectBasePtr)this);
            
            // create all observers
            makeTimeEventCacheElement(aTimeEvent, aCacheElement);
            
            // store time event object and observers
            mTimeEventList.append(aCacheElement);
            
            // update the CSP depending on the type of the time event
            timeEventType = aTimeEvent->getName();
            
            if (timeEventType == TTSymbol("StaticEvent")) {
                
                // TODO : update the CSP in StaticEvent case
                return kTTErrGeneric;
            }
            else if (timeEventType == TTSymbol("InteractiveEvent")) {
                
                // TODO : update the CSP in InteractiveEvent case
                return kTTErrGeneric;
            }
            // else if ...
            
        }
    }
    
    return kTTErrGeneric;
}

TTErr Scenario::TimeEventRemove(const TTValue& inputValue, TTValue& outputValue)
{
    TimeEventPtr    aTimeEvent;
    TTValue         aCacheElement;
    TTSymbol        timeEventType;
    
    if (inputValue.size() == 1) {
        
        if (inputValue[0].type() == kTypeObject) {
            
            aTimeEvent = TimeEventPtr((TTObjectBasePtr)inputValue[0]);
            
            // set no scenario as parent scenario for the time event
            //aTimeProcess->setAttributeValue(TTSymbol("scenario"), (TTObjectBasePtr)NULL);
            
            // try to find the time event
            mTimeEventList.find(&ScenarioFindTimeEvent, (TTPtr)aTimeEvent, aCacheElement);
            
            // couldn't find the same time event in the scenario :
            if (aCacheElement == kTTValNONE)
                return kTTErrValueNotFound;
            else {
                
                // remove time event object and observers
                mTimeEventList.remove(aCacheElement);
                
                // delete all observers
                deleteTimeEventCacheElement(aCacheElement);
                
                // update the CSP depending on the type of the time process
                timeEventType = aTimeEvent->getName();
                
                if (timeEventType == TTSymbol("StaticEvent")) {
                    
                    // TODO : update the CSP in StaticEvent case
                    return kTTErrGeneric;
                }
                else if (timeEventType == TTSymbol("InteractiveEvent")) {
                    
                    // TODO : update the CSP in InteractiveEvent case
                    return kTTErrGeneric;
                }
                // else if ...
            }
        }
    }
    
    return kTTErrGeneric;
}

TTErr Scenario::TimeProcessActiveChange(const TTValue& inputValue, TTValue& outputValue)
{
    TimeProcessPtr  aTimeProcess;
    TTBoolean       active;
	
    if (inputValue.size() == 2) {
        
        // TODO : use dictionnary
        if (inputValue[0].type() == kTypeObject && inputValue[1].type() == kTypeBoolean) {
            
            // get time process where the change comes from
            aTimeProcess = TimeProcessPtr((TTObjectBasePtr)inputValue[0]);
            
            // get new active value
            active = inputValue[1];
            
            // TODO : warn CSP that this time process active state have changed
            // TODO : update all CSP consequences by setting time processes attributes that are affected by the consequence
        }
    }
    
    return kTTErrGeneric;
}

TTErr Scenario::TimeEventDateChange(const TTValue& inputValue, TTValue& outputValue)
{
    TimeEventPtr    aTimeEvent;
    TTSymbol        timeEventType;
    TTUInt32        date;
	
    if (inputValue.size() == 2) {
        
        // TODO : use dictionnary
        if (inputValue[0].type() == kTypeObject && inputValue[1].type() == kTypeUInt32) {
            
            // get time event where the change comes from
            aTimeEvent = TimeEventPtr((TTObjectBasePtr)inputValue[0]);
            
            // get new date value
            date = inputValue[1];
            
            // TODO : warn CSP that this time event date have changed
            // TODO : update all CSP consequences by setting time processes or time event attributes that are affected by the consequence
            timeEventType = aTimeEvent->getName();
            
            if (timeEventType == TTSymbol("StaticEvent")) {
                
                // TODO : update the CSP in StaticEvent case
                return kTTErrGeneric;
            }
            else if (timeEventType == TTSymbol("InteractiveEvent")) {
                
                // TODO : update the CSP in InteractiveEvent case
                return kTTErrGeneric;
            }
            // else if ...
        }
    }
    
    return kTTErrGeneric;
}

TTErr Scenario::WriteAsXml(const TTValue& inputValue, TTValue& outputValue)
{
	TTXmlHandlerPtr	aXmlHandler = NULL;
	
	aXmlHandler = TTXmlHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// TODO : pass the XmlHandlerPtr to all the time processes and their time events of the scenario to write their content into the file
	
	return kTTErrGeneric;
}

TTErr Scenario::ReadFromXml(const TTValue& inputValue, TTValue& outputValue)
{
	TTXmlHandlerPtr	aXmlHandler = NULL;
	
	aXmlHandler = TTXmlHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// TODO : pass the XmlHandlerPtr to all the time processes of the scenario to read their content from the file
	
	return kTTErrGeneric;
}

TTErr Scenario::WriteAsText(const TTValue& inputValue, TTValue& outputValue)
{
	TTTextHandlerPtr	aTextHandler;
	
	aTextHandler = TTTextHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// TODO : pass the TextHandlerPtr to all the time processes of the scenario to write their content into the file
	
	return kTTErrGeneric;
}

TTErr Scenario::ReadFromText(const TTValue& inputValue, TTValue& outputValue)
{
	TTTextHandlerPtr aTextHandler;
	TTValue	v;
	
	aTextHandler = TTTextHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
    // TODO : pass the TextHandlerPtr to all the time processes of the scenario to read their content from the file
	
	return kTTErrGeneric;
}

void Scenario::makeTimeProcessCacheElement(TimeProcessPtr aTimeProcess, TTValue& newCacheElement)
{
    TTValue			v;
    TTAttributePtr	anAttribute;
	TTObjectBasePtr	aScheduler, runningObserver, progressionObserver;
    TTValuePtr		runningBaton, progressionBaton;
    TTErr           err;
	
	// 0 : cache time process object
	newCacheElement.append((TTObjectBasePtr)aTimeProcess);
    
    // get the scheduler
    aTimeProcess->getAttributeValue(TTSymbol("scheduler"), v);
    aScheduler = v[0];
    
    // 1 : create and cache scheduler running attribute observer on this time process
    err = aScheduler->findAttribute(TTSymbol("running"), &anAttribute);
    
    if (!err) {
        
        runningObserver = NULL;
        TTObjectBaseInstantiate(TTSymbol("callback"), &runningObserver, kTTValNONE);
        
        runningBaton = new TTValue(TTObjectBasePtr(this));
        runningBaton->append(TTObjectBasePtr(aTimeProcess));
        
        runningObserver->setAttributeValue(kTTSym_baton, TTPtr(runningBaton));
        runningObserver->setAttributeValue(kTTSym_function, TTPtr(&ScenarioSchedulerRunningAttributeCallback));
        
        anAttribute->registerObserverForNotifications(*runningObserver);
    }
    
    newCacheElement.append(runningObserver);
    
    // 2 : create and cache scheduler progression attribute observer on this time process
    err = aScheduler->findAttribute(TTSymbol("progression"), &anAttribute);
    
    if (!err) {
        
        progressionObserver = NULL;
        TTObjectBaseInstantiate(TTSymbol("callback"), &progressionObserver, kTTValNONE);
        
        progressionBaton = new TTValue(TTObjectBasePtr(this));
        progressionBaton->append(TTObjectBasePtr(aTimeProcess));
        
        progressionObserver->setAttributeValue(kTTSym_baton, TTPtr(progressionBaton));
        progressionObserver->setAttributeValue(kTTSym_function, TTPtr(&ScenarioSchedulerProgressionAttributeCallback));
        
        anAttribute->registerObserverForNotifications(*progressionObserver);
    }
    
    newCacheElement.append(progressionObserver);
}

void Scenario::deleteTimeProcessCacheElement(const TTValue& oldCacheElement)
{
	TTValue			v;
    TimeProcessPtr	aTimeProcess;
    TTObjectBasePtr aScheduler, anObserver;
	TTAttributePtr	anAttribute;
	TTErr			err;
    
    // 0 : get cached time process
    aTimeProcess = TimeProcessPtr((TTObjectBasePtr)oldCacheElement[0]);
    
    // get the scheduler
    aTimeProcess->getAttributeValue(TTSymbol("scheduler"), v);
    aScheduler = v[0];
    
    // 1 : delete scheduler running attribute observer
    anObserver = NULL;
    anObserver = oldCacheElement[1];
    
    anAttribute = NULL;
    err = aScheduler->findAttribute(TTSymbol("running"), &anAttribute);
    
    if (!err) {
        
        err = anAttribute->unregisterObserverForNotifications(*anObserver);
        
        if (!err)
            TTObjectBaseRelease(&anObserver);
    }
    
    // 2 : delete scheduler progression attribute observer
    anObserver = NULL;
    anObserver = oldCacheElement[2];
    
    anAttribute = NULL;
    err = aScheduler->findAttribute(TTSymbol("progression"), &anAttribute);
    
    if (!err) {
        
        err = anAttribute->unregisterObserverForNotifications(*anObserver);
        
        if (!err)
            TTObjectBaseRelease(&anObserver);
    }    
}

void Scenario::makeTimeEventCacheElement(TimeEventPtr aTimeEvent, TTValue& newCacheElement)
{
    TTValue			v;
    TTMessagePtr	aMessage;
	TTObjectBasePtr	triggerObserver;
    TTValuePtr		triggerBaton;
    TTErr           err;
	
	// 0 : cache time process object
	newCacheElement.append((TTObjectBasePtr)aTimeEvent);
    
    // 1 : create and cache event trigger message observer on this time event
    err = aTimeEvent->findMessage(TTSymbol("Trigger"), &aMessage);
    
    if (!err) {
        triggerObserver = NULL;
        TTObjectBaseInstantiate(TTSymbol("callback"), &triggerObserver, kTTValNONE);
        
        triggerBaton = new TTValue(TTObjectBasePtr(this));
        triggerBaton->append(TTObjectBasePtr(aTimeEvent));
        
        triggerObserver->setAttributeValue(kTTSym_baton, TTPtr(triggerBaton));
        triggerObserver->setAttributeValue(kTTSym_function, TTPtr(&ScenarioSchedulerRunningAttributeCallback));
        
        aMessage->registerObserverForNotifications(*triggerObserver);
    }
    
    newCacheElement.append(triggerObserver);
}

void Scenario::deleteTimeEventCacheElement(const TTValue& oldCacheElement)
{
    TTValue			v;
    TimeEventPtr	aTimeEvent;
    TTObjectBasePtr anObserver;
	TTMessagePtr	aMessage;
	TTErr			err;
    
    // 0 : get cached time event
    aTimeEvent = TimeEventPtr((TTObjectBasePtr)oldCacheElement[0]);
    
    // 1 : delete event trigger message observer observer
    anObserver = NULL;
    anObserver = oldCacheElement[1];
    
    aMessage = NULL;
    err = aTimeEvent->findMessage(TTSymbol("Trigger"), &aMessage);
    
    if (!err) {
        
        err = aMessage->unregisterObserverForNotifications(*anObserver);
        
        if (!err)
            TTObjectBaseRelease(&anObserver);
    }
}

#if 0
#pragma mark -
#pragma mark Some Methods
#endif

void ScenarioFindTimeProcess(const TTValue& aValue, TTPtr timeProcessPtrToMatch, TTBoolean& found)
{
	found = (TTObjectBasePtr)aValue[0] == (TTObjectBasePtr)timeProcessPtrToMatch;
}

void ScenarioFindTimeEvent(const TTValue& aValue, TTPtr timeEventPtrToMatch, TTBoolean& found)
{
    found = (TTObjectBasePtr)aValue[0] == (TTObjectBasePtr)timeEventPtrToMatch;
}

TTErr ScenarioSchedulerRunningAttributeCallback(TTPtr baton, TTValue& data)
{
    TimeProcessPtr  aScenario, aTimeProcess;
	TTValuePtr      b;
    TTSymbol        timeProcessType;
    TTBoolean       running;
	
	// unpack baton (a scenario and a time process)
	b = (TTValuePtr)baton;
	aScenario = TimeProcessPtr((TTObjectBasePtr)(*b)[0]);
    aTimeProcess = TimeProcessPtr((TTObjectBasePtr)(*b)[1]);
    
    // get new running value
    running = data[0];
    
    timeProcessType = aTimeProcess->getName();
    
    if (timeProcessType == TTSymbol("Automation")) {
        
        // TODO : update scenario's ECOMachine scheduler in Automation case
        return kTTErrGeneric;
    }
    else if (timeProcessType == TTSymbol("Scenario")) {
        
        // TODO : update scenario's ECOMachine scheduler in Scenario case
        return kTTErrGeneric;
    }
    else if (timeProcessType == TTSymbol("Interval")) {
        
        // TODO : update scenario's ECOMachine scheduler in Interval case
        return kTTErrGeneric;
    }
    // else if ...
    
    return kTTErrGeneric;
}

TTErr ScenarioSchedulerProgressionAttributeCallback(TTPtr baton, TTValue& data)
{
    TimeProcessPtr  aScenario, aTimeProcess;
	TTValuePtr      b;
    TTSymbol        timeProcessType;
    TTFloat64       progression;
	
	// unpack baton (a scenario and a time process)
	b = (TTValuePtr)baton;
	aScenario = TimeProcessPtr((TTObjectBasePtr)(*b)[0]);
    aTimeProcess = TimeProcessPtr((TTObjectBasePtr)(*b)[1]);
    
    // get new progression value
    progression = data[0];
    
    // TODO : warn Scheduler that this time process progression have changed (?)
    timeProcessType = aTimeProcess->getName();
    
    if (timeProcessType == TTSymbol("Automation")) {
        
        // TODO : update scenario's ECOMachine scheduler in Automation case
        return kTTErrGeneric;
    }
    else if (timeProcessType == TTSymbol("Scenario")) {
        
        // TODO : update scenario's ECOMachine scheduler in Scenario case
        return kTTErrGeneric;
    }
    else if (timeProcessType == TTSymbol("Interval")) {
        
        // TODO : update scenario's ECOMachine scheduler in Interval case
        return kTTErrGeneric;
    }
    // else if ...
    
    return kTTErrGeneric;
}