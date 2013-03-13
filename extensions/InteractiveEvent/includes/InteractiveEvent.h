/*
 * InteractiveEvent time event
 * Copyright © 2013, Théo de la Hogue
 *
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

/*!
 * \class InteractiveEvent
 *
 *  InteractiveEvent time event class handle a callback to use it when it receive a value from an address
 *
 */

#ifndef __INTERACTIVE_EVENT_H__
#define __INTERACTIVE_EVENT_H__

#include "TimeEvent.h"

class InteractiveEvent : public TimeEvent
{
	TTCLASS_SETUP(InteractiveEvent)
	
private :
    
    TTAddress           mAddress;               ///< ATTRIBUTE : the address to listen
    
    TTObjectBasePtr     mReceiver;              ///< a receiver used to bind on the address
    
    /** Get parameters names needed by this time event
     @param	value           the returned parameter names
     @return                kTTErrNone */
	TTErr getParameterNames(TTValue& value);
    
    /** Specific trigger method
     @return                an error code returned by the process method */
    TTErr Trigger(const TTValue& inputValue, TTValue& outputValue);
    
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
    
    /** Set the address to listen 
     @param	value           an address
     @return                kTTErrNone */
    TTErr	setAddress(const TTValue& value);

    
    friend TTErr TT_EXTENSION_EXPORT InteractiveEventReceiverCallback(TTPtr baton, TTValue& data);
};

typedef InteractiveEvent* InteractiveEventPtr;

/** The receiver callback to trigger the event when the address is received
 @param	baton                       a time event to trigger
 @param	data                        a value to pass thru the trigger method
 @return							.. */
TTErr TT_EXTENSION_EXPORT InteractiveEventReceiverCallback(TTPtr baton, TTValue& data);

#endif // __INTERACTIVE_EVENT_H__
