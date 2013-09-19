/** @file
 *
 * @ingroup scoreExtension
 *
 * @brief a curve handles a function unit and some other features to avoid redundency, sample rate, ...
 *
 * @details The Curve class allows to ... @n@n
 *
 * @see Automation
 *
 * @authors Théo de la Hogue & Clément Bossut
 *
 * @copyright Copyright © 2013, Théo de la Hogue & Clément Bossut @n
 * This code is licensed under the terms of the "CeCILL-C" @n
 * http://www.cecill.info
 */

#ifndef __CURVE_H__
#define __CURVE_H__

#include "TimePluginLib.h"

/**	The Curve class allows to ...
 
 @see Automation
 */
class Curve : public TTDataObjectBase
{
	TTCLASS_SETUP(Curve)
	
private :
    
    TTBoolean                           mActive;                        ///< is the curve ready to run ?
    TTBoolean                           mRedundancy;                    ///< is the curve allow repetitions ?
    TTUInt32                            mSampleRate;                    ///< time precision of the curve
    TTObjectBasePtr                     mFunction;						///< a freehand function unit

    /** Set curve's parameters
     @param value           x1 y1 b1 x2 y2 b2 ... with x[0. :: 1.], y[min, max], b[-1. :: 1.]
     @return                an error code if the operation fails */
    TTErr   setParameters(const TTValue& value);
    
    /** Get curve's parameters
     @param value           x1 y1 b1 x2 y2 b2 ... with x[0. :: 1.], y[min, max], b[-1. :: 1.]
     @return                an error code if the operation fails */
    TTErr   getParameters(TTValue& value);
    
    /** Get all curve's values
     @param inputvalue      duration
     @param outputvalue     all x y point of the curve
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
    
public:
    
    /** Calculate curve's values for a given x
     @param x               a float64 between [0. :: 1.]
     @param y               a float64 between [min :: max]
     @return                an error code if the operation fails */
    TTErr   calculate(TTFloat64& x, TTFloat64& y);
};

typedef Curve* CurvePtr;

#endif // __CURVE_H__
