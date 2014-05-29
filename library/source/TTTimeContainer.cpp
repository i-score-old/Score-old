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
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlreader.h>

#define thisTTClass         TTTimeContainer
#define thisTTClassName     "TimeContainer"
#define thisTTClassTags     "time, container"

/****************************************************************************************************/

TTObjectBasePtr TTTimeContainer::instantiate (TTSymbol name, const TTValue arguments)
{
	return new TTTimeContainer(arguments);
}


extern "C" void TTTimeContainer::registerClass()
{
	TTClassRegister(TTSymbol(thisTTClassName), thisTTClassTags, TTTimeContainer::instantiate);
}


TTTimeContainer :: TTTimeContainer (const TTValue& arguments) :
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
    
    addMessageWithArguments(TimeEventDispose);
    addMessageProperty(TimeEventDispose, hidden, YES);

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
    
    addMessageWithArguments(SchedulerSpeedChanged);
    addMessageProperty(SchedulerSpeedChanged, hidden, YES);
    
    TTObject thisObject(this);
    mScheduler.registerObserverForNotifications(thisObject);
}

TTTimeContainer::~TTTimeContainer()
{
    TTObject thisObject(this);
    mScheduler.unregisterObserverForNotifications(thisObject);
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
        value.append(mTimeEventList.current()[0]); // théo : here we expect the Container plugin fills the list with the object at the [0] index ...
    
    return kTTErrNone;
}

TTErr TTTimeContainer::getTimeConditions(TTValue& value)
{
    value.clear();
    
    if (mTimeConditionList.isEmpty())
        return kTTErrGeneric;
    
    for (mTimeConditionList.begin(); mTimeConditionList.end(); mTimeConditionList.next())
        value.append(mTimeConditionList.current()[0]); // théo : here we expect the Container plugin fills the list with the object at the [0] index ...
    
    return kTTErrNone;
}

TTErr TTTimeContainer::TimeEventFind(const TTValue& inputValue, TTValue& outputValue)
{
    TTValue aCacheElement;
    
    // Find the process using his name inside the container
    mTimeEventList.find(&TTTimeContainerFindTimeEventWithName, (TTPtr)&inputValue, outputValue);
    
    if (outputValue.size() == 0)
        return kTTErrValueNotFound;
    
    return kTTErrNone;
}

TTSymbol TTTimeContainer::getTimeEventName(TTObject& aTimeEvent)
{
    return TTTimeEventPtr(aTimeEvent.instance())->mName;
}

TTUInt32 TTTimeContainer::getTimeEventDate(TTObject& aTimeEvent)
{
    return TTTimeEventPtr(aTimeEvent.instance())->mDate;
}

TTObject& TTTimeContainer::getTimeEventState(TTObject& aTimeEvent)
{
    return TTTimeEventPtr(aTimeEvent.instance())->mState;
}

TTObject& TTTimeContainer::getTimeEventCondition(TTObject& aTimeEvent)
{
    return TTTimeEventPtr(aTimeEvent.instance())->mCondition;
}

void TTTimeContainer::writeTimeEventAsXml(TTXmlHandlerPtr aXmlHandler, TTObject& aTimeEvent)
{
    // Start an event node
    xmlTextWriterStartElement((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "event");
    
    // Write the name
    xmlTextWriterWriteAttribute((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "name", BAD_CAST TTTimeEventPtr(aTimeEvent.instance())->mName.c_str());
    
    // Pass the xml handler to the event to fill his attribute
    aXmlHandler->setAttributeValue(kTTSym_object, aTimeEvent);
    aXmlHandler->sendMessage(kTTSym_Write);
    
    // Close the event node
    xmlTextWriterEndElement((xmlTextWriterPtr)aXmlHandler->mWriter);
}

TTErr TTTimeContainer::readTimeEventFromXml(TTXmlHandlerPtr aXmlHandler, TTObject& aNewTimeEvent)
{
    TTValue v, out;
    TTErr   err = kTTErrGeneric;
    
    if (aXmlHandler->mXmlNodeStart) {
        
        // Get the date
        if (!aXmlHandler->getXmlAttribute(kTTSym_date, v, NO)) {
            
            if (v.size() == 1) {
                
                if (v[0].type() == kTypeUInt32) {
                    
                    // an event cannot be created after the end even of its container
                    if (TTUInt32(v[0]) > TTTimeEventPtr(getEndEvent().instance())->mDate) {
                        
                        TTLogError("TTTimeContainer::readTimeEventFromXml : event created after the end event of its container\n");
                        return kTTErrGeneric;
                    }
                    
                    // Create the time event
                    err = this->TimeEventCreate(v, out);
                    
                    if (!err) {
                        
                        aNewTimeEvent = out[0];
                        
                        // Get the name
                        if (!aXmlHandler->getXmlAttribute(kTTSym_name, v, YES)) {
                            
                            if (v.size() == 1) {
                                
                                if (v[0].type() == kTypeSymbol) {
                                    
                                    aNewTimeEvent.set(kTTSym_name, v);
                                }
                            }
                        }
                        
                        // Pass the xml handler to the new event to fill his attribute
                        aXmlHandler->setAttributeValue(kTTSym_object, out);
                        return aXmlHandler->sendMessage(kTTSym_Read);
                    }
                }
            }
        }
    }
    
    return err;
}

TTSymbol TTTimeContainer::getTimeProcessName(TTObject& aTimeProcess)
{
    return TTTimeProcessPtr(aTimeProcess.instance())->mName;
}

TTObject& TTTimeContainer::getTimeProcessStartEvent(TTObject& aTimeProcess)
{
    return TTTimeProcessPtr(aTimeProcess.instance())->getStartEvent();
}

void TTTimeContainer::setTimeProcessStartEvent(TTObject& aTimeProcess, TTObject& aTimeEvent)
{
    TTTimeProcessPtr(aTimeProcess.instance())->setStartEvent(aTimeEvent);
}

TTObject& TTTimeContainer::getTimeProcessEndEvent(TTObject& aTimeProcess)
{
    return TTTimeProcessPtr(aTimeProcess.instance())->getEndEvent();
}

void TTTimeContainer::setTimeProcessEndEvent(TTObject& aTimeProcess, TTObject& aTimeEvent)
{
    TTTimeProcessPtr(aTimeProcess.instance())->setEndEvent(aTimeEvent);
}

TTUInt32 TTTimeContainer::getTimeProcessDurationMin(TTObject& aTimeProcess)
{
    return TTTimeProcessPtr(aTimeProcess.instance())->mDurationMin;
}

TTUInt32 TTTimeContainer::getTimeProcessDurationMax(TTObject& aTimeProcess)
{
    return TTTimeProcessPtr(aTimeProcess.instance())->mDurationMax;
}

TTBoolean TTTimeContainer::getTimeProcessRunning(TTObject& aTimeProcess)
{
    return TTTimeProcessPtr(aTimeProcess.instance())->mRunning;
}

void TTTimeContainer::writeTimeProcessAsXml(TTXmlHandlerPtr aXmlHandler, TTObject& aTimeProcess)
{
    TTTimeProcessPtr aTimeProcessInstance = TTTimeProcessPtr(aTimeProcess.instance());
    TTValue     v;
    TTString    s;
    
    // If the process is handled by a upper scenario
    if (aTimeProcessInstance->mContainer.valid()) {
        
        // Start a node with the type of the process
        xmlTextWriterStartElement((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST aTimeProcessInstance->getName().c_str());
    }
    
    // Write the name
    xmlTextWriterWriteAttribute((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "name", BAD_CAST aTimeProcessInstance->mName.c_str());
    
    // If the process is handled by a upper scenario
    if (aTimeProcessInstance->mContainer.valid()) {
    
        // Write the start event name
        aTimeProcessInstance->getStartEvent().get("name", v);
        v.toString();
        s = TTString(v[0]);
        xmlTextWriterWriteAttribute((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "start", BAD_CAST s.data());
    
        // Write the end event name
        aTimeProcessInstance->getEndEvent().get("name", v);
        v.toString();
        s = TTString(v[0]);
        xmlTextWriterWriteAttribute((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "end", BAD_CAST s.data());
    }
    
    
    if (aTimeProcessInstance->mDurationMin > 0 || aTimeProcessInstance->mDurationMax > 0) {
        
        // Write the duration min
        v = aTimeProcessInstance->mDurationMin;
        v.toString();
        s = TTString(v[0]);
        xmlTextWriterWriteAttribute((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "durationMin", BAD_CAST s.data());
        
        // Write the duration max
        v = aTimeProcessInstance->mDurationMax;
        v.toString();
        s = TTString(v[0]);
        xmlTextWriterWriteAttribute((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "durationMax", BAD_CAST s.data());
    }
    
    // Write the color
    v = aTimeProcessInstance->mColor;
    v.toString();
    s = TTString(v[0]);
    xmlTextWriterWriteAttribute((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "color", BAD_CAST s.data());
    
    // If the process is handled by a upper scenario
    if (aTimeProcessInstance->mContainer.valid()) {
        
        // Write the vertical position
        v = aTimeProcessInstance->mVerticalPosition;
        v.toString();
        s = TTString(v[0]);
        xmlTextWriterWriteAttribute((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "verticalPosition", BAD_CAST s.data());
        
        // Write the vertical size
        v = aTimeProcessInstance->mVerticalSize;
        v.toString();
        s = TTString(v[0]);
        xmlTextWriterWriteAttribute((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "verticalSize", BAD_CAST s.data());
        
        // Pass the xml handler to the process to fill his attribute
        aXmlHandler->setAttributeValue(kTTSym_object, v);
        aXmlHandler->sendMessage(kTTSym_Write);
        
        // Close the process node
        xmlTextWriterEndElement((xmlTextWriterPtr)aXmlHandler->mWriter);
    }
}

TTErr TTTimeContainer::readTimeProcessFromXml(TTXmlHandlerPtr aXmlHandler, TTObject& aNewTimeProcess)
{
    TTObject    start;
    TTObject    end;
    TTValue     v, out, aCacheElement;
    TTErr       err;
    
    // Get the name of the start event
    if (!aXmlHandler->getXmlAttribute(kTTSym_start, v, YES)) {
        
        if (v.size() == 1) {
            
            if (v[0].type() == kTypeSymbol) {
                
                // Find the start event using his name inside the container
                mTimeEventList.find(&TTTimeContainerFindTimeEventWithName, (TTPtr)&v, aCacheElement);
                
                if (aCacheElement.size() == 0)
                    return kTTErrGeneric;
                
                start = aCacheElement[0];
            }
        }
    }
    
    // Get the name of the end event
    if (!aXmlHandler->getXmlAttribute(kTTSym_end, v, YES)) {
        
        if (v.size() == 1) {
            
            if (v[0].type() == kTypeSymbol) {
                
                // Find the end event using his name inside the container
                mTimeEventList.find(&TTTimeContainerFindTimeEventWithName, (TTPtr)&v, aCacheElement);
                
                if (aCacheElement.size() == 0)
                    return kTTErrGeneric;
                
                end = aCacheElement[0];
            }
        }
    }
    
    if (!start.valid() || !end.valid())
        return kTTErrGeneric;
    
    // check start and end events are different
    if (start == end)
        return kTTErrGeneric;
    
    // Create the time process
    v = TTValue(aXmlHandler->mXmlNodeName, start, end);
    err = this->TimeProcessCreate(v, out);
    
    if (!err) {
        
        aNewTimeProcess = out[0];
        
        // Get all generic time process atttributes
        
        // Get the time process name
        if (!aXmlHandler->getXmlAttribute(kTTSym_name, v, YES)) {
            
            if (v.size() == 1) {
                
                if (v[0].type() == kTypeSymbol) {
                    
                    aNewTimeProcess.set(kTTSym_name, v);
                }
            }
        }
        
        // Get the durationMin
        if (!aXmlHandler->getXmlAttribute(kTTSym_durationMin, v, NO)) {
            
            if (v.size() == 1) {
                
                if (v[0].type() == kTypeUInt32) {
                    
                    aNewTimeProcess.set(kTTSym_durationMin, v);
                }
            }
        }
        
        // Get the durationMax
        if (!aXmlHandler->getXmlAttribute(kTTSym_durationMax, v, NO)) {
            
            if (v.size() == 1) {
                
                if (v[0].type() == kTypeUInt32) {
                    
                    aNewTimeProcess.set(kTTSym_durationMax, v);
                }
            }
        }
        
        // Get the color
        if (!aXmlHandler->getXmlAttribute(kTTSym_color, v, NO)) {
            
            if (v.size() == 3) {
                
                if (v[0].type() == kTypeInt32 && v[1].type() == kTypeInt32 && v[2].type() == kTypeInt32) {
                    
                    aNewTimeProcess.set(kTTSym_color, v);
                }
            }
        }
        
        // Get the vertical position
        if (!aXmlHandler->getXmlAttribute(kTTSym_verticalPosition, v, NO)) {
            
            if (v.size() == 1) {
                
                if (v[0].type() == kTypeUInt32) {
                    
                    aNewTimeProcess.set(kTTSym_verticalPosition, v);
                }
            }
        }
        
        // Get the vertical size
        if (!aXmlHandler->getXmlAttribute(kTTSym_verticalSize, v, NO)) {
            
            if (v.size() == 1) {
                
                if (v[0].type() == kTypeUInt32) {
                    
                    aNewTimeProcess.set(kTTSym_verticalSize, v);
                }
            }
        }
    }
    
    return err;
}

void TTTimeContainer::writeTimeConditionAsXml(TTXmlHandlerPtr aXmlHandler, TTObject& aTimeCondition)
{
    // Start a condition node
    xmlTextWriterStartElement((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "condition");
    
    // Pass the xml handler to the condition to fill his attribute
    aXmlHandler->setAttributeValue(kTTSym_object, aTimeCondition);
    aXmlHandler->sendMessage(kTTSym_Write);
    
    // Close the condition node
    xmlTextWriterEndElement((xmlTextWriterPtr)aXmlHandler->mWriter);
}

TTErr TTTimeContainer::readTimeConditionFromXml(TTXmlHandlerPtr aXmlHandler, TTObject& aNewTimeCondition)
{
    TTValue v, out;
    TTErr   err = kTTErrGeneric;
    
    if (aXmlHandler->mXmlNodeStart) {
        
        // Create the time condition
        err = this->TimeConditionCreate(v, out);
        
        if (!err) {
            
            aNewTimeCondition = out[0];
            
            // Pass the xml handler to the new condition to fill his attribute
            aXmlHandler->setAttributeValue(kTTSym_object, out);
            return aXmlHandler->sendMessage(kTTSym_Read);
        }
    }
    
    return err;
}

#if 0
#pragma mark -
#pragma mark Notifications
#endif

TTErr TTTimeContainer::SchedulerSpeedChanged(const TTValue& inputValue, TTValue& outputValue)
{
    TT_ASSERT("TTTimeContainer::SchedulerSpeedChanged : inputValue is correct", inputValue.size() == 1 && inputValue[0].type() == kTypeFloat64);
    
    TTObject    aTimeProcess;
    TTObject    aScheduler;
    TTValue     v;
    
    // for each time process
    for (mTimeProcessList.begin(); mTimeProcessList.end(); mTimeProcessList.next()) {
        
        aTimeProcess = mTimeProcessList.current()[0];
        
        // get the actual time process scheduler
        aTimeProcess.get("scheduler", v);
        aScheduler = v[0];
        
        // set the time process scheduler speed value with the container scheduler speed value
        aScheduler.set(kTTSym_speed, inputValue);
    }
    
    return kTTErrNone;
}

#if 0
#pragma mark -
#pragma mark Some Methods
#endif

void TTTimeContainerFindTimeProcess(const TTValue& aValue, TTPtr timeProcessPtrToMatch, TTBoolean& found)
{
    TTObject anObject = aValue[0];
	found = anObject.instance() == (TTObjectBasePtr)timeProcessPtrToMatch;
}

void TTTimeContainerFindTimeEvent(const TTValue& aValue, TTPtr timeEventPtrToMatch, TTBoolean& found)
{
    TTObject anObject = aValue[0];
	found = anObject.instance() == (TTObjectBasePtr)timeEventPtrToMatch;
}

void TTTimeContainerFindTimeEventWithName(const TTValue& aValue, TTPtr timeEventNamePtrToMatch, TTBoolean& found)
{
    TTValuePtr  b = TTValuePtr(timeEventNamePtrToMatch);
    TTSymbol    nameToMatch = (*b)[0];
    TTObject    timeEventToTest = aValue[0];
    
    found = TTTimeEventPtr(timeEventToTest.instance())->mName == nameToMatch;
}

void TTTimeContainerFindTimeProcessWithTimeEvent(const TTValue& aValue, TTPtr timeEventPtrToMatch, TTBoolean& found)
{
    TTObject    aTimeProcess = aValue[0];
    TTValue     v;
    
    // check start event
    found = TTTimeProcessPtr(aTimeProcess.instance())->getStartEvent() == TTObjectBasePtr(timeEventPtrToMatch);
        
    if (found)
        return;
        
    // check end event
    found = TTTimeProcessPtr(aTimeProcess.instance())->getEndEvent() == TTObjectBasePtr(timeEventPtrToMatch);
        
    return;
}

void TTTimeContainerFindTimeCondition(const TTValue& aValue, TTPtr timeConditionPtrToMatch, TTBoolean& found)
{
    TTObject anObject = aValue[0];
	found = anObject.instance() == (TTObjectBasePtr)timeConditionPtrToMatch;
}
