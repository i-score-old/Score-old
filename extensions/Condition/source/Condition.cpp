/*
 * Condition time process
 * Copyright © 2013, Clément Bossut
 *
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

/*!
 * \class Condition
 *
 *  Condition time process class manage ...
 *
 */

#include "Condition.h"

#define thisTTClass                 Condition
#define thisTTClassName             "Condition"
#define thisTTClassTags             "time, process, condition"

extern "C" TT_EXTENSION_EXPORT TTErr TTLoadJamomaExtension_Condition(void)
{
	TTFoundationInit();
	Condition::registerClass();
	return kTTErrNone;
}

TIME_PROCESS_CONSTRUCTOR
{
    TIME_PROCESS_INITIALIZE
    
	TT_ASSERT("Correct number of args to create Automation", arguments.size() == 0);
    
    // addAttribute
    
    // addMessage
}

Condition::~Condition()
{
    ;
}

TTErr Condition::getParameterNames(TTValue& value)
{
    value.clear();
	//value.append(TTSymbol("aParameterName"));
	
	return kTTErrNone;
}

TTErr Condition::ProcessStart()
{
    // do something on start
    return kTTErrNone;
}

TTErr Condition::ProcessEnd()
{
    // do something on end
    return kTTErrNone;
}

TTErr Condition::Process(const TTValue& inputValue, TTValue& outputValue)
{
    TTFloat64       progression;
    
    if (inputValue.size() == 1) {
        
        if (inputValue[0].type() == kTypeFloat64) {
            
            progression = inputValue[0];
            
            // do something during progression
            return kTTErrNone;
        }
    }
    
    return kTTErrGeneric;
}

TTErr Condition::WriteAsXml(const TTValue& inputValue, TTValue& outputValue)
{
	TTXmlHandlerPtr	aXmlHandler = NULL;
	
	aXmlHandler = TTXmlHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// TODO : write the condition attributes, the cue start and end content
	
	return kTTErrGeneric;
}

TTErr Condition::ReadFromXml(const TTValue& inputValue, TTValue& outputValue)
{
	TTXmlHandlerPtr	aXmlHandler = NULL;
	
	aXmlHandler = TTXmlHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// TODO : parse the condition attributes, the cue start and end content
	
	return kTTErrGeneric;
}

TTErr Condition::WriteAsText(const TTValue& inputValue, TTValue& outputValue)
{
	TTTextHandlerPtr	aTextHandler;
	
	aTextHandler = TTTextHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// TODO : write the condition attributes, the cue start and end content
	
	return kTTErrGeneric;
}

TTErr Condition::ReadFromText(const TTValue& inputValue, TTValue& outputValue)
{
	TTTextHandlerPtr aTextHandler;
	TTValue	v;
	
	aTextHandler = TTTextHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
    // TODO : parse the condition attributes, the cue start and end content
	
	return kTTErrGeneric;
}

#if 0
#pragma mark -
#pragma mark Some Methods
#endif
