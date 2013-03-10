/*
 * Automation time Process
 * Copyright © 2013, Théo de la Hogue
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
    
	TT_ASSERT("Correct number of args to create TTTimeBox", arguments.size() == 0);
    
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

TTErr Automation::Process()
{
    // process the interpolation between the start state and the end state
    return TTCueInterpolate(TTCuePtr(mStartCue), TTCuePtr(mEndCue), mProgression);
}

TTErr Automation::WriteAsXml(const TTValue& inputValue, TTValue& outputValue)
{
	TTXmlHandlerPtr	aXmlHandler = NULL;
	
	aXmlHandler = TTXmlHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// TODO : write the time box attributes, the cue start and end content, start and end receiver, ...
	
	return kTTErrGeneric;
}

TTErr Automation::ReadFromXml(const TTValue& inputValue, TTValue& outputValue)
{
	TTXmlHandlerPtr	aXmlHandler = NULL;
	
	aXmlHandler = TTXmlHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// TODO : parse the time box attributes, the cue start and end content, start and end receiver, ...
	
	return kTTErrGeneric;
}

TTErr Automation::WriteAsText(const TTValue& inputValue, TTValue& outputValue)
{
	TTTextHandlerPtr	aTextHandler;
	
	aTextHandler = TTTextHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// TODO : write the time box attributes, the cue start and end content, start and end receiver, ...
	
	return kTTErrGeneric;
}

TTErr Automation::ReadFromText(const TTValue& inputValue, TTValue& outputValue)
{
	TTTextHandlerPtr aTextHandler;
	TTValue	v;
	
	aTextHandler = TTTextHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
    // TODO : parse the time box attributes, the cue start and end content, start and end receiver, ...
	
	return kTTErrGeneric;
}

#if 0
#pragma mark -
#pragma mark Some Methods
#endif
