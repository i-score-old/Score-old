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

#ifndef NO_EDITION_SOLVER
#include "ScenarioSolver.h"
#endif

#ifndef NO_EXECUTION_GRAPH
#include "ScenarioGraph.h"
#endif

#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlreader.h>

/**	The Scenario class allows to ...
 
 @see TimePluginLib, TTTimeProcess, TTTimeContainer
 */
class Scenario : public TimeContainerPlugin {
    
	TTCLASS_SETUP(Scenario)
	
    TTAddressItemPtr            mNamespace;                     ///< the namespace workspace of the scenario
    
    TTValue                     mViewZoom;                      ///< the zoom factor (x and y) into the scenario view (useful for gui)
    TTValue                     mViewPosition;                  ///< the position (x and y) of the scenario view (useful for gui)
#ifndef NO_EDITION_SOLVER
    SolverPtr                   mEditionSolver;                 ///< an internal gecode solver to assist scenario edition
    SolverObjectMap             mVariablesMap;                  ///< an internal map to store and retreive SolverVariablePtr using TTTimeEventPtr
    SolverObjectMap             mConstraintsMap;                ///< an internal map to store and retreive SolverConstraintPtr using TTTimeProcessPtr
    SolverObjectMap             mRelationsMap;                  ///< an internal map to store and retreive SolverRelationPtr using TTTimeProcessPtr
#endif
#ifndef NO_EXECUTION_GRAPH
    GraphPtr                    mExecutionGraph;                ///< an internal petri net to execute the scenario according time event relations

    GraphObjectMap              mTransitionsMap;                ///< an internal map to store and retreive TransitionPtr using TTTimeEventPtr
	GraphObjectMap              mArcsMap;                       ///< an internal map to store and retreive Arc* using TTTimeProcessPtr
    GraphObjectMap              mMergedTransitionsMap;          ///< an internal map to store and retreive TransitionPtr using another TransitionPtr
   
    ExtendedInt                 plusInfinity;
	ExtendedInt                 minusInfinity;
	ExtendedInt                 integer0;
#endif     
    TTObject                    mCurrentTimeEvent;              ///< an internal pointer to remember the current time event being read
    TTObject                    mCurrentTimeProcess;            ///< an internal pointer to remember the current time process being read
    TTObject                    mCurrentTimeCondition;          ///< an internal pointer to remember the current time condition being read
	TTObject           	 		mCurrentScenario;               ///< an internal pointer to remember the current scenario being read
    
    TTBoolean                   mLoading;                       ///< a flag true when the scenario is loading (mainly used to mute the edition solver)
    TTBoolean                   mAttributeLoaded;               ///< a flag true when the scenario is loading (mainly used to mute the edition solver)
    
	
    /** Get parameters names needed by this time process
     @param	value           the returned parameter names
     @return                kTTErrNone */
	TTErr   getParameterNames(TTValue& value);
    
    /** Set the view zoom factor
     @param	value           zoomX and zoomY
     @return                kTTErrNone */
    TTErr   setViewZoom(const TTValue& value);
    
    /** Set the view position
     @param	value           zoomX and zoomY
     @return                kTTErrNone */
    TTErr   setViewPosition(const TTValue& value);
    
    /** Specific compilation method used to pre-processed data in order to accelarate Process method.
     the compiled attribute allows to know if the process needs to be compiled or not.
     @return                an error code returned by the compile method */
    TTErr   Compile();
    
    /** Specific process method on start
     @return                an error code returned by the process end method */
    TTErr   ProcessStart();
    
    /** Specific process method on end
     @return                an error code returned by the process end method */
    TTErr   ProcessEnd();
    
    /** Specific process method
     @param	inputValue      position of the scheduler
     @param	outputValue     return an error of the processing
     @return                an error code returned by the process method */
    TTErr   Process(const TTValue& inputValue, TTValue& outputValue);
    
    /** Specific process method for pause/resume
     @param	inputValue      boolean paused state of the scheduler
     @param	outputValue     return an error of the processing
     @return                an error code returned by the process paused method */
    TTErr   ProcessPaused(const TTValue& inputValue, TTValue& outputValue);
    
    /** Specific go to method to set the process at a date
     @param	inputValue      a date where to go relative to the duration of the time process, an optional boolean to temporary mute the process 
     @param	outputValue     nothing
     @return                an error code if the operation fails */
    TTErr   Goto(const TTValue& inputValue, TTValue& outputValue);
    
    /**  needed to be handled by a TTXmlHandler
     @param	inputValue      ..
     @param	outputValue     ..
     @return                .. */
	TTErr	WriteAsXml(const TTValue& inputValue, TTValue& outputValue);
	TTErr	ReadFromXml(const TTValue& inputValue, TTValue& outputValue);
    

    
    /** Create a time event
     @param inputvalue      a date
     @param outputvalue     a new time event
     @return                an error code if the creation fails */
    TTErr   TimeEventCreate(const TTValue& inputValue, TTValue& outputValue);
    
    /** Release a time event
     @param inputvalue      a time event object to release
     @param outputvalue     nothing            
     @return                an error code if the destruction fails */
    TTErr   TimeEventRelease(const TTValue& inputValue, TTValue& outputValue);
    
    /** Move a time event
     @param inputvalue      a time event object, new date
     @param outputvalue     nothing            
     @return                an error code if the movement fails */
    TTErr   TimeEventMove(const TTValue& inputValue, TTValue& outputValue);
    
    /** Link a time event to a condition
     @param inputvalue      a time event object, a time condition object
     @param outputvalue     nothing            
     @return                an error code if the setting fails */
    TTErr   TimeEventCondition(const TTValue& inputValue, TTValue& outputValue);
    
    /** Trigger a time event to make it happens
     @param inputvalue      a time event object
     @param outputvalue     nothing            
     @return                an error code if the triggering fails */
    TTErr   TimeEventTrigger(const TTValue& inputValue, TTValue& outputValue);
    
    /** Dispose a time event
     @param inputValue      a time event object
     @param outputvalue     nothing            
     @return                an error code if the disposing fails */
    TTErr   TimeEventDispose(const TTValue& inputValue, TTValue& outputValue);

    /** Replace a time event by another one (copying date and active attribute)
     @param inputvalue      a former time event object, a new time event object
     @param outputvalue     nothing            
     @return                an error code if the replacement fails */
    TTErr   TimeEventReplace(const TTValue& inputValue, TTValue& outputValue);
    
    
    
    /** Create a time process
     @param inputvalue      a time process type, a start event, a end event
     @param outputvalue     a new time process
     @return                an error code if the creation fails */
    TTErr   TimeProcessCreate(const TTValue& inputValue, TTValue& outputValue);
    
    /** Release a time process
     @param inputvalue      a time process object to release
     @param outputvalue     its the start and the end event
     @return                an error code if the destruction fails */
    TTErr   TimeProcessRelease(const TTValue& inputValue, TTValue& outputValue);
    
    /** Move a time process
     @param inputvalue      a time process object, new start date, new end date
     @param outputvalue     nothing            
     @return                an error code if the movement fails */
    TTErr   TimeProcessMove(const TTValue& inputValue, TTValue& outputValue);
    
    /** Limit a time process duration
     @param inputvalue      a time process object, new duration min, new duration max
     @param outputvalue     nothing            
     @return                an error code if the limitation fails */
    TTErr   TimeProcessLimit(const TTValue& inputValue, TTValue& outputValue);
    
    
    
    /** Create a time condition
     @param inputvalue      nothing
     @param outputvalue     a new time condition
     @return                an error code if the creation fails */
    TTErr   TimeConditionCreate(const TTValue& inputValue, TTValue& outputValue);
    
    /** Release a time process
     @param inputvalue      a time condition object to release
     @param outputvalue     nothing            
     @return                an error code if the destruction fails */
    TTErr   TimeConditionRelease(const TTValue& inputValue, TTValue& outputValue);
    
    
    /** an internal method used to create all time process attribute observers */
    void    makeTimeProcessCacheElement(TTObject& aTimeProcess, TTValue& newCacheElement);
    
    /** an internal method used to delete all time process attribute observers */
    void    deleteTimeProcessCacheElement(const TTValue& oldCacheElement);
    
    /** an internal method used to create all time event attribute observers */
    void    makeTimeEventCacheElement(TTObject& aTimeEvent, TTValue& newCacheElement);
    
    /** an internal method used to delete all time event attribute observers */
    void    deleteTimeEventCacheElement(const TTValue& oldCacheElement);
    
    /** an internal method used to create all time condition attribute observers */
    void    makeTimeConditionCacheElement(TTObject& aTimeCondition, TTValue& newCacheElement);
    
    /** an internal method used to delete all time condition attribute observers */
    void    deleteTimeConditionCacheElement(const TTValue& oldCacheElement);
    
    
 #ifndef NO_EXECUTION_GRAPH
    /** internal methods used to compile the execution graph */
    void    clearGraph();
    void    compileGraph(TTUInt32 timeOffset);
    void    compileTimeProcess(TTObject& aTimeProcess, TransitionPtr *previousTransition, TransitionPtr endTransition, TTUInt32 timeOffset);
    void    compileInterval(TTObject& aTimeProcess);
    void    compileTimeEvent(TTObject& aTimeEvent, TTUInt32 time, TransitionPtr previousTransition, TransitionPtr currentTransition, Place* currentPlace);
    void    compileInteractiveEvent(TTObject& aTimeEvent, TTUInt32 timeOffset);
    
    friend void TT_EXTENSION_EXPORT ScenarioGraphTimeEventCallBack(TTPtr arg, TTBoolean active);
    friend void TT_EXTENSION_EXPORT ScenarioGraphIsEventReadyCallBack(TTPtr arg, TTBoolean isReady);
#endif
};

typedef Scenario* ScenarioPtr;

#ifndef NO_EXECUTION_GRAPH
/** The callback method used by the execution graph when ...
 @param	arg                         a time event instance
 @param	arg                         is time event becomes active or passive ? */
void TT_EXTENSION_EXPORT ScenarioGraphTimeEventCallBack(TTPtr arg, TTBoolean active);

/** The callback method used by the execution graph when ...
 @param	arg                         a time event instance
 @param	isReady                     is the time event ready to be triggered ? */
void TT_EXTENSION_EXPORT ScenarioGraphIsEventReadyCallBack(TTPtr arg, TTBoolean isReady);
#endif

#endif // __SCENARIO_H__
