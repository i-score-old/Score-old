/*
 * A Time Event interface
 * Copyright © 2013, Théo de la Hogue
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

typedef void (*TimeEventTriggerCallback)(TimeProcessPtr, const TTValue&);

/****************************************************************************************************/
// Class Specification

/**	Time Process is the base class for all Time Process Unit.
 It still has knowledge and support for ...
 */
class TimeEvent : public TTObjectBase {
    
public:
	TTSymbol                        mName;                          ///< ATTRIBUTE : the name of the time process
	TTSymbol                        mVersion;                       ///< ATTRIBUTE : the version of the time process
	TTSymbol                        mAuthor;                        ///< ATTRIBUTE : the author of the time process
    
protected:
    
    // TimeProcessPtr               mParent; ?
    
    TTUInt32                        mDate;                          ///< ATTRIBUTE : the date of the event
    
    TTCallbackPtr                   mCallback;                      ///< a callback to notify the time process owner that the event appends
    
private:
    
    TTAttributePtr                  dateAttribute;                  ///< cache active attribute for observer notification
    TTMessagePtr                    triggerMessage;                 ///< cache trigger message for observer notification
    
public:
	//** Constructor.	*/
	TimeEvent(TTValue& arguments);
	
	/** Destructor. */
	virtual ~TimeEvent();
	
	/** Get parameters names needed by this time event 
     @param	value           the returned parameter names
     @return                kTTErrNone */
	virtual TTErr getParameterNames(TTValue& value) = 0;
    
    /** Specific trigger method
     @return                an error code returned by the trigger method */
    virtual TTErr Trigger() = 0;

private :
    
    /** Set the date of the time event
     @param	value           a date
     @return                an error code if the date is wrong */
    //TTErr	setDate(const TTValue& value);
    
	/**  needed to be handled by a TTXmlHandler
     @param	value           ..
     @return                .. */
	TTErr	WriteAsXml(const TTValue& inputValue, TTValue& outputValue);
	TTErr	ReadFromXml(const TTValue& inputValue, TTValue& outputValue);
	
	/**  needed to be handled by a TTTextHandler
     @param	value           ..
     @return                .. */
	TTErr	WriteAsText(const TTValue& inputValue, TTValue& outputValue);
	TTErr	ReadFromText(const TTValue& inputValue, TTValue& outputValue);
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