/*
 * Curve object
 * Copyright © 2013, Théo de la Hogue
 *
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

/*!
 * \class Curve
 *
 *  a curve handles a function unit and some other features to avoid redundency, sample rate, ... 
 *
 */

#ifndef __CURVE_H__
#define __CURVE_H__

#include "TTFoundationAPI.h"
#include "TTModular.h"

class Curve : public TTDataObjectBase
{
	TTCLASS_SETUP(Curve)
	
private :
    
    TTBoolean                           mActive;                        ///< ATTRIBUTE : is the curve ready to run ?
    TTBoolean                           mRedundancy;                    ///< ATTRIBUTE : is the curve allow repetitions ?
    TTUInt32                            mSampleRate;                    ///< ATTRIBUTE : time precision of the curve
    TTObjectBasePtr                     mFunction;						///< ATTRIBUTE : a freehand function unit

    /** Set curve's parameters at an address
     @value                 address x1 y1 b1 x2 y2 b2 ... with x[0. :: 1.], y[min, max], b[-1. :: 1.]
     @return                an error code if the operation fails */
    TTErr   setParameters(const TTValue& value);
    
    /** Get curve's parameters at an address
     @value                 x1 y1 b1 x2 y2 b2 ... with x[0. :: 1.], y[min, max], b[-1. :: 1.]
     @return                an error code if the operation fails */
    TTErr   getParameters(TTValue& value);
    
    /** Get curve's values at an address
     @inputvalue            duration
     @outputvalue           all x y point of the curve
     @return                an error code if the operation fails */
    TTErr   Sample(const TTValue& inputValue, TTValue& outputValue);
    
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

typedef Curve* CurvePtr;

#endif // __CURVE_H__
