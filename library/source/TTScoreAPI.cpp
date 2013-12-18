/** @file
 *
 * @ingroup scoreLibrary
 *
 * @brief the Score Application Programming Interface
 *
 * @details The Score API allows to simply includes all the files needed to use Score inside another application @n@n
 *
 * @see TTScore
 *
 * @authors Théo de la Hogue & Clément Bossut
 *
 * @copyright Copyright © 2013, Théo de la Hogue & Clément Bossut @n
 * This code is licensed under the terms of the "CeCILL-C" @n
 * http://www.cecill.info
 */

#include "TTScoreAPI.h"

/* Main Score functions */

static bool TTScoreInitialized = false;

void TTScoreInitialize()
{
    // Initialized Foundation framework
	TTFoundationInit();
    
	if (!TTScoreInitialized) {
		
		TTScoreInitialized = true;
		
		// register classes -- both internal and external
        TTTimeCondition::registerClass();
        TTTimeContainer::registerClass();
        TTTimeEvent::registerClass();
        TTTimeProcess::registerClass();
        
		TTScoreTest::registerClass();
        
#ifdef TT_DEBUG
		TTLogMessage("Score -- Version %s -- Debugging Enabled\n", TTSCORE_VERSION_STRING);
#else
		TTLogMessage("Score -- Version %s\n", TTSCORE_VERSION_STRING);
#endif
        
	}
}

TTErr TTScoreTimeEventCreate(TTTimeEventPtr *timeEvent, TTUInt32 date, TTTimeContainerPtr timeContainer)
{
    *timeEvent = NULL;
    
    if (timeContainer) {
        
        TTErr   err;
        TTValue out;
        
        err = timeContainer->sendMessage(TTSymbol("TimeEventCreate"), date, out);
        
        if (!err)
            *timeEvent = TTTimeEventPtr(TTObjectBasePtr(out[0]));
        
        return err;
    }
    else
        return TTObjectBaseInstantiate(kTTSym_TimeEvent, TTObjectBaseHandle(timeEvent), date);
}

TTErr TTScoreTimeEventRelease(TTTimeEventPtr *timeEvent, TTTimeContainerPtr timeContainer)
{
    if (timeContainer) {
        
        TTValue none;
        
        return timeContainer->sendMessage(TTSymbol("TimeEventRelease"), TTObjectBasePtr(*timeEvent), none);
    }
    else
        return TTObjectBaseRelease(TTObjectBaseHandle(timeEvent));
}

TTErr TTScoreTimeEventStatusCallbackCreate(TTTimeEventPtr timeEvent, TTObjectBasePtr *statusCallback, TTScoreTimeEventStatusCallbackPtr statusCallbackFunction)
{
    TTValue         v, none;
    TTValuePtr      statusAttributeBaton;
    TTAttributePtr  anAttribute;
    TTErr           err;
    
    statusCallback = NULL;
    
    // Create a TTCallback to observe time event status attribute (using internal_TTScoreTimeEventStatusCallback)
    TTObjectBaseInstantiate(TTSymbol("callback"), statusCallback, none);
    
    statusAttributeBaton = new TTValue(TTPtr(timeEvent)); // statusAttributeBaton will be deleted during the callback destruction
    statusAttributeBaton->append(TTPtr(statusCallbackFunction));
    
    (*statusCallback)->setAttributeValue(kTTSym_baton, TTPtr(statusAttributeBaton));
    (*statusCallback)->setAttributeValue(kTTSym_function, TTPtr(&internal_TTScoreTimeEventStatusCallback));
    
    // register for status attribute observation
    err = timeEvent->findAttribute(kTTSym_status, &anAttribute);
    
    if (!err)
        anAttribute->registerObserverForNotifications(**statusCallback);
    
    return err;
}

TTErr TTScoreTimeEventStatusCallbackRelease(TTTimeEventPtr timeEvent, TTObjectBasePtr *statusCallback)
{
    
}

TTErr TTScoreTimeProcessCreate(TTTimeProcessPtr *timeProcess, const std::string timeProcessClass, TTTimeEventPtr startEvent, TTTimeEventPtr endEvent, TTTimeContainerPtr timeContainer)
{
    TTValue args;
    
    *timeProcess = NULL;
    
    if (timeContainer) {
        
        TTValue out;
        TTErr   err;
        
        args = TTSymbol(timeProcessClass);
        args.append(TTObjectBasePtr(startEvent));
        args.append(TTObjectBasePtr(endEvent));
        
        err = timeContainer->sendMessage(TTSymbol("TimeProcessCreate"), args, out);
        
        if (!err)
           *timeProcess = TTTimeProcessPtr(TTObjectBasePtr(out[0]));
        
        return err;
    }
    else {
        
        args = TTObjectBasePtr(startEvent);
        args.append(TTObjectBasePtr(endEvent));
        
        return TTObjectBaseInstantiate(timeProcessClass.c_str(), TTObjectBaseHandle(timeProcess), args);
    }
}

TTErr TTScoreTimeProcessRelease(TTTimeProcessPtr *timeProcess, TTTimeContainerPtr timeContainer, TTTimeEventPtr *startEvent, TTTimeEventPtr *endEvent)
{
    if (timeContainer) {
        
        TTValue out;
        TTErr   err;
        
        err = timeContainer->sendMessage(TTSymbol("TimeProcessRelease"), TTObjectBasePtr(*timeProcess), out);
        
        if (!err && *startEvent && *endEvent) {
            
            *startEvent = TTTimeEventPtr(TTObjectBasePtr(out[0]));
            *endEvent =  TTTimeEventPtr(TTObjectBasePtr(out[1]));
        }
        
        return err;
    }
    else
        return TTObjectBaseRelease(TTObjectBaseHandle(timeProcess));
}

TTErr TTScoreTimeProcessGetStartEvent(TTTimeProcessPtr timeProcess, TTTimeEventPtr *startEvent)
{
    TTValue v;
    
    TTErr err = timeProcess->getAttributeValue(TTSymbol("startEvent"), v);
    
    if (!err)
        *startEvent = TTTimeEventPtr(TTObjectBasePtr(v[0]));
    
    return err;
}

TTErr TTScoreTimeProcessGetEndEvent(TTTimeProcessPtr timeProcess, TTTimeEventPtr *endEvent)
{
    TTValue v;
    
    TTErr err = timeProcess->getAttributeValue(TTSymbol("endEvent"), v);
    
    if (!err)
        *endEvent = TTTimeEventPtr(TTObjectBasePtr(v[0]));
    
    return err;
}

TTErr TTScoreTimeProcessSetName(TTTimeProcessPtr timeProcess, const std::string name)
{
    return timeProcess->setAttributeValue(kTTSym_name, name.c_str());
}

TTErr TTScoreTimeProcessSetRigid(TTTimeProcessPtr timeProcess, const TTBoolean rigid)
{
    return timeProcess->setAttributeValue(kTTSym_rigid, rigid);
}

TTErr TTScoreTimeProcessMove(TTTimeProcessPtr timeProcess, const TTUInt32 startDate, const TTUInt32 endDate)
{
    TTValue v, none;
    
    v = startDate;
    v.append(endDate);
    
    return timeProcess->sendMessage(kTTSym_Move, v, none);
}

TTErr TTScoreTimeProcessLimit(TTTimeProcessPtr timeProcess, const TTUInt32 durationMin, const TTUInt32 durationMax)
{
    TTValue v, none;
    
    v = durationMin;
    v.append(durationMax);
    
    return timeProcess->sendMessage(kTTSym_Limit, v, none);
}

TTErr TTScoreTimeProcessStartCallbackCreate(TTTimeProcessPtr timeProcess, TTObjectBasePtr *startCallback, TTScoreTimeProcessStartCallbackPtr startCallbackFunction)
{
    TTValue        v, none;
    TTValuePtr     startMessageBaton;
    TTMessagePtr   aMessage;
    TTErr          err;
    
    *startCallback = NULL;
    
    // create a TTCallback to observe when time process starts (using internal_TTScoreTimeProcessStartCallback)
    TTObjectBaseInstantiate(TTSymbol("callback"), startCallback, none);
    
    // store the function to call and the time process to pass
    startMessageBaton = new TTValue(TTPtr(timeProcess));    // startMessageBaton will be deleted during the callback destruction
    startMessageBaton->append(TTPtr(startCallbackFunction));
    
    (*startCallback)->setAttributeValue(kTTSym_baton, TTPtr(startMessageBaton));
    (*startCallback)->setAttributeValue(kTTSym_function, TTPtr(&internal_TTScoreTimeProcessStartCallback));
    
    // register for ProcessStart message observation
    err = timeProcess->findMessage(kTTSym_ProcessStart, &aMessage);
    
    if (!err)
        aMessage->registerObserverForNotifications(**startCallback);
    
    return err;
}

TTErr TTScoreTimeProcessStartCallbackRelease(TTTimeProcessPtr timeProcess, TTObjectBasePtr *startCallback)
{
    TTValue        v;
    TTMessagePtr   aMessage;
    TTErr          err;
    
    // unregister for ProcessStart message observation
    err = timeProcess->findMessage(kTTSym_ProcessStart, &aMessage);
    
    if (!err) {
        
        err = aMessage->unregisterObserverForNotifications(**startCallback);
        
        if (!err) {
            delete (TTValuePtr)TTCallbackPtr(*startCallback)->getBaton();
            TTObjectBaseRelease(startCallback);
        }
    }
    
    return err;
}

TTErr TTScoreTimeProcessEndCallbackCreate(TTTimeProcessPtr timeProcess, TTObjectBasePtr *endCallback, TTScoreTimeProcessEndCallbackPtr endCallbackFunction)
{
    TTValue        v, none;
    TTValuePtr     endMessageBaton;
    TTMessagePtr   aMessage;
    TTErr          err;
    
    *endCallback = NULL;
    
    // create a TTCallback to observe when time process starts (using internal_TTScoreTimeProcessEndCallback)
    TTObjectBaseInstantiate(TTSymbol("callback"), endCallback, none);
    
    // store the function to call and the time process to pass
    endMessageBaton = new TTValue(TTPtr(timeProcess));    // endMessageBaton will be deleted during the callback destruction
    endMessageBaton->append(TTPtr(endCallbackFunction));
    
    (*endCallback)->setAttributeValue(kTTSym_baton, TTPtr(endMessageBaton));
    (*endCallback)->setAttributeValue(kTTSym_function, TTPtr(&internal_TTScoreTimeProcessEndCallback));
    
    // register for ProcessEnd message observation
    err = timeProcess->findMessage(kTTSym_ProcessEnd, &aMessage);
    
    if (!err)
        aMessage->registerObserverForNotifications(**endCallback);
    
    return err;
}

TTErr TTScoreTimeProcessEndCallbackRelease(TTTimeProcessPtr timeProcess, TTObjectBasePtr *endCallback)
{
    TTValue        v;
    TTMessagePtr   aMessage;
    TTErr          err;
    
    // unregister for ProcessEnd message observation
    err = timeProcess->findMessage(kTTSym_ProcessEnd, &aMessage);
    
    if (!err) {
        
        err = aMessage->unregisterObserverForNotifications(**endCallback);
        
        if (!err) {
            delete (TTValuePtr)TTCallbackPtr(*endCallback)->getBaton();
            TTObjectBaseRelease(endCallback);
        }
    }
    
    return err;
}

#if 0
#pragma mark -
#pragma mark some internal functions
#endif

void internal_TTScoreTimeEventStatusCallback(TTPtr baton, TTValue& data)
{
    TTValuePtr          b;
    TTTimeEventPtr      timeEvent;
    TTScoreTimeEventStatusCallbackPtr statusCallbackFunction;
	
	// unpack baton (time process, function)
	b = (TTValuePtr)baton;
	timeEvent = TTTimeEventPtr((TTPtr)(*b)[0]);
    statusCallbackFunction = TTScoreTimeEventStatusCallbackPtr((TTPtr)(*b)[1]);
    
    // call the function
    (*statusCallbackFunction)(timeEvent, TTBoolean(data[0]));
}

void internal_TTScoreTimeProcessStartCallback(TTPtr baton, TTValue& data)
{
    TTValuePtr          b;
    TTTimeProcessPtr    timeProcess;
    TTScoreTimeProcessStartCallbackPtr startCallbackFunction;
	
	// unpack baton (time process, function)
	b = (TTValuePtr)baton;
	timeProcess = TTTimeProcessPtr((TTPtr)(*b)[0]);
    startCallbackFunction = TTScoreTimeProcessStartCallbackPtr((TTPtr)(*b)[1]);
    
    // call the function
    (*startCallbackFunction)(timeProcess);
}

void internal_TTScoreTimeProcessEndCallback(TTPtr baton, TTValue& data)
{
    TTValuePtr          b;
    TTTimeProcessPtr    timeProcess;
    TTScoreTimeProcessEndCallbackPtr endCallbackFunction;
	
	// unpack baton (time process, function)
	b = (TTValuePtr)baton;
	timeProcess = TTTimeProcessPtr((TTPtr)(*b)[0]);
    endCallbackFunction = TTScoreTimeProcessEndCallbackPtr((TTPtr)(*b)[1]);
    
    // call the function
    (*endCallbackFunction)(timeProcess);
}