/*
Copyright: LaBRI (http://www.labri.fr)

Author(s): Raphael Marczak
Last modification: 08/03/2010

Adviser(s): Myriam Desainte-Catherine (myriam.desainte-catherine@labri.fr)

This software is a computer program whose purpose is to propose
a library for interactive scores edition and execution.

This software is governed by the CeCILL-C license under French law and
abiding by the rules of distribution of free software.  You can  use,
modify and/ or redistribute the software under the terms of the CeCILL-C
license as circulated by CEA, CNRS and INRIA at the following URL
"http://www.cecill.info".

As a counterpart to the access to the source code and  rights to copy,
modify and redistribute granted by the license, users are provided only
with a limited warranty  and the software's author,  the holder of the
economic rights,  and the successive licensors  have only  limited
liability.

In this respect, the user's attention is drawn to the risks associated
with loading,  using,  modifying and/or developing or reproducing the
software by the user in light of its specific status of free software,
that may mean  that it is complicated to manipulate,  and  that  also
therefore means  that it is reserved for developers  and  experienced
professionals having in-depth computer knowledge. Users are therefore
encouraged to load and test the software's suitability as regards their
requirements in conditions enabling the security of their systems and/or
data to be ensured and,  more generally, to use and operate it in the
same conditions as regards security.

The fact that you are presently reading this means that you have had
knowledge of the CeCILL-C license and that you accept its terms.
*/

#include "PetriNet.hpp"

/*!
 * \file PetriNet.cpp
 * \author Raphael Marczak (LaBRI), based on Antoine Allombert (LaBRI) LISP code
 * \date 2008-2009
 */

#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>

#include <iostream>
#include <algorithm>

using namespace std;

PetriNet::PetriNet(unsigned int nbColors):
m_nbColors((nbColors > 0)?nbColors:1),
m_mustCrossAllTransitionWithoutWaitingEvent(false)
{
	m_parentPetriNet = NULL;

	m_updateFactor = 1;
	m_isEventReadyCallback = NULL;

	m_mustStop = false;
    
    m_currentTime = 0;
    m_isRunning = false;

	resetEvents();
}

void PetriNet::start()
{
    m_startPlace->produceTokens(1);
    m_endPlace->consumeTokens(m_endPlace->nbOfTokens());
    
    m_isRunning = true;
}

bool PetriNet::makeOneStep(unsigned int currentTime)
{
    m_currentTime = currentTime;
    
    if (m_endPlace->nbOfTokens() == 0 && !m_mustStop) {
        
        bool stop = false;
        
        while (!stop) {
            if (m_priorityTransitionsActionQueue.size() == 0) {
                stop = true; // CB should check that in while condition
            } else {
                PriorityTransitionAction* topAction = getTopActionOnPriorityQueue();
                
                if (!topAction->isEnable()) { // CB why disable actions without destructing them ?
                    removeTopActionOnPriorityQueue();
                } else if ((unsigned int) topAction->getDate().getValue() > currentTime) {
                    stop = true; // CB because it's a priority queue, so it is ordered
                } else {
                    Transition* topTransition = topAction->getTransition();
                    
                    if (topAction->getType() == START) { // CB START means actually min duration for the interval
                        if (topTransition->couldBeSensitize()) { // CB if there is no subnet running
                            
                            turnIntoSensitized(topTransition); // CB listen to the event (even if it's a static transition ?!)
                            
                            if (topTransition->getEvent() != NULL && m_isEventReadyCallback != NULL) { // CB if it's not static
                                
                                // DEBUG
                                std::cout << "PetriNet::makeOneStep : sensitize event " << topTransition->getEvent() << " at " << currentTime << " ms" << std::endl;
                                
                                m_isEventReadyCallback(topTransition->getEvent(), true); // CB tell to Score to listen to the event
                            }
                            
                            removeTopActionOnPriorityQueue(); // CB Done
                            
                        } else { // CB if there is a subnet running
                            topAction->setDate(currentTime + 1); // CB delay
                            removeTopActionOnPriorityQueue();
                            m_priorityTransitionsActionQueue.push(topAction);
                        }
                        
                        //stop = true;
                    } else { // CB if type END, actually max duration for the interval
                        if (topTransition->areAllInGoingArcsActive()) {
                            topTransition->crossTransition(true, currentTime - topAction->getDate().getValue()); // CB force the transition
                            removeTopActionOnPriorityQueue();
                        } else { // CB should be part of debug, like avery IncoherentStateException actually
                            removeTopActionOnPriorityQueue();
                            throw IncoherentStateException();
                        }
                    }
                }
            }
        }
        
        // among all sensitized transitions
        for (unsigned int i = 0 ; i < m_sensitizedTransitions.size() ; ++i) {
            
            Transition* sensitizedTransitionToTestTheEvent;
            sensitizedTransitionToTestTheEvent = m_sensitizedTransitions[i];
            
            // if all the going arc are not active ; CB in fact, if we already forced the transition because of the max duration of the interval (or if there is an IncoherentState)
            if (!sensitizedTransitionToTestTheEvent->areAllInGoingArcsActive()) {
                
                //remove the sensitized transition
                m_sensitizedTransitions.erase(m_sensitizedTransitions.begin() + i);
                --i;
                
                // CB if it was an interactive event
                if (sensitizedTransitionToTestTheEvent->getEvent() != NULL && m_isEventReadyCallback != NULL) {
                    
                    // DEBUG
                    std::cout << "PetriNet::makeOneStep : sensitized event " << sensitizedTransitionToTestTheEvent->getEvent() << " not ready anymore" << std::endl;
                    
                    m_isEventReadyCallback(sensitizedTransitionToTestTheEvent->getEvent(), false); // CB tell Score to stop listening to the event
                }
            }

            // cf triggerpoint : else check if the transition event is part of the recent incomming events (or if all transition have to pass)
            else if (isAnEvent(sensitizedTransitionToTestTheEvent->getEvent()) || m_mustCrossAllTransitionWithoutWaitingEvent){
                
                if (sensitizedTransitionToTestTheEvent->isStatic()) { // CB if it's in fact a static event, actually listening to nothing though
                    
                    sensitizedTransitionToTestTheEvent->crossTransition(true, currentTime - sensitizedTransitionToTestTheEvent->getStartDate().getValue());
                    
                } else { // CB if it's a real interactive event that has arrived
                    
                    // DEBUG
                    std::cout << "PetriNet::makeOneStep : sensitized event happened " << sensitizedTransitionToTestTheEvent->getEvent() << " at " << currentTime << " ms" << std::endl;
                    
                    sensitizedTransitionToTestTheEvent->crossTransition(true,0);
                }
                
                m_sensitizedTransitions.erase(m_sensitizedTransitions.begin() + i);
                --i; // CB 0 - 1 in an unsigned int isn't a very good idea I think
                
                if (sensitizedTransitionToTestTheEvent->getEvent() != NULL && m_isEventReadyCallback != NULL) {
                    
                    m_isEventReadyCallback(sensitizedTransitionToTestTheEvent->getEvent(), false); // CB tell Score to stop listening to the event
                }
            }

            // CB check if the transition should be deactivated because of a condition
            else {
                for (transitionList::iterator it = m_deactivatedTransitions.begin() ; sensitizedTransitionToTestTheEvent && it != m_deactivatedTransitions.end() ; it++) {
                    if (sensitizedTransitionToTestTheEvent == *it) {
                        sensitizedTransitionToTestTheEvent->crossTransition(true, -1); // CB -1 for inactive token

                        m_sensitizedTransitions.erase(m_sensitizedTransitions.begin() + i);
                        --i;

                        sensitizedTransitionToTestTheEvent = NULL;

                        m_deactivatedTransitions.erase(it);
                    }
                }
            }
        }
        
        resetEvents(); // CB discards events that nobody listens to
        
        while(!m_transitionsToCrossWhenAcceleration.empty()) {
            Transition* currentTransition = m_transitionsToCrossWhenAcceleration.back();
            
            if (currentTransition->areAllInGoingArcsActive()) {
                currentTransition->crossTransition(false, 0);
            } else {
                throw IncoherentStateException();
            }
            
            m_transitionsToCrossWhenAcceleration.pop_back();
        }
        
        m_transitionsToCrossWhenAcceleration.clear();
        
        return true;
    }
    
    m_isRunning = false;
    return false;
}

unsigned int PetriNet::getCurrentTimeInMs()
{
    return m_currentTime;
}

bool PetriNet::isRunning()
{
    return m_isRunning;
}

bool PetriNet::isColorValid(unsigned int color)
{
	return ((color > 0) && (color <= nbOfPossibleColors()));
}

// gets the number of colors in this PetriNet.
unsigned int PetriNet::nbOfPossibleColors()
{
	return m_nbColors;
}

// sets the number of colors in this PetriNet.
void PetriNet::changeNbOfColors(unsigned int newNbColors)
{
	if (newNbColors <= m_nbColors) {
		throw IllegalArgumentException();
	}

	for (unsigned int i = 0; i < m_places.size(); i++) {
		m_places[i]->changeNbOfColors(newNbColors);
	}

	for (unsigned int i = 0; i < m_transitions.size(); i++) {
		m_transitions[i]->changeNbOfColors(newNbColors);
	}

	m_nbColors = newNbColors;
}

void PetriNet::resetEvents()
{
	m_mustCrossAllTransitionWithoutWaitingEvent = false;
	m_incomingEvents.clear();
    m_incomingEvents.push_back(STATIC_EVENT);
}

void PetriNet::putAnEvent(void* event)
{
	m_incomingEvents.push_back(event);

	for (std::map<PetriNet*, PetriNet*>::iterator it = m_activeChildPetriNet.begin(); it != m_activeChildPetriNet.end(); ++it)
	{
		it->second->putAnEvent(event);
	}

}

bool PetriNet::isAnEvent(void* event)
{
	eventList tempList = m_incomingEvents.getList();
	std::list<void*>::iterator it = tempList.begin();

	while (it != tempList.end()) {
        
		if (*it == event)
			return true;
        
		++it;
	}
    
	return false;
}

Place* PetriNet::createPlace()
{
	Place* newPlace;

	newPlace = new Place(this);
	m_places.push_back(newPlace);

	return newPlace;
}

Transition* PetriNet::createTransition()
{
	Transition* newTransition;

	newTransition = new Transition(this);
	m_transitions.push_back(newTransition);

	return newTransition;
}

Arc* PetriNet::newArc(PetriNetNode* from, PetriNetNode* to, int color)
{
	Arc* existArc = to->haveArcFrom(from,color);

	if (existArc != NULL) {
		return existArc;
	}

	return new Arc(this, from, to, color);
}

Arc* PetriNet::createArc(Place* from, Transition* to, int color)
{
	return newArc(from, to, color);
}

Arc* PetriNet::createArc(Transition* from, Place* to, int color)
{
	return newArc(from, to, color);
}

void PetriNet::deleteArc(PetriNetNode* from, PetriNetNode* to)
{
	for (unsigned int i = 1; i <= nbOfPossibleColors(); i++) {
		Arc* arcToDelete = to->haveArcFrom(from, i);

		if (arcToDelete != NULL) {
			to->removeInGoingArcs(arcToDelete, i);
			from->removeOutGoingArcs(arcToDelete, i);

			delete arcToDelete;
		}
	}
}

void PetriNet::deleteArc(PetriNetNode* from, PetriNetNode* to, int color)
{
	Arc* arcToDelete = to->haveArcFrom(from, color);

	if (arcToDelete == NULL) {
		return;
	}

	to->removeInGoingArcs(arcToDelete, color);
	from->removeOutGoingArcs(arcToDelete, color);

	delete arcToDelete;
}

void PetriNet::deleteItem(PetriNetNode* nodeToDelete)
{
	petriNetNodeList successorsList = nodeToDelete->returnSuccessors();
	for (unsigned int i = 0; i < successorsList.size(); ++i) {
		deleteArc(nodeToDelete, successorsList[i]);
	}

	petriNetNodeList predecessorsList = nodeToDelete->returnPredecessors();
	for (unsigned int i = 0; i < predecessorsList.size(); ++i) {
		deleteArc(predecessorsList[i], nodeToDelete);
	}
}

void PetriNet::deleteItem(Place* & placeToDelete)
{
	if (placeToDelete == NULL) {
		return;
	}

	int indexToRemove = -1;
	unsigned int i = 0;

	while (indexToRemove == -1 && i < m_places.size()) {
	//for (unsigned int i = 0; i < m_places.size(); i++) {
		if (m_places[i] == placeToDelete) {
			indexToRemove = i;
		}
		++i;
	}

	if (indexToRemove != -1) {
		m_places.erase(m_places.begin() + indexToRemove);
	}

	deleteItem((PetriNetNode*) placeToDelete);

	delete placeToDelete;
	placeToDelete = NULL;

}

void PetriNet::deleteItem(Transition* & transitionToDelete)
{
	if (transitionToDelete == NULL) {
		return;
	}

	int indexToRemove = -1;
	unsigned int i = 0;

	while (indexToRemove == -1 && i < m_transitions.size()) {
	//for (unsigned int i = 0; i < m_transitions.size(); i++) {
		if (m_transitions[i] == transitionToDelete) {
			indexToRemove = i;
		}
		++i;
	}

	if (indexToRemove != -1) {
		m_transitions.erase(m_transitions.begin() + indexToRemove);
	}

	deleteItem((PetriNetNode*) transitionToDelete);

	delete transitionToDelete;
	transitionToDelete = NULL;
}

void PetriNet::turnIntoSensitized(Transition* t)
{
	m_sensitizedTransitions.push_back(t);
}

void PetriNet::deactivateTransition(Transition* t)
{
    m_deactivatedTransitions.push_back(t);
}

//
//void PetriNet::turnIntoUnsensitized(Transition* t)
//{
//	if (t->hasATokenInPredecessors()) {
//		m_activeTransitions.push_back(t);
//	}
//}

placeList PetriNet::getPlaces()
{
	return m_places;
}

void PetriNet::setStartPlace(Place* place)
{
	m_startPlace = place;
}

Place* PetriNet::getStartPlace()
{
	return m_startPlace;
}

void PetriNet::setEndPlace(Place* place)
{
	m_endPlace = place;
}

Place* PetriNet::getEndPlace()
{
	return m_endPlace;
}

transitionList PetriNet::getTransitions()
{
	return m_transitions;
}

//transitionList PetriNet::getActiveTransitions() {
//	return m_activeTransitions;
//}

transitionList PetriNet::getSensitizedTransitions()
{
	return m_sensitizedTransitions;
}

eventList PetriNet::getEvents()
{
	return m_incomingEvents.getList();
}

void PetriNet::mustStop()
{
	m_mustStop = true;
}

void PetriNet::addActionToPriorityQueue(PriorityTransitionAction* action)
{
	m_priorityTransitionsActionQueue.push(action);
}

priorityTransitionActionQueue PetriNet::getPriorityQueue()
{
	return m_priorityTransitionsActionQueue;
}

PriorityTransitionAction* PetriNet::getTopActionOnPriorityQueue()
{
	if (isEmptyPriorityQueue()) {
		throw IncoherentStateException();
	}

	return m_priorityTransitionsActionQueue.top();
}

void PetriNet::removeTopActionOnPriorityQueue()
{
	if (isEmptyPriorityQueue()) {
		throw IncoherentStateException();
	}

	m_priorityTransitionsActionQueue.pop();
}

bool PetriNet::isEmptyPriorityQueue()
{
	return m_priorityTransitionsActionQueue.empty();
}

void PetriNet::setUpdateFactor(float updateFactor)
{
	m_updateFactor = updateFactor;

	std::map<PetriNet*, PetriNet*>::iterator it;

	for (it = m_activeChildPetriNet.begin(); it != m_activeChildPetriNet.end(); ++it) {
		it->second->setUpdateFactor(updateFactor);
	}
}

float PetriNet::getUpdateFactor()
{
	return m_updateFactor;
}

void PetriNet::ignoreEventsForOneStep()
{
	m_mustCrossAllTransitionWithoutWaitingEvent = true;
}

void PetriNet::addIsEventReadyCallback(void(*pt2Func)(void*, bool))
{
	m_isEventReadyCallback = pt2Func;
}

void PetriNet::removeIsEventReadyCallback()
{
	m_isEventReadyCallback = NULL;
}

void PetriNet::pushTransitionToCrossWhenAcceleration(Transition* t)
{
// This function is useless because the implementation is wrong
// The GOTO and Acceleration will work without this function (but will
// not be optimized).
//
// This fucntion should be better implemented in next version !
//
//	transitionList::iterator it;
//
//	it = std::find(m_transitionsToCrossWhenAcceleration.begin(),
// m_transitionsToCrossWhenAcceleration.end(), t);
//
//	if (it != m_transitionsToCrossWhenAcceleration.end()) {
//		m_transitionsToCrossWhenAcceleration.push_back(t);
//	}
}


void PetriNet::addInternPetriNet(Transition* startTransition, Transition* endTransition, PetriNet* petriNet)
{
	petriNet->m_parentPetriNet = this;
	m_childrenPetriNet[petriNet] = petriNet;

	startTransition->addExternAction(&externLaunch, petriNet);
	endTransition->addExternAction(&externMustStop, petriNet);

	if (m_isEventReadyCallback != NULL)
		petriNet->addIsEventReadyCallback(m_isEventReadyCallback);
}

void PetriNet::print()
{
	std::cout << "PETRI NET" << std::endl;

	std::cout << "TransitionList elements " << m_sensitizedTransitions.size() << std::endl;
	std::cout << "PriorityQueu elements " << m_priorityTransitionsActionQueue.size() << std::endl;

	std::cout << std::endl;
	for (unsigned int i = 0; i < m_places.size(); ++i) {
		m_places[i]->print();
		std::cout << std::endl;
	}

	for (unsigned int i = 0; i < m_transitions.size(); ++i) {
		m_transitions[i]->print();
		std::cout << std::endl;
	}

	std::cout << "********" << std::endl;
}
/*
void PetriNet::cleanGraph(Transition* endTransition)
{
//    if (mExecutionGraph == NULL) {
//        return;
//    }

    std::map<Transition*, std::set<Transition*> >* transitionsSets = new std::map<Transition*, std::set<Transition*> >;

    if (endTransition == NULL) {
        computeTransitionsSet(endTransition, transitionsSets);
    }

    std::map<Transition*, std::set<Transition*> >::iterator it  = transitionsSets->begin();
    while (it != transitionsSets->end())
    {
        Transition*                 currentTransition = it->first;
        std::set<Transition*>            currentSet = it->second;
        std::set<Place*>                 successorsOfNDepth;
        std::set<Place*>                   currentTransitionPredecessors;
        std::set<Place*>                   placesToDelete;
        std::set <Transition*>::iterator transitionSetIterator;
        std::set <Place*>::iterator        placeSetIterator;

        for (transitionSetIterator = currentSet.begin(); transitionSetIterator != currentSet.end(); transitionSetIterator++) {

            petriNetNodeList    successors = (*transitionSetIterator)->returnSuccessors();
//            set<Place*>         successorsOfNDepthTemp;

//            for (unsigned int i = 0; i < successors.size() ; ++i)
//                successorsOfNDepthTemp.insert((Place*) successors[i]);

            successorsOfNDepth.insert(successors.begin(), successors.end());
        }

        petriNetNodeList predecessors = currentTransition->returnPredecessors();

        for (unsigned int i = 0; i < predecessors.size() ; ++i) {
//            currentTransitionPredecessors.insert((Place*) predecessors[i]);
        }

        set_intersection(successorsOfNDepth.begin(),successorsOfNDepth.end(),
                         currentTransitionPredecessors.begin(),currentTransitionPredecessors.end(),
                         std::inserter( placesToDelete, placesToDelete.end() ) );

        for (placeSetIterator = placesToDelete.begin(); placeSetIterator != placesToDelete.end(); placeSetIterator++) {
//            Place* currentPlaceToDelete = *placeSetIterator;
            deleteItem(currentPlaceToDelete);
        }

        ++it;
    }
}

std::set<Transition*> PetriNet::computeTransitionsSet(Transition* endTransition, std::map<Transition*, std::set<Transition*>>* transitionsSets)
{
    std::set <Transition*>::iterator setIterator;

    std::set<Transition*> oneDepthPredecessors = getOneDepthPredecessorsTransitions(endTransition);

    std::set<Transition*> twoDepthPredecessors;
    std::set<Transition*> nDepthPredecessors;

    for (setIterator = oneDepthPredecessors.begin(); setIterator != oneDepthPredecessors.end(); setIterator++) {
        std::set<Transition*> twoDepthPredecessorsTemp = getOneDepthPredecessorsTransitions(*setIterator);
        twoDepthPredecessors.insert(twoDepthPredecessorsTemp.begin(), twoDepthPredecessorsTemp.end());
    }

    if (twoDepthPredecessors.size() > 0) {
        for (setIterator = twoDepthPredecessors.begin(); setIterator != twoDepthPredecessors.end(); setIterator++) {
            std::set<Transition*> nDepthPredecessorsTemp = computeTransitionsSet(*setIterator, transitionsSets);
            nDepthPredecessors.insert(nDepthPredecessorsTemp.begin(), nDepthPredecessorsTemp.end());
        }

        nDepthPredecessors.insert(twoDepthPredecessors.begin(), twoDepthPredecessors.end());

        (*transitionsSets)[endTransition] = nDepthPredecessors;
    }

    return nDepthPredecessors;
}

std::set<Transition*> PetriNet::getOneDepthPredecessorsTransitions(Transition* transition)
{
    std::set<Transition*> oneDepthPredecessors;
    petriNetNodeList oneDepthPlaces = transition->returnPredecessors();

    for (unsigned int i = 0; i < oneDepthPlaces.size(); ++i) {
//        Place* currentPlace = (Place*) oneDepthPlaces[i];
//        petriNetNodeList oneDepthTransitions = currentPlace->returnPredecessors();

//        for(unsigned int j = 0; j < oneDepthTransitions.size(); ++j) {
//            oneDepthPredecessors.insert((Transition*)oneDepthTransitions[j]);
        }
    }

    return oneDepthPredecessors;
}
*/

PetriNet::~PetriNet()
{
	for (unsigned int i = 0; i < m_places.size(); ++i) {
		if (m_places[i] != NULL) {
			delete m_places[i];
		}
	}

	for (unsigned int i = 0; i < m_transitions.size(); ++i) {
		if (m_transitions[i] != NULL) {
			delete m_transitions[i];
		}
	}

//	while (!m_transitions.empty()) {
//		deleteItem(m_transitions[0]);
//	}
}

void externLaunch(void* arg, bool option)
{
	PetriNet* petriNet = (PetriNet*) arg;

	if (petriNet->m_parentPetriNet != NULL) {
		petriNet->m_parentPetriNet->m_activeChildPetriNet[petriNet] = petriNet;
		petriNet->setUpdateFactor(petriNet->m_parentPetriNet->getUpdateFactor());
	}

    // NOTE : the line below is commented out because we don't need to launch the thread execution anymore
    // see in : Scenario.cpp
	//petriNet->launch();
}

void externMustStop(void* arg, bool option)
{
	PetriNet* petriNet = (PetriNet*) arg;

	if (petriNet->m_parentPetriNet != NULL) {
		petriNet->m_parentPetriNet->m_activeChildPetriNet.erase(petriNet);
	}

	petriNet->mustStop();
}
