/*
 * A Time Process interface
 * Copyright © 2013, Théo de la Hogue, Clément Bossut
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
registerAttribute(TTSymbol("ParameterNames"), kTypeLocalValue, NULL, (TTGetterMethod)& thisTTClass::getParameterNames); \
/*addAttributeProperty(ParameterNames, readOnly, YES); \ */

class TimeProcess;

typedef TimeProcess* TimeProcessPtr;

typedef void (*TimeProcessProgressionCallback)(TTPtr, TTFloat64);

/** A type to define an unordered map to store and retreive a value relative to a TimeProcessPtr */
#ifdef TT_PLATFORM_WIN
    #include <hash_map>
    using namespace stdext;	// Visual Studio 2008 puts the hash_map in this namespace
    typedef hash_map<TimeProcessPtr,TTValuePtr>    TimeProcessMap;
#else
    //	#ifdef TT_PLATFORM_LINUX
    // at least for GCC 4.6 on the BeagleBoard, the unordered map is standard
    #include <unordered_map>
    //	#else
    //		#include "boost/unordered_map.hpp"
    //		using namespace boost;
    //	#endif
    typedef std::unordered_map<TimeProcessPtr,TTValuePtr>	TimeProcessMap;
#endif

typedef	TimeProcessMap*	TimeProcessMapPtr;
typedef TimeProcessMap::const_iterator	TimeProcessMapIterator;

/****************************************************************************************************/
// Class Specification

/**	Time Process is the base class for all Time Process Unit.
 It still has knowledge and support for ...
 */
class TimeProcess : public TTObjectBase {
    
protected:

    TTObjectBasePtr                 mScenario;                      ///< ATTRIBUTE : the parent scenario which constrains the time process
    
    TTUInt32                        mDurationMin;                   ///< ATTRIBUTE : the minimal duration of the time process
    TTUInt32                        mDurationMax;                   ///< ATTRIBUTE : the maximal duration of the time process
    
    TTBoolean                       mActive;                        ///< ATTRIBUTE : is the time process active ?
    
    TTObjectBasePtr                 mStartEvent;                    ///< ATTRIBUTE : the event object which handles the time process execution start
    TTObjectBasePtr                 mStartEventCallback;            ///< a callback to subscribe for start event notification
    
    TTList                          mIntermediateEvents;            ///< ATTRIBUTE : the list of all intermediate events
    
    TTObjectBasePtr                 mEndEvent;                      ///< ATTRIBUTE : the event object which handles the time process execution stop
    TTObjectBasePtr                 mEndEventCallback;              ///< a callback to subscribe for end event notification
    
    TTObjectBasePtr                 mScheduler;                     ///< ATTRIBUTE : the scheduler object which handles the time process execution

private:
    
    TTAttributePtr                  activeAttribute;                ///< cache active attribute for observer notification
    
public:
    
	/** Constructor	
     @param	arguments       kTTValNONE  */
	TimeProcess(TTValue& arguments);
	
	/** Destructor
        The events are not destroyed here but their destruction can 
        be managed using StartEventRelease or EndEventRelease message   */
	virtual ~TimeProcess();
	
	/** Get parameters names needed by this time process 
     @param	value           the returned parameter names
     @return                kTTErrNone */
	virtual TTErr   getParameterNames(TTValue& value) = 0;
    
    /** Specific process method on start
     @return                an error code returned by the process end method */
    virtual TTErr   ProcessStart() = 0;
    
    /** Specific process method on end
     @return                an error code returned by the process end method */
    virtual TTErr   ProcessEnd() = 0;
    
    /** Specific process method
     @param	inputValue      progression of the scheduler
     @param	outputValue     return an error of the processing
     @return                an error code returned by the process method */
    virtual TTErr   Process(const TTValue& inputValue, TTValue& outputValue) = 0;
    
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

private :
    
    /** set the scenario object to add the time process to it
     @param	value           a new scenario to be part of
     @return                kTTErrNone */
    TTErr	setScenario(const TTValue& value);
    
    /** get the time process rigidity
     @param	value           rigidity state
     @return                kTTErrNone */
    TTErr	getRigid(TTValue& value);
    
    /** set the time process rigidity
     @param	value           a new rigidity state
     @return                kTTErrNone */
    TTErr	setRigid(const TTValue& value);
    
    /** set the time process minimal duration
     @param	value           a new minimal duration
     @return                kTTErrNone */
    TTErr	setDurationMin(const TTValue& value);
    
    /** set the time process maximal duration
     @param	value           a new maximal duration
     @return                kTTErrNone */
    TTErr	setDurationMax(const TTValue& value);
    
    /** get the time process start date
        this method eases the getting of the start event date
     @param	value           the returned start date
     @return                kTTErrNone */
    TTErr	getStartDate(TTValue& value);
    
    /** set the time process start date
        this method eases the setting of the start event date
     @param	value           a new start date
     @return                kTTErrNone */
    TTErr	setStartDate(const TTValue& value);
    
    /** get the time process end date
        this method eases the getting of the end event date
     @param	value           the returned end date
     @return                kTTErrNone */
    TTErr	getEndDate(TTValue& value);
    
    /** set the time process end date
        this method eases the setting of the end event date
     @param	value           a new end date
     @return                kTTErrNone */
    TTErr	setEndDate(const TTValue& value);
    
    /** get the time process duration
     @param	value           the returned duration
     @return                kTTErrNone */
    TTErr	getDuration(TTValue& value);
    
    /** Set the time process active or not
     @param	value           a boolean
     @return                kTTErrNone */
    TTErr	setActive(const TTValue& value);
    
    /** Set the start event of the time process
     @param	value           a date
     @return                an error code if the date is wrong */
    TTErr	setStartEvent(const TTValue& value);
    
    /** Get intermediate events of the time process
     @param	value           returned events
     @return                kTTErrNone */
    TTErr	getIntermediateEvents(TTValue& value);
    
    /** Set the end event of the time process
     @param	value           a date
     @return                an error code if the date is wrong */
    TTErr	setEndEvent(const TTValue& value);
    
    /** Create a start event for the time process
     @param	value           event type, a date (optional)
     @return                an error code if the event can't be created */
    TTErr	StartEventCreate(const TTValue& value);
    
    /** Replace a start event by a new one at the same date.
        This will release the former event.
     @param	value           event type
     @return                an error code if the event can't be replaced */
    TTErr	StartEventReplace(const TTValue& value);
    
    /** Release the start event of the time process
     @return                an error code if the event can't be destroyed */
    TTErr	StartEventRelease();
    
    /** Create a end event for the time process
     @param	value           event type, a date (optional)
     @return                an error code if the event can't be created */
    TTErr	EndEventCreate(const TTValue& value);
    
    /** Replace a end event by a new one at the same date.
        This will release the former event.
     @param	value           event type
     @return                an error code if the event can't be replaced */
    TTErr	EndEventReplace(const TTValue& value);
    
    /** Release the end event of the time process
     @return                an error code if the event can't be destroyed */
    TTErr	EndEventRelease();
    
    /** Move the time process
     this method eases the setting of the start and end event dates
     @param	value           new start date, new end date
     @return                an error code if the movement fails */
    TTErr	Move(const TTValue& inputValue, TTValue& outputValue);
    
    /** Limit the time process duration
        this method eases the setting of the minimal and maximal durations
     @param	value           duration min, duration max
     @return                an error code if the limitation fails */
    TTErr	Limit(const TTValue& inputValue, TTValue& outputValue);
    
    /** Play the time process
        this method eases the managment of the scheduler object
     @return                an error code if the play fails */
    TTErr	Play();
    
    /** Stop the time process
     this method eases the managment of the scheduler object
     @return                an error code if the stop fails */
    TTErr	Stop();
    
    /** Pause the time process
        this method eases the managment of the scheduler object
     @return                an error code if the pause fails */
    TTErr	Pause();
    
    /** Resume the time process
        this method eases the managment of the scheduler object
     @return                an error code if the resume fails */
    TTErr	Resume();
    
    friend TTErr TT_EXTENSION_EXPORT TimeProcessStartEventHappenCallback(TTPtr baton, TTValue& data);
    friend TTErr TT_EXTENSION_EXPORT TimeProcessEndEventHappenCallback(TTPtr baton, TTValue& data);
    friend void TT_EXTENSION_EXPORT TimeProcessSchedulerCallback(TTPtr object, TTFloat64 progression);
};

/** The start event happen callback to start the time process execution
 @param	baton               a time process instance
 @param	data                a value relative to the event
 @return					an error code */
TTErr TT_EXTENSION_EXPORT TimeProcessStartEventHappenCallback(TTPtr baton, TTValue& data);

/** The end event happen callback to end the time process execution
 @param	baton               a time process instance
 @param	data                a value relative to the event
 @return					an error code */
TTErr TT_EXTENSION_EXPORT TimeProcessEndEventHappenCallback(TTPtr baton, TTValue& data);

/** The scheduler time progression callback
 @param	object				a time process instance
 @param	progression			the time progression
 @return					an error code */
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