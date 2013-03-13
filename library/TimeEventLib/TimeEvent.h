/*
 * A Time Event interface
 * Copyright © 2013, Théo de la Hogue, Clément Bossut
 *
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#ifndef __TIME_EVENT_H__
#define __TIME_EVENT_H__

#include "TTFoundationAPI.h"
#include "TTModular.h"

#define TIME_EVENT_CONSTRUCTOR \
TTObjectBasePtr thisTTClass :: instantiate (TTSymbol& name, TTValue& arguments) {return new thisTTClass (arguments);} \
\
extern "C" void thisTTClass :: registerClass () {TTClassRegister( TTSymbol(thisTTClassName), thisTTClassTags, thisTTClass :: instantiate );} \
\
thisTTClass :: thisTTClass (TTValue& arguments) : TimeEvent(arguments)

#define TIME_EVENT_INITIALIZE \
mName = TTSymbol(thisTTClassName); \
mVersion = TTSymbol(thisTimeEventVersion); \
mAuthor = TTSymbol(thisTimeEventAuthor); \
registerAttribute(TTSymbol("ParameterNames"), kTypeLocalValue, NULL, (TTGetterMethod)& thisTTClass::getParameterNames); \
/*addAttributeProperty(ParameterNames, readOnly, YES); \ */

/****************************************************************************************************/
// Class Specification

/**	Time Event is the base class for all Time Event Unit.
 A TimeEvent allows subcription to be notified depending a specific strategy based on the processing of the list of all triggered values
 */
class TimeEvent : public TTObjectBase {
    
public:
	TTSymbol                        mName;                          ///< ATTRIBUTE : the name of the time process
	TTSymbol                        mVersion;                       ///< ATTRIBUTE : the version of the time process
	TTSymbol                        mAuthor;                        ///< ATTRIBUTE : the author of the time process
    
protected:
    
    TTUInt32                        mDate;                          ///< ATTRIBUTE : the date of the event
    TTList                          mTriggerList;                   ///< ATTRIBUTE : all the triggered values to evaluate in Notify method
    TTList                          mSubscriberList;                ///< ATTRIBUTE : all the callbacks to use in Notify method
    
private:
    
    TTAttributePtr                  dateAttribute;                  ///< cache date attribute for observer notification
    
public:
	//** Constructor.	*/
	TimeEvent(TTValue& arguments);
	
	/** Destructor. */
	virtual ~TimeEvent();
	
	/** Get parameters names needed by this time event 
     @param	value           the returned parameter names
     @return                kTTErrNone */
	virtual TTErr   getParameterNames(TTValue& value) = 0;

    /** Specific triggering method : append the triggered value to the trigger list depending on a specific strategy
     @param	inputValue      a value to pass thru the TimeEventTriggerCallback
     @param	outputValue     kTTValNone
     @return                an error code returned by the trigger method */
    virtual TTErr   Trigger(const TTValue& inputValue, TTValue& outputValue) = 0;
    
    /** Specific notification method : evaluate the trigger list to notify the subscriber's depending on a specific strategy
     @return                an error code returned by the notify method */
    virtual TTErr   Notify() = 0;
    
    /**  needed to be handled by a TTXmlHandler
     @param	inputValue      ..
     @param	outputValue     ..
     @return                .. */
	virtual TTErr	WriteAsXml(const TTValue& inputValue, TTValue& outputValue) = 0;
	virtual TTErr	ReadFromXml(const TTValue& inputValue, TTValue& outputValue) = 0;
	
	/**  needed to be handled by a TTTextHandler
     @param	inputValue      ..
     @param	outputValue     ..
     @return                .. */
	virtual TTErr	WriteAsText(const TTValue& inputValue, TTValue& outputValue) = 0;
	virtual TTErr	ReadFromText(const TTValue& inputValue, TTValue& outputValue) = 0;
    
    /** Subscribe for event triggering
     @param	inputValue      a TTCallbackPtr to notify
     @param	outputValue     kTTValNone
     @return                kTTErrNone */
    TTErr           Subscribe(const TTValue& inputValue, TTValue& outputValue);
    
    /** Unsubscribe for event triggering
     @param	inputValue      a TTCallbackPtr to don't notify anymore
     @param	outputValue     kTTValNone
     @return                kTTErrNone */
    TTErr           Unsubscribe(const TTValue& inputValue, TTValue& outputValue);

private :
    
    /** Set the date of the time event
     @param	value           a date
     @return                an error code if the date is wrong */
    TTErr	setDate(const TTValue& value);
    
};

typedef TimeEvent* TimeEventPtr;

#endif // __TIME_EVENT_H__

#ifndef __TIME_EVENT_LIB_H__
#define __TIME_EVENT_LIB_H__

class TT_EXTENSION_EXPORT TimeEventLib {
    
public:

	/**	Return a list of all available time events 
     @param	value           the returned time event names*/
	static void getTimeEventNames(TTValue& timeProcessNames);
};

#endif	//__TIME_EVENT_LIB_H__