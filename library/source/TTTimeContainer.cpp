/** @file
 *
 * @ingroup scoreLibrary
 *
 * @brief a container class to manage time events and time processes instances in the time
 *
 * @see TTTimeEvent, TTTimeProcess
 *
 * @authors Théo de la Hogue & Clément Bossut
 *
 * @copyright Copyright © 2013, Théo de la Hogue & Clément Bossut @n
 * This code is licensed under the terms of the "CeCILL-C" @n
 * http://www.cecill.info
 */

#include "TTTimeContainer.h"

#define thisTTClass         TTTimeContainer
#define thisTTClassName     "TimeContainer"
#define thisTTClassTags     "time, container"

/****************************************************************************************************/

TTObjectBasePtr TTTimeContainer::instantiate (TTSymbol& name, TTValue& arguments)
{
	return new TTTimeContainer(arguments);
}


extern "C" void TTTimeContainer::registerClass()
{
	TTClassRegister(TTSymbol(thisTTClassName), thisTTClassTags, TTTimeContainer::instantiate);
}


TTTimeContainer :: TTTimeContainer (TTValue& arguments) :
TTTimeProcess(arguments)
{
    TT_ASSERT("Correct number of args to create TTTimeContainer", arguments.size() == 0);
    
    registerAttribute(TTSymbol("timeProcesses"), kTypeLocalValue, NULL, (TTGetterMethod)& TTTimeContainer::getTimeProcesses, NULL);
    registerAttribute(TTSymbol("timeEvents"), kTypeLocalValue, NULL, (TTGetterMethod)& TTTimeContainer::getTimeEvents, NULL);
    registerAttribute(TTSymbol("timeConditions"), kTypeLocalValue, NULL, (TTGetterMethod)& TTTimeContainer::getTimeConditions, NULL);
    
    addMessageWithArguments(TimeEventCreate);
    addMessageProperty(TimeEventCreate, hidden, YES);
    
    addMessageWithArguments(TimeEventRelease);
    addMessageProperty(TimeEventRelease, hidden, YES);
    
    addMessageWithArguments(TimeEventMove);
    addMessageProperty(TimeEventMove, hidden, YES);
    
    addMessageWithArguments(TimeEventCondition);
    addMessageProperty(TimeEventCondition, hidden, YES);
    
    addMessageWithArguments(TimeEventTrigger);
    addMessageProperty(TimeEventTrigger, hidden, YES);
    
    addMessageWithArguments(TimeEventReplace);
    addMessageProperty(TimeEventReplace, hidden, YES);
    
    addMessageWithArguments(TimeEventFind);
    addMessageProperty(TimeEventFind, hidden, YES);
    
    
    addMessageWithArguments(TimeProcessCreate);
    addMessageProperty(TimeProcessCreate, hidden, YES);
    
    addMessageWithArguments(TimeProcessRelease);
    addMessageProperty(TimeProcessRelease, hidden, YES);
    
    addMessageWithArguments(TimeProcessMove);
    addMessageProperty(TimeProcessMove, hidden, YES);
    
    addMessageWithArguments(TimeProcessLimit);
    addMessageProperty(TimeProcessLimit, hidden, YES);
    
    
    addMessageWithArguments(TimeConditionCreate);
    addMessageProperty(TimeConditionCreate, hidden, YES);
    
    addMessageWithArguments(TimeConditionRelease);
    addMessageProperty(TimeConditionRelease, hidden, YES);
}

TTTimeContainer::~TTTimeContainer()
{
    ;
}

TTErr TTTimeContainer::getTimeProcesses(TTValue& value)
{
    value.clear();
    
    if (mTimeProcessList.isEmpty())
        return kTTErrGeneric;
    
    for (mTimeProcessList.begin(); mTimeProcessList.end(); mTimeProcessList.next())
        value.append(mTimeProcessList.current()[0]); // théo : here we expect the Container plugin filled the list with the object at the [0] index ...
    
    return kTTErrNone;
}

TTErr TTTimeContainer::getTimeEvents(TTValue& value)
{
    value.clear();
    
    if (mTimeEventList.isEmpty())
        return kTTErrGeneric;
    
    for (mTimeEventList.begin(); mTimeEventList.end(); mTimeEventList.next())
        value.append(mTimeEventList.current()[0]); // théo : here we expect the Container plugin filled the list with the object at the [0] index ...
    
    return kTTErrNone;
}

TTErr TTTimeContainer::getTimeConditions(TTValue& value)
{
    value.clear();
    
    if (mTimeConditionList.isEmpty())
        return kTTErrGeneric;
    
    for (mTimeConditionList.begin(); mTimeConditionList.end(); mTimeConditionList.next())
        value.append(mTimeConditionList.current()[0]); // théo : here we expect the Container plugin filled the list with the object at the [0] index ...
    
    return kTTErrNone;
}

TTErr TTTimeContainer::TimeEventFind(const TTValue& inputValue, TTValue& outputValue)
{
    TTValue aCacheElement;
    
    // Find the process using his name inside the container
    mTimeEventList.find(&TTTimeContainerFindTimeEventWithName, (TTPtr)&inputValue, outputValue);
    
    if (outputValue == kTTValNONE)
        return kTTErrValueNotFound;
    
    return kTTErrNone;
}

TTSymbol TTTimeContainer::getTimeEventName(TTTimeEventPtr aTimeEvent)
{
    return aTimeEvent->mName;
}

TTUInt32 TTTimeContainer::getTimeEventDate(TTTimeEventPtr aTimeEvent)
{
    return aTimeEvent->mDate;
}

TTObjectBasePtr TTTimeContainer::getTimeEventCondition(TTTimeEventPtr aTimeEvent)
{
    return aTimeEvent->mCondition;
}

void TTTimeContainer::writeTimeEventAsXml(TTXmlHandlerPtr aXmlHandler, TTTimeEventPtr aTimeEvent)
{
    TTValue v;
    
    // Start an event node
    xmlTextWriterStartElement((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "event");
    
    // Write the name
    xmlTextWriterWriteAttribute((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "name", BAD_CAST aTimeEvent->mName.c_str());
    
    // Pass the xml handler to the event to fill his attribute
    v = TTObjectBasePtr(aTimeEvent);
    aXmlHandler->setAttributeValue(kTTSym_object, v);
    aXmlHandler->sendMessage(TTSymbol("Write"));
    
    // Close the event node
    xmlTextWriterEndElement((xmlTextWriterPtr)aXmlHandler->mWriter);
}

TTTimeEventPtr TTTimeContainer::readTimeEventFromXml(TTXmlHandlerPtr aXmlHandler)
{
    TTTimeEventPtr  aTimeEvent = NULL;
    TTValue         v, out;
    
    if (aXmlHandler->mXmlNodeStart) {
        
        // Get the date
        if (!aXmlHandler->getXmlAttribute(TTSymbol("date"), v, NO)) {
            
            if (v.size() == 1) {
                
                if (v[0].type() == kTypeUInt32) {
                    
                    // Create the time event
                    if (!this->TimeEventCreate(v, out)) {
                        
                        aTimeEvent = TTTimeEventPtr(TTObjectBasePtr(out[0]));
                        
                        // Get the name
                        if (!aXmlHandler->getXmlAttribute(kTTSym_name, v, YES)) {
                            
                            if (v.size() == 1) {
                                
                                if (v[0].type() == kTypeSymbol) {
                                    
                                    aTimeEvent->mName = v[0];
                                }
                            }
                        }
                        
                        // Pass the xml handler to the new event to fill his attribute
                        aXmlHandler->setAttributeValue(kTTSym_object, out);
                        aXmlHandler->sendMessage(TTSymbol("Read"));
                    }
                }
            }
        }
    }
    
    return aTimeEvent;
}

TTSymbol TTTimeContainer::getTimeProcessName(TTTimeProcessPtr aTimeProcess)
{
    return aTimeProcess->mName;
}

TTTimeEventPtr TTTimeContainer::getTimeProcessStartEvent(TTTimeProcessPtr aTimeProcess)
{
    return aTimeProcess->getStartEvent();
}

void TTTimeContainer::setTimeProcessStartEvent(TTTimeProcessPtr aTimeProcess, TTTimeEventPtr aTimeEvent)
{
    aTimeProcess->setStartEvent(aTimeEvent);
}

TTTimeEventPtr TTTimeContainer::getTimeProcessEndEvent(TTTimeProcessPtr aTimeProcess)
{
    return aTimeProcess->getEndEvent();
}

void TTTimeContainer::setTimeProcessEndEvent(TTTimeProcessPtr aTimeProcess, TTTimeEventPtr aTimeEvent)
{
    aTimeProcess->setEndEvent(aTimeEvent);
}

TTUInt32 TTTimeContainer::getTimeProcessDurationMin(TTTimeProcessPtr aTimeProcess)
{
    return aTimeProcess->mDurationMin;
}

TTUInt32 TTTimeContainer::getTimeProcessDurationMax(TTTimeProcessPtr aTimeProcess)
{
    return aTimeProcess->mDurationMax;
}

void TTTimeContainer::writeTimeProcessAsXml(TTXmlHandlerPtr aXmlHandler, TTTimeProcessPtr aTimeProcess)
{
    TTValue     v;
    TTString    s;
    
    // Start a node with the type of the process
    xmlTextWriterStartElement((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST aTimeProcess->getName().c_str());
    
    // Write the name
    xmlTextWriterWriteAttribute((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "name", BAD_CAST aTimeProcess->mName.c_str());
    
    // Write the start event name
    xmlTextWriterWriteAttribute((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "start", BAD_CAST aTimeProcess->getStartEvent()->mName.c_str());
    
    // Write the end event name
    xmlTextWriterWriteAttribute((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "end", BAD_CAST aTimeProcess->getEndEvent()->mName.c_str());
    
    
    if (aTimeProcess->mDurationMin > 0 || aTimeProcess->mDurationMax > 0) {
        
        // Write the duration min
        v = aTimeProcess->mDurationMin;
        v.toString();
        s = TTString(v[0]);
        xmlTextWriterWriteAttribute((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "durationMin", BAD_CAST s.data());
        
        // Write the duration max
        v = aTimeProcess->mDurationMax;
        v.toString();
        s = TTString(v[0]);
        xmlTextWriterWriteAttribute((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "durationMax", BAD_CAST s.data());
    }
    
    // Pass the xml handler to the process to fill his attribute
    v = TTObjectBasePtr(aTimeProcess);
    aXmlHandler->setAttributeValue(kTTSym_object, v);
    aXmlHandler->sendMessage(TTSymbol("Write"));
    
    // Close the process node
    xmlTextWriterEndElement((xmlTextWriterPtr)aXmlHandler->mWriter);
}

TTTimeProcessPtr TTTimeContainer::readTimeProcessFromXml(TTXmlHandlerPtr aXmlHandler)
{
    TTTimeProcessPtr    aTimeProcess = NULL;
    TTTimeEventPtr      start = NULL;
    TTTimeEventPtr      end = NULL;
    TTValue             v, out, aCacheElement;
    
    // Get the name of the start event
    if (!aXmlHandler->getXmlAttribute(TTSymbol("start"), v, YES)) {
        
        if (v.size() == 1) {
            
            if (v[0].type() == kTypeSymbol) {
                
                // Find the start event using his name inside the container
                mTimeEventList.find(&TTTimeContainerFindTimeEventWithName, (TTPtr)&v, aCacheElement);
                
                if (aCacheElement == kTTValNONE)
                    return NULL;
                
                start = TTTimeEventPtr(TTObjectBasePtr(aCacheElement[0]));
            }
        }
    }
    
    // Get the name of the end event
    if (!aXmlHandler->getXmlAttribute(TTSymbol("end"), v, YES)) {
        
        if (v.size() == 1) {
            
            if (v[0].type() == kTypeSymbol) {
                
                // Find the end event using his name inside the container
                mTimeEventList.find(&TTTimeContainerFindTimeEventWithName, (TTPtr)&v, aCacheElement);
                
                if (aCacheElement == kTTValNONE)
                    return NULL;
                
                end = TTTimeEventPtr(TTObjectBasePtr(aCacheElement[0]));
            }
        }
    }
    
    if (!start || !end)
        return NULL;
    
    // DEBUG
    TTSymbol name = aXmlHandler->mXmlNodeName;
    
    // Create the time process
    v = aXmlHandler->mXmlNodeName;
    v.append(TTObjectBasePtr(start));
    v.append(TTObjectBasePtr(end));
    
    if (!this->TimeProcessCreate(v, out)) {
        
        aTimeProcess = TTTimeProcessPtr(TTObjectBasePtr(out[0]));
        
        // Get the time process name
        if (!aXmlHandler->getXmlAttribute(kTTSym_name, v, YES)) {
            
            if (v.size() == 1) {
                
                if (v[0].type() == kTypeSymbol) {
                    
                    aTimeProcess->setAttributeValue(kTTSym_name, v);
                }
            }
        }
        
        // Get the durationMin
        if (!aXmlHandler->getXmlAttribute(TTSymbol("durationMin"), v, NO)) {
            
            if (v.size() == 1) {
                
                if (v[0].type() == kTypeUInt32) {
                    
                    aTimeProcess->setAttributeValue(TTSymbol("durationMin"), v);
                }
            }
        }
        
        // Get the durationMax
        if (!aXmlHandler->getXmlAttribute(TTSymbol("durationMax"), v, NO)) {
            
            if (v.size() == 1) {
                
                if (v[0].type() == kTypeUInt32) {
                    
                    aTimeProcess->setAttributeValue(TTSymbol("durationMax"), v);
                }
            }
        }
        
        // Pass the xml handler to the new process to fill his attribute
        aXmlHandler->setAttributeValue(kTTSym_object, out);
        aXmlHandler->sendMessage(TTSymbol("Read"));
    }
    
    return aTimeProcess;
}

void TTTimeContainer::writeTimeConditionAsXml(TTXmlHandlerPtr aXmlHandler, TTTimeConditionPtr aTimeCondition)
{
    TTValue v;
    
    // Start a condition node
    xmlTextWriterStartElement((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "condition");
    
    // Pass the xml handler to the condition to fill his attribute
    v = TTObjectBasePtr(aTimeCondition);
    aXmlHandler->setAttributeValue(kTTSym_object, v);
    aXmlHandler->sendMessage(TTSymbol("Write"));
    
    // Close the condition node
    xmlTextWriterEndElement((xmlTextWriterPtr)aXmlHandler->mWriter);
}

TTTimeConditionPtr TTTimeContainer::readTimeConditionFromXml(TTXmlHandlerPtr aXmlHandler)
{
    TTTimeConditionPtr  aTimeCondition = NULL;
    TTValue             v, out;
    
    if (aXmlHandler->mXmlNodeStart) {
        
        // Create the time condition
        if (!this->TimeConditionCreate(v, out)) {
            
            aTimeCondition = TTTimeConditionPtr(TTObjectBasePtr(out[0]));
            
            // Pass the xml handler to the new condition to fill his attribute
            aXmlHandler->setAttributeValue(kTTSym_object, out);
            aXmlHandler->sendMessage(TTSymbol("Read"));
        }
    }
    
    return aTimeCondition;
}

#if 0
#pragma mark -
#pragma mark Some Methods
#endif

void TTTimeContainerFindTimeProcess(const TTValue& aValue, TTPtr timeProcessPtrToMatch, TTBoolean& found)
{
	found = (TTObjectBasePtr)aValue[0] == (TTObjectBasePtr)timeProcessPtrToMatch;
}

void TTTimeContainerFindTimeEvent(const TTValue& aValue, TTPtr timeEventPtrToMatch, TTBoolean& found)
{
    found = (TTObjectBasePtr)aValue[0] == (TTObjectBasePtr)timeEventPtrToMatch;
}

void TTTimeContainerFindTimeEventWithName(const TTValue& aValue, TTPtr timeEventNamePtrToMatch, TTBoolean& found)
{
    TTValuePtr      b = TTValuePtr(timeEventNamePtrToMatch);
    TTSymbol        nameToMatch = (*b)[0];
    TTTimeEventPtr  timeEventToTest = TTTimeEventPtr(TTObjectBasePtr(aValue[0]));
    
    found = timeEventToTest->mName == nameToMatch;
}

void TTTimeContainerFindTimeProcessWithTimeEvent(const TTValue& aValue, TTPtr timeEventPtrToMatch, TTBoolean& found)
{
    TTTimeProcessPtr    aTimeProcess = TTTimeProcessPtr(TTObjectBasePtr(aValue[0]));
    TTValue             v;
    
    // check start event
    found = aTimeProcess->getStartEvent() == TTObjectBasePtr(timeEventPtrToMatch);
        
    if (found)
        return;
        
    // check end event
    found = aTimeProcess->getEndEvent() == TTObjectBasePtr(timeEventPtrToMatch);
        
    return;
}

void TTSCORE_EXPORT TTTimeContainerFindTimeCondition(const TTValue& aValue, TTPtr timeConditionPtrToMatch, TTBoolean& found)
{
    found = (TTObjectBasePtr)aValue[0] == (TTObjectBasePtr)timeConditionPtrToMatch;
}
