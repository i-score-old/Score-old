/** @file
 *
 * @ingroup scoreLibrary
 *
 * @brief a class to define a process
 *
 * @details The TTTimeProcess class allows to ... @n@n
 *
 * @see TTTimeEvent
 *
 * @authors Théo de la Hogue & Clément Bossut
 *
 * @copyright Copyright © 2013, Théo de la Hogue & Clément Bossut @n
 * This code is licensed under the terms of the "CeCILL-C" @n
 * http://www.cecill.info
 */

#ifndef __TT_TIME_PROCESS_H__
#define __TT_TIME_PROCESS_H__

#include "TTScore.h"
#include "TTTimeEvent.h"

/**	a class to define a process
 
 The TTTimeProcess class allows to ...
 
 @see TTTimeEvent
 */
class TTSCORE_EXPORT TTTimeProcess : public TTObjectBase {
    
    TTCLASS_SETUP(TTTimeProcess)
    
    friend class TTTimeContainer;

    TTObjectBasePtr                 mContainer;                     ///< the container which handles the time process
    
protected :
    
    TTSymbol                        mName;                          ///< the name of the time process
    
    TTUInt32                        mDurationMin;                   ///< the minimal duration of the time process
    TTUInt32                        mDurationMax;                   ///< the maximal duration of the time process
    
    TTBoolean                       mActive;                        ///< is the time process active ?
    
    TTValue                         mColor;                         ///< the color of the process (useful for gui)
    TTUInt32                        mPosition;                      ///< the position of the process on the Y axe (useful for gui)
    TTUInt32                        mSize;                          ///< the size of the process on the Y axe (useful for gui)
    
    TTObjectBasePtr                 mScheduler;                     ///< the scheduler object which handles the time process execution
    
private :
    
    TTObjectBasePtr                 mStartEvent;                    ///< the event object which handles the time process execution start
    TTObjectBasePtr                 mStartEventCallback;            ///< a callback to subscribe for start event notification
    
    TTList                          mIntermediateEvents;            ///< the list of all intermediate events
    
    TTObjectBasePtr                 mEndEvent;                      ///< the event object which handles the time process execution stop
    TTObjectBasePtr                 mEndEventCallback;              ///< a callback to subscribe for end event notification
    
    TTAttributePtr                  activeAttribute;                ///< cache active attribute for observer notification
    
    /** Specific process method on start
     @return                an error code returned by the process end method */
    virtual TTErr   ProcessStart() {return kTTErrGeneric;};
    
    /** Specific process method on end
     @return                an error code returned by the process end method */
    virtual TTErr   ProcessEnd() {return kTTErrGeneric;};
    
    /** Specific process method
     @param	inputValue      progression of the scheduler
     @param	outputValue     return an error of the processing
     @return                an error code returned by the process method */
    virtual TTErr   Process(const TTValue& inputValue, TTValue& outputValue) {return kTTErrGeneric;};
    
    /**  needed to be handled by a TTXmlHandler
     @param	inputValue      ..
     @param	outputValue     ..
     @return                .. */
	virtual TTErr	WriteAsXml(const TTValue& inputValue, TTValue& outputValue) {return kTTErrGeneric;};
	virtual TTErr	ReadFromXml(const TTValue& inputValue, TTValue& outputValue) {return kTTErrGeneric;};
    
    /** get the time process rigidity
     @param	value           rigidity state
     @return                kTTErrNone */
    TTErr           getRigid(TTValue& value);
    
    /** set the time process rigidity
     @param	value           a new rigidity state
     @return                kTTErrNone */
    TTErr           setRigid(const TTValue& value);
    
    /** set the time process minimal duration
     @param	value           a new minimal duration
     @return                kTTErrNone */
    TTErr           setDurationMin(const TTValue& value);
    
    /** set the time process maximal duration
     @param	value           a new maximal duration
     @return                kTTErrNone */
    TTErr           setDurationMax(const TTValue& value);
    
    /** get the time process start date
        this method eases the getting of the start event date
     @param	value           the returned start date
     @return                kTTErrNone */
    TTErr           getStartDate(TTValue& value);
    
    /** set the time process start date
        this method eases the setting of the start event date
     @param	value           a new start date
     @return                kTTErrNone */
    TTErr           setStartDate(const TTValue& value);
    
    /** get start event interactive value
        this method eases the getting of the start event interactive value
     @param	value           the returned interactive value
     @return                an error code if the event doesn't exist */
    TTErr           getStartInteractive(TTValue& value);
    
    /** set start event interactive or not
        this method eases the setting of the start event interactive value
     @param	value           boolean
     @return                an error code if the event doesn't exist */
    TTErr           setStartInteractive(const TTValue& value);
    
    /** get the time process end date
        this method eases the getting of the end event date
     @param	value           the returned end date
     @return                kTTErrNone */
    TTErr           getEndDate(TTValue& value);
    
    /** set the time process end date
        this method eases the setting of the end event date
     @param	value           a new end date
     @return                kTTErrNone */
    TTErr           setEndDate(const TTValue& value);
    
    /** get end event interactive value
        this method eases the getting of the end event interactive value
     @param	value           the returned interactive value
     @return                an error code if the event doesn't exist */
    TTErr           getEndInteractive(TTValue& value);
    
    /** set end event interactive or not
        this method eases the setting of the end event interactive value
     @param	value           boolean
     @return                an error code if the event doesn't exist */
    TTErr           setEndInteractive(const TTValue& value);
    
    /** get the time process duration
     @param	value           the returned duration
     @return                kTTErrNone */
    TTErr           getDuration(TTValue& value);
    
    /** Set the time process active or not
     @param	value           a boolean
     @return                kTTErrNone */
    TTErr           setActive(const TTValue& value);
    
    /** Get intermediate events of the time process
     @param	value           returned events
     @return                kTTErrNone */
    TTErr           getIntermediateEvents(TTValue& value);
    
    /** Move the time process
     this method eases the setting of the start and end event dates
     @param	value           new start date, new end date
     @return                an error code if the movement fails */
    TTErr           Move(const TTValue& inputValue, TTValue& outputValue);
    
    /** Limit the time process duration
        this method eases the setting of the minimal and maximal durations
     @param	value           duration min, duration max
     @return                an error code if the limitation fails */
    TTErr           Limit(const TTValue& inputValue, TTValue& outputValue);
    
    /** Play the time process
        this method eases the managment of the scheduler object
     @return                an error code if the play fails */
    TTErr           Play();
    
    /** Stop the time process
     this method eases the managment of the scheduler object
     @return                an error code if the stop fails */
    TTErr           Stop();
    
    /** Pause the time process
        this method eases the managment of the scheduler object
     @return                an error code if the pause fails */
    TTErr           Pause();
    
    /** Resume the time process
        this method eases the managment of the scheduler object
     @return                an error code if the resume fails */
    TTErr           Resume();

protected :
    
    /** get the start event
     @return                a time event object */
    TTTimeEventPtr  getStartEvent();
    
    /** set the end event
     @return                a time event object */
    TTTimeEventPtr  getEndEvent();
    
    /** set the start event
     @param aTimeEvent      a time event object
     @return                an error code if it fails */
    TTErr           setStartEvent(TTTimeEventPtr aTimeEvent);
    
    /** set the end event
     @param aTimeEvent      a time event object
     @return                an error code if it fails */
    TTErr           setEndEvent(TTTimeEventPtr aTimeProcess);
    
    friend TTErr TTSCORE_EXPORT TTTimeProcessStartEventHappenCallback(TTPtr baton, TTValue& data);
    friend TTErr TTSCORE_EXPORT TTTimeProcessEndEventHappenCallback(TTPtr baton, TTValue& data);
    friend void TTSCORE_EXPORT TTTimeProcessSchedulerCallback(TTPtr object, TTFloat64 progression);
    
    friend void TTSCORE_EXPORT TTTimeContainerFindTimeProcessWithTimeEvent(const TTValue& aValue, TTPtr timeEventPtrToMatch, TTBoolean& found);
};

typedef TTTimeProcess* TTTimeProcessPtr;

/** The start event happen callback to start the time process execution
 @param	baton               a time process instance
 @param	data                a value relative to the event
 @return					an error code */
TTErr TTSCORE_EXPORT TTTimeProcessStartEventHappenCallback(TTPtr baton, TTValue& data);

/** The end event happen callback to end the time process execution
 @param	baton               a time process instance
 @param	data                a value relative to the event
 @return					an error code */
TTErr TTSCORE_EXPORT TTTimeProcessEndEventHappenCallback(TTPtr baton, TTValue& data);

/** The scheduler time progression callback
 @param	object				a time process instance
 @param	progression			the time progression
 @return					an error code */
void TTSCORE_EXPORT TTTimeProcessSchedulerCallback(TTPtr object, TTFloat64 progression);


/** Define some macros to ease the access of events attributes */
#define mStartDate TTTimeEventPtr(mStartEvent)->mDate
#define mStartInteractive TTTimeEventPtr(mStartEvent)->mInteractive

#define mEndDate TTTimeEventPtr(mEndEvent)->mDate
#define mEndInteractive TTTimeEventPtr(mEndEvent)->mInteractive

#define mDuration mEndDate - mStartDate

/** Define callback function to get progression back from the scheduler */
typedef void (*TTTimeProcessProgressionCallback)(TTPtr, TTFloat64);

/** Define an unordered map to store and retreive a value relative to a TTTimeProcessPtr */
#ifdef TT_PLATFORM_WIN
    #include <hash_map>
    using namespace stdext;	// Visual Studio 2008 puts the hash_map in this namespace
    typedef hash_map<TTTimeProcessPtr,TTValuePtr>    TTTimeProcessMap;
#else
    //	#ifdef TT_PLATFORM_LINUX
    // at least for GCC 4.6 on the BeagleBoard, the unordered map is standard
    #include <unordered_map>
    //	#else
    //		#include "boost/unordered_map.hpp"
    //		using namespace boost;
    //	#endif
    typedef std::unordered_map<TTTimeProcessPtr,TTValuePtr>	TTTimeProcessMap;
#endif

typedef	TTTimeProcessMap*                   TTTimeProcessMapPtr;
typedef TTTimeProcessMap::const_iterator	TTTimeProcessMapIterator;


#endif // __TT_TIME_PROCESS_H__