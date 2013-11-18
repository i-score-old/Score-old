/** @file
 *
 * @ingroup scoreLibrary
 *
 * @brief an interface to easily add other time process or time container
 *
 * @see TTScore, TTTimeProcess, TTTimeEvent, TTTimeContainer
 *
 * @authors Théo de la Hogue & Clément Bossut
 *
 * @copyright Copyright © 2013, Théo de la Hogue & Clément Bossut @n
 * This code is licensed under the terms of the "CeCILL-C" @n
 * http://www.cecill.info
 */

#include "TimePluginLib.h"

TimeProcess::TimeProcess(const TTValue& arguments) :
TTTimeProcess(arguments)
{
    TT_ASSERT("Correct number of args to create TimeProcess", arguments.size() == 0);
}

TimeProcess::~TimeProcess()
{
    ;
}


TimeContainer::TimeContainer(const TTValue& arguments) :
TTTimeContainer(arguments)
{
    TT_ASSERT("Correct number of args to create TimeContainer", arguments.size() == 0);
}

TimeContainer::~TimeContainer()
{
    ;
}

/***************************************************************************
 
 TTTimePluginLib
 
 ***************************************************************************/

void TimePluginLib::getTimePluginNames(TTValue& timePluginNames)
{
	timePluginNames.clear();
	timePluginNames.append(TTSymbol("Automation"));
    timePluginNames.append(TTSymbol("Scenario"));
    timePluginNames.append(TTSymbol("Interval"));
}

