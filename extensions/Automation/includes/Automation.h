/** @file
 *
 * @ingroup scoreExtension
 *
 * @brief Automation time process class manage interpolation between the start event state and end event state depending on the scheduler position
 *
 * @details The Automation class allows to ... @n@n
 *
 * @see TimePluginLib, TTTimeProcess, TTCurve
 *
 * @authors Théo de la Hogue & Clément Bossut
 *
 * @copyright Copyright © 2013, Théo de la Hogue & Clément Bossut @n
 * This code is licensed under the terms of the "CeCILL-C" @n
 * http://www.cecill.info
 */

#ifndef __AUTOMATION_H__
#define __AUTOMATION_H__

#include "TimePluginLib.h"


/**	The Automation class allows to ...
 
 @see TimePluginLib, TTTimeProcess, TTCurve
 */
class Automation : public TimeProcessPlugin
{
	TTCLASS_SETUP(Automation)
	
private :
    
    TTHash                      mCurves;						///< a table of freehand function units stored by address
    TTHash                      mSenders;						///< a table of TTSender to send curves
    TTHash                      mReceivers;						///< a table of TTReceiver to record curves
   
    TTValue                     mCurrentObjects;                ///< useful for file parsing
    TTFloat64                   mCurrentPosition;            ///< useful for recording
    
    /** Get parameters names needed by this time process
     @param	value           the returned parameter names
     @return                kTTErrNone */
	TTErr   getParameterNames(TTValue& value);
    
    /** Get curve addresses
     @param	value           the returned curve addresses
     @return                kTTErrNone */
	TTErr   getCurveAddresses(TTValue& value);
    
    /** Specific compilation method used to pre-processed data in order to accelarate Process method.
     the compiled attribute allows to know if the process needs to be compiled or not.
     @return                an error code returned by the compile method */
    TTErr   Compile();
    
    /** Specific process method on start
     @return                an error code returned by the process end method */
    TTErr   ProcessStart();
    
    /** Specific process method on end
     @return                an error code returned by the process end method */
    TTErr   ProcessEnd();
    
    /** Specific process method
     @param	inputValue      position and date of the scheduler
     @param	outputValue     return an error of the processing
     @return                an error code returned by the process method */
    TTErr   Process(const TTValue& inputValue, TTValue& outputValue);
    
    /** Specific process method for pause/resume
     @param	inputValue      boolean paused state of the scheduler
     @param	outputValue     return an error of the processing
     @return                an error code returned by the process paused method */
    TTErr   ProcessPaused(const TTValue& inputValue, TTValue& outputValue);
    
    /** Specific go to method to set the process at a date
     @param	inputValue      a date where to go relative to the duration of the time process, an optional boolean to temporary mute the process 
     @param	outputValue     nothing
     @return                an error code if the operation fails */
    TTErr   Goto(const TTValue& inputValue, TTValue& outputValue);
    
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
    
    /** Add a curve at an address
     @param inputvalue      address
     @param outputvalue     a curve object
     @return                an error code if the operation fails */
    TTErr   CurveAdd(const TTValue& inputValue, TTValue& outputValue);
    
    /** Get curve's parameters at an address
     @param inputvalue      address
     @param outputvalue     a curve object
     @return                an error code if the operation fails */
    TTErr   CurveGet(const TTValue& inputValue, TTValue& outputValue);
    
    /** Update a curve at an address (when start or end state has changed)
     @param inputvalue      address
     @param outputvalue     nothing
     @return                an error code if the operation fails */
    TTErr   CurveUpdate(const TTValue& inputValue, TTValue& outputValue);
    
    /** Remove a curve at an address
     @param inputvalue      address
     @param outputvalue     nothing
     @return                an error code if the operation fails */
    TTErr   CurveRemove(const TTValue& inputValue, TTValue& outputValue);
    
    /** Clear all curves
     @return                an error code if the operation fails */
    TTErr   Clear();
    
    /** Enable/Disable the recording of a curve during the next execution of the process
     @param inputvalue      address, boolean
     @param outputvalue     nothing
     @return                an error code if the operation fails */
    TTErr   CurveRecord(const TTValue& inputValue, TTValue& outputValue);
    
    void    addSender(TTAddress anAddress);
    void    removeSender(TTAddress anAddress);
    
    void    addReceiver(TTAddress anAddress);
    void    removeReceiver(TTAddress anAddress);
    
    friend TTErr TTSCORE_EXPORT AutomationReceiverReturnValueCallback(const TTValue& baton, const TTValue& data);
};

typedef Automation* AutomationPtr;

/** The receiver callback return back the value of observed addresses
 @param	baton               a automation instance, an address
 @param	data                a value to test
 @return					an error code */
TTErr TTSCORE_EXPORT AutomationReceiverReturnValueCallback(const TTValue& baton, const TTValue& data);

#endif // __AUTOMATION_H__
