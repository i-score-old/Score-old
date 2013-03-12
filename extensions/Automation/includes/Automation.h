/*
 * Automation time process
 * Copyright © 2013, Théo de la Hogue
 *
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

/*!
 * \class Automation
 *
 *  Automation time process class manage 2 states and their interpolation depending on the scheduler progression
 *
 */

#ifndef __AUTOMATION_H__
#define __AUTOMATION_H__

#include "TimeProcess.h"

class Automation : public TimeProcess
{
	TTCLASS_SETUP(Automation)
	
private :
    
    // Clément n'est pas convaincu pas le nom Cue (cf pratique théatrale) : choisir State ?
	TTObjectBasePtr				mStartCue;						///< ATTRIBUTE : the beginning state handled by the time process
    TTObjectBasePtr				mCurveCue;						///< ATTRIBUTE : the curve function parameter of all rampable parameters
    TTObjectBasePtr				mEndCue;						///< ATTRIBUTE : the final state handled by the time process
    
    TTAddressItemPtr            mNamespace;                     ///< ATTRIBUTE : the namespace workspace to use for state storage
	
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

typedef Automation* AutomationPtr;

#endif // __AUTOMATION_H__
