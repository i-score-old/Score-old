/*
 * Automation time process
 * Copyright © 2013, Théo de la Hogue, Clément Bossut
 *
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

/*!
 * \class Automation
 *
 *  Automation time process class manage interpolation between the start event state and end event state depending on the scheduler progression
 *
 */

#ifndef __AUTOMATION_H__
#define __AUTOMATION_H__

#include "TimeProcess.h"

class Automation : public TimeProcess
{
	TTCLASS_SETUP(Automation)
	
private :
    
    TTHash                      mCurves;						///< a table of freehand function units stored by address
	
    /** Get parameters names needed by this time process
     @param	value           the returned parameter names
     @return                kTTErrNone */
	TTErr   getParameterNames(TTValue& value);
    
    /** Get curve addresses
     @param	value           the returned curve addresses
     @return                kTTErrNone */
	TTErr   getCurveAddresses(TTValue& value);
    
    /** Specific process method on start
     @return                an error code returned by the process end method */
    TTErr   ProcessStart();
    
    /** Specific process method on end
     @return                an error code returned by the process end method */
    TTErr   ProcessEnd();
    
    /** Specific process method
     @param	inputValue      progression of the scheduler
     @param	outputValue     return an error of the processing
     @return                an error code returned by the process method */
    TTErr   Process(const TTValue& inputValue, TTValue& outputValue);
    
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
     @inputvalue            address
     @outputvalue           kTTValNONE
     @return                an error code if the operation fails */
    TTErr   CurveAdd(const TTValue& inputValue, TTValue& outputValue);
    
    /** Set curve's parameters at an address
     @inputvalue            address x1 y1 b1 x2 y2 b2 ... 
     @outputvalue           kTTValNONE
     @return                an error code if the operation fails */
    TTErr   CurveSet(const TTValue& inputValue, TTValue& outputValue);
    
    /** Get curve's values at an address
     @inputvalue            address
     @outputvalue           all x y point of the curve
     @return                an error code if the operation fails */
    TTErr   CurveValues(const TTValue& inputValue, TTValue& outputValue);
    
    /** Remove a curve at an address
     @inputValue            address
     @outputvalue           kTTValNONE
     @return                an error code if the operation fails */
    TTErr   CurveRemove(const TTValue& inputValue, TTValue& outputValue);
    
    /** Clear all curves
     @return                an error code if the operation fails */
    TTErr   Clear();
};

typedef Automation* AutomationPtr;

#endif // __AUTOMATION_H__
