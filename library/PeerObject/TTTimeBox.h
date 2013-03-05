/* 
 * A TimeBox Object
 * Copyright © 2013, Théo de la Hogue
 * 
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#ifndef __TT_TIMEBOX_H__
#define __TT_TIMEBOX_H__

#include "TTScore.h"

/**	TTTimeBox ... TODO : an explanation
 
 
 */

class TTSCORE_EXPORT TTTimeBox : public TTDataObjectBase
{
	TTCLASS_SETUP(TTTimeBox)
	
private :
    
    TTBoolean                   mActive;                        ///< ATTRIBUTE : is the time box active ?
	
	TTUInt32					mStart;                         ///< ATTRIBUTE : the start date of the time box
    TTUInt32					mEnd;                           ///< ATTRIBUTE : the end date of the time box
    
	TTObjectBasePtr				mStartCue;						///< ATTRIBUTE : the beginning state handled by the time box
    TTObjectBasePtr				mEndCue;						///< ATTRIBUTE : the final state handled by the time box
    
    TTObjectBasePtr             mStartReceiver;                 ///< ATTRIBUTE : an internal receiver to launch the time box execution
    TTObjectBasePtr             mEndReceiver;                   ///< ATTRIBUTE : an internal receiver to stop the time box execution
    
    TTCallbackPtr               mStartCallback;                 ///< ATTRIBUTE : a callback to notify the beginning of the time box
    TTCallbackPtr               mEndCallback;                   ///< ATTRIBUTE : a callback to notify the end of the time box
    
    TTAddressItemPtr            mNamespace;                     ///< ATTRIBUTE : the namespace workspace to use for cue storage
    
    TTObjectBasePtr             mScheduler;                     ///< ATTRIBUTE : the scheduler object which handles the time box execution
	
    /** Set the start date of the time box 
     @param	value           a date
     @return                an error code if the date is wrong */
    TTErr	setStart(const TTValue& value);
    
    /** Set the end date of the time box
     @param	value           a date
     @return                an error code if the date is wrong */
    TTErr	setEnd(const TTValue& value);
    
    /** Add a start trigger on an address
     @param	value           a TTAddress to listen
     @return                 an error code if trigger can't be added */
    TTErr   TriggerStartAdd(const TTValue& inputValue, TTValue& outputValue);
    
    /** Remove a start trigger
     @return                kTTerrNone */
    TTErr   TriggerStartRemove();
    
    /** Add a end trigger on an address
     @param	value           a TTAddress to listen
     @return                an error code if trigger can't be added */
    TTErr   TriggerEndAdd(const TTValue& inputValue, TTValue& outputValue);
    
    /** Remove a end trigger
     @return                kTTerrNone */
    TTErr   TriggerEndRemove();
    
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
    
    friend TTErr TTSCORE_EXPORT TTTimeBoxStartReceiverCallback(TTPtr baton, TTValue& data);
    friend TTErr TTSCORE_EXPORT TTTimeBoxEndReceiverCallback(TTPtr baton, TTValue& data);
    friend void TTSCORE_EXPORT TTTimeBoxSchedulerCallback(TTPtr object, TTFloat64 progression);
};

typedef TTTimeBox* TTTimeBoxPtr;

/** The receiver callback to start the time box
 @param	baton						a time box instance
 @param	data						a value to test in a logical expression
 @return							an error code */
TTErr TTSCORE_EXPORT TTTimeBoxStartReceiverCallback(TTPtr baton, TTValue& data);

/** The receiver callback to stop the time box
 @param	baton						a time box instance
 @param	data						a value to test in a logical expression
 @return							an error code */
TTErr TTSCORE_EXPORT TTTimeBoxEndReceiverCallback(TTPtr baton, TTValue& data);

/** The scheduler time progression callback
 @param	baton						a time box instance
 @param	data						the time progression
 @return							an error code */
void TTSCORE_EXPORT TTTimeBoxSchedulerCallback(TTPtr object, TTFloat64 progression);

#endif // __TT_TIMEBOX_H__
