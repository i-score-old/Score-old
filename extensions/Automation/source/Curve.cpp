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

#include "Curve.h"

#define thisTTClass                 Curve
#define thisTTClassName             "Curve"
#define thisTTClassTags             "curve"

TT_OBJECT_CONSTRUCTOR,
mActive(YES),
mRedundancy(NO),
mSampleRate(20),
mFunction(NULL)
{
	TT_ASSERT("Correct number of args to create Curve", arguments.size() == 0);
    
    registerAttribute(TTSymbol("parameters"), kTypeLocalValue, NULL, (TTGetterMethod)& Curve::getParameters, (TTSetterMethod)& Curve::setParameters);
    
    addAttribute(Active, kTypeBoolean);
    addAttribute(Redundancy, kTypeBoolean);
    addAttribute(SampleRate, kTypeUInt32);
    addAttribute(Function, kTypeObject);
    
    addMessageWithArguments(Sample);
    
	// needed to be handled by a TTXmlHandler
	addMessageWithArguments(WriteAsXml);
	addMessageProperty(WriteAsXml, hidden, YES);
	addMessageWithArguments(ReadFromXml);
	addMessageProperty(ReadFromXml, hidden, YES);
	
	// needed to be handled by a TTTextHandler
	addMessageWithArguments(WriteAsText);
	addMessageProperty(WriteAsText, hidden, YES);
	addMessageWithArguments(ReadFromText);
	addMessageProperty(ReadFromText, hidden, YES);
    
    TTObjectBaseInstantiate(TTSymbol("freehand"), TTObjectBaseHandle(&mFunction), 1); // for 1 channel only
}

Curve::~Curve()
{
    TTObjectBaseRelease(&mFunction);
}

TTErr Curve::getParameters(TTValue& value)
{
    TTValue         curveList;
    TTUInt32        i, j;
    
    if (!mFunction->getAttributeValue(TTSymbol("curveList"), curveList)) {
        
        // edit function value
        // curveList    : x1 y1 exponential base b1 x2 y2 exponential base b2 . . . . .
        // value        : x1 y1 b1 x2 y2 b2 . . .
        
        value.resize((curveList.size() / 5) * 3);
        
        j = 0;
        for (i = 0; i < curveList.size(); i = i+5) {
            
            value[j] = curveList[i];
            value[j+1] = curveList[i+1];
            value[j+2] = curveList[i+4];
            
            j = j+3;
        }
        
        return kTTErrNone;
    }
    
    return kTTErrGeneric;
}

TTErr Curve::setParameters(const TTValue& value)
{
    TTValue     curveList;
    TTUInt32    i, j;
    
    if (value.size() > 0) {
        
        // edit function curve list
        // value        : x1 y1 b1 x2 y2 b2 . . .
        // curveList    : x1 y1 exponential base b1 x2 y2 exponential base b2 . . . . .
        
        curveList.resize((value.size() / 3) * 5);
        
        // check inputValue format
        if ( (curveList.size() / 5) * 3 != value.size())
            return kTTErrGeneric;
        
        j = 0;
        for (i = 0; i < value.size(); i = i+3) {
            
            if (value[i].type() == kTypeFloat64 &&
                value[i+1].type() == kTypeFloat64 &&
                value[i+2].type() == kTypeFloat64) {
                
                curveList[j] = value[i];
                curveList[j+1] = value[i+1];
                curveList[j+2] = TTSymbol("exponential");
                curveList[j+3] = TTSymbol("base");
                curveList[j+4] = value[i+2];
                
                j = j+5;
            }
            else
                return kTTErrGeneric;
        }
        
        // set function curve list
        return mFunction->setAttributeValue(TTSymbol("curveList"), curveList);
    }
    
    return kTTErrGeneric;
}

TTErr Curve::Sample(const TTValue& inputValue, TTValue& outputValue)
{
    TTUInt32        i, duration, nbPoints;
    TTFloat64       x, y;
    
    if (inputValue.size() == 1) {
        
        if (inputValue[0].type() == kTypeUInt32) {
            
            duration = inputValue[0];
            
            nbPoints = duration / mSampleRate;
            
            // get function values
            outputValue.clear();
            for (i = 0; i < nbPoints; i++) {
                
                x = TTFloat64(i) / TTFloat64(nbPoints);
                TTAudioObjectBasePtr(mFunction)->calculate(x, y);
                
                outputValue.append(y);
            }
            
            return kTTErrNone;
        }
    }
    
    return kTTErrGeneric;
}

TTErr Curve::WriteAsXml(const TTValue& inputValue, TTValue& outputValue)
{
	TTXmlHandlerPtr	aXmlHandler = NULL;
	
	aXmlHandler = TTXmlHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// TODO : write the curve attributes
	
	return kTTErrGeneric;
}

TTErr Curve::ReadFromXml(const TTValue& inputValue, TTValue& outputValue)
{
	TTXmlHandlerPtr	aXmlHandler = NULL;
	
	aXmlHandler = TTXmlHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// TODO : parse the curve attributes
	
	return kTTErrGeneric;
}

TTErr Curve::WriteAsText(const TTValue& inputValue, TTValue& outputValue)
{
	TTTextHandlerPtr	aTextHandler;
	
	aTextHandler = TTTextHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// TODO : write the curve attributes
	
	return kTTErrGeneric;
}

TTErr Curve::ReadFromText(const TTValue& inputValue, TTValue& outputValue)
{
	TTTextHandlerPtr aTextHandler;
	TTValue	v;
	
	aTextHandler = TTTextHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
    // TODO : parse the curve attributes
	
	return kTTErrGeneric;
}

#if 0
#pragma mark -
#pragma mark Some Methods
#endif
