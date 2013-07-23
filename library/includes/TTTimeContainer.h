/** @file
 *
 * @ingroup scoreLibrary
 *
 * @brief a container class to manage time events and time processes instances in the time
 *
 * @details The TTTimeContainer class allows to ... @n@n
 *
 * @see TTTimeEvent, TTTimeProcess
 *
 * @authors Théo de la Hogue & Clément Bossut
 *
 * @copyright Copyright © 2013, Théo de la Hogue & Clément Bossut @n
 * This code is licensed under the terms of the "CeCILL-C" @n
 * http://www.cecill.info
 */

#ifndef __TT_TIME_CONTAINER_H__
#define __TT_TIME_CONTAINER_H__

#include "TTTimeProcess.h"

/**	The TTTimeContainer class allows to ...
 
 @see TTTimeProcess, TTTimeEvent
 */
class TTSCORE_EXPORT TTTimeContainer : public TTTimeProcess {
    
    TTCLASS_SETUP(TTTimeContainer)
    
protected :
    
    TTList                      mTimeProcessList;               ///< all registered time processes and their observers
    TTList                      mTimeEventList;                 ///< all registered time events and their observers
    
private :
    
    /** Create a time event
     @inputvalue            a date
     @outputvalue           a new time event
     @return                an error code if the creation fails */
    virtual TTErr   TimeEventCreate(const TTValue& inputValue, TTValue& outputValue) {return kTTErrGeneric;};
    
    /** Release a time event
     @inputValue            a time event object to release
     @outputvalue           kTTValNONE
     @return                an error code if the destruction fails */
    virtual TTErr   TimeEventRelease(const TTValue& inputValue, TTValue& outputValue) {return kTTErrGeneric;};
    
    /** Move a time event
     @inputValue            a time event object, new date
     @outputvalue           kTTValNONE
     @return                an error code if the movement fails */
    virtual TTErr   TimeEventMove(const TTValue& inputValue, TTValue& outputValue) {return kTTErrGeneric;};
    
    /** Make a time event interactive
     @inputValue            a time event object, new boolean value
     @outputvalue           kTTValNONE
     @return                an error code if the setting fails */
    virtual TTErr   TimeEventInteractive(const TTValue& inputValue, TTValue& outputValue) {return kTTErrGeneric;};
    
    /** Trigger a time event to make it happens
     @inputValue            a time event object
     @outputvalue           kTTValNONE
     @return                an error code if the triggering fails */
    virtual TTErr   TimeEventTrigger(const TTValue& inputValue, TTValue& outputValue) {return kTTErrGeneric;};
    
    /** Replace a time event by another one (copying date and active attribute)
     @inputValue            a former time event object, a new time event object
     @outputvalue           kTTValNONE
     @return                an error code if the replacement fails */
    virtual TTErr   TimeEventReplace(const TTValue& inputValue, TTValue& outputValue) {return kTTErrGeneric;};
    
    
    
    /** Create a time process
     @inputvalue            a time process type, a start event, a end event
     @outputvalue           a new time process
     @return                an error code if the creation fails */
    virtual TTErr   TimeProcessCreate(const TTValue& inputValue, TTValue& outputValue) {return kTTErrGeneric;};
    
    /** Release a time process
     @inputValue            a time process object to release
     @outputvalue           its the start and the end event
     @return                an error code if the destruction fails */
    virtual TTErr   TimeProcessRelease(const TTValue& inputValue, TTValue& outputValue) {return kTTErrGeneric;};
    
    /** Move a time process
     @inputValue            a time process object, new start date, new end date
     @outputvalue           kTTValNONE
     @return                an error code if the movement fails */
    virtual TTErr   TimeProcessMove(const TTValue& inputValue, TTValue& outputValue) {return kTTErrGeneric;};
    
    /** Limit a time process duration
     @inputValue            a time process object, new duration min, new duration max
     @outputvalue           kTTValNONE
     @return                an error code if the limitation fails */
    virtual TTErr   TimeProcessLimit(const TTValue& inputValue, TTValue& outputValue) {return kTTErrGeneric;};
    
protected :
    
    /* a time container can access too the protected members of any time event or time process */
    
    /** Getter on event's name protected member
     @aTimeProcess          a time event object
     @return                a name symbol */
    TTSymbol        getTimeEventName(TTTimeEventPtr aTimeEvent);
    
    /** Getter on date time event protected member
     @aTimeProcess          a time event object
     @return                a date value */
    TTUInt32        getTimeEventDate(TTTimeEventPtr aTimeEvent);
    
    /** Getter on interactive time event protected member
     @aTimeProcess          a time event object
     @return                a boolean value */
    TTBoolean       isTimeEventInteractive(TTTimeEventPtr aTimeEvent);
    
    /** Getter on process's name protected member
     @aTimeProcess          a time process object
     @return                a name symbol */
    TTSymbol        getTimeProcessName(TTTimeProcessPtr aTimeProcess);
    
    /** Getter on start event time process protected member
     @aTimeProcess          a time process object
     @return                a time event object */
    TTTimeEventPtr  getTimeProcessStartEvent(TTTimeProcessPtr aTimeProcess);
    
    /** Setter on start event time process protected member
     @aTimeProcess          a time process object
     @aTimeEvent            a time event object */
    void            setTimeProcessStartEvent(TTTimeProcessPtr aTimeProcess, TTTimeEventPtr aTimeEvent);
    
    /** Getter on end event time process protected member
     @aTimeProcess          a time process object
     @return                a time event object */
    TTTimeEventPtr  getTimeProcessEndEvent(TTTimeProcessPtr aTimeProcess);
    
    /** Setter on end event time process protected member
     @aTimeProcess          a time process object
     @aTimeEvent            a time event object */
    void            setTimeProcessEndEvent(TTTimeProcessPtr aTimeProcess, TTTimeEventPtr aTimeEvent);
    
    /** Getter on duration min time process protected member
     @aTimeProcess          a time process object
     @return                a duration value */
    TTUInt32        getTimeProcessDurationMin(TTTimeProcessPtr aTimeProcess);
    
    /** Getter on duration max time process protected member
     @aTimeProcess          a time process object
     @return                a duration value */
    TTUInt32        getTimeProcessDurationMax(TTTimeProcessPtr aTimeProcess);
    
    
    friend void TTSCORE_EXPORT TTTimeContainerFindTimeProcess(const TTValue& aValue, TTPtr timeProcessPtrToMatch, TTBoolean& found);
    friend void TTSCORE_EXPORT TTTimeContainerFindTimeEvent(const TTValue& aValue, TTPtr timeEventPtrToMatch, TTBoolean& found);
    friend void TTSCORE_EXPORT TTTimeContainerFindTimeEventWithName(const TTValue& aValue, TTPtr timeEventNamePtrToMatch, TTBoolean& found);
    friend void TTSCORE_EXPORT TTTimeContainerFindTimeProcessWithTimeEvent(const TTValue& aValue, TTPtr timeEventPtrToMatch, TTBoolean& found);
};

typedef TTTimeContainer* TTTimeContainerPtr;

void TTSCORE_EXPORT TTTimeContainerFindTimeProcess(const TTValue& aValue, TTPtr timeProcessPtrToMatch, TTBoolean& found);

void TTSCORE_EXPORT TTTimeContainerFindTimeEvent(const TTValue& aValue, TTPtr timeEventPtrToMatch, TTBoolean& found);

void TTSCORE_EXPORT TTTimeContainerFindTimeEventWithName(const TTValue& aValue, TTPtr timeEventNamePtrToMatch, TTBoolean& found);

void TTSCORE_EXPORT TTTimeContainerFindTimeProcessWithTimeEvent(const TTValue& aValue, TTPtr timeEventPtrToMatch, TTBoolean& found);

#endif // __TT_TIME_CONTAINER_H__
