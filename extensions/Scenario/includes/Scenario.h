/*
 * Scenario time process
 * Copyright © 2013, Théo de la Hogue
 *
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

/*!
 * \class Scenario
 *
 *  Scenario time process class is a container class to manage other time processes instances in the time
 *
 */

#ifndef __SCENARIO_H__
#define __SCENARIO_H__

#include "TimeProcess.h"

class Scenario : public TimeProcess
{
	TTCLASS_SETUP(Scenario)
	
private :
    
    TTAddressItemPtr            mNamespace;                     ///< ATTRIBUTE : the namespace workspace of the scenario
	
    /** Get parameters names needed by this time process
     @param	value           the returned parameter names
     @return                kTTErrNone */
	TTErr getParameterNames(TTValue& value);
    
    /** Specific process method on start
     @return                an error code returned by the process end method */
    TTErr ProcessStart();
    
    /** Specific process method on end
     @return                an error code returned by the process end method */
    TTErr ProcessEnd();
    
    /** Specific process method
     @return                an error code returned by the process method */
    TTErr Process();
    
    /** Register a time process for scenario management
     @value                 a time process object
     @return                an error code returned by the process method */
    TTErr RegisterTimeProcess(const TTValue& inputValue, TTValue& outputValue);
    
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
    
    friend TTErr TT_EXTENSION_EXPORT TimeProcessActiveAttributeCallback(TTPtr baton, TTValue& data);
    friend TTErr TT_EXTENSION_EXPORT TimeProcessRunningAttributeCallback(TTPtr baton, TTValue& data);
    friend TTErr TT_EXTENSION_EXPORT TimeProcessStartAttributeCallback(TTPtr baton, TTValue& data);
    friend TTErr TT_EXTENSION_EXPORT TimeProcessEndAttributeCallback(TTPtr baton, TTValue& data);
    friend TTErr TT_EXTENSION_EXPORT TimeProcessProgressionAttributeCallback(TTPtr baton, TTValue& data);
};

typedef Scenario* ScenarioPtr;

/** The callback method used to observe time processes active attribute change
 @param	baton						a time process instance
 @param	data						a new active value
 @return							an error code */
TTErr TT_EXTENSION_EXPORT TimeProcessActiveAttributeCallback(TTPtr baton, TTValue& data);

/** The callback method used to observe time processes running attribute change
 @param	baton						a time process instance
 @param	data						a new running value
 @return							an error code */
TTErr TT_EXTENSION_EXPORT TimeProcessRunningAttributeCallback(TTPtr baton, TTValue& data);

/** The callback method used to observe time processes start attribute change
 @param	baton						a time process instance
 @param	data						a new start value
 @return							an error code */
TTErr TT_EXTENSION_EXPORT TimeProcessStartAttributeCallback(TTPtr baton, TTValue& data);

/** The callback method used to observe time processes end attribute change
 @param	baton						a time process instance
 @param	data						a new end value
 @return							an error code */
TTErr TT_EXTENSION_EXPORT TimeProcessEndAttributeCallback(TTPtr baton, TTValue& data);

/** The callback method used to observe time processes progression attribute change
 @param	baton						a time process instance
 @param	data						a new progression value
 @return							an error code */
TTErr TT_EXTENSION_EXPORT TimeProcessProgressionAttributeCallback(TTPtr baton, TTValue& data);

#endif // __SCENARIO_H__
