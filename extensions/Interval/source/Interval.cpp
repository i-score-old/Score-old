/*
 * Interval time process
 * Copyright © 2013, Théo de la Hogue, Clément Bossut
 *
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#include "Interval.h"

#define thisTTClass         Interval
#define thisTTClassName     "Interval"
#define thisTTClassTags     "time, process, interval"

extern "C" TT_EXTENSION_EXPORT TTErr TTLoadJamomaExtension_Interval(void)
{
	TTFoundationInit();
	Interval::registerClass();
	return kTTErrNone;
}

TIME_PROCESS_CONSTRUCTOR
{
    TIME_PLUGIN_INITIALIZE
    
	TT_ASSERT("Correct number of args to create Interval", arguments.size() == 0);
}

Interval::~Interval()
{
    ;
}

TTErr Interval::getParameterNames(TTValue& value)
{
    value.clear();
	//value.append(TTSymbol("aParameterName"));
	
	return kTTErrNone;
}

TTErr Interval::ProcessStart()
{
    return kTTErrNone;
}

TTErr Interval::ProcessEnd()
{
    return kTTErrNone;
}

TTErr Interval::Process(const TTValue& inputValue, TTValue& outputValue)
{
    return kTTErrNone;
}

TTErr Interval::WriteAsXml(const TTValue& inputValue, TTValue& outputValue)
{
	TTXmlHandlerPtr	aXmlHandler = NULL;
    TTValue         v;
    TTSymbol        name;
	
	aXmlHandler = TTXmlHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
    xmlTextWriterStartElement((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "interval");
    
    // Write the name
    xmlTextWriterWriteAttribute((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "name", BAD_CAST mName.c_str());
    
    // Write the start event name
    getStartEvent()->getAttributeValue(kTTSym_name, v);
    name = v[0];
    xmlTextWriterWriteAttribute((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "start", BAD_CAST name.c_str());
    
    // Write the end event name
    getEndEvent()->getAttributeValue(kTTSym_name, v);
    name = v[0];
    xmlTextWriterWriteAttribute((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "end", BAD_CAST name.c_str());
    
    xmlTextWriterEndElement((xmlTextWriterPtr)aXmlHandler->mWriter);
	
	return kTTErrNone;
}

TTErr Interval::ReadFromXml(const TTValue& inputValue, TTValue& outputValue)
{
	TTXmlHandlerPtr	aXmlHandler = NULL;
	
	aXmlHandler = TTXmlHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// TODO : parse the time box attributes, the cue start and end content, start and end receiver, ...
	
	return kTTErrGeneric;
}

TTErr Interval::WriteAsText(const TTValue& inputValue, TTValue& outputValue)
{
	TTTextHandlerPtr	aTextHandler;
	
	aTextHandler = TTTextHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// TODO : write the time box attributes, the cue start and end content, start and end receiver, ...
	
	return kTTErrGeneric;
}

TTErr Interval::ReadFromText(const TTValue& inputValue, TTValue& outputValue)
{
	TTTextHandlerPtr aTextHandler;
	TTValue	v;
	
	aTextHandler = TTTextHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
    // TODO : parse the time box attributes, the cue start and end content, start and end receiver, ...
	
	return kTTErrGeneric;
}