/** @file
 *
 * @ingroup scoreLibrary
 *
 * @brief the Score Application Programming Interface
 *
 * @details The Score API allows to use Score inside another application @n@n
 *
 * @see TTScore
 *
 * @authors Théo de la Hogue & Clément Bossut
 *
 * @copyright Copyright © 2013, Théo de la Hogue & Clément Bossut @n
 * This code is licensed under the terms of the "CeCILL-C" @n
 * http://www.cecill.info
 */

#include "TTScore.h"
#include "Expression.h"
#include "TTTimeEvent.h"
#include "TTTimeProcess.h"
#include "TTTimeContainer.h"
#include "TTTimeCondition.h"

/** Initialise Score framework */
void TTSCORE_EXPORT TTScoreInitialize();

/*
    Functions for time events
 */

/** Create a time event
 @param	timeEvent               a handler on a time event instance
 @param	date                    the date of the time event
 @param timeContainer           an instance of a time container to create the event into it
 @return                        kTTErrGeneric if the creation fails */
TTErr TTSCORE_EXPORT TTScoreTimeEventCreate(TTTimeEventPtr *timeEvent, TTUInt32 date, TTTimeContainerPtr timeContainer = NULL);

/** Delete a time event
 @param	timeEvent               a handler on a time event instance
 @param timeContainer           an instance of a time container to release the event from it
 @return                        kTTErrGeneric if the deletion fails */
TTErr TTSCORE_EXPORT TTScoreTimeEventRelease(TTTimeEventPtr *timeEvent, TTTimeContainerPtr timeContainer = NULL);

/*
    Functions for time processes
 */

/** Create a time process
 @param	timeProcess             a handler on a time process instance
 @param timeProcessClass        the name of the time process class to instanciate
 @param	startEvent              a time event for the start of the process
 @param	endEvent                a time event for the end of the process
 @param timeContainer           an instance of a time container to create the process into it
 @return                        kTTErrGeneric if the creation fails */
TTErr TTSCORE_EXPORT TTScoreTimeProcessCreate(TTTimeProcessPtr *timeProcess, const std::string timeProcessClass, TTTimeEventPtr startEvent, TTTimeEventPtr endEvent, TTTimeContainerPtr timeContainer = NULL);

/** Delete a time process
 @param	timeProcess             a handler on a time process instance
 @param timeContainer           an instance of a time container to release the process from it
 @param startEvent              a handler to get the start event the start event back when the time process is released from a time container
 @param endEvent                a handler to get the end event the start event back when the time process is released from a time container
 @return                        kTTErrGeneric if the deletion fails */
TTErr TTSCORE_EXPORT TTScoreTimeProcessRelease(TTTimeProcessPtr *timeProcess, TTTimeContainerPtr timeContainer = NULL, TTTimeEventPtr *startEvent = NULL, TTTimeEventPtr *endEvent = NULL);

/** Get start event of a time process
 @param	timeProcess             an instance of a time process
 @param startEvent              a handler to get the start event the start event back
 @return                        kTTErrGeneric if the access fails */
TTErr TTSCORE_EXPORT TTScoreTimeProcessGetStartEvent(TTTimeProcessPtr timeProcess, TTTimeEventPtr *startEvent);

/** Get end event of a time process
 @param	timeProcess             an instance of a time process
 @param startEvent              a handler to get the end event the start event back
 @return                        kTTErrGeneric if the access fails */
TTErr TTSCORE_EXPORT TTScoreTimeProcessGetEndEvent(TTTimeProcessPtr timeProcess, TTTimeEventPtr *endEvent);

/** Set the name of a time process
 @param	timeProcess             a time process instance
 @param name                    the name of the time process
 @return                        kTTErrGeneric if the operation fails */
TTErr TTSCORE_EXPORT TTScoreTimeProcessSetName(TTTimeProcessPtr timeProcess, const std::string name);

/** Set the rigid attribute of a time process
 @param	timeProcess             a time process instance
 @param rigid                   a boolean to enable/disable time process rigidity
 @return                        kTTErrGeneric if the operation fails */
TTErr TTSCORE_EXPORT TTScoreTimeProcessSetRigid(TTTimeProcessPtr timeProcess, const TTBoolean rigid);

/** Move a time process
 @param	timeProcess             a time process instance
 @param startDate               start date of the time process
 @param endDate                 end date of the time process
 @return                        kTTErrGeneric if the operation fails */
//TTErr TTSCORE_EXPORT TTScoreTimeProcessMove(TTTimeProcessPtr timeProcess, const TTUInt32 startDate, const TTUInt32 endDate);

/** Limit a time process duration
 @param	timeProcess             a time process instance
 @param durationMin             the minimal duration of the time process (default : 0)
 @param durationMax             the maximal duration of the time process (default : 0)
 @return                        kTTErrGeneric if the operation fails */
//TTErr TTSCORE_EXPORT TTScoreTimeProcessLimit(TTTimeProcessPtr timeProcess, const TTUInt32 durationMin, const TTUInt32 durationMax);

/** Define callback function to be notified when a time process starts */
typedef void (*TTScoreTimeProcessStartCallback)(TTTimeProcessPtr);
typedef	TTScoreTimeProcessStartCallback* TTScoreTimeProcessStartCallbackPtr;

/** Create a callback to be notified when a time process starts
 @param	timeProcess             a time process instance
 @param startCallback           a handler on a callback instance
 @param startCallbackFunction   the TTScoreTimeProcessStartCallback function to call
 @return                        kTTErrGeneric if the creation fails */
TTErr TTSCORE_EXPORT TTScoreTimeProcessStartCallbackCreate(TTTimeProcessPtr timeProcess, TTObjectBasePtr *startCallback, TTScoreTimeProcessStartCallbackPtr startCallbackFunction);

/** Delete a callback used to be notified when a time process starts
 @param startCallback           a handler on a callback instance
 @return                        kTTErrGeneric if the deletion fails */
TTErr TTSCORE_EXPORT TTScoreTimeProcessStartCallbackRelease(TTTimeProcessPtr timeProcess, TTObjectBasePtr *startCallback);

/** Define callback function to be notified when a time process ends */
typedef void (*TTScoreTimeProcessEndCallback)(TTTimeProcessPtr);
typedef	TTScoreTimeProcessEndCallback* TTScoreTimeProcessEndCallbackPtr;

/** Create a callback to be notified when a time process ends
 @param	timeProcess             a time process instance
 @param startCallback           a handler on a callback instance
 @param startCallbackFunction   the TTScoreTimeProcessEndCallback function to call
 @return                        kTTErrGeneric if the creation fails */
TTErr TTSCORE_EXPORT TTScoreTimeProcessEndCallbackCreate(TTTimeProcessPtr timeProcess, TTObjectBasePtr *endCallback, TTScoreTimeProcessEndCallbackPtr endCallbackFunction);

/** Delete a callback used to be notified when a time process ends
 @param startCallback           a handler on a callback instance
 @return                        kTTErrGeneric if the deletion fails */
TTErr TTSCORE_EXPORT TTScoreTimeProcessEndCallbackRelease(TTTimeProcessPtr timeProcess, TTObjectBasePtr *endCallback);

#if 0
#pragma mark -
#pragma mark some internal functions
#endif

/** Define callback function used internally to call a TTScoreTimeProcessStartCallback relative to a time process
 @param	baton                   a time process instance and a TTScoreTimeProcessStartCallback method to call with
 @param	data                    nothing */
void internal_TTScoreTimeProcessStartCallback(TTPtr baton, TTValue& data);

/** Define callback function used internally to call a TTScoreTimeProcessEndCallback relative to a time process
 @param	baton                   a time process instance and a TTScoreTimeProcessEndCallback method to call with
 @param	data                    nothing */
void internal_TTScoreTimeProcessEndCallback(TTPtr baton, TTValue& data);
