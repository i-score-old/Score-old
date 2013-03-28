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

TIME_PROCESS_CONSTRUCTOR,
mStartCue(NULL),
mEndCue(NULL),
mNamespace(NULL)
{
    TIME_PROCESS_INITIALIZE
    
    TTErr           err;
    
	TT_ASSERT("Correct number of args to create Automation", arguments.size() == 0);
    
    addAttribute(StartCue, kTypeObject);
    addAttributeProperty(StartCue, hidden, YES);
    
    addAttribute(EndCue, kTypeObject);
    addAttributeProperty(StartCue, hidden, YES);
	
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
    
    // Creation of a static time event for the start and subscribe to it
    err = TTObjectBaseInstantiate(TTSymbol("StaticEvent"), TTObjectBaseHandle(&mStartEvent), kTTValNONE);
    
	if (err) {
        mStartEvent = NULL;
		logError("TimeProcess failed to load a static start event");
    }
    
    mStartEvent->sendMessage(TTSymbol("Subscribe"), mStartEventCallback, kTTValNONE);
    
    // Creation of a static time event for the end and subscribe to it
    err = TTObjectBaseInstantiate(TTSymbol("StaticEvent"), TTObjectBaseHandle(&mEndEvent), kTTValNONE);
    
	if (err) {
        mStartEvent = NULL;
		logError("TimeProcess failed to load a static end event");
    }
    
    mEndEvent->sendMessage(TTSymbol("Subscribe"), mEndEventCallback, kTTValNONE);
}

Automation::~Automation()
{
    if (mStartCue) {
        TTObjectBaseRelease(TTObjectBaseHandle(&mStartCue));
        mStartCue = NULL;
    }
    
    if (mEndCue) {
        TTObjectBaseRelease(TTObjectBaseHandle(&mEndCue));
        mEndCue = NULL;
    }
    
    if (mNamespace) {
        delete mNamespace;
        mNamespace = NULL;
    }
}

TTErr Automation::getParameterNames(TTValue& value)
{
    value.clear();
	//value.append(TTSymbol("aParameterName"));
	
	return kTTErrNone;
}

TTErr Automation::ProcessStart()
{
    // recall the start state
    return mStartCue->sendMessage(TTSymbol("Recall"), kTTValNONE, kTTValNONE);
}

TTErr Automation::ProcessEnd()
{
    // recall the end state
    return mEndCue->sendMessage(TTSymbol("Recall"), kTTValNONE, kTTValNONE);
}

TTErr Automation::Process(const TTValue& inputValue, TTValue& outputValue)
{
    TTFloat64 progression;
    
    if (inputValue.size() == 1) {
        
        if (inputValue[0].type() == kTypeFloat64) {
            
            progression = inputValue[0];
            
            // process the interpolation between the start state and the end state
            return TTCueInterpolate(TTCuePtr(mStartCue), TTCuePtr(mEndCue), progression);
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

#if 0
#pragma mark -
#pragma mark Some Methods
#endif