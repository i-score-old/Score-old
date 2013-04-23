/*
 * Scenario time process
 * Copyright © 2013, Théo de la Hogue, Clément Bossut
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

#ifndef __SCENARIO_H__
#define __SCENARIO_H__

#include "TimeProcess.h"
#include "ScenarioSolver.h"
#include "PetriNet.hpp"

class Scenario : public TimeProcess
{
	TTCLASS_SETUP(Scenario)
	
private :
    
    TTList                      mTimeProcessList;               ///< ATTRIBUTE : all registered time processes and their observers
    TTList                      mTimeEventList;                 ///< ATTRIBUTE : all registered time events and their observers
    
    TTAddressItemPtr            mNamespace;                     ///< ATTRIBUTE : the namespace workspace of the scenario
    
    TTObjectBasePtr             mFirstEvent;                    ///< the first event of the scenario (which is not the start event)
    TTObjectBasePtr             mLastEvent;                     ///< the last event of the scenario (which is not the end event)
    
    SolverPtr                   mEditionSolver;                 ///< an internal gecode solver to assist scenario edition
    SolverObjectMap             mVariablesMap;                  ///< a map to store and retreive SolverVariablePtr using TimeEventPtr
    SolverObjectMap             mConstraintsMap;                ///< a map to store and retreive SolverConstraintPtr using TimeProcessPtr
    SolverObjectMap             mRelationsMap;                  ///< a map to store and retreive SolverRelationPtr using TimeProcessPtr
    
    PetriNet*                   mExecutionGraph;                ///< an internal petri net to execute the scenario according time event relations
	
    /** Get parameters names needed by this time process
     @param	value           the returned parameter names
     @return                kTTErrNone */
	TTErr   getParameterNames(TTValue& value);
    
    /** Specific process method on start
     @return                an error code returned by the process end method */
    TTErr   ProcessStart();
    
    /** Specific process method on end
     @return                an error code returned by the process end method */
    TTErr   ProcessEnd();
    
    /** Specific process method
     @param	inputValue      progression of the scheduler
     @param	outputValue     return an error of the processing
     @return                an error code returned by the process method */
    TTErr   Process(const TTValue& inputValue, TTValue& outputValue);
    
    /** Compile the scenario to prepare the petri net before execution
     @return                an error code returned by the compile method */
    //TTErr   Compile();
    
    /**  needed to be handled by a TTXmlHandler
     @param	inputValue      ..
     @param	outputValue     ..
     @return                .. */
	TTErr	WriteAsXml(const TTValue& inputValue, TTValue& outputValue);
	TTErr	ReadFromXml(const TTValue& inputValue, TTValue& outputValue);
	
	/**  needed to be handled by a TTTextHandler
     @param	inputValue      ..
     @param	outputValue     ..
     @return                .. */
	TTErr	WriteAsText(const TTValue& inputValue, TTValue& outputValue);
	TTErr	ReadFromText(const TTValue& inputValue, TTValue& outputValue);
    
    /** Register a time process for scenario management
     @inputvalue            a time process object
     @outputvalue           kTTValNONE
     @return                an error code if the registration fails */
    TTErr   TimeProcessAdd(const TTValue& inputValue, TTValue& outputValue);
    
    /** Unregister a time process for scenario management
     @inputValue            a time process object
     @outputvalue           kTTValNONE
     @return                an error code if the unregistration fails */
    TTErr   TimeProcessRemove(const TTValue& inputValue, TTValue& outputValue);
    
    /** Move a time process into the scenario
     @inputValue            a time process object, new start date, new end date
     @outputvalue           kTTValNONE
     @return                an error code if the movement fails */
    TTErr   TimeProcessMove(const TTValue& inputValue, TTValue& outputValue);
    
    /** imit a time process duration into the scenario
     @inputValue            a time process object, new duration min, new duration max
     @outputvalue           kTTValNONE
     @return                an error code if the limitation fails */
    TTErr   TimeProcessLimit(const TTValue& inputValue, TTValue& outputValue);
    
    /** Register a time event for scenario management
     @inputvalue            a time event object
     @outputvalue           kTTValNONE
     @return                an error code if the registration fails */
    TTErr   TimeEventAdd(const TTValue& inputValue, TTValue& outputValue);
    
    /** Unregister a time event for scenario management
     @inputValue            a time event object
     @outputvalue           kTTValNONE
     @return                an error code if the unregistration fails */
    TTErr   TimeEventRemove(const TTValue& inputValue, TTValue& outputValue);
    
    /** Move a time event into the scenario
     @inputValue            a time event object, new date
     @outputvalue           kTTValNONE
     @return                an error code if the movement fails */
    TTErr   TimeEventMove(const TTValue& inputValue, TTValue& outputValue);
    
    /** Change a time process active state into the scenario
     note : this method doesn't update the time process internal acive state
     but it can change attribute of other time processes or time events connected to this one.
     
     @inputvalue            a time process object, a new active state
     @outputvalue           kTTValNONE
     @return                an error code if the active state change fails */
    TTErr   TimeProcessActiveChange(const TTValue& inputValue, TTValue& outputValue);
    
    /** an internal method used to create all time process attribute observers */
    void makeTimeProcessCacheElement(TimeProcessPtr aTimeProcess, TTValue& newCacheElement);
    
    /** an internal method used to delete all time process attribute observers */
    void deleteTimeProcessCacheElement(const TTValue& oldCacheElement);
    
    /** an internal method used to create all time event attribute observers */
    void makeTimeEventCacheElement(TimeEventPtr aTimeEvent, TTValue& newCacheElement);
    
    /** an internal method used to delete all time event attribute observers */
    void deleteTimeEventCacheElement(const TTValue& oldCacheElement);
    
    friend TTErr TT_EXTENSION_EXPORT ScenarioSchedulerRunningAttributeCallback(TTPtr baton, TTValue& data);
    friend TTErr TT_EXTENSION_EXPORT ScenarioSchedulerProgressionAttributeCallback(TTPtr baton, TTValue& data);
};

typedef Scenario* ScenarioPtr;

/* a TTFunctionMatch to find a time process and all his observers in the scenario depending on the time process object him self */
void TT_EXTENSION_EXPORT ScenarioFindTimeProcess(const TTValue& aValue, TTPtr timeProcessPtrToMatch, TTBoolean& found);

/* a TTFunctionMatch to find a time process and all his observers in the scenario depending on a time event it binds.
   This function ignores Interval process. */
void TT_EXTENSION_EXPORT ScenarioFindTimeProcessWithTimeEvent(const TTValue& aValue, TTPtr timeEventPtrToMatch, TTBoolean& found);

/* a TTFunctionMatch to find a time event and all his observers in the scenario depending on the time event object him self */
void TT_EXTENSION_EXPORT ScenarioFindTimeEvent(const TTValue& aValue, TTPtr timeEventPtrToMatch, TTBoolean& found);

/** The callback method used to observe time processes scheduler running attribute change
 @param	baton						a time process instance
 @param	data						a new running value
 @return							an error code */
TTErr TT_EXTENSION_EXPORT ScenarioSchedulerRunningAttributeCallback(TTPtr baton, TTValue& data);

/** The callback method used to observe time processes scheduler progression attribute change
 @param	baton						a time process instance
 @param	data						a new progression value
 @return							an error code */
TTErr TT_EXTENSION_EXPORT ScenarioSchedulerProgressionAttributeCallback(TTPtr baton, TTValue& data);

#endif // __SCENARIO_H__
