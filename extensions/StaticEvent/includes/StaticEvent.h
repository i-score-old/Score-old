/*
 * StaticEvent time event
 * Copyright © 2013, Théo de la Hogue, Clément Bossut
 *
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

/*!
 * \class StaticEvent
 *
 *  StaticEvent time event class handle a callback to use it at a planned date
 *
 */

#ifndef __STATIC_EVENT_H__
#define __STATIC_EVENT_H__

#include "TimeEvent.h"

class StaticEvent : public TimeEvent
{
	TTCLASS_SETUP(StaticEvent)
	
private :
    
    /** Get parameters names needed by this time event
     @param	value           the returned parameter names
     @return                kTTErrNone */
	TTErr   getParameterNames(TTValue& value);
    
    /** Specific triggering method : append the triggered value to the trigger list
     @param	inputValue      a value to pass thru the TimeEventTriggerCallback
     @param	outputValue     kTTValNone
     @return                an error code returned by the trigger method */
    TTErr   Trigger(const TTValue& inputValue, TTValue& outputValue);
    
    /** Specific notification method : notify all subscribers if the trigger list have at least one value
     @return                an error code returned by the notify method */
    TTErr   Notify();
    
	/**  needed to be handled by a TTXmlHandler
     @param	inputValue      ..
     @param	outputValue     ..
     @return                .. */
	TTErr	WriteAsXml(const TTValue& inputValue, TTValue& outputValue);
	TTErr	ReadFromXml(const TTValue& inputValue, TTValue& outputValue);
	
	/**  needed to be handled by a TTTextHandler
     @param	inputValue      ..
     @param	outputValue     ..
     @return                .. */
	TTErr	WriteAsText(const TTValue& inputValue, TTValue& outputValue);
	TTErr	ReadFromText(const TTValue& inputValue, TTValue& outputValue);
};

typedef StaticEvent* StaticEventPtr;

#endif // __STATIC_EVENT_H__
