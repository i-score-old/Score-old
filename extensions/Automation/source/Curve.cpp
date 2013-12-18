/** @file
 *
 * @ingroup scoreExtension
 *
 * @brief a curve samples a freehand function unit at a sample rate
 *
 * @see Automation
 *
 * @authors Théo de la Hogue & Clément Bossut
 *
 * @copyright Copyright © 2013, Théo de la Hogue & Clément Bossut @n
 * This code is licensed under the terms of the "CeCILL-C" @n
 * http://www.cecill.info
 */

#include "Curve.h"

#define thisTTClass                 Curve
#define thisTTClassName             "Curve"
#define thisTTClassTags             "curve"

TT_BASE_OBJECT_CONSTRUCTOR,
mActive(YES),
mRedundancy(NO),
mSampleRate(20),
mFunction(NULL),
mRecorded(NO),
mSampled(NO),
mLastSample(0.)
{
	TT_ASSERT("Correct number of args to create Curve", arguments.size() == 0);
    
    registerAttribute(TTSymbol("functionParameters"), kTypeLocalValue, NULL, (TTGetterMethod)& Curve::getFunctionParameters, (TTSetterMethod)& Curve::setFunctionParameters);
    
    addAttribute(Active, kTypeBoolean);
    addAttribute(Redundancy, kTypeBoolean);
    addAttributeWithSetter(SampleRate, kTypeUInt32);
    addAttribute(Recorded, kTypeBoolean);
    addAttribute(Sampled, kTypeBoolean);
    
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

TTErr Curve::getFunctionParameters(TTValue& value)
{
    TTValue         curveList;
    TTUInt32        i, j;
    
    if (mRecorded)
        return kTTErrGeneric;
    
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

TTErr Curve::setFunctionParameters(const TTValue& value)
{
    TTValue     curveList, none;
    TTUInt32    i, j, duration;
    
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
        
        // retreive the current duration
        duration = getSize() * mSampleRate;
        
        // clear the samples
        clear();
        
        // it is not based on a record anymore
        mRecorded = NO;
        
        // set function curve list
        mFunction->setAttributeValue(TTSymbol("curveList"), curveList);
        
        // sample the curve
        mSampled = NO;
        if (duration)
            Sample(duration, none);
        
        return kTTErrNone;
    }
    
    return kTTErrGeneric;
}

TTErr Curve::setSampleRate(const TTValue& value)
{
    TTUInt32    newSampleRate, duration;
    TTValue     none;
    
    if (value.size() == 1) {
        
        if (value[0].type() == kTypeUInt32) {
            
            newSampleRate = value;
            
            // filter repetitions
            if (newSampleRate != mSampleRate) {
                
                // retreive the current duration from the old sample rate
                duration = getSize() * mSampleRate;
                
                // set the new sample rate
                mSampleRate = newSampleRate;
                
                // resample the curve
                mSampled = NO;
                if (duration)
                    Sample(duration, none);
                
                return kTTErrNone;
            }
        }
    }
    
    return kTTErrGeneric;
}

TTErr Curve::Sample(const TTValue& inputValue, TTValue& outputValue)
{
    if (inputValue.size() == 1) {
        
        if (inputValue[0].type() == kTypeUInt32) {
            
            TTUInt32    i, nbPoints, duration;
            TTFloat64   x, y;
            
            duration = inputValue[0];
            
            nbPoints = duration / mSampleRate;
            
            // for a same number of points and already sampled curve
            if (nbPoints == getSize() && mSampled) {
                
                // return the samples
                outputValue.clear();
                for (begin(); end(); next())
                    outputValue.append(current()[1]);
                
                mSampled = YES;
                
                return kTTErrNone;
            }
            
            // for a record based curve
            if (mRecorded) {
                
                TTList newSamples;
                
                // get new samples from current samples
                begin();
                outputValue.clear();
                for (i = 0; i < nbPoints; i++) {
                    
                    x = TTFloat64(i) / TTFloat64(nbPoints);
                    nextSampleAt(x, y);
                    
                    newSamples.append(TTValue(x, y));
                    outputValue.append(y);
                }
                
                // copy the new samples
                clear();
                for (newSamples.begin(); newSamples.end(); newSamples.next())
                    append(newSamples.current());
            }
            
            // for a function based curve
            else {
                
                // get new samples from function
                clear();
                outputValue.clear();
                for (i = 0; i < nbPoints; i++) {
                    
                    x = TTFloat64(i) / TTFloat64(nbPoints);
                    TTAudioObjectBasePtr(mFunction)->calculate(x, y);
                    
                    append(TTValue(x, y));
                    outputValue.append(y);
                }
            }
            
            mSampled = YES;
            
            return kTTErrNone;
        }
    }

    return kTTErrGeneric;
}

TTErr Curve::WriteAsXml(const TTValue& inputValue, TTValue& outputValue)
{
	TTXmlHandlerPtr	aXmlHandler = NULL;
    TTValue         v;
    TTString        s;
	
	aXmlHandler = TTXmlHandlerPtr((TTObjectBasePtr)inputValue[0]);
    
    xmlTextWriterStartElement((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "curve");
	
    // Write if it is active
    v = mActive;
    v.toString();
    s = TTString(v[0]);
    xmlTextWriterWriteAttribute((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "active", BAD_CAST s.data());
    
    // Write the redundancy
    v = mRedundancy;
    v.toString();
    s = TTString(v[0]);
    xmlTextWriterWriteAttribute((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "redundancy", BAD_CAST s.data());
    
    // Write the sample rate
    v = mSampleRate;
    v.toString();
    s = TTString(v[0]);
    xmlTextWriterWriteAttribute((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "sampleRate", BAD_CAST s.data());
    
    if (!mRecorded) {
        
        // Write the function parameters
        getFunctionParameters(v);
        v.toString();
        s = TTString(v[0]);
        xmlTextWriterWriteAttribute((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "function", BAD_CAST s.data());
    }
    else {
        
        // Write the samples
        v.clear();
        for (begin(); end(); next()) {
            
            v.append(current()[0]);
            v.append(current()[1]);
        }
            
        v.toString();
        s = TTString(v[0]);
        xmlTextWriterWriteAttribute((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "samples", BAD_CAST s.data());
    }
    
    xmlTextWriterEndElement((xmlTextWriterPtr)aXmlHandler->mWriter);
	
	return kTTErrNone;
}

TTErr Curve::ReadFromXml(const TTValue& inputValue, TTValue& outputValue)
{
	TTXmlHandlerPtr	aXmlHandler = NULL;
    TTValue         v;
	
	aXmlHandler = TTXmlHandlerPtr((TTObjectBasePtr)inputValue[0]);
    
    // get the active state
    if (!aXmlHandler->getXmlAttribute(kTTSym_active, v, NO)) {
        
        if (v.size() == 1) {
            
            if (v[0].type() == kTypeInt32) {
                
                mActive = v[0] == 1;
            }
        }
    }
    
    // get the redundancy
    if (!aXmlHandler->getXmlAttribute(kTTSym_redundancy, v, NO)) {
        
        if (v.size() == 1) {
            
            if (v[0].type() == kTypeInt32) {
                
                mRedundancy = v[0] == 1;
            }
        }
    }
    
	// get the sample rate
    if (!aXmlHandler->getXmlAttribute(kTTSym_sampleRate, v, NO)) {
        
        if (v.size() == 1) {
            
            if (v[0].type() == kTypeUInt32) {
                
                mSampleRate = v[0];
            }
        }
    }
    
    // get the function parameters
    if (!aXmlHandler->getXmlAttribute(kTTSym_function, v, NO)) {
        
        setFunctionParameters(v);
        
        mRecorded = NO;
    }
    
    // get the function samples
    else if (!aXmlHandler->getXmlAttribute(kTTSym_samples, v, NO)) {
        
        clear();
        for (TTUInt32 i = 0; i < v.size(); i = i+2)
            append(TTValue(v[i], v[i+1]));
        
        mRecorded = YES;
        mSampled = YES;
    }
    
	return kTTErrNone;
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

TTErr Curve::nextSampleAt(TTFloat64& x, TTFloat64& y)
{
    if (mActive) {
        
        TTBoolean   found = NO;
        TTErr       err = kTTErrNone;
        
        // while the list doesn't reach the end
        while (end()) {
                
            if (TTFloat64(current()[0]) < x)
                next();
            
            else {
                y = current()[1];
                found = YES;
                break;
            }
        }
        
        if (found) {
            
            if (!mRedundancy && y == mLastSample)
                err = kTTErrGeneric;
            
            mLastSample = y;
            
            return err;
        }
    }
    
    return kTTErrValueNotFound;
}

#if 0
#pragma mark -
#pragma mark Some Methods
#endif
