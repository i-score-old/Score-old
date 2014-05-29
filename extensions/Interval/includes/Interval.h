/** @file
 *
 * @ingroup scoreExtension
 *
 * @brief a simple amount of time process
 *
 * @details The Interval class allows to ... @n@n
 *
 * @see TimePluginLib, TTTimeProcess
 *
 * @authors Théo de la Hogue & Clément Bossut
 *
 * @copyright Copyright © 2013, Théo de la Hogue & Clément Bossut @n
 * This code is licensed under the terms of the "CeCILL-C" @n
 * http://www.cecill.info
 */

#ifndef __INTERVAL_H__
#define __INTERVAL_H__

#include "TimePluginLib.h"

/**	The Interval class allows to ...
 
 @see TimePluginLib, TTTimeProcess
 */
class Interval : public TimeProcessPlugin
{
	TTCLASS_SETUP(Interval)
	
private :
       
    /** Get parameters names needed by this time process
     @param	value           the returned parameter names
     @return                kTTErrNone */
	TTErr   getParameterNames(TTValue& value);
    
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
     @param	inputValue      a date where to go relative to the duration of the time process
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
};

typedef Interval* IntervalPtr;

#endif // __INTERVAL_H__
