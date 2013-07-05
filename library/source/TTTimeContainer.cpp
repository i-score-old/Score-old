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
