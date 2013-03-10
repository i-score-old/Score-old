/*
 * Scenario time Process
 * Copyright © 2013, Théo de la Hogue
 *
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

/*!
 * \class Scenario
 *
 *  Scenario time process class is a container class to manage other time processes instances in the time
 *
 */

#include "Scenario.h"

#define thisTTClass                 Scenario
#define thisTTClassName             "Scenario"
#define thisTTClassTags             "time, process, scenario"

#define thisTimeProcessVersion		"0.1"
#define thisTimeProcessAuthor        "Theo de la Hogue"

extern "C" TT_EXTENSION_EXPORT TTErr TTLoadJamomaExtension_Scenario(void)
{
	TTFoundationInit();
	Scenario::registerClass();
	return kTTErrNone;
}

TIME_PROCESS_CONSTRUCTOR,
mNamespace(NULL)
{
    TIME_PROCESS_INITIALIZE
    
	TT_ASSERT("Correct number of args to create TTTimeBox", arguments.size() == 0);
	
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

Scenario::~Scenario()
{
    if (mNamespace) {
        delete mNamespace;
        mNamespace = NULL;
    }
}

TTErr Scenario::getParameterNames(TTValue& value)
{
    value.clear();
	//value.append(TTSymbol("aParameterName"));
	
	return kTTErrNone;
}

TTErr Scenario::ProcessStart()
{
    // TODO : launch the processing of the first time process (?)
    return kTTErrGeneric;
}

TTErr Scenario::ProcessEnd()
{
    // TODO : is there something to do at the end of a scenario ?
    return kTTErrGeneric;
}

TTErr Scenario::Process()
{
    // TODO : normally there is nothing to do as the scenario only contains other time processes which are doing things (?)
    return kTTErrNone;
}

TTErr RegisterTimeProcess(const TTValue& inputValue, TTValue& outputValue)
{
    
    // TODO : create all CSP constrain
    return kTTErrGeneric;
}

TTErr Scenario::WriteAsXml(const TTValue& inputValue, TTValue& outputValue)
{
	TTXmlHandlerPtr	aXmlHandler = NULL;
	
	aXmlHandler = TTXmlHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// TODO : pass the XmlHandlerPtr to all the time processes of the scenario to write their content into the file
	
	return kTTErrGeneric;
}

TTErr Scenario::ReadFromXml(const TTValue& inputValue, TTValue& outputValue)
{
	TTXmlHandlerPtr	aXmlHandler = NULL;
	
	aXmlHandler = TTXmlHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// TODO : pass the XmlHandlerPtr to all the time processes of the scenario to read their content from the file
	
	return kTTErrGeneric;
}

TTErr Scenario::WriteAsText(const TTValue& inputValue, TTValue& outputValue)
{
	TTTextHandlerPtr	aTextHandler;
	
	aTextHandler = TTTextHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// TODO : pass the TextHandlerPtr to all the time processes of the scenario to write their content into the file
	
	return kTTErrGeneric;
}

TTErr Scenario::ReadFromText(const TTValue& inputValue, TTValue& outputValue)
{
	TTTextHandlerPtr aTextHandler;
	TTValue	v;
	
	aTextHandler = TTTextHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
    // TODO : pass the TextHandlerPtr to all the time processes of the scenario to read their content from the file
	
	return kTTErrGeneric;
}

#if 0
#pragma mark -
#pragma mark Some Methods
#endif

TTErr TimeProcessActiveAttributeCallback(TTPtr baton, TTValue& data)
{
    TimeProcessPtr  aTimeProcess;
	TTValuePtr      b;
	
	// unpack baton (a time process)
	b = (TTValuePtr)baton;
	aTimeProcess = TimeProcessPtr((TTObjectBasePtr)(*b)[0]);
    
    // TODO : warn CSP that this time process active state have changed
    // TODO : update all CSP consequences by setting time processes attributes that are affected by the consequence (is here we have to do that ?)
    return kTTErrGeneric;
}

TTErr TT_EXTENSION_EXPORT TimeProcessRunningAttributeCallback(TTPtr baton, TTValue& data)
{
    TimeProcessPtr  aTimeProcess;
	TTValuePtr      b;
	
	// unpack baton (a time process)
	b = (TTValuePtr)baton;
	aTimeProcess = TimeProcessPtr((TTObjectBasePtr)(*b)[0]);
    
    // TODO : warn Scheduler that this time process running state have changed (?)
    return kTTErrGeneric;
}

TTErr TT_EXTENSION_EXPORT TimeProcessStartAttributeCallback(TTPtr baton, TTValue& data)
{
    TimeProcessPtr  aTimeProcess;
	TTValuePtr      b;
	
	// unpack baton (a time process)
	b = (TTValuePtr)baton;
	aTimeProcess = TimeProcessPtr((TTObjectBasePtr)(*b)[0]);
    
    // TODO : warn CSP that this time process start date have changed
    // TODO : update all CSP consequences by setting time processes attributes that are affected by the consequence (is here we have to do that ?)
    return kTTErrGeneric;
}

TTErr TT_EXTENSION_EXPORT TimeProcessEndAttributeCallback(TTPtr baton, TTValue& data)
{
    TimeProcessPtr  aTimeProcess;
	TTValuePtr      b;
	
	// unpack baton (a time process)
	b = (TTValuePtr)baton;
	aTimeProcess = TimeProcessPtr((TTObjectBasePtr)(*b)[0]);
    
    // TODO : warn CSP that this time process end date have changed
    // TODO : update all CSP consequences by setting time processes attributes that are affected by the consequence (is here we have to do that ?)
    return kTTErrGeneric;
}

TTErr TT_EXTENSION_EXPORT TimeProcessProgressionAttributeCallback(TTPtr baton, TTValue& data)
{
    TimeProcessPtr  aTimeProcess;
	TTValuePtr      b;
	
	// unpack baton (a time process)
	b = (TTValuePtr)baton;
	aTimeProcess = TimeProcessPtr((TTObjectBasePtr)(*b)[0]);
    
    // TODO : warn Scheduler that this time process progression have changed (?)
    return kTTErrGeneric;
}
