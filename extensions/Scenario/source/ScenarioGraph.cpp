/*
 * Scenario Graph Features
 * Copyright © 2013, Théo de la Hogue, Clément Bossut
 *
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#include "Scenario.h"
#include "ScenarioGraph.h"

void Scenario::compileScenario(TTUInt32 timeOffset)
{
    TimeProcessPtr  aTimeProcess;
    TimeEventPtr    aTimeEvent;
    
    // cf : ECOMachine::compilePetriNet
    
    // clear the former graph
    if (mExecutionGraph != NULL) {
		delete mExecutionGraph;
		mExecutionGraph = NULL;
	}
    
    mExecutionGraph = new PetriNet();
    
    // clear all maps
    mTransitionsMap.clear();
	mArcsMap.clear();
    mMergedTransitionsMap.clear();
    
    // TODO : set the callback used to get expected interactive event state back
    // TODO : the best would be to pass a callback to each transition in order the transition call back with a TimeEventPtr argument
	//mExecutionGraph->addWaitedTriggerPointMessageAction(this, triggerAction);
    
	// start the graph
	Place*          startPlace = mExecutionGraph->createPlace();
	TransitionPtr   startTransition = mExecutionGraph->createTransition();
	Arc*            startArc = mExecutionGraph->createArc(startPlace, startTransition);
    
	Place*          endPlace = mExecutionGraph->createPlace();
	TransitionPtr   endTransition = mExecutionGraph->createTransition();
	Arc*            endArc = mExecutionGraph->createArc(endTransition, endPlace);
    
	mExecutionGraph->setStartPlace(startPlace);
	mExecutionGraph->setEndPlace(endPlace);
    
	startArc->changeRelativeTime(integer0, plusInfinity);
	endArc->changeRelativeTime(integer0, plusInfinity);
    
    // TODO : sort the time process to order them in time (?)
    
    // compile all time processes except the interval processes
    TransitionPtr previousTransition = startTransition;
    for (mTimeProcessList.begin(); mTimeProcessList.end(); mTimeProcessList.next()) {
        
        aTimeProcess = TimeProcessPtr(TTObjectBasePtr(mTimeProcessList.current()[0]));
        
        if (aTimeProcess->getName() != TTSymbol("Interval"))
            compileTimeProcess(aTimeProcess, &previousTransition, endTransition, timeOffset);
	}
    
	// compile intervals processes
    for (mTimeProcessList.begin(); mTimeProcessList.end(); mTimeProcessList.next()) {
        
        aTimeProcess = TimeProcessPtr(TTObjectBasePtr(mTimeProcessList.current()[0]));
        
        if (aTimeProcess->getName() == TTSymbol("Interval"))
            compileInterval(aTimeProcess);
	}
    
	// clean the graph
	//cleanGraph(endTransition);
    
	// comppile interactive events
	for (mTimeEventList.begin(); mTimeEventList.end(); mTimeEventList.next()) {
        
        aTimeEvent = TimeEventPtr(TTObjectBasePtr(mTimeEventList.current()[0]));
        
        if (aTimeEvent->getName() != TTSymbol("StaticEvent"))
            compileInteractiveEvent(aTimeEvent, timeOffset);
	}
}

void Scenario::compileTimeProcess(TimeProcessPtr aTimeProcess, TransitionPtr* previousTransition, TransitionPtr endTransition, TTUInt32 timeOffset)
{
    TransitionPtr     currentTransition;
    TransitionPtr     startTransition = NULL;
    TransitionPtr     lastTransition = NULL;
    Place*          currentPlace;
    Arc*            arcFromPreviousTransitionToCurrentPlace;
    Arc*            arcFromCurrentPlaceToTheEnd;
    
    TimeEventPtr    startEvent, endEvent;
    TTUInt32        startDate, endDate;
    TTValue         v;
    
    // get start event
    aTimeProcess->getAttributeValue(TTSymbol("startEvent"), v);
    startEvent = TimeEventPtr(TTObjectBasePtr(v[0]));
    
    // get end event
    aTimeProcess->getAttributeValue(TTSymbol("endEvent"), v);
    endEvent = TimeEventPtr(TTObjectBasePtr(v[0]));
    
    // get start event date
    aTimeProcess->getAttributeValue(TTSymbol("startDate"), v);
    startDate = TTUInt32(v[0]);
    
    // get end event date
    aTimeProcess->getAttributeValue(TTSymbol("endDate"), v);
    endDate = TTUInt32(v[0]);
    
    // if the date to start is in the middle of a time process
    if (startDate < timeOffset && endDate > timeOffset)
        
        // TODO : prepare the scheduler to start in the middle of his process
        ;//aTimeProcess->setTimeOffsetInMs(timeOffset - startDate);
    
    // compile start event
    currentTransition = mExecutionGraph->createTransition();
    currentPlace = mExecutionGraph->createPlace();
    startTransition = currentTransition;
    
    compileTimeEvent(startEvent, startDate, *previousTransition, currentTransition, currentPlace);
    
    mTransitionsMap[startEvent] = currentTransition;
    *previousTransition = currentTransition;
    
    // compile intermediate events
    /*
    currentTransition = mExecutionGraph->createTransition();
    currentPlace = mExecutionGraph->createPlace();
    
    ExtendedInt intermediatePointValue;
    intermediatePointValue.setAsInteger(aTimeProcess->getControlPoint(controlPointID->at(j))->beginValue() - aTimeProcess->getControlPoint(controlPointID->at(j-1))->beginValue());
    
    compileTimeEvent(anEvent, previousDate - currentDate, *previousTransition, currentTransition, currentPlace);
    
    mTransitionsMap[intermediateEvent] = currentTransition;
    previousTransition = currentTransition;
    */
    
    // compile end event
    currentTransition = mExecutionGraph->createTransition();
    currentPlace = mExecutionGraph->createPlace();
    lastTransition = currentTransition;
    
    compileTimeEvent(endEvent, endDate - startDate, *previousTransition, currentTransition, currentPlace);  // normally it is not the startDate but the last intermediate event date
    
    mTransitionsMap[endEvent] = currentTransition;
    *previousTransition = currentTransition;
    
    /* IS THIS RELATIVE TO COMPILATION OF SUB SCENARIO ?
     note : it was in compileEvent
     
     // after compiling the end event
     // if this is not the last time process
     if (mTimeProcessList.end()) {
     
     std::vector<unsigned int> aTimeProcessIdToStop;
     
     unsigned int currentTime = timeOffset > startDate ? timeOffset - startDate : 0;
     PetriNet* currentPetriNet = compileScenario(hierarchyStoryLine[aTimeProcessId], hierarchyStoryLine, aTimeProcessIdToStop, currentTime, triggerAction);
     
     mExecutionGraph->addInternPetriNet(startTransition, lastTransition, currentPetriNet);
     
     lastTransition->setmExecutionGraphToEnd(currentPetriNet);
     
     if (endDate - startDate >= timeOffset) {
     
     lastTransition->setMustWaitThePetriNetToEnd(true);
     
     arcList incomingArcs = currentTransition->inGoingArcsOf();
     for (unsigned int k = 0; k < incomingArcs.size(); ++k) {
     incomingArcs[k]->changeRelativeTime(incomingArcs[k]->getRelativeMinValue(), plusInfinity);
     }
     }
     
     currentControlPointInformations->m_processIdToStop = aTimeProcessIdToStop;
     
     for (unsigned int k = 0; k < aTimeProcessIdToStop.size(); ++k) {
     processIdToStop.push_back(aTimeProcessIdToStop[k]);
     }
     }
     */
    
    // close the compilation of the process
    currentPlace = mExecutionGraph->createPlace();
    
    arcFromPreviousTransitionToCurrentPlace = mExecutionGraph->createArc(*previousTransition, currentPlace);
    
    arcFromCurrentPlaceToTheEnd = mExecutionGraph->createArc(currentPlace, endTransition);
    arcFromCurrentPlaceToTheEnd->changeRelativeTime(integer0, plusInfinity);
    
    // note : this is useless as we can find it by asking the duration
    //aTimeProcess->setWrittenTime(aTimeProcess->lengthValue());
    
    // note : this seems useless because the vector is unused afterward
    //processIdToStop.push_back(aTimeProcessId);
    
    // Special case : Automation process
    // note : is this needed ?
    if (aTimeProcess->getName() == TTSymbol("Automation"))
        ;// TODO : aTimeProcess->computeCurves(endDate - startDate);
}

void Scenario::compileInterval(TimeProcessPtr aTimeProcess)
{
    TransitionPtr             startTransition;
    TransitionPtr             endTransition;
    Place*                  currentPlace;
    Arc*                    arcFromstartTransitionToCurrentPlace;
    Arc*                    arcFromCurrentPlaceToendTransition;
    
    ExtendedInt             intervalValue;
    
    TimeEventPtr            startEvent, endEvent;
    TTUInt32                startDate, endDate;
    TTValue                 v;
    
    GraphObjectMapIterator  mergeIterator;
    
    // get start event
    aTimeProcess->getAttributeValue(TTSymbol("startEvent"), v);
    startEvent = TimeEventPtr(TTObjectBasePtr(v[0]));
    
    // get end event
    aTimeProcess->getAttributeValue(TTSymbol("endEvent"), v);
    endEvent = TimeEventPtr(TTObjectBasePtr(v[0]));
    
    // get start event date
    aTimeProcess->getAttributeValue(TTSymbol("startEventDate"), v);
    startDate = TTUInt32(v[0]);
    
    // get end event date
    aTimeProcess->getAttributeValue(TTSymbol("endEventDate"), v);
    endDate = TTUInt32(v[0]);
    
    // note : this is useless because it is checked during edition
    //if (startEvent->getContainingBoxId() != endEvent->getContainingBoxId()) {
    
    // retreive start transition
    startTransition = TransitionPtr(mTransitionsMap[startEvent]);
    mergeIterator = mMergedTransitionsMap.find(startTransition);
    
    if (mergeIterator != mMergedTransitionsMap.end())
        startTransition = TransitionPtr(mergeIterator->second);
    
    // retreive end transition
    endTransition = TransitionPtr(mTransitionsMap[endEvent]);
    mergeIterator = mMergedTransitionsMap.find(endTransition);
    
    if (mergeIterator != mMergedTransitionsMap.end())
        endTransition = TransitionPtr(mergeIterator->second);
    
    // if the interval have no duration
    if (endDate - startDate <= 0) {
        
        startTransition->merge(endTransition);
        mMergedTransitionsMap[endTransition] = startTransition;
    }
    else {
        
        currentPlace = mExecutionGraph->createPlace();
        
        intervalValue.setAsInteger(endDate - startDate);
        
        arcFromstartTransitionToCurrentPlace = mExecutionGraph->createArc(startTransition, currentPlace);
        
        arcFromCurrentPlaceToendTransition = mExecutionGraph->createArc(currentPlace, endTransition);
        arcFromCurrentPlaceToendTransition->changeRelativeTime(intervalValue, plusInfinity);
        
        mArcsMap[arcFromCurrentPlaceToendTransition] = aTimeProcess;
        
        // First cleaning
        petriNetNodeList placesAfterStartTransition = startTransition->returnSuccessors();
        for (unsigned j = 0; j < placesAfterStartTransition.size(); ++j) {
            
            Place* placeToCheckIfLinkedWithEndTransition = (Place*) placesAfterStartTransition[j];
            TransitionPtr transitionToCheckIfEqualToEndTransition = (TransitionPtr) placeToCheckIfLinkedWithEndTransition->returnSuccessors()[0];
            
            if (transitionToCheckIfEqualToEndTransition == endTransition)
                mExecutionGraph->deleteItem(placeToCheckIfLinkedWithEndTransition);
            
        }
        
        petriNetNodeList placesBeforeEndTransition = endTransition->returnPredecessors();
        for (unsigned j = 0; j < placesBeforeEndTransition.size() ; ++j) {
            
            Place* placeToCheckIfLinkedWithStartTransition = (Place*) placesBeforeEndTransition[j];
            TransitionPtr transitionToCheckIfEqualToStartTransition = (TransitionPtr) placeToCheckIfLinkedWithStartTransition->returnPredecessors()[0];
            
            if (transitionToCheckIfEqualToStartTransition == startTransition)
                mExecutionGraph->deleteItem(placeToCheckIfLinkedWithStartTransition);
            
        }
    }
}

void Scenario::compileTimeEvent(TimeEventPtr aTimeEvent, TTUInt32 time, TransitionPtr previousTransition, TransitionPtr currentTransition, Place* currentPlace)
{
    ExtendedInt timeValue;
    Arc*        arcFromPreviousTransitionToCurrentPlace = mExecutionGraph->createArc(previousTransition, currentPlace);
    Arc*        arcFromCurrentPlaceToCurrentTransition = mExecutionGraph->createArc(currentPlace, currentTransition);
    
    (void) arcFromPreviousTransitionToCurrentPlace;
    
    timeValue.setAsInteger(time);
    arcFromCurrentPlaceToCurrentTransition->changeRelativeTime(timeValue, plusInfinity);
    
    // prepare transition
    currentTransition->addExternAction(&ScenarioGraphTransitionTimeEventCallBack, aTimeEvent);
    
    // note : this extern action will be redondant
    //currentTransition->addExternAction(&ScenarioGraphTransitionTimeProcessCallBack, aTimeProcess);
}

void Scenario::compileInteractiveEvent(TimeEventPtr aTimeEvent, TTUInt32 timeOffset)
{
    TransitionPtr             currentTransition;
    Arc*                    currentArc;
    
    TTBoolean               active;
    TTUInt32                date;
    TTUInt32                durationMin, durationMax;
    TTValue                 v;
    
    GraphObjectMapIterator  mergeIterator;
    
    // get event active state
    aTimeEvent->getAttributeValue(TTSymbol("active"), v);
    active = TTBoolean(v[0]);
    
    // get event date
    aTimeEvent->getAttributeValue(TTSymbol("date"), v);
    date = TTUInt32(v[0]);
    
    if (active && date >= timeOffset) {
        
        // retreive transition
        currentTransition = TransitionPtr(mTransitionsMap[aTimeEvent]);
        mergeIterator = mMergedTransitionsMap.find(currentTransition);
        
        if (mergeIterator != mMergedTransitionsMap.end())
            currentTransition = TransitionPtr(mergeIterator->second);
        
        // prepare transition
        // TODO : we don't want to centralize the reception of network message ...
        //currentTransition->setEvent(addNetworkMessage(aTimeEvent->getTriggerMessage()));
        currentTransition->setMustWaitThePetriNetToEnd(false);
        
        // prepare time event informations
        /* THIS IS USELESS OR CAN BE RETREIVED USING INFO INSIDE THE INTERACTIVE EVENT (m_waitedString)
         
         aTimeEventInformations.m_triggerId = aTimeEvent->getTriggerId();
         aTimeEventInformations.m_waitedString = aTimeEvent->getTriggerMessage();
         aTimeEventInformations.m_boxId = aTimeEvent->getaTimeEvent()->getContainingBoxId();
         aTimeEventInformations.m_controlPointIndex = aTimeEvent->getaTimeEvent()->getId();
         aTimeEventInformations.m_waitedTriggerPointMessageAction = m_waitedTriggerPointMessageAction;
         
         m_transitionToTriggerPointInformations[currentTransition] = aTimeEventInformations;
         */
        
        /* IS THIS USEFULL ?
         if (aTimeEvent->getType() == TRIGGER_END_TEMPO_CHANGE) {
         
         unsigned int tempoChangeLastValue = aTimeEvent->beginValue();
         
         unsigned int previousTriggerId = aTimeEvent->getPreviousTriggerId();
         TriggerPoint* previousTriggerPoint = storyLineToCompile.m_triggerPoints[previousTriggerId];
         
         ControlPoint* previousRelatedTrigerPoint = previousTriggerPoint->getaTimeEvent();
         TransitionPtr previousTransition = mTransitionsMap[previousRelatedTrigerPoint];
         
         unsigned int tempoChangeFirstValue = previousRelatedTrigerPoint->beginValue();
         
         ChangeTempo* tempo = new ChangeTempo(this);
         tempo->m_writtenTime = tempoChangeLastValue - tempoChangeFirstValue;
         
         currentTransition->addExternAction(&lastTriggerReception, tempo);
         previousTransition->addExternAction(&firstTriggerReception, tempo);
         
         }
         */
        
        // prepare transition for incoming ranged interval
        arcList incomingArcs = currentTransition->inGoingArcsOf();
        for (unsigned int i = 0; i < incomingArcs.size(); ++i) {
            
            currentArc = incomingArcs[i];
            ExtendedInt minBound;
            ExtendedInt maxBound;
            
            minBound.setAsInteger(0);
            maxBound.setAsPlusInfinity();
            
            if (mArcsMap.find(currentArc) != mArcsMap.end()) {
                
                TimeProcessPtr aTimeProcess = TimeProcessPtr(mArcsMap[currentArc]);
                
                // get start event date
                aTimeProcess->getAttributeValue(TTSymbol("durationMin"), v);
                durationMin = TTUInt32(v[0]);
                
                // get end event date
                aTimeProcess->getAttributeValue(TTSymbol("durationMax"), v);
                durationMax = TTUInt32(v[0]);
                
                if (durationMin)
                    minBound.setAsInteger(durationMin);
                
                if (durationMax)
                    maxBound.setAsInteger(durationMax);
            }
            
            currentArc->changeRelativeTime(minBound, maxBound);
        }
    }
}
/*
void Scenario::cleanGraph(TransitionPtr endTransition)
{
    if (mExecutionGraph == NULL)
        return;
    
    map<TransitionPtr, set<TransitionPtr> >* transitionsSets = new map<TransitionPtr, set<TransitionPtr> >;
    
    if (endTransition == NULL)
        computeTransitionsSet(endTransition, transitionsSets);
    
    map<TransitionPtr, set<TransitionPtr> >::iterator it  = transitionsSets->begin();
    while (it != transitionsSets->end())
    {
        TransitionPtr                 currentTransition = it->first;
        set<TransitionPtr>            currentSet = it->second;
        set<Place*>                 successorsOfNDepth;
        set<Place*>                 currentTransitionPredecessors;
        set<Place*>                 placesToDelete;
        set <TransitionPtr>::iterator transitionSetIterator;
        set <Place*>::iterator      placeSetIterator;
        
        for (transitionSetIterator = currentSet.begin(); transitionSetIterator != currentSet.end(); transitionSetIterator++) {
            
            petriNetNodeList    successors = (*transitionSetIterator)->returnSuccessors();
            set<Place*>         successorsOfNDepthTemp;
            
            for (unsigned int i = 0; i < successors.size() ; ++i)
                successorsOfNDepthTemp.insert((Place*) successors[i]);
            
            successorsOfNDepth.insert(successorsOfNDepthTemp.begin(), successorsOfNDepthTemp.end());
        }
        
        petriNetNodeList predecessors = currentTransition->returnPredecessors();
        
        for (unsigned int i = 0; i < predecessors.size() ; ++i)
            currentTransitionPredecessors.insert((Place*) predecessors[i]);
        
        set_intersection(successorsOfNDepth.begin(),successorsOfNDepth.end(),
                         currentTransitionPredecessors.begin(),currentTransitionPredecessors.end(),
                         std::inserter( placesToDelete, placesToDelete.end() ) );
        
        for (placeSetIterator = placesToDelete.begin(); placeSetIterator != placesToDelete.end(); placeSetIterator++) {
            Place* currentPlaceToDelete = *placeSetIterator;
            mExecutionGraph->deleteItem(currentPlaceToDelete);
        }
        
        ++it;
    }
}
*/

#ifdef NEW_GRAPH
// Trying to rewrite PetriNet::makeOneStep method with the TimeEventLib and TimeProcessLib formalism

/* About TimeEvent :
    
    we will certainly need to have :
        - the date attribute (choosen during edition time)
        - the execution date attribute (being incremented during the execution)
    or we could memorize all time event dates during compilation and then set those dates back when the scenario stops ?
 
    we will also need to know if the time event is a start event of a time process (but not to an interval !)
    how to do that ?
 
    we will also need a way to status about what they call "waited" : one more attribute or a combination of several boolean state ?
 
*/ 

/* About m_priorityTransitionsActionQueue :
 
    The m_priorityTransitionsActionQueue is filled in Transition::setArcAsActive method which is called in :
 
        - a Transition::crossTransition which is only called in :
            - PetriNet::makeOneStep             => could a Scenario observes the Notify method of a TimeEvent ?
 
        - a Place::produceTokens which is called in :
            - PetriNet::start                   => could it be done in Scenario::ProcessStart in other way ?
 
            - Arc::produceTokenInTo which is only called in :
                - Transition::crossTransition   => same question as above
    
    So is the mTimeEventList and the mTimeProcessList can provide us enough information to fill a timeEventQueue in the same way ?
 
*/

/* About sensitize notion :
 
    A transition couldBeSensitize if it haven't to wait another petri net to end (certainly relative to hierachy managment ...).
    
    Sensitize a transition means to move the transition from the m_priorityTransitionsActionQueue to the m_sensitizedTransitions list.
    If a transition cannot be sensitize, the transition is move to the tail of the list and his "execution" date is incremented.
 
    So maybe we could have a timeEventReadyList
*/

bool Scenario::step(TTUInt32 currentTimeInMs)
{
    TTBoolean stop = NO;
    
    if (NO) // TODO : check there is one more step to do
        return NO;
    
    // process events in the timeEventQueue
    while (!stop) {
        
        // TODO : if there is no more event in the timeEventQueue : stop
        if (m_priorityTransitionsActionQueue.size() == 0) {
            
            stop = true;
        }
        else {
            
            // TODO : get the event in a timeEventQueue
            PriorityTransitionAction* topAction = getTopActionOnPriorityQueue();
            
            // TODO : if the event is not active
            if (!topAction->isEnable()) {
                
                removeTopActionOnPriorityQueue();
            }
            // TODO : or if the event have to append later
            else if ((unsigned int) topAction->getDate().getValue() > currentTime) {
                
                stop = true;
            }
            else {
                
                Transition* topTransition = topAction->getTransition();
                
                // TODO : is the event a start event of a time process (but not to an interval process) ?
                if (topAction->getType() == START) {
                    
                    // NOTE : this should be always true ...
                    if (topTransition->couldBeSensitize()) {
                        
                        // TODO : append the event to the timeEventReadyList
                        turnIntoSensitized(topTransition);
                        
                        // TODO : set "waited" state of the event
                        if (m_waitedTriggerPointMessageAction != NULL)
                            m_waitedTriggerPointMessageAction(m_waitedTriggerPointMessageArgument, true, topTransition);
                        
                        // TODO : remove the event from the timeEventQueue
                        removeTopActionOnPriorityQueue();
                        
                    }
                    // NOTE : this should never append ...
                    else {
                        topAction->setDate(currentTime + 1);
                        removeTopActionOnPriorityQueue();
                        m_priorityTransitionsActionQueue.push(topAction);
                    }
                    
                }
                else {
                    
                    // TODO : err = timeEvent->sendMessage(TTSymbol("Notify"));
                    
                    // if (!err)
                    if (topTransition->areAllInGoingArcsActive()) {
                        
                        // NOTE : this should be done inside the Notify method (which is observed by the Scenario to see if it succeed or not)
                        topTransition->crossTransition(true, currentTime - topAction->getDate().getValue());
                        
                        // TODO : remove the event from the timeEventQueue
                        removeTopActionOnPriorityQueue();
                    }
                    else {
                        
                        // TODO : remove the event from the timeEventQueue
                        removeTopActionOnPriorityQueue();
                        
                        // TODO : return kTTGraphErrorGeneric;
                        throw IncoherentStateException();
                    }
                    
                }
            }
        }
    }
    
    // process events in the timeEventReadyList
    for (unsigned int i = 0 ; i < m_sensitizedTransitions.size() ; ++i) {
        
        // TODO : get the event in a timeEventReadyList
        Transition* sensitizedTransitionToTestTheEvent;
        sensitizedTransitionToTestTheEvent = m_sensitizedTransitions[i];
        
        // TODO : err = timeEvent->sendMessage(TTSymbol("Notify"));
        
        // NOTE : if the Notify method fails (because all triggers haven't triggered)
        if (err) {
        // if (!sensitizedTransitionToTestTheEvent->areAllInGoingArcsActive()) {
            
            // TODO : set "waited" state of the event
            if (m_waitedTriggerPointMessageAction != NULL)
                m_waitedTriggerPointMessageAction(m_waitedTriggerPointMessageArgument, false, sensitizedTransitionToTestTheEvent);
            
            // TODO : remove the event from the timeEventReadyList
            m_sensitizedTransitions.erase(m_sensitizedTransitions.begin() + i);
            --i;
            
        }
        
        /* NOTE : this is useless here. it should be moved inside the Notify method
         
        else if (isAnEvent(sensitizedTransitionToTestTheEvent->getEvent()) || m_mustCrossAllTransitionWithoutWaitingEvent){
            
            if (sensitizedTransitionToTestTheEvent->isStatic())
                sensitizedTransitionToTestTheEvent->crossTransition(true, currentTime - sensitizedTransitionToTestTheEvent->getStartDate().getValue());
            else
                sensitizedTransitionToTestTheEvent->crossTransition(true,0);
            
            // TODO : set "waited" state of the event
            if (m_waitedTriggerPointMessageAction != NULL)
                m_waitedTriggerPointMessageAction(m_waitedTriggerPointMessageArgument, false, sensitizedTransitionToTestTheEvent);
            
            // TODO : remove the event from the timeEventReadyList
            m_sensitizedTransitions.erase(m_sensitizedTransitions.begin() + i);
            --i;
            
        }
         */
    }
    
    // NOTE : this is useless
    //resetEvents();
    
    return YES;
}
#endif
