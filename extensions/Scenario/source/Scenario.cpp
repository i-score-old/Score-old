/*
 * Scenario time Process
 * Copyright © 2013, Théo de la Hogue
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
mNamespace(NULL)
{
    TIME_PROCESS_INITIALIZE
    
	TT_ASSERT("Correct number of args to create Scenario", arguments.size() == 0);
    
    addMessageWithArguments(TimeProcessAdd);
    addMessageWithArguments(TimeProcessRemove);
    
    // all messages below are hidden because they are internaly
    addMessageWithArguments(TimeProcessStartChange);
    addMessageProperty(TimeProcessStartChange, hidden, YES);
    
    addMessageWithArguments(TimeProcessEndChange);
    addMessageProperty(TimeProcessEndChange, hidden, YES);
    
    addMessageWithArguments(TimeProcessStartTriggerAdd);
    addMessageProperty(TimeProcessStartTriggerAdd, hidden, YES);
    
    addMessageWithArguments(TimeProcessStartTriggerRemove);
    addMessageProperty(TimeProcessStartTriggerRemove, hidden, YES);
    
    addMessageWithArguments(TimeProcessEndTriggerAdd);
    addMessageProperty(TimeProcessEndTriggerAdd, hidden, YES);
    
    addMessageWithArguments(TimeProcessEndTriggerRemove);
    addMessageProperty(TimeProcessEndTriggerRemove, hidden, YES);
	
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
}

Scenario::~Scenario()
{
    if (mNamespace) {
        delete mNamespace;
        mNamespace = NULL;
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
    // TODO : launch the processing of the first time process (?)
    return kTTErrGeneric;
}

TTErr Scenario::ProcessEnd()
{
    // TODO : is there something to do at the end of a scenario ?
    return kTTErrGeneric;
}

TTErr Scenario::Process()
{
    // TODO : normally there is nothing to do as the scenario only contains other time processes which are doing things (?)
    return kTTErrNone;
}

TTErr Scenario::TimeProcessAdd(const TTValue& inputValue, TTValue& outputValue)
{
    TimeProcessPtr  aTimeProcess;
    TTValue         aCacheElement;
    TTSymbol        timeProcessType;
    
    if (inputValue.size() == 1) {
        
        if (inputValue[0].type() == kTypeObject) {
            
            aTimeProcess = TimeProcessPtr((TTObjectBasePtr)inputValue[0]);
            
            // set this scenario as parent scenario for the time process
            aTimeProcess->setAttributeValue(TTSymbol("scenario"), (TTObjectBasePtr)this);
            
            // create all attribute observers
            makeCacheElement(aTimeProcess, aCacheElement);
            
            // store time process object and observers
            mTimeProcessList.append(aCacheElement);

            // update the CSP depending on the type of the time process
            timeProcessType = aTimeProcess->getName();
            
            if (timeProcessType == TTSymbol("Automation")) {
                
                // TODO : update the CSP in Automation case
                // cf : CSP::addBox
                return kTTErrGeneric;
            }
            else if (timeProcessType == TTSymbol("Scenario")) {
                
                // TODO : update the CSP in Scenario case
                // cf : CSP::addBox
                return kTTErrGeneric;
            }
            else if (timeProcessType == TTSymbol("Relation")) {
                
                // TODO : update the CSP in Relation case
                // cf : CSP::addAntPostRelation
                return kTTErrGeneric;
            }
            // else if ...

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
                
                // delete all attribute observers
                deleteCacheElement(aCacheElement);
            
                // update the CSP depending on the type of the time process
                timeProcessType = aTimeProcess->getName();
                
                if (timeProcessType == TTSymbol("Automation")) {
                    
                    // TODO : update the CSP in Automation case
                    // cf : CSP::removeBox
                    return kTTErrGeneric;
                }
                else if (timeProcessType == TTSymbol("Scenario")) {
                    
                    // TODO : update the CSP in Scenario case
                    // cf : CSP::removeBox
                    return kTTErrGeneric;
                }
                else if (timeProcessType == TTSymbol("Relation")) {
                    
                    // TODO : update the CSP in Relation case
                    // cf : CSP::removeTemporalRelation
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

TTErr Scenario::TimeProcessStartChange(const TTValue& inputValue, TTValue& outputValue)
{
    TimeProcessPtr  aTimeProcess;
    TTSymbol        timeProcessType;
    TTUInt32        start;
	
    if (inputValue.size() == 2) {
        
        // TODO : use dictionnary
        if (inputValue[0].type() == kTypeObject && inputValue[1].type() == kTypeUInt32) {
            
            // get time process where the change comes from
            aTimeProcess = TimeProcessPtr((TTObjectBasePtr)inputValue[0]);
            
            // get new start value
            start = inputValue[1];
            
            // TODO : warn CSP that this time process start date have changed
            // TODO : update all CSP consequences by setting time processes attributes that are affected by the consequence
            timeProcessType = aTimeProcess->getName();
            
            if (timeProcessType == TTSymbol("Automation")) {
                
                // TODO : update the CSP in Automation case
                return kTTErrGeneric;
            }
            else if (timeProcessType == TTSymbol("Scenario")) {
                
                // TODO : update the CSP in Scenario case
                return kTTErrGeneric;
            }
            else if (timeProcessType == TTSymbol("Relation")) {
                
                // TODO : update the CSP in Relation case
                return kTTErrGeneric;
            }
            // else if ...
        }
    }
    
    return kTTErrGeneric;
}

TTErr Scenario::TimeProcessEndChange(const TTValue& inputValue, TTValue& outputValue)
{
    TimeProcessPtr  aTimeProcess;
    TTSymbol        timeProcessType;
    TTUInt32        end;
	
    if (inputValue.size() == 2) {
        
        // TODO : use dictionnary
        if (inputValue[0].type() == kTypeObject && inputValue[1].type() == kTypeUInt32) {
            
            // get time process where the change comes from
            aTimeProcess = TimeProcessPtr((TTObjectBasePtr)inputValue[0]);
            
            // get new end value
            end = inputValue[1];
            
            // TODO : warn CSP that this time process end date have changed
            // TODO : update all CSP consequences by setting time processes attributes that are affected by the consequence
            timeProcessType = aTimeProcess->getName();
            
            if (timeProcessType == TTSymbol("Automation")) {
                
                // TODO : update the CSP in Automation case
                return kTTErrGeneric;
            }
            else if (timeProcessType == TTSymbol("Scenario")) {
                
                // TODO : update the CSP in Scenario case
                return kTTErrGeneric;
            }
            else if (timeProcessType == TTSymbol("Relation")) {
                
                // TODO : update the CSP in Relation case
                return kTTErrGeneric;
            }
            // else if ...
        }
    }
    
    return kTTErrGeneric;
}

TTErr Scenario::TimeProcessStartTriggerAdd(const TTValue& inputValue, TTValue& outputValue)
{
    TimeProcessPtr  aTimeProcess;
	
    if (inputValue.size() == 1) {
        
        // TODO : use dictionnary
        if (inputValue[0].type() == kTypeObject) {
            
            // get time process where the start trigger is added
            aTimeProcess = TimeProcessPtr((TTObjectBasePtr)inputValue[0]);
            
            // TODO : warn CSP that this time process have a start trigger
            // TODO : update all CSP consequences by setting time processes attributes that are affected by the consequence
        }
    }
    
    return kTTErrGeneric;
}

TTErr Scenario::TimeProcessStartTriggerRemove(const TTValue& inputValue, TTValue& outputValue)
{
    TimeProcessPtr  aTimeProcess;
	
    if (inputValue.size() == 1) {
        
        // TODO : use dictionnary
        if (inputValue[0].type() == kTypeObject) {
            
            // get time process where the start trigger is removed
            aTimeProcess = TimeProcessPtr((TTObjectBasePtr)inputValue[0]);
            
            // TODO : warn CSP that this time process have no start trigger
            // TODO : update all CSP consequences by setting time processes attributes that are affected by the consequence
        }
    }
    
    return kTTErrGeneric;
}

TTErr Scenario::TimeProcessEndTriggerAdd(const TTValue& inputValue, TTValue& outputValue)
{
    TimeProcessPtr  aTimeProcess;
	
    if (inputValue.size() == 1) {
        
        // TODO : use dictionnary
        if (inputValue[0].type() == kTypeObject) {
            
            // get time process where the end trigger is added
            aTimeProcess = TimeProcessPtr((TTObjectBasePtr)inputValue[0]);
            
            // TODO : warn CSP that this time process have an end trigger
            // TODO : update all CSP consequences by setting time processes attributes that are affected by the consequence
        }
    }
    
    return kTTErrGeneric;
}

TTErr Scenario::TimeProcessEndTriggerRemove(const TTValue& inputValue, TTValue& outputValue)
{
    TimeProcessPtr  aTimeProcess;
	
    if (inputValue.size() == 1) {
        
        // TODO : use dictionnary
        if (inputValue[0].type() == kTypeObject) {
            
            // get time process where the end trigger is removed
            aTimeProcess = TimeProcessPtr((TTObjectBasePtr)inputValue[0]);
            
            // TODO : warn CSP that this time process have no end trigger
            // TODO : update all CSP consequences by setting time processes attributes that are affected by the consequence
        }
    }
    
    return kTTErrGeneric;
}

TTErr Scenario::WriteAsXml(const TTValue& inputValue, TTValue& outputValue)
{
	TTXmlHandlerPtr	aXmlHandler = NULL;
	
	aXmlHandler = TTXmlHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// TODO : pass the XmlHandlerPtr to all the time processes of the scenario to write their content into the file
	
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

void Scenario::makeCacheElement(TimeProcessPtr aTimeProcess, TTValue& newCacheElement)
{
    TTValue			v;
    TTAttributePtr	anAttribute = NULL;
	TTObjectBasePtr	runningObserver, progressionObserver;
    TTValuePtr		runningBaton, progressionBaton;
	
	// 0 : cache time process object
	newCacheElement.append((TTObjectBasePtr)aTimeProcess);
    
    // 1 : create and cache running Attribute observer on this time process
    aTimeProcess->findAttribute(TTSymbol("running"), &anAttribute);
    
    runningObserver = NULL;
    TTObjectBaseInstantiate(TTSymbol("callback"), &runningObserver, kTTValNONE);
    
    runningBaton = new TTValue(TTObjectBasePtr(this));
    runningBaton->append(TTObjectBasePtr(aTimeProcess));
    
    runningObserver->setAttributeValue(kTTSym_baton, TTPtr(runningBaton));
    runningObserver->setAttributeValue(kTTSym_function, TTPtr(&ScenarioTimeProcessRunningAttributeCallback));
    
    anAttribute->registerObserverForNotifications(*runningObserver);
    
    newCacheElement.append(runningObserver);
    
    // 2 : create and cache progression Attribute observer on this time process
    aTimeProcess->findAttribute(TTSymbol("progression"), &anAttribute);
    
    progressionObserver = NULL;
    TTObjectBaseInstantiate(TTSymbol("callback"), &progressionObserver, kTTValNONE);
    
    progressionBaton = new TTValue(TTObjectBasePtr(this));
    progressionBaton->append(TTObjectBasePtr(aTimeProcess));
    
    progressionObserver->setAttributeValue(kTTSym_baton, TTPtr(progressionBaton));
    progressionObserver->setAttributeValue(kTTSym_function, TTPtr(&ScenarioTimeProcessProgressionAttributeCallback));
    
    anAttribute->registerObserverForNotifications(*progressionObserver);
    
    newCacheElement.append(progressionObserver);
}

void Scenario::deleteCacheElement(const TTValue& oldCacheElement)
{
	TTValue			v;
    TTObjectBasePtr	aTimeProcess, anObserver;
	TTAttributePtr	anAttribute;
	TTErr			err;
    
    // 0 : get cached time process
    aTimeProcess = oldCacheElement[0];
    
    // 1 : delete running attribute observer
    anObserver = NULL;
    anObserver = oldCacheElement[1];
    
    anAttribute = NULL;
    err = aTimeProcess->findAttribute(TTSymbol("running"), &anAttribute);
    
    if (!err) {
        
        err = anAttribute->unregisterObserverForNotifications(*anObserver);
        
        if (!err)
            TTObjectBaseRelease(&anObserver);
    }
    
    // 2 : delete end attribute observer
    anObserver = NULL;
    anObserver = oldCacheElement[2];
    
    anAttribute = NULL;
    err = aTimeProcess->findAttribute(TTSymbol("progression"), &anAttribute);
    
    if (!err) {
        
        err = anAttribute->unregisterObserverForNotifications(*anObserver);
        
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

TTErr ScenarioTimeProcessRunningAttributeCallback(TTPtr baton, TTValue& data)
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
    
    // TODO : warn Scheduler that this time process running state have changed (?)
    timeProcessType = aTimeProcess->getName();
    
    if (timeProcessType == TTSymbol("Automation")) {
        
        // TODO : update the CSP in Automation case
        return kTTErrGeneric;
    }
    else if (timeProcessType == TTSymbol("Scenario")) {
        
        // TODO : update the CSP in Scenario case
        return kTTErrGeneric;
    }
    else if (timeProcessType == TTSymbol("Relation")) {
        
        // TODO : update the CSP in Relation case
        return kTTErrGeneric;
    }
    // else if ...
    
    return kTTErrGeneric;
}

TTErr ScenarioTimeProcessProgressionAttributeCallback(TTPtr baton, TTValue& data)
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
        
        // TODO : update the CSP in Automation case
        return kTTErrGeneric;
    }
    else if (timeProcessType == TTSymbol("Scenario")) {
        
        // TODO : update the CSP in Scenario case
        return kTTErrGeneric;
    }
    else if (timeProcessType == TTSymbol("Relation")) {
        
        // TODO : update the CSP in Relation case
        return kTTErrGeneric;
    }
    // else if ...
    
    return kTTErrGeneric;
}