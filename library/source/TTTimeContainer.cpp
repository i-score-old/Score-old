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
    
    
    addMessageWithArguments(TimeEventCreate);
    addMessageProperty(TimeEventCreate, hidden, YES);
    
    addMessageWithArguments(TimeEventRelease);
    addMessageProperty(TimeEventRelease, hidden, YES);
    
    addMessageWithArguments(TimeEventMove);
    addMessageProperty(TimeEventMove, hidden, YES);
    
    addMessageWithArguments(TimeEventInteractive);
    addMessageProperty(TimeEventInteractive, hidden, YES);
    
    addMessageWithArguments(TimeEventTrigger);
    addMessageProperty(TimeEventTrigger, hidden, YES);
    
    addMessageWithArguments(TimeEventReplace);
    addMessageProperty(TimeEventReplace, hidden, YES);
    
    
    addMessageWithArguments(TimeProcessCreate);
    addMessageProperty(TimeProcessCreate, hidden, YES);
    
    addMessageWithArguments(TimeProcessRelease);
    addMessageProperty(TimeProcessRelease, hidden, YES);
    
    addMessageWithArguments(TimeProcessMove);
    addMessageProperty(TimeProcessMove, hidden, YES);
    
    addMessageWithArguments(TimeProcessLimit);
    addMessageProperty(TimeProcessLimit, hidden, YES);
}

TTTimeContainer::~TTTimeContainer()
{
    ;
}

TTSymbol TTTimeContainer::getTimeEventName(TTTimeEventPtr aTimeEvent)
{
    return aTimeEvent->mName;
}

TTUInt32 TTTimeContainer::getTimeEventDate(TTTimeEventPtr aTimeEvent)
{
    return aTimeEvent->mDate;
}

TTBoolean TTTimeContainer::isTimeEventInteractive(TTTimeEventPtr aTimeEvent)
{
    return aTimeEvent->mInteractive;
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
