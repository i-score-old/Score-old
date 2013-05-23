/*
 * Automation time Process
 * Copyright © 2013, Théo de la Hogue, Clément Bossut
 *
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

/*!
 * \class Automation
 *
 *  Automation time Process class
 *
 */

#include "Automation.h"

#define thisTTClass                 Automation
#define thisTTClassName             "Automation"
#define thisTTClassTags             "time, process, automation"

#define thisTimeProcessVersion		"0.1"
#define thisTimeProcessAuthor        "Theo de la Hogue"

extern "C" TT_EXTENSION_EXPORT TTErr TTLoadJamomaExtension_Automation(void)
{
	TTFoundationInit();
	Automation::registerClass();
	return kTTErrNone;
}

TIME_PROCESS_CONSTRUCTOR
{
    TIME_PROCESS_INITIALIZE
    
	TT_ASSERT("Correct number of args to create Automation", arguments.size() == 0);
    
    registerAttribute(TTSymbol("curveAddresses"), kTypeLocalValue, NULL, (TTGetterMethod)& Automation::getCurveAddresses);
    
    addMessageWithArguments(CurveAdd);
    addMessageWithArguments(CurveSet);
    addMessageWithArguments(CurveValues);
    addMessageWithArguments(CurveRemove);
    addMessage(Clear);
    
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
}

Automation::~Automation()
{
    Clear();
}

TTErr Automation::getParameterNames(TTValue& value)
{
    value.clear();
	//value.append(TTSymbol("aParameterName"));
	
	return kTTErrNone;
}

TTErr Automation::getCurveAddresses(TTValue& value)
{
    return mCurves.getKeys(value);
}

TTErr Automation::ProcessStart()
{
    TTObjectBasePtr state;
    TTValue         v;
    
    mStartEvent->getAttributeValue(TTSymbol("state"), v);
    state = v[0];
    
    // recall the start state
    return state->sendMessage(kTTSym_Run);
}

TTErr Automation::ProcessEnd()
{
    TTObjectBasePtr state;
    TTValue         v;
    
    mEndEvent->getAttributeValue(TTSymbol("state"), v);
    state = v[0];
    
    // recall the start state
    return state->sendMessage(kTTSym_Run);
}

TTErr Automation::Process(const TTValue& inputValue, TTValue& outputValue)
{
    TTObjectBasePtr startState, endState;
    TTFloat64       progression;
    TTValue         v;
    
    if (inputValue.size() == 1) {
        
        if (inputValue[0].type() == kTypeFloat64) {
            
            progression = inputValue[0];
            
            mStartEvent->getAttributeValue(TTSymbol("state"), v);
            startState = v[0];
            
            mEndEvent->getAttributeValue(TTSymbol("state"), v);
            endState = v[0];
            
            // process the interpolation between the start state and the end state
            return TTScriptInterpolate(TTScriptPtr(startState), TTScriptPtr(endState), progression);
        }
    }
    
    return kTTErrGeneric;
}

TTErr Automation::WriteAsXml(const TTValue& inputValue, TTValue& outputValue)
{
	TTXmlHandlerPtr	aXmlHandler = NULL;
	
	aXmlHandler = TTXmlHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// TODO : write the automation attributes, the cue start and end content
	
	return kTTErrGeneric;
}

TTErr Automation::ReadFromXml(const TTValue& inputValue, TTValue& outputValue)
{
	TTXmlHandlerPtr	aXmlHandler = NULL;
	
	aXmlHandler = TTXmlHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// TODO : parse the automation attributes, the cue start and end content
	
	return kTTErrGeneric;
}

TTErr Automation::WriteAsText(const TTValue& inputValue, TTValue& outputValue)
{
	TTTextHandlerPtr	aTextHandler;
	
	aTextHandler = TTTextHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// TODO : write the automation attributes, the cue start and end content
	
	return kTTErrGeneric;
}

TTErr Automation::ReadFromText(const TTValue& inputValue, TTValue& outputValue)
{
	TTTextHandlerPtr aTextHandler;
	TTValue	v;
	
	aTextHandler = TTTextHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
    // TODO : parse the automation attributes, the cue start and end content
	
	return kTTErrGeneric;
}

TTErr Automation::CurveAdd(const TTValue& inputValue, TTValue& outputValue)
{
    TTValue         v;
    TTAddress       address;
    TTObjectBasePtr function = NULL;
    TTErr           err;
    
    if (inputValue.size() == 1) {
        
        if (inputValue[0].type() == kTypeSymbol) {
            
            address = inputValue[0];
            
            // if there is no curve at the address
            if (mCurves.lookup(address, v)) {
                
                // create a freehand function unit
                err = TTObjectBaseInstantiate(TTSymbol("freehand"), TTObjectBaseHandle(&function), 1); // for 1 channel only
                
                if (!err) {
                    
                    // register the function unit at address
                    return mCurves.append(address, function);
                }
            }
        }
    }
    
    return kTTErrGeneric;
}

TTErr Automation::CurveSet(const TTValue& inputValue, TTValue& outputValue)
{
    TTValue         v, curveList;
    TTAddress       address;
    TTObjectBasePtr function;
    TTUInt32        i;
    
    if (inputValue.size() > 0) {
        
        if (inputValue[0].type() == kTypeSymbol) {
            
            address = inputValue[0];
            
            // if there is a curve at the address
            if (!mCurves.lookup(address, v)) {
                
                function = v[0];
                
                // edit function curve list
                // inputValue   : address x1 y1 b1 x2 y2 b2 . . .
                // curveList    : x1 y1 exponential base b1 x2 y2 exponential base b2 . . . . .
                
                curveList.resize( ((inputValue.size() - 1) / 3) * 5);
                
                // check inputValue format
                if ( ((curveList.size() / 5) * 3) + 1 != inputValue.size())
                    return kTTErrGeneric;
                
                for (i = 1; i < inputValue.size(); i = i+3) {
                    
                    if (inputValue[i].type() == kTypeFloat64 &&
                        inputValue[i+1].type() == kTypeFloat64 &&
                        inputValue[i+2].type() == kTypeFloat64) {
                    
                        curveList[i] = inputValue[i];
                        curveList[i+1] = inputValue[i+1];
                        curveList[i+2] = TTSymbol("exponential");
                        curveList[i+3] = TTSymbol("base");
                        curveList[i+4] = inputValue[i+2];
                    }
                    else
                        return kTTErrGeneric;
                }
                
                // set function curve list
                return function->setAttributeValue(TTSymbol("curveList"), curveList);
            }
        }
    }
    
    return kTTErrGeneric;
}

TTErr Automation::CurveValues(const TTValue& inputValue, TTValue& outputValue)
{
    TTValue         v;
    TTAddress       address;
    TTObjectBasePtr function;
    TTUInt32        i, granularity, duration, nbPoints;
    TTFloat64       mapped;
    
    if (inputValue.size() == 1) {
        
        if (inputValue[0].type() == kTypeSymbol) {
            
            address = inputValue[0];
            
            // if there is a curve at the address
            if (!mCurves.lookup(address, v)) {
                
                function = v[0];
                
                // compute how many points to process
                mScheduler->getAttributeValue(TTSymbol("granularity"), v);
                granularity = 20; // v[0];
                
                this->getAttributeValue(TTSymbol("duration"), v);
                duration = v[0];
                
                nbPoints = duration / granularity;
                
                // get function values
                outputValue.clear();
                for (i = 0; i < nbPoints; i++) {
                    
                    TTAudioObjectBasePtr(function)->calculate(TTFloat64(i/nbPoints), mapped);
                    
                    outputValue.append(mapped);
                }
                
                return kTTErrNone;
            }
        }
    }
    
    return kTTErrGeneric;
}

TTErr Automation::CurveRemove(const TTValue& inputValue, TTValue& outputValue)
{
    TTValue         v;
    TTAddress       address;
    TTObjectBasePtr function;
    
    if (inputValue.size() > 0) {
        
        if (inputValue[0].type() == kTypeSymbol) {
            
            address = inputValue[0];
            
            // if there is a curve at the address
            if (!mCurves.lookup(address, v)) {
                
                function = v[0];
                
                // unregister the function unit
                mCurves.remove(address);
                
                // delete function unit
                return TTObjectBaseRelease(&function);
            }
        }
    }
    
    return kTTErrGeneric;
}

TTErr Automation::Clear()
{
    TTValue         keys,v;
    TTSymbol        key;
    TTUInt32        i;
    TTObjectBasePtr function;
    
    // delete all function units
    mCurves.getKeys(keys);
    for (i = 0; i < keys.size(); i++) {
        
        key = keys[i];
        mCurves.lookup(key, v);
        
        function = v[0];
        
        TTObjectBaseRelease(&function);
    }
    
    return kTTErrNone;
}

#if 0
#pragma mark -
#pragma mark Some Methods
#endif
