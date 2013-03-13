/*
 * ECOMachine Scheduler
 * Copyright © 2013, Théo de la Hogue
 *
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

/*!
 * \class ECOMachine
 *
 *  ECOMachine scheduler class
 *
 */

#include "Scheduler.h"
#include "ECOMachine.h"

#define thisTTClass                 ECOMachine
#define thisTTClassName             "ECOMachine"
#define thisTTClassTags             "scheduler, ECOMachine"

#define thisSchedulerVersion		"0.1"
#define thisSchedulerAuthor         "Theo de la Hogue"
#define thisSchedulerStretchable	YES

extern "C" TT_EXTENSION_EXPORT TTErr TTLoadJamomaExtension_ECOMachine(void)
{
	TTFoundationInit();
	ECOMachine::registerClass();
	return kTTErrNone;
}

SCHEDULER_CONSTRUCTOR
{	
	SCHEDULER_INITIALIZE
}

ECOMachine::~ECOMachine()
{
    ;
}

TTErr ECOMachine::getParameterNames(TTValue& value)
{
	value.clear();
	//value.append(TTSymbol("aParameter"));
	
	return kTTErrNone;
}

TTErr ECOMachine::Go()
{
    // do we need to ramp at all ?
    if (mDuration <= 0.) {
        
        mRunning = NO;
        mProgression = 0.;
        mElapsedTime = 0.;
        
        (mCallback)(mBaton, mProgression);
        
        // notify each running attribute observers
        runningAttribute->sendNotification(kTTSym_notify, mRunning);          // we use kTTSym_notify because we know that observers are TTCallback
        
        // notify each progression attribute observers
        progressionAttribute->sendNotification(kTTSym_notify, mProgression);  // we use kTTSym_notify because we know that observers are TTCallback
        
        // notify each elapsed time attribute observers
        elapsedTimeAttribute->sendNotification(kTTSym_notify, mElapsedTime);  // we use kTTSym_notify because we know that observers are TTCallback
    }
    else {
        
        mRunning = YES;
        mProgression = 0.;
        mElapsedTime = 0.;
        
        (mCallback)(mBaton, mProgression);
        
        // notify each running attribute observers
        runningAttribute->sendNotification(kTTSym_notify, mRunning);          // we use kTTSym_notify because we know that observers are TTCallback
        
        // notify each progression attribute observers
        progressionAttribute->sendNotification(kTTSym_notify, mProgression);  // we use kTTSym_notify because we know that observers are TTCallback
        
        // notify each elapsed time attribute observers
        elapsedTimeAttribute->sendNotification(kTTSym_notify, mElapsedTime);  // we use kTTSym_notify because we know that observers are TTCallback
    }
    
    return kTTErrNone;
}

void ECOMachine::Stop()
{
	mRunning = NO;
    
    // notify each running attribute observers
    runningAttribute->sendNotification(kTTSym_notify, mRunning);          // we use kTTSym_notify because we know that observers are TTCallback
}

void ECOMachine::Tick()
{
	if (mRunning) {
        
#ifdef SHEDULER_DEBUG
        cout << "ECOMachine::Tick !" << endl;
#endif
        
        //mProgression = ?;
        //mElapsedTime = ?;
        
        (mCallback)(mBaton, mProgression);
            
        // notify each progression attribute observers
        progressionAttribute->sendNotification(kTTSym_notify, mProgression);  // we use kTTSym_notify because we know that observers are TTCallback
        
        // notify each elapsed time attribute observers
        elapsedTimeAttribute->sendNotification(kTTSym_notify, mElapsedTime);  // we use kTTSym_notify because we know that observers are TTCallback
	}
}

void ECOMachineClockCallback(ECOMachine* aECOMachineScheduler)
{
    aECOMachineScheduler->Tick();
}
