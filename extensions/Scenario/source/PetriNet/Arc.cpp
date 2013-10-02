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

#include "Arc.hpp"

/*!
 * \file Arc.cpp
 * \author Raphael Marczak, based on Antoine Allombert LISP code
 * \date 2008-2009
 */

#include "Transition.hpp"
#include "Place.hpp"
#include "PetriNet.hpp"

#include <iostream>
using namespace std;

Arc::Arc() {

}

Arc::Arc(PetriNet* petriNet, PetriNetNode* from, PetriNetNode* to, int color)
:PetriNetItem(petriNet), m_condition(NULL){
	if (!petriNet->isColorValid(color)) {
		throw IllegalArgumentException("Arc : Invalid color");
	}


	if (dynamic_cast<Place*>(from) && dynamic_cast<Place*>(to)) {
		throw IllegalArgumentException("Arc from Place to Place");
	}

	if (dynamic_cast<Transition*>(from) && dynamic_cast<Transition*>(to)) {
		throw IllegalArgumentException("Arc from Transition to Transition");
	}

	m_color = color;

	m_nodeFrom = from;
	m_nodeTo = to;

	m_nodeFrom->addOutGoingArcs(this, color);
	m_nodeTo->addInGoingArcs(this, color);

	// m_internTimer.setAsInteger(0);
	m_relativeMaxValue.setAsPlusInfinity();
	m_relativeMinValue.setAsMinusInfinity();

	m_absoluteMaxValue.setAsPlusInfinity();
	m_absoluteMinValue.setAsMinusInfinity();

	m_number = NO_NUMBER;
    
    m_condition = true; // théo : I've added to pass the test in Transition::produceToken

	if (dynamic_cast<Transition*>(m_nodeTo)) {
		((Transition*)m_nodeTo)->createBitArray(); // TODO : numbers should remain the same for all arcs, unless you have multiple colors

	}
}

PetriNetNode* Arc::getFrom() {
	return m_nodeFrom;
}

PetriNetNode* Arc::getTo() {
	return m_nodeTo;
}

void Arc::changeRelativeMinTime(ExtendedInt minTime) {
	if (minTime >= m_relativeMaxValue) {
		throw IllegalArgumentException("Arc : relativeMin > relativeMax");
	}

	m_relativeMinValue = minTime;
}

void Arc::changeRelativeMaxTime(ExtendedInt maxTime) {
	if (maxTime <= m_relativeMinValue) {
		throw IllegalArgumentException("Arc : relativeMax < relativeMin");
	}

	m_relativeMaxValue = maxTime;
}

void Arc::changeRelativeTime(ExtendedInt minTime, ExtendedInt maxTime) {
	if (maxTime <= minTime) {
		throw IllegalArgumentException("Arc : relativeMin > relativeMax (both)");
	}

	m_relativeMinValue = minTime;
	m_relativeMaxValue = maxTime;
}

void Arc::changeAbsoluteMinTime(ExtendedInt minTime) {
	if (minTime >= m_absoluteMaxValue) {
		throw IllegalArgumentException("Arc : absoluteMin > absoluteMax");
	}

	m_absoluteMinValue = minTime;
}

void Arc::changeAbsoluteMaxTime(ExtendedInt maxTime) {
	if (maxTime <= m_absoluteMinValue) {
		throw IllegalArgumentException("Arc : absoluteMax < absoluteMin");
	}

	m_absoluteMaxValue = maxTime;
}

void Arc::changeAbsoluteTime(ExtendedInt minTime, ExtendedInt maxTime) {
	if (maxTime <= minTime) {
		throw IllegalArgumentException("Arc : absoluteMin > absoluteMax (both)");
	}

	m_absoluteMinValue = minTime;
	m_absoluteMaxValue = maxTime;
}

//void Arc::passingTime(unsigned int timeStep) {
//	m_internTimer += timeStep;
//}
//
//bool Arc::haveReachedMinLimit() {
//	return (m_relativeMinValue <= m_internTimer);
//}
//
//bool Arc::haveReachedMaxLimit() {
//	return (m_relativeMaxValue <= m_internTimer);
//}

int Arc::nbOfArcColorLabelTokensInFrom() {
	if (!dynamic_cast<Place*>(m_nodeFrom)) {
		throw IllegalArgumentException("Can only count Tokens from Places");
	}

	return ((Place*)m_nodeFrom)->getNbOfTokens(getColor());
}

unsigned int Arc::consumeTokenInFrom() {
	if (!dynamic_cast<Place*>(m_nodeFrom)) {
		throw IllegalArgumentException("Can only consume Tokens from Places");
	}

	return ((Place*)m_nodeFrom)->consumeTokens(NB_OF_TOKEN_TO_CONSUME, getColor());
}

void Arc::produceTokenInTo(unsigned int tokenValue) {
	if (!dynamic_cast<Place*>(m_nodeTo)) {
		throw IllegalArgumentException("Can only produce Tokens into Places");
	}

	((Place*)m_nodeTo)->produceTokens(NB_OF_TOKEN_TO_PRODUCE, getColor(), tokenValue);
}

bool Arc::isActive() {
	return (nbOfArcColorLabelTokensInFrom() >= NB_OF_TOKEN_TO_ACTIVE_ARC);
}

bool Arc::haveEnoughTokensInFrom() {
	return isActive();
}

int Arc::getColor() {
	return m_color;
}

ExtendedInt Arc::getRelativeMinValue() {
	return m_relativeMinValue;
}

ExtendedInt Arc::getRelativeMaxValue() {
	return m_relativeMaxValue;
}

ExtendedInt Arc::getAbsoluteMinValue() {
	return m_absoluteMinValue;
}

ExtendedInt Arc::getAbsoluteMaxValue() {
	return m_absoluteMaxValue;
}

//ExtendedInt Arc::getTimer() {
//	return m_internTimer;
//}

void Arc::setNumber(unsigned int arcNumber) {
	m_number = arcNumber;
}

int Arc::getNumber() {
	return m_number;
}


Arc::~Arc() {
	if (dynamic_cast<Transition*>(m_nodeTo)) {
		((Transition*)m_nodeTo)->createBitArray(); // TODO : Doesn't really update the Transition bit array because the arc isn't removed from the transition's InGoingArcs
	}
}
