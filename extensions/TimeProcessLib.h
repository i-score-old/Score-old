/** @file
 *
 * @ingroup scoreLibrary
 *
 * @brief Add easily a specific time process to the TimeProcessLib
 *
 * @details The TimeProcessLib allows to ... @n@n
 *
 * @see TTTimeProcess
 *
 * @authors Théo de la Hogue & Clément Bossut
 *
 * @copyright Copyright © 2013, Théo de la Hogue & Clément Bossut @n
 * This code is licensed under the terms of the "CeCILL-C" @n
 * http://www.cecill.info
 */

#ifndef __TIME_PROCESS_H__
#define __TIME_PROCESS_H__

#include "TTScore.h"





#define TIME_PROCESS_CONSTRUCTOR \
TTObjectBasePtr thisTTClass :: instantiate (TTSymbol& name, TTValue& arguments) {return new thisTTClass (arguments);} \
\
extern "C" void thisTTClass :: registerClass () {TTClassRegister( TTSymbol(thisTTClassName), thisTTClassTags, thisTTClass :: instantiate );} \
\
thisTTClass :: thisTTClass (TTValue& arguments) : TimeProcess(arguments)

#define TIME_PROCESS_INITIALIZE \
registerAttribute(TTSymbol("ParameterNames"), kTypeLocalValue, NULL, (TTGetterMethod)& thisTTClass::getParameterNames); \
//addAttributeProperty(ParameterNames, readOnly, YES);





/**	The TimeProcess class allows to ...
 
 @see TimeProcessLib, TTTimeProcess
 */
class TimeProcess : public TTTimeProcess {
    
    TTCLASS_SETUP(TimeProcess)
    
public:
	
	/** Get parameters names needed by this time process 
     @param	value           the returned parameter names
     @return                kTTErrNone */
	virtual TTErr   getParameterNames(TTValue& value) = 0;
};

#endif // __TIME_PROCESS_H__





#ifndef __TIME_PROCESS_LIB_H__
#define __TIME_PROCESS_LIB_H__

/**	The TimeProcessLib class allows to ...
 
 @see TimeProcess
 */
class TT_EXTENSION_EXPORT TimeProcessLib {
    
public:
    
	/**	Return a list of all available time processes
     @param	value           the returned time process names*/
	static void getTimeProcessNames(TTValue& timeProcessNames);
};

#endif	//__TIME_PROCESS_LIB_H__
