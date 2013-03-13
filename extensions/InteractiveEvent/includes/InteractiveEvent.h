/*
 * InteractiveEvent time event
 * Copyright © 2013, Théo de la Hogue, Clément Bossut
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
    
    TTUInt32                        mDateMin;                       ///< ATTRIBUTE : the minimal date of the interactive event
    TTUInt32                        mDateMax;                       ///< ATTRIBUTE : the maximal date of the interactive event
    
    TTAddress                       mAddress;                       ///< ATTRIBUTE : the address to listen
    
    TTObjectBasePtr                 mReceiver;                      ///< a receiver used to bind on the address
    
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
