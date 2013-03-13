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

#ifndef __ECOMACHINE_H__
#define __ECOMACHINE_H__

#include "Scheduler.h"

class ECOMachine : public Scheduler {
	
	TTCLASS_SETUP(ECOMachine)
    
private:
    
    ///< ATTRIBUTE : any attribute needed for the ECOMachine
    
	/** Get parameters names needed by this scheduler */
	TTErr getParameterNames(TTValue& value);
    
    /** Start the scheduler */
    TTErr Go();
    
    /** Halt the sheduler */
    void Stop();
    
    /** Called every time a new step should be processed */
    void Tick();
    
    friend void TT_EXTENSION_EXPORT ECOMachineClockCallback(ECOMachine* anECOMachineScheduler);
    
};
typedef ECOMachine* ECOMachinePtr;

/** Called by the ECOMachine queue
 @param	baton						..
 @param	data						..
 @return							an error code */
void TT_EXTENSION_EXPORT ECOMachineClockCallback(ECOMachine* anECOMachineScheduler);

#endif // __ECOMACHINE_H__
