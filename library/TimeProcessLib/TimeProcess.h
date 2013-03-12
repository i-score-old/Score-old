/*
 * A Time Process interface
 * Copyright © 2013, Théo de la Hogue
 *
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#ifndef __TIME_PROCESS_H__
#define __TIME_PROCESS_H__

#include "TTFoundationAPI.h"
#include "TTModular.h"

#include "TimeEvent.h"

#define TIME_PROCESS_CONSTRUCTOR \
TTObjectBasePtr thisTTClass :: instantiate (TTSymbol& name, TTValue& arguments) {return new thisTTClass (arguments);} \
\
extern "C" void thisTTClass :: registerClass () {TTClassRegister( TTSymbol(thisTTClassName), thisTTClassTags, thisTTClass :: instantiate );} \
\
thisTTClass :: thisTTClass (TTValue& arguments) : TimeProcess(arguments)

#define TIME_PROCESS_INITIALIZE \
mName = TTSymbol(thisTTClassName); \
mVersion = TTSymbol(thisTimeProcessVersion); \
mAuthor = TTSymbol(thisTimeProcessAuthor); \
registerAttribute(TTSymbol("ParameterNames"), kTypeLocalValue, NULL, (TTGetterMethod)& thisTTClass::getParameterNames); \
/*addAttributeProperty(ParameterNames, readOnly, YES); \ */

typedef void (*TimeProcessProgressionCallback)(TTPtr, TTFloat64);

// we assume the Scenario plugin exists and it is loaded
class Scenario;
typedef Scenario* ScenarioPtr;

/****************************************************************************************************/
// Class Specification

/**	Time Process is the base class for all Time Process Unit.
 It still has knowledge and support for ...
 */
class TimeProcess : public TTObjectBase {
    
public:
	TTSymbol                        mName;                          ///< ATTRIBUTE : the name of the time process
	TTSymbol                        mVersion;                       ///< ATTRIBUTE : the version of the time process
	TTSymbol                        mAuthor;                        ///< ATTRIBUTE : the author of the time process
    
protected:

    ScenarioPtr                     mScenario;                      ///< ATTRIBUTE : the parent scenario which constrains the time process
    
    TTBoolean                       mActive;                        ///< ATTRIBUTE : is the time process active ?
    
    TimeEventPtr                    mStartEvent;                    ///< ATTRIBUTE : the event object which handles the time process execution start
    TimeEventPtr                    mEndEvent;                      ///< ATTRIBUTE : the event object which handles the time process execution stop
    
    TTObjectBasePtr                 mScheduler;                     ///< ATTRIBUTE : the scheduler object which handles the time process execution
    
private:
    
    TTAttributePtr                  activeAttribute;                ///< cache active attribute for observer notification
    
public:
	//** Constructor.	*/
	TimeProcess(TTValue& arguments);
	
	/** Destructor. */
	virtual ~TimeProcess();
	
	/** Get parameters names needed by this time process 
     @param	value           the returned parameter names
     @return                kTTErrNone */
	virtual TTErr getParameterNames(TTValue& value) = 0;
    
    /** Specific process method on start
     @return                an error code returned by the process end method */
    virtual TTErr ProcessStart() = 0;
    
    /** Specific process method on end
     @return                an error code returned by the process end method */
    virtual TTErr ProcessEnd() = 0;
    
    /** Specific process method
     @return                an error code returned by the process method */
    virtual TTErr Process() = 0;

private :
    
    /** Set the time process active or not
     @param	value           a boolean
     @return                kTTErrNone */
    TTErr	setActive(const TTValue& value);
    
    /** Set the start event date of the time process
     @param	value           a date
     @return                an error code if the date is wrong */
    TTErr	setStart(const TTValue& value);
    
    /** Set the end event date of the time process
     @param	value           a date
     @return                an error code if the date is wrong */
    TTErr	setEnd(const TTValue& value);
    
    /** Add a start trigger on an address
     @param	value           a TTAddress to listen
     @return                an error code if trigger can't be added */
    TTErr   StartTriggerAdd(const TTValue& inputValue, TTValue& outputValue);
    
    /** Remove a start trigger
     @return                kTTerrNone */
    TTErr   StartTriggerRemove();
    
    /** Add a end trigger on an address
     @param	value           a TTAddress to listen
     @return                an error code if trigger can't be added */
    TTErr   EndTriggerAdd(const TTValue& inputValue, TTValue& outputValue);
    
    /** Remove a end trigger
     @return                kTTerrNone */
    TTErr   EndTriggerRemove();
    
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
    
    friend TTErr TT_EXTENSION_EXPORT TimeProcessStartEventCallback(TimeProcessPtr  aTimeProcess, const TTValue& eventValue);
    friend TTErr TT_EXTENSION_EXPORT TimeProcessEndEventCallback(TimeProcessPtr  aTimeProcess, const TTValue& eventValue);
    friend void TT_EXTENSION_EXPORT TimeProcessSchedulerCallback(TTPtr object, TTFloat64 progression);
};

typedef TimeProcess* TimeProcessPtr;

/** The start event callback to start the time process execution
 @param	aTimeProcess                a time process instance
 @param	eventValue					a value to test in a logical expression
 @return							an error code */
TTErr TT_EXTENSION_EXPORT TimeProcessStartEventCallback(TimeProcessPtr aTimeProcess, const TTValue& eventValue);

/** The end event callback to end the time process execution
 @param	aTimeProcess                a time process instance
 @param	eventValue					a value to test in a logical expression
 @return							an error code */
TTErr TT_EXTENSION_EXPORT TimeProcessEndEventCallback(TimeProcessPtr aTimeProcess, const TTValue& eventValue);

/** The scheduler time progression callback
 @param	baton						a time process instance
 @param	data						the time progression
 @return							an error code */
void TT_EXTENSION_EXPORT TimeProcessSchedulerCallback(TTPtr object, TTFloat64 progression);

#endif // __TIME_PROCESS_H__

#ifndef __TIME_PROCESS_LIB_H__
#define __TIME_PROCESS_LIB_H__

class TT_EXTENSION_EXPORT TimeProcessLib {
    
public:

	/**	Return a list of all available time processes 
     @param	value           the returned time process names*/
	static void getTimeProcessNames(TTValue& timeProcessNames);
};

#endif	//__TIME_PROCESS_LIB_H__