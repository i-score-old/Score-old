/** @file
 *
 * @ingroup scoreExtension
 *
 * @brief Scenario class is a time container class to manage time events and time processes in the time
 *
 * @details The Scenario class allows to ... @n@n
 *
 * @see TimePluginLib, TTTimeContainer
 *
 * @authors Théo de la Hogue & Clément Bossut
 *
 * @copyright Copyright © 2013, Théo de la Hogue & Clément Bossut @n
 * This code is licensed under the terms of the "CeCILL-C" @n
 * http://www.cecill.info
 */

#ifndef __SCENARIO_H__
#define __SCENARIO_H__

#include "TimePluginLib.h"
#include "ScenarioSolver.h"
#include "ScenarioGraph.h"

/**	The Scenario class allows to ...
 
 @see TimePluginLib, TTTimeProcess, TTTimeContainer
 */
class Scenario : public TimeContainer {
    
	TTCLASS_SETUP(Scenario)
	
    TTAddressItemPtr            mNamespace;                     ///< the namespace workspace of the scenario
    
    SolverPtr                   mEditionSolver;                 ///< an internal gecode solver to assist scenario edition
    SolverObjectMap             mVariablesMap;                  ///< an internal map to store and retreive SolverVariablePtr using TTTimeEventPtr
    SolverObjectMap             mConstraintsMap;                ///< an internal map to store and retreive SolverConstraintPtr using TTTimeProcessPtr
    SolverObjectMap             mRelationsMap;                  ///< an internal map to store and retreive SolverRelationPtr using TTTimeProcessPtr
    
    GraphPtr                    mExecutionGraph;                ///< an internal petri net to execute the scenario according time event relations
    
    GraphObjectMap              mTransitionsMap;                ///< an internal map to store and retreive TransitionPtr using TTTimeEventPtr
	GraphObjectMap              mArcsMap;                       ///< an internal map to store and retreive Arc* using TTTimeProcessPtr
    GraphObjectMap              mMergedTransitionsMap;          ///< an internal map to store and retreive TransitionPtr using another TransitionPtr
    
    ExtendedInt                 plusInfinity;
	ExtendedInt                 minusInfinity;
	ExtendedInt                 integer0;
    
    TTTimeEventPtr              mCurrentTimeEvent;               ///< an internal pointer to remember the current time event being read
    TTTimeProcessPtr            mCurrentTimeProcess;             ///< an internal pointer to remember the current time process being read
    TTTimeConditionPtr          mCurrentTimeCondition;           ///< an internal pointer to remember the current time condition being read
    
	
    /** Get parameters names needed by this time process
     @param	value           the returned parameter names
     @return                kTTErrNone */
	TTErr   getParameterNames(TTValue& value);
    
    
    
    /** Compile the scenario to prepare the petri net before execution
     @return                an error code returned by the compile method */
    TTErr   Compile();
    
    
    
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
    
    /**  needed to be handled by a TTXmlHandler
     @param	inputValue      ..
     @param	outputValue     ..
     @return                .. */
	TTErr	WriteAsXml(const TTValue& inputValue, TTValue& outputValue);
	TTErr	ReadFromXml(const TTValue& inputValue, TTValue& outputValue);
    

    
    /** Create a time event
     @inputvalue            a date
     @outputvalue           a new time event
     @return                an error code if the creation fails */
    TTErr   TimeEventCreate(const TTValue& inputValue, TTValue& outputValue);
    
    /** Release a time event
     @inputValue            a time event object to release
     @outputvalue           kTTValNONE
     @return                an error code if the destruction fails */
    TTErr   TimeEventRelease(const TTValue& inputValue, TTValue& outputValue);
    
    /** Move a time event
     @inputValue            a time event object, new date
     @outputvalue           kTTValNONE
     @return                an error code if the movement fails */
    TTErr   TimeEventMove(const TTValue& inputValue, TTValue& outputValue);
    
    /** Link a time event to a condition
     @inputValue            a time event object, a time condition object
     @outputvalue           kTTValNONE
     @return                an error code if the setting fails */
    TTErr   TimeEventCondition(const TTValue& inputValue, TTValue& outputValue);
    
    /** Trigger a time event to make it happens
     @inputValue            a time event object
     @outputvalue           kTTValNONE
     @return                an error code if the triggering fails */
    TTErr   TimeEventTrigger(const TTValue& inputValue, TTValue& outputValue);
    
    /** Replace a time event by another one (copying date and active attribute)
     @inputValue            a former time event object, a new time event object
     @outputvalue           kTTValNONE
     @return                an error code if the replacement fails */
    TTErr   TimeEventReplace(const TTValue& inputValue, TTValue& outputValue);
    
    
    
    /** Create a time process
     @inputvalue            a time process type, a start event, a end event
     @outputvalue           a new time process
     @return                an error code if the creation fails */
    TTErr   TimeProcessCreate(const TTValue& inputValue, TTValue& outputValue);
    
    /** Release a time process
     @inputValue            a time process object to release
     @outputvalue           its the start and the end event
     @return                an error code if the destruction fails */
    TTErr   TimeProcessRelease(const TTValue& inputValue, TTValue& outputValue);
    
    /** Move a time process
     @inputValue            a time process object, new start date, new end date
     @outputvalue           kTTValNONE
     @return                an error code if the movement fails */
    TTErr   TimeProcessMove(const TTValue& inputValue, TTValue& outputValue);
    
    /** Limit a time process duration
     @inputValue            a time process object, new duration min, new duration max
     @outputvalue           kTTValNONE
     @return                an error code if the limitation fails */
    TTErr   TimeProcessLimit(const TTValue& inputValue, TTValue& outputValue);
    
    
    
    /** Create a time condition
     @inputvalue            optionnal expression symbols : < "address operator value", "address operator value", ... >
     @outputvalue           a new time condition
     @return                an error code if the creation fails */
    TTErr   TimeConditionCreate(const TTValue& inputValue, TTValue& outputValue);
    
    /** Release a time process
     @inputValue            a time condition object to release
     @outputvalue           kTTValNONE
     @return                an error code if the destruction fails */
    TTErr   TimeConditionRelease(const TTValue& inputValue, TTValue& outputValue);
    
    
    /** an internal method used to create all time process attribute observers */
    void    makeTimeProcessCacheElement(TTTimeProcessPtr aTimeProcess, TTValue& newCacheElement);
    
    /** an internal method used to delete all time process attribute observers */
    void    deleteTimeProcessCacheElement(const TTValue& oldCacheElement);
    
    /** an internal method used to create all time event attribute observers */
    void    makeTimeEventCacheElement(TTTimeEventPtr aTimeEvent, TTValue& newCacheElement);
    
    /** an internal method used to delete all time event attribute observers */
    void    deleteTimeEventCacheElement(const TTValue& oldCacheElement);
    
    /** an internal method used to create all time condition attribute observers */
    void    makeTimeConditionCacheElement(TTTimeConditionPtr aTimeCondition, TTValue& newCacheElement);
    
    /** an internal method used to delete all time condition attribute observers */
    void    deleteTimeConditionCacheElement(const TTValue& oldCacheElement);
    
    
    
    /** internal methods used to compile the execution graph */
    void    clearGraph();
    void    compileGraph(TTUInt32 timeOffset);
    void    compileTimeProcess(TTTimeProcessPtr aTimeProcess, TransitionPtr *previousTransition, TransitionPtr endTransition, TTUInt32 timeOffset);
    void    compileInterval(TTTimeProcessPtr aTimeProcess);
    void    compileTimeEvent(TTTimeEventPtr aTimeEvent, TTUInt32 time, TransitionPtr previousTransition, TransitionPtr currentTransition, Place* currentPlace);
    void    compileInteractiveEvent(TTTimeEventPtr aTimeEvent, TTUInt32 timeOffset);
    //void    cleanGraph(TransitionPtr endTransition);
    
    friend void TT_EXTENSION_EXPORT ScenarioGraphTimeEventCallBack(TTPtr arg);
    friend void TT_EXTENSION_EXPORT ScenarioGraphIsEventReadyCallBack(TTPtr arg, TTBoolean isReady);
};

typedef Scenario* ScenarioPtr;

/** The callback method used by the execution graph when ...
 @param	arg                         a time event instance */
void TT_EXTENSION_EXPORT ScenarioGraphTimeEventCallBack(TTPtr arg);

/** The callback method used by the execution graph when ...
 @param	arg                         a time event instance
 @param	isReady                     is the time event ready to be triggered ? */
void TT_EXTENSION_EXPORT ScenarioGraphIsEventReadyCallBack(TTPtr arg, TTBoolean isReady);

#endif // __SCENARIO_H__
