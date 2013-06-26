/** @file
 *
 * @ingroup scoreLibrary
 *
 * @brief an interface to easily add other time process
 *
 * @see TTScore, TTTimeProcess, TTTimeEvent
 *
 * @authors Théo de la Hogue & Clément Bossut
 *
 * @copyright Copyright © 2013, Théo de la Hogue & Clément Bossut @n
 * This code is licensed under the terms of the "CeCILL-C" @n
 * http://www.cecill.info
 */

#include "TimeProcessLib.h"

#define thisTTClass		TimeProcess

TimeProcess::TimeProcess(TTValue& arguments) :
TTTimeProcess(arguments)
{
    TT_ASSERT("Correct number of args to create TimeProcess", arguments.size() == 0);
}

TimeProcess::~TimeProcess()
{
    ;
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
		
		if (attributeName == TTSymbol("active")         ||
            attributeName == TTSymbol("start")          ||
            attributeName == TTSymbol("end"))
			continue;
		
		value.append(attributeName);
	}
	
	return kTTErrNone;
}

/***************************************************************************
 
 TTTimeProcessLib
 
 ***************************************************************************/

void TimeProcessLib::getTimeProcessNames(TTValue& timeProcessNames)
{
	timeProcessNames.clear();
	timeProcessNames.append(TTSymbol("Automation"));
    timeProcessNames.append(TTSymbol("Scenario"));
    timeProcessNames.append(TTSymbol("Interval"));
}

