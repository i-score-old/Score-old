/*
 * Temporary class to get libIscore as a dylib
 * Copyright © 2012, Théo de la Hogue, Raphael Marczak
 *
 * License: This code is licensed under the terms of the "CeCILL-C"
 * http://www.cecill.info
 */

#include "TTEngine.h"

#include <stdio.h>
#include <math.h>

#include <libxml/tree.h>
#include <libxml/parser.h>

#include "EnginesPrivate.hpp"
#include "StoryLine.hpp"

using namespace std;


/*!
 * \file TTEngine.cpp
 * \author Théo de la Hogue, based on Engine.cpp written by Raphael Marczak (LaBRI) for the libIscore.
 * \date 2012-2013
 */

#define thisTTClass			TTEngine
#define thisTTClassName		"Engine"
#define thisTTClassTags		"engine"

TT_SCORE_CONSTRUCTOR
{
    TTUInt32 maxSceneWidth;
    TTString plugginsLocation;
    
    maxSceneWidth = arguments[0];
    plugginsLocation = TTString(arguments[1]);
    
    initializeObjects(maxSceneWidth, plugginsLocation.c_str());
}

void TTEngine::initializeObjects(unsigned int maxSceneWidth, std::string plugginsLocation)
{
	m_implementation = new EnginesPrivate();
    
	m_implementation->m_editor = new Editor(maxSceneWidth);
	m_implementation->m_executionMachine = new ECOMachine();
    
	m_implementation->m_networkController = new DeviceManager("Virage");
	m_implementation->m_networkController->pluginLoad(plugginsLocation);
	m_implementation->m_networkController->deviceSetCurrent();
	m_implementation->m_networkController->namespaceSetAddCallback(this, &receiveNetworkMessageCallBack);
    
	m_implementation->m_networkController->pluginLaunch();
    
	m_implementation->m_executionMachine->addWaitedTriggerPointMessageAction(this, &triggerPointStateCallBack);
	m_implementation->m_executionMachine->addIsECOFinishedAction(this, &executionFinishedCallBack);
    
	m_implementation->m_enginesUpdateWithNetworkMessageAction = NULL;
}

TTEngine::~TTEngine()
{
	delete m_implementation->m_executionMachine;
	delete m_implementation->m_editor;
	delete m_implementation->m_networkController;
}

ECOMachine* TTEngine::getExecutionMachine()
{
	return m_implementation->m_executionMachine;
}

Editor* TTEngine::getEditor()
{
	return m_implementation->m_editor;
}

void TTEngine::reset(unsigned int maxSceneWidth)
{
	delete m_implementation->m_executionMachine;
	delete m_implementation->m_editor;
    
	m_implementation->m_editor = new Editor(maxSceneWidth);
	m_implementation->m_executionMachine = new ECOMachine();
    
	m_implementation->m_executionMachine->addWaitedTriggerPointMessageAction(this, &triggerPointStateCallBack);
	m_implementation->m_executionMachine->addIsECOFinishedAction(this, &executionFinishedCallBack);
}

void TTEngine::reset(void(*crossAction)(unsigned int, unsigned int, std::vector<unsigned int>),
					void(*triggerAction)(bool, unsigned int, unsigned int, unsigned int, std::string),
					unsigned int maxSceneWidth)
{
	reset(maxSceneWidth);
    
	if (crossAction != NULL) {
		addCrossingCtrlPointCallback(crossAction);
	}
    
	if (triggerAction != NULL) {
		addCrossingTrgPointCallback(triggerAction);
	}
}

void deprecated(std::string a, std::string b)
{
	std::cout << std::endl;
	std::cout << "**** " << a << " DEPECRATED ! Please use " << b << " ****" ;
	std::cout << std::endl << std::endl;
}

void TTEngine::addCrossingCtrlPointCallback(void(*pt2Func)(unsigned int, unsigned int, std::vector<unsigned int>))
{
	getExecutionMachine()->addCrossAControlPointAction(pt2Func);
}

void TTEngine::removeCrossingCtrlPointCallback()
{
	getExecutionMachine()->removeCrossAControlPointAction();
}

void TTEngine::addCrossingTrgPointCallback(void(*pt2Func)(bool, unsigned int, unsigned int, unsigned int, std::string))
{
	m_implementation->m_waitedTriggerPointMessageAction = pt2Func;
}

void TTEngine::removeCrossingTrgPointCallback()
{
	getExecutionMachine()->removeWaitedTriggerPointMessageAction();
}

void TTEngine::addExecutionFinishedCallback(void(*pt2Func)())
{
	m_implementation->m_isECOMachineFinish = pt2Func;
}

void TTEngine::removeExecutionFinishedCallback()
{
	m_implementation->m_executionMachine->removeIsECOFinishedAction();
}

void TTEngine::addEnginesNetworkUpdateCallback(void(*pt2Func)(unsigned int, std::string, std::string))
{
	m_implementation->m_enginesUpdateWithNetworkMessageAction = pt2Func;
}

void TTEngine::removeEnginesNetworkUpdateCallback()
{
	m_implementation->m_enginesUpdateWithNetworkMessageAction = NULL;
}




// Edition ////////////////////////////////////////////////////////////////////////

unsigned int TTEngine::addBox(int boxBeginPos, int boxLength, unsigned int motherId)
{
	unsigned int newBoxId = getEditor()->addBox(boxBeginPos, boxLength, motherId);
    
	if (newBoxId != NO_ID) {
		getExecutionMachine()->newProcess(PROCESS_TYPE_NETWORK_MESSAGE_TO_SEND, newBoxId, m_implementation->m_networkController);
	}
    
	return newBoxId;
}

unsigned int TTEngine::addBox(int boxBeginPos, int boxLength, unsigned int motherId, int minBound, int maxBound)
{
	unsigned int newBoxId = addBox(boxBeginPos, boxLength, motherId);
    
	if (newBoxId != NO_ID) {
		changeBoxBounds(newBoxId, minBound, maxBound);
	}
    
	return newBoxId;
}

void TTEngine::changeBoxBounds(unsigned int boxId, int minBound, int maxBound)
{
	std::vector<unsigned int> movedBoxes;
	unsigned int flexibilityRelationID = getEditor()->addAntPostRelation(boxId, BEGIN_CONTROL_POINT_INDEX, boxId, END_CONTROL_POINT_INDEX, ANTPOST_ANTERIORITY, minBound, maxBound, movedBoxes);
    
	if (flexibilityRelationID != NO_ID) {
		ConstrainedBox* currentConstrainedBox = getEditor()->getBoxById(boxId);
        
		if (currentConstrainedBox->getFlexibilityRelationId() != NO_ID) {
			removeTemporalRelation(currentConstrainedBox->getFlexibilityRelationId());
		}
        
		currentConstrainedBox->setFlexibilityInformations(flexibilityRelationID, minBound, maxBound);
	}
}

void TTEngine::removeBox(unsigned int boxId)
{
	getEditor()->removeBox(boxId);
    
	getExecutionMachine()->removeProcess(boxId);
}

void TTEngine::setBoxOptionalArgument(unsigned int boxId, std::string key, std::string value)
{
	getEditor()->setOptionalArgument(boxId, key, value);
}

void TTEngine::removeBoxOptionalArgument(unsigned int boxId, std::string key)
{
	getEditor()->removeOptionalArgument(boxId, key);
}

std::map<std::string, std::string> TTEngine::getBoxOptionalArguments(unsigned int boxId)
{
	return getEditor()->getOptionalArguments(boxId);
}

unsigned int TTEngine::addTemporalRelation(unsigned int boxId1,
										  unsigned int controlPoint1,
										  unsigned int boxId2,
										  unsigned int controlPoint2,
										  TemporalRelationType type,
										  int minBound,
										  int maxBound,
										  vector<unsigned int>& movedBoxes)
{
	if (boxId1 == boxId2) {
		return NO_ID;
	}
    
	return getEditor()->addAntPostRelation(boxId1, controlPoint1, boxId2, controlPoint2, type, minBound, maxBound, movedBoxes);
}

unsigned int TTEngine::addTemporalRelation(unsigned int boxId1,
										  unsigned int controlPoint1,
										  unsigned int boxId2,
										  unsigned int controlPoint2,
										  TemporalRelationType type,
										  vector<unsigned int>& movedBoxes)
{
	return addTemporalRelation(boxId1, controlPoint1, boxId2, controlPoint2, type, NO_BOUND, NO_BOUND, movedBoxes);
}

void TTEngine::changeTemporalRelationBounds(unsigned int relationId, int minBound, int maxBound, vector<unsigned int>& movedBoxes)
{
	getEditor()->changeAntPostRelationBounds(relationId, minBound, maxBound, movedBoxes);
}

bool TTEngine::isTemporalRelationExisting(unsigned int boxId1, unsigned int controlPoint1Index, unsigned int boxId2, unsigned int controlPoint2Index)
{
	return getEditor()->isAntPostRelationAlreadyExist(boxId1, controlPoint1Index, boxId2, controlPoint2Index);
}



void TTEngine::removeTemporalRelation(unsigned int relationId)
{
	getEditor()->removeTemporalRelation(relationId);
}

unsigned int TTEngine::getRelationFirstBoxId(unsigned int relationId)
{
	return getEditor()->getRelationFirstBoxId(relationId);
}

unsigned int TTEngine::getRelationFirstCtrlPointIndex(unsigned int relationId)
{
	return getEditor()->getRelationFirstControlPointIndex(relationId);
}

unsigned int TTEngine::getRelationSecondBoxId(unsigned int relationId)
{
	return getEditor()->getRelationSecondBoxId(relationId);
}

unsigned int TTEngine::getRelationSecondCtrlPointIndex(unsigned int relationId)
{
	return getEditor()->getRelationSecondControlPointIndex(relationId);
}

int TTEngine::getRelationMinBound(unsigned int relationId)
{
	return getEditor()->getRelationMinBound(relationId);
}

int TTEngine::getRelationMaxBound(unsigned int relationId)
{
	return getEditor()->getRelationMaxBound(relationId);
}

bool TTEngine::performBoxEditing(unsigned int boxId, int x, int y, vector<unsigned int>& movedBoxes, unsigned int maxModification)
{
	return getEditor()->performMoving(boxId, x, y, movedBoxes, maxModification);
}

int TTEngine::getBoxBeginTime(unsigned int boxId)
{
	return getEditor()->getBeginValue(boxId);
}

int TTEngine::getBoxEndTime(unsigned int boxId)
{
	return getEditor()->getEndValue(boxId);
}

int TTEngine::getBoxDuration(unsigned int boxId)
{
	return getEditor()->getEndValue(boxId) - getEditor()->getBeginValue(boxId);
}

int TTEngine::getBoxNbCtrlPoints(unsigned int boxId)
{
	return getEditor()->nbControlPoint(boxId);
}

int TTEngine::getBoxFirstCtrlPointIndex(unsigned int boxId)
{
	return getEditor()->getFirstControlPointIndex(boxId);
}

int TTEngine::getBoxLastCtrlPointIndex(unsigned int boxId)
{
	return getEditor()->getLastControlPointIndex(boxId);
}

void TTEngine::setCtrlPointMessagesToSend(unsigned int boxId, unsigned int controlPointIndex, std::vector<std::string> messageToSend, bool muteState)
{
	ECOProcess* currentProcess = getExecutionMachine()->getProcess(boxId);
    
	if ((currentProcess->getType() == PROCESS_TYPE_NETWORK_MESSAGE_TO_SEND)) {
		SendNetworkMessageProcess* currentSendOSCProcess = (SendNetworkMessageProcess*) currentProcess;
		ControlPoint* currentControlPoint = getEditor()->getBoxById(boxId)->getControlPoint(controlPointIndex);
		currentSendOSCProcess->addMessages(messageToSend, currentControlPoint->getProcessStepId(), muteState);
	}
}

void TTEngine::getCtrlPointMessagesToSend(unsigned int boxId, unsigned int controlPointIndex, std::vector<std::string>& messages)
{
	ECOProcess* currentProcess = getExecutionMachine()->getProcess(boxId);
    
	if ((currentProcess->getType() == PROCESS_TYPE_NETWORK_MESSAGE_TO_SEND)) {
		SendNetworkMessageProcess* currentSendOSCProcess = (SendNetworkMessageProcess*) currentProcess;
		ControlPoint* currentControlPoint = getEditor()->getBoxById(boxId)->getControlPoint(controlPointIndex);
        
		currentSendOSCProcess->getMessages(messages, currentControlPoint->getProcessStepId());
	}
}

void TTEngine::removeCtrlPointMessagesToSend(unsigned int boxId, unsigned int controlPointIndex)
{
	ECOProcess* currentProcess = getExecutionMachine()->getProcess(boxId);
    
	if ((currentProcess->getType() == PROCESS_TYPE_NETWORK_MESSAGE_TO_SEND)) {
		SendNetworkMessageProcess* currentSendOSCProcess = (SendNetworkMessageProcess*) currentProcess;
		ControlPoint* currentControlPoint = getEditor()->getBoxById(boxId)->getControlPoint(controlPointIndex);
		currentSendOSCProcess->removeMessage(currentControlPoint->getProcessStepId());
	}
}

void TTEngine::setCtrlPointMutingState(unsigned int boxId, unsigned int controlPointIndex, bool mute)
{
	ECOProcess* currentProcess = getExecutionMachine()->getProcess(boxId);
    
	if ((currentProcess->getType() == PROCESS_TYPE_NETWORK_MESSAGE_TO_SEND)) {
		SendNetworkMessageProcess* currentSendOSCProcess = (SendNetworkMessageProcess*) currentProcess;
		ControlPoint* currentControlPoint = getEditor()->getBoxById(boxId)->getControlPoint(controlPointIndex);
		currentSendOSCProcess->setMessageMuteState(currentControlPoint->getProcessStepId(), mute);
	}
}

bool TTEngine::getCtrlPointMutingState(unsigned int boxId, unsigned int controlPointIndex)
{
	ECOProcess* currentProcess = getExecutionMachine()->getProcess(boxId);
    
	if ((currentProcess->getType() == PROCESS_TYPE_NETWORK_MESSAGE_TO_SEND)) {
		SendNetworkMessageProcess* currentSendOSCProcess = (SendNetworkMessageProcess*) currentProcess;
		ControlPoint* currentControlPoint = getEditor()->getBoxById(boxId)->getControlPoint(controlPointIndex);
		return currentSendOSCProcess->getMessageMuteState(currentControlPoint->getProcessStepId());
	}
    
	return false;
}

void TTEngine::setProcessMutingState(unsigned int boxId, bool mute)
{
	ECOProcess* currentProcess = getExecutionMachine()->getProcess(boxId);
    
	if ((currentProcess->getType() == PROCESS_TYPE_NETWORK_MESSAGE_TO_SEND)) {
		SendNetworkMessageProcess* currentSendOSCProcess = (SendNetworkMessageProcess*) currentProcess;
        
		currentSendOSCProcess->setCurvesMuteState(mute);
	}
}

bool TTEngine::getProcessMutingState(unsigned int boxId)
{
	ECOProcess* currentProcess = getExecutionMachine()->getProcess(boxId);
    
	if ((currentProcess->getType() == PROCESS_TYPE_NETWORK_MESSAGE_TO_SEND)) {
		SendNetworkMessageProcess* currentSendOSCProcess = (SendNetworkMessageProcess*) currentProcess;
        
		return currentSendOSCProcess->getCurvesMuteState();
	}
    
	return false;
}

void TTEngine::setBoxMutingState(unsigned int boxId, bool mute)
{
	ECOProcess* currentProcess = getExecutionMachine()->getProcess(boxId);
    
	if ((currentProcess->getType() == PROCESS_TYPE_NETWORK_MESSAGE_TO_SEND)) {
		SendNetworkMessageProcess* currentSendOSCProcess = (SendNetworkMessageProcess*) currentProcess;
        
		currentSendOSCProcess->setAllMessagesMuteState(mute);
		currentSendOSCProcess->setCurvesMuteState(mute);
	}
}


//CURVES ////////////////////////////////////////////////////////////////////////////////////

void TTEngine::addCurve(unsigned int boxId, const std::string & address)
{
	ECOProcess* currentProcess = getExecutionMachine()->getProcess(boxId);
    
	if ((currentProcess->getType() == PROCESS_TYPE_NETWORK_MESSAGE_TO_SEND)) {
		SendNetworkMessageProcess* currentSendOSCProcess = (SendNetworkMessageProcess*) currentProcess;
        
		currentSendOSCProcess->addCurves(address);
        
	}
}

void TTEngine::removeCurve(unsigned int boxId, const std::string & address)
{
	ECOProcess* currentProcess = getExecutionMachine()->getProcess(boxId);
    
	if ((currentProcess->getType() == PROCESS_TYPE_NETWORK_MESSAGE_TO_SEND)) {
		SendNetworkMessageProcess* currentSendOSCProcess = (SendNetworkMessageProcess*) currentProcess;
        
		currentSendOSCProcess->removeCurves(address);
        
	}
}

void TTEngine::clearCurves(unsigned int boxId)
{
	ECOProcess* currentProcess = getExecutionMachine()->getProcess(boxId);
    
	if ((currentProcess->getType() == PROCESS_TYPE_NETWORK_MESSAGE_TO_SEND)) {
		SendNetworkMessageProcess* currentSendOSCProcess = (SendNetworkMessageProcess*) currentProcess;
        
		currentSendOSCProcess->removeAllCurves();
	}
}

std::vector<std::string> TTEngine::getCurvesAddress(unsigned int boxId)
{
	ECOProcess* currentProcess = getExecutionMachine()->getProcess(boxId);
    
	std::vector<std::string> addressToReturn;
    
	if ((currentProcess->getType() == PROCESS_TYPE_NETWORK_MESSAGE_TO_SEND)) {
		SendNetworkMessageProcess* currentSendOSCProcess = (SendNetworkMessageProcess*) currentProcess;
        
		addressToReturn = currentSendOSCProcess->getCurvesAdress();
	} else {
		//TODO : exception
	}
    
	return addressToReturn;
}

void TTEngine::setCurveSampleRate(unsigned int boxId, const std::string & address, unsigned int nbSamplesBySec)
{
	ECOProcess* currentProcess = getExecutionMachine()->getProcess(boxId);
    
	if ((currentProcess->getType() == PROCESS_TYPE_NETWORK_MESSAGE_TO_SEND)) {
		SendNetworkMessageProcess* currentSendOSCProcess = (SendNetworkMessageProcess*) currentProcess;
        
		currentSendOSCProcess->setNbSamplesBySec(address, nbSamplesBySec);
	}
}

unsigned int TTEngine::getCurveSampleRate(unsigned int boxId, const std::string & address)
{
	ECOProcess* currentProcess = getExecutionMachine()->getProcess(boxId);
    
	unsigned int sampleRate = 0;
    
	if ((currentProcess->getType() == PROCESS_TYPE_NETWORK_MESSAGE_TO_SEND)) {
		SendNetworkMessageProcess* currentSendOSCProcess = (SendNetworkMessageProcess*) currentProcess;
        
		sampleRate = currentSendOSCProcess->getNbSamplesBySec(address);
	} else {
		//TODO : exception
	}
    
	return sampleRate;
}

void TTEngine::setCurveRedundancy(unsigned int boxId, const std::string & address, bool redondancy)
{
	ECOProcess* currentProcess = getExecutionMachine()->getProcess(boxId);
    
	if ((currentProcess->getType() == PROCESS_TYPE_NETWORK_MESSAGE_TO_SEND)) {
		SendNetworkMessageProcess* currentSendOSCProcess = (SendNetworkMessageProcess*) currentProcess;
		currentSendOSCProcess->setAvoidRedondanceInformation(address, !redondancy);
	}
}

bool TTEngine::getCurveRedundancy(unsigned int boxId, const std::string & address)
{
	ECOProcess* currentProcess = getExecutionMachine()->getProcess(boxId);
    
	bool curveRedundancyInformation = false;
    
	if ((currentProcess->getType() == PROCESS_TYPE_NETWORK_MESSAGE_TO_SEND)) {
		SendNetworkMessageProcess* currentSendOSCProcess = (SendNetworkMessageProcess*) currentProcess;
		curveRedundancyInformation = !currentSendOSCProcess->getAvoidRedondanceInformation(address);
	} else {
		//TODO : exception
	}
    
	return curveRedundancyInformation;
}

void TTEngine::setCurveMuteState(unsigned int boxId, const std::string & address, bool muteState)
{
	ECOProcess* currentProcess = getExecutionMachine()->getProcess(boxId);
    
	if ((currentProcess->getType() == PROCESS_TYPE_NETWORK_MESSAGE_TO_SEND)) {
		SendNetworkMessageProcess* currentSendOSCProcess = (SendNetworkMessageProcess*) currentProcess;
		currentSendOSCProcess->setCurveMuteStateInformation(address, muteState);
	}
}

bool TTEngine::getCurveMuteState(unsigned int boxId, const std::string & address)
{
	ECOProcess* currentProcess = getExecutionMachine()->getProcess(boxId);
    
	bool curveMuteState = false;
    
	if ((currentProcess->getType() == PROCESS_TYPE_NETWORK_MESSAGE_TO_SEND)) {
		SendNetworkMessageProcess* currentSendOSCProcess = (SendNetworkMessageProcess*) currentProcess;
		curveMuteState = currentSendOSCProcess->getCurveMuteStateInformation(address);
	} else {
		//TODO : exception
	}
    
	return curveMuteState;
}

void TTEngine::getCurveArgTypes(std::string stringToParse, std::vector<std::string>& result)
{
	result.clear();
    
	StringParser addressAndArgs(stringToParse);
    
	result.push_back(addressAndArgs.getAddress());
    
	for (unsigned int i = 0; i < addressAndArgs.getNbArg(); ++i) {
		StringParser::argType currentType = addressAndArgs.getType(i);
        
		if (currentType == StringParser::TYPE_FLOAT) {
			result.push_back("FLOAT");
		} else if (currentType == StringParser::TYPE_INT) {
			result.push_back("INT");
		} else if (currentType == StringParser::TYPE_STRING) {
			result.push_back("STRING");
		} else {
			result.push_back("SYMBOL");
		}
	}
}

bool TTEngine::setCurveSections(unsigned int boxId, std::string address, unsigned int argNb,
                               const std::vector<float> & percent, const std::vector<float> & y, const std::vector<short> & sectionType, const std::vector<float> & coeff)
{
	ECOProcess* currentProcess = getExecutionMachine()->getProcess(boxId);
    
	if ((currentProcess->getType() == PROCESS_TYPE_NETWORK_MESSAGE_TO_SEND)) {
		SendNetworkMessageProcess* currentSendOSCProcess = (SendNetworkMessageProcess*) currentProcess;
        
		return currentSendOSCProcess->setCurvesSections(address, argNb, percent, y, sectionType, coeff);
	} else {
		//TODO : exception
	}
    
	return false;
}

bool TTEngine::getCurveSections(unsigned int boxId, std::string address, unsigned int argNb,
                               std::vector<float> & percent,  std::vector<float> & y,  std::vector<short> & sectionType,  std::vector<float> & coeff)
{
	ECOProcess* currentProcess = getExecutionMachine()->getProcess(boxId);
    
	if ((currentProcess->getType() == PROCESS_TYPE_NETWORK_MESSAGE_TO_SEND)) {
		SendNetworkMessageProcess* currentSendOSCProcess = (SendNetworkMessageProcess*) currentProcess;
        
		return currentSendOSCProcess->getCurveSections(address, argNb, percent, y, sectionType, coeff);
	} else {
		//TODO : exception
	}
    
	return false;
}

bool TTEngine::getCurveValues(unsigned int boxId, const std::string & address, unsigned int argNb, std::vector<float>& result)
{
	result.clear();
	ECOProcess* currentProcess = getExecutionMachine()->getProcess(boxId);
    
	if ((currentProcess->getType() == PROCESS_TYPE_NETWORK_MESSAGE_TO_SEND)) {
		SendNetworkMessageProcess* currentSendOSCProcess = (SendNetworkMessageProcess*) currentProcess;
        
		return currentSendOSCProcess->getCurves(address, argNb, getBoxEndTime(boxId) - getBoxBeginTime(boxId), BEGIN_CONTROL_POINT_INDEX, END_CONTROL_POINT_INDEX, result);
	} else {
		return false;
	}
}





unsigned int TTEngine::addTriggerPoint(unsigned int containingBoxId)
{
	return getEditor()->addTriggerPoint(containingBoxId);
}

void TTEngine::removeTriggerPoint(unsigned int triggerId)
{
	//TODO: m_implementation->m_networkController->removeTriggerPointLeave(triggerId);
	getEditor()->removeTriggerPoint(triggerId);
}

bool TTEngine::assignCtrlPointToTriggerPoint(unsigned int triggerId, unsigned int boxId, unsigned int controlPointIndex)
{
	return getEditor()->setTriggerPointRelatedControlPoint(triggerId, boxId, controlPointIndex);
}

void TTEngine::freeTriggerPointFromCtrlPoint(unsigned int triggerId)
{
	getEditor()->removeTriggerPointRelatedControlPoint(triggerId);
}

void TTEngine::setTriggerPointMessage(unsigned int triggerId, std::string triggerMessage)
{
	//TODO: m_implementation->m_networkController->addTriggerPointLeave(triggerId, triggerMessage);
	getEditor()->setTriggerPointMessage(triggerId, triggerMessage);
}

std::string TTEngine::getTriggerPointMessage(unsigned int triggerId)
{
	return getEditor()->getTriggerPointMessage(triggerId);
}

unsigned int TTEngine::getTriggerPointRelatedBoxId(unsigned int triggerId)
{
	return getEditor()->getTriggerPointRelatedBoxId(triggerId);
}

unsigned int TTEngine::getTriggerPointRelatedCtrlPointIndex(unsigned int triggerId)
{
	return getEditor()->getTriggerPointRelatedControlPointIndex(triggerId);
}

void TTEngine::setTriggerPointMutingState(unsigned int triggerId, bool muteState)
{
	getEditor()->setTriggerPointMuteState(triggerId, muteState);
}

bool TTEngine::getTriggerPointMutingState(unsigned int triggerId)
{
	return getEditor()->getTriggerPointMuteState(triggerId);
}

void TTEngine::getBoxesId(vector<unsigned int>& boxesID)
{
	getEditor()->getAllBoxesId(boxesID);
}

void TTEngine::getRelationsId(vector<unsigned int>& relationsID)
{
	getEditor()->getAllAntPostRelationsId(relationsID);
}

void TTEngine::getTriggersPointId(vector<unsigned int>& triggersID)
{
	getEditor()->getAllTriggersId(triggersID);
}

//Execution ///////////////////////////////////////////////////////////
void TTEngine::setGotoValue(unsigned int gotoValue)
{
	getExecutionMachine()->setGotoInformation(gotoValue);
}

unsigned int TTEngine::getGotoValue()
{
	return getExecutionMachine()->getGotoInformation();
}

bool TTEngine::play()
{
	if (!isRunning()) {
		compile();
		return run();
        
	} else {
		return false;
	}
}

void TTEngine::pause(bool pauseValue)
{
	getExecutionMachine()->pause(pauseValue);
}

bool TTEngine::isPaused()
{
	return getExecutionMachine()->isPaused();
}

bool TTEngine::stop()
{
	return getExecutionMachine()->stop();
}

bool TTEngine::isRunning()
{
	return getExecutionMachine()->isRunning();
}

void TTEngine::compile()
{
	getExecutionMachine()->reset();
    
	std::map<unsigned int, StoryLine> hierarchyStoryLine;
    
	computeHierarchyStoryLine(ROOT_BOX_ID, getEditor()->getCSP(), hierarchyStoryLine);
    
    //	StoryLine storyLine(getEditor()->getCSP());
    
	if (hierarchyStoryLine[ROOT_BOX_ID].m_constrainedBoxes.size() >= 1) {
		getExecutionMachine()->compileECO(hierarchyStoryLine, getExecutionMachine()->getGotoInformation());
	}
}

bool TTEngine::run()
{
	bool runIsOk = getExecutionMachine()->run();
    
	return runIsOk;
}

unsigned int TTEngine::getCurrentExecutionTime()
{
	return getExecutionMachine()->getTimeInMs();
}

void TTEngine::setExecutionSpeedFactor(float factor)
{
	getExecutionMachine()->setSpeedFactor(factor);
}

float TTEngine::getExecutionSpeedFactor()
{
	return getExecutionMachine()->getSpeedFactor();
}

float TTEngine::getProcessProgression(unsigned int processId)
{
	if (getExecutionMachine()->isRunning()) {
		return getExecutionMachine()->getProcessProgressionPercent(processId);
	} else {
		return 0;
	}
}

void TTEngine::ignoreTriggerPointOnce()
{
	if (getExecutionMachine()->isRunning()) {
		getExecutionMachine()->ignoreTriggerPointForOneStep();
	}
}

bool TTEngine::receiveNetworkMessage(std::string netMessage)
{
	if (getExecutionMachine()->isRunning()) {
		return getExecutionMachine()->receiveNetworkMessage(netMessage);
	} else {
		return false;
	}
}

void TTEngine::simulateNetworkMessageReception(const std::string & netMessage)
{
	std::string value("");
	receiveNetworkMessageCallBack(this, netMessage, "", value);
    //	if (getExecutionMachine()->isRunning()) {
    //		getExecutionMachine()->receiveNetworkMessage(netMessage);
    //	}
}

void TTEngine::getLoadedNetworkPlugins(std::vector<std::string>& pluginsName, std::vector<unsigned int>& listeningPort)
{
	pluginsName = m_implementation->m_networkController->pluginGetLoadedByName();
    
	for (unsigned int i = 0; i < pluginsName.size(); ++i) {
		listeningPort.push_back(toInt(m_implementation->m_networkController->pluginGetCommParameter(pluginsName[i], "pluginReceptionPort")));
	}
}

void TTEngine::addNetworkDevice(const std::string & deviceName, const std::string & pluginToUse, const std::string & DeviceIp, const std::string & DevicePort)
{
	//m_implementation->m_networkController->addDevice(deviceName, pluginToUse, DeviceIp, DevicePort);
	std::map<std::string, std::string>* commParameters = new std::map<std::string, std::string>();
    
	(*commParameters)["ip"] = DeviceIp;
	(*commParameters)["port"] = DevicePort;
    
	m_implementation->m_networkController->deviceAdd(deviceName, pluginToUse, commParameters);
    
	delete commParameters;
    
}

void TTEngine::removeNetworkDevice(const std::string & deviceName)
{
	m_implementation->m_networkController->deviceRemove(deviceName);
}

void TTEngine::sendNetworkMessage(const std::string & stringToSend)
{
	m_implementation->m_networkController->deviceSendSetRequest(stringToSend);
}

void TTEngine::getNetworkDevicesName(std::vector<std::string>& devicesName, std::vector<bool>& couldSendNamespaceRequest)
{
	std::map<std::string, Device*> mapDevices = *(m_implementation->m_networkController->deviceGetCurrent());
    
	map<string, Device*>::iterator it = mapDevices.begin();
    
	while (it != mapDevices.end()) {
		if (m_implementation->m_networkController->deviceIsVisible(it->first)) {
			devicesName.push_back(it->first);
			couldSendNamespaceRequest.push_back(m_implementation->m_networkController->deviceUnderstandDiscoverRequest(it->first));
		}
        
		++it;
	}
}

std::vector<std::string> TTEngine::requestNetworkSnapShot(const std::string & address)
{
	return m_implementation->m_networkController->deviceSnapshot(address);
}

int TTEngine::requestNetworkNamespace(const std::string & address, vector<string>& nodes, vector<string>& leaves, vector<string>& attributs, vector<string>& attributsValue)
{
    //	int state = TIMEOUT_EXCEEDED;
    //	bool namespaceSent = true;
    
	return m_implementation->m_networkController->deviceSendDiscoverRequest(address, &nodes, &leaves, &attributs, &attributsValue);
    
    //	namespaceSent = m_implementation->m_networkController->sendNamespaceRequest(address);
    //
    //	if (namespaceSent){
    //		do  {
    //			usleep(50);
    //			state = m_implementation->m_networkController->getNamespaceRequestAnswer(address, nodes, leaves, attributs, attributsValue);
    //		} while (state == NO_ANSWER);
    //	}
    //
    //	return state;
}



// LOAD AND STORE

void TTEngine::store(std::string fileName)
{
	xmlDocPtr doc = NULL;
	xmlNodePtr root_node = NULL;
    
	doc = xmlNewDoc(BAD_CAST "1.0");
	root_node = xmlNewNode(NULL, BAD_CAST "ENGINES");
	xmlSetProp(root_node, BAD_CAST "version", BAD_CAST ENGINE_VERSION);
    
	std::ostringstream oss;
	oss << getEditor()->scenarioSize();
	xmlSetProp(root_node, BAD_CAST "scenarioSize", BAD_CAST oss.str().data());
    
	xmlDocSetRootElement(doc, root_node);
    
	getEditor()->store(root_node);
	getExecutionMachine()->store(root_node);
    
	xmlSaveFormatFileEnc(fileName.data(), doc, "UTF-8", 1);
    
	xmlFreeDoc(doc);
}

void TTEngine::load(std::string fileName)
{
	xmlNodePtr racine = NULL;
	xmlNodePtr n = NULL;
	xmlDocPtr doc = xmlReadFile(fileName.data(), "UTF-8", 0);
    
	racine = xmlDocGetRootElement(doc);
    
	xmlChar* version = xmlGetProp(racine, BAD_CAST "version");
	xmlChar* xmlScenarioSize = xmlGetProp(racine, BAD_CAST "scenarioSize");
    
	(void) version;
    
	int scenarioSize = XMLConversion::xmlCharToInt(xmlScenarioSize);
    
	reset(scenarioSize);
    
	for (n = racine->children; n != NULL; n = n->next) {
		if (n->type == XML_ELEMENT_NODE) {
			if (xmlStrEqual(n->name, BAD_CAST "EDITOR")) {
				std::cout << "LOAD: EDITOR" << std::endl;
				m_implementation->m_editor->load(n);
			} else if (xmlStrEqual(n->name, BAD_CAST "ECOMACHINE")) {
				std::cout << "LOAD: ECO-MACHINE" << std::endl;
				m_implementation->m_executionMachine->load(n, m_implementation->m_networkController);
			}
		}
	}
    
    
	xmlFreeDoc(doc);
}

void TTEngine::load(std::string fileName,
				   void(*crossAction)(unsigned int, unsigned int, std::vector<unsigned int>),
				   void(*triggerAction)(bool, unsigned int, unsigned int, unsigned int, std::string))
{
	load(fileName);
    
	if (crossAction != NULL) {
		addCrossingCtrlPointCallback(crossAction);
	}
    
	if (triggerAction != NULL) {
		addCrossingTrgPointCallback(triggerAction);
	}
}

void TTEngine::storeUsingAntoineFormat(std::string fileName)
{
    
}

void TTEngine::loadUsingAntoineFormat(std::string fileName)
{
    
}

void TTEngine::loadUsingAntoineFormat(std::string fileName,
                                     void(*crossAction)(unsigned int, unsigned int, std::vector<unsigned int>),
                                     void(*triggerAction)(bool, unsigned int, unsigned int, unsigned int, std::string))
{
	loadUsingAntoineFormat(fileName);
    
	if (crossAction != NULL) {
		addCrossingCtrlPointCallback(crossAction);
	}
    
	if (triggerAction != NULL) {
		addCrossingTrgPointCallback(triggerAction);
	}
}

// NETWORK



void TTEngine::print() {
	getExecutionMachine()->getPetriNet()->print();
}



void TTEngine::printExecutionInLinuxConsole()
{
	std::vector<unsigned int> boxesId;
    
	getBoxesId(boxesId);
    
	bool mustDisplay = true;
    
	while(isRunning()){
		if ((getCurrentExecutionTime()/60)%2 == 0) {
			if (mustDisplay) {
				std::system("clear");
				for (unsigned int i = 0; i < boxesId.size(); ++i) {
					unsigned int processPercent;
                    
					processPercent = getProcessProgression(boxesId[i]);
                    
					if ((processPercent > 0) && (processPercent < 99)) {
						std::cout << "[*";
                        
						for (unsigned j = 10; j <= processPercent; j += 10) {
							std::cout << "*";
						}
                        
						for (unsigned j = processPercent + 1; j <= 90; j += 10) {
							std::cout << ".";
						}
                        
						std::cout << "] -> (" << i << ") (" << processPercent << ")" << std::endl;
					}
				}
				mustDisplay = false;
			}
		} else {
			mustDisplay = true;
		}
		usleep(50);
	}
}

#if 0
#pragma mark -
#pragma mark Some Methods
#endif


void receiveNetworkMessageCallBack(void* arg, std::string message, std::string attribut, std::string& value)
{
	std::string play = "/Transport/Play";
	std::string stop = "/Transport/Stop";
	std::string pause = "/Transport/Pause";
	std::string rewind = "/Transport/Rewind";
	std::string startPoint = "/Transport/StartPoint";
	std::string speed = "/Transport/Speed";
    
	TTEnginePtr engines = (TTEnginePtr) arg;
    
	std::string argument = value;
    
	unsigned int action = UNKNOWN_MESSAGE;
    
	void (*updateAction)(unsigned int, std::string, std::string) = engines->m_implementation->m_enginesUpdateWithNetworkMessageAction;
    
	if (engines->receiveNetworkMessage(message)) {
		action = TRIGGER_MESSAGE;
	} else {
		std::string currentAddress = message;
        
		if (currentAddress == play) {
			engines->play();
			action = ENGINES_PLAY;
		} else if (currentAddress == stop) {
			engines->stop();
			action = ENGINES_STOP;
		} else if (currentAddress == pause) {
			engines->pause(!engines->isPaused());
			action = ENGINES_PAUSE_MODIFICATION;
		} else if (currentAddress == rewind) {
			engines->stop();
			engines->setGotoValue(0);
			action = ENGINES_REWIND;
		} else if (currentAddress == startPoint) {
			if (isFloat(argument)) {
				engines->setGotoValue(toInt(argument));
				action = ENGINES_GOTO_MODIFICATION;
			}
		} else if (currentAddress == speed) {
			if (isFloat(argument)) {
				float speedFactor = toFloat(argument);
				engines->setExecutionSpeedFactor(speedFactor);
				action = ENGINES_SPEED_MODIFICATION;
			}
		}
        
		if (updateAction != NULL) {
			updateAction(action, message, argument);
		}
	}
}

void triggerPointStateCallBack(void* arg, bool isWaited, unsigned int triggerId, unsigned int boxId, unsigned int controlPointIndex, std::string waitedString)
{
	TTEnginePtr engines = (TTEnginePtr) arg;
    
	if(isWaited) {
		//TODO: engines->m_implementation->m_networkController->setNamespaceValue(waitedString, 1 /* TRIGGER_WAITED */, engines->getBoxOptionalArguments(boxId));
	} else {
		//TODO: engines->m_implementation->m_networkController->setNamespaceValue(waitedString, 2 /* TRIGGER_PUSHED */, engines->getBoxOptionalArguments(boxId));
	}
    
	if (engines->m_implementation->m_waitedTriggerPointMessageAction != NULL) {
		engines->m_implementation->m_waitedTriggerPointMessageAction(isWaited, triggerId, boxId, controlPointIndex, waitedString);
	}
}

void executionFinishedCallBack(void* arg)
{
	TTEnginePtr engines = (TTEnginePtr) arg;
    
	//TODO: engines->m_implementation->m_networkController->resetTriggerPointStates();
    
	if (engines->m_implementation->m_isECOMachineFinish != NULL) {
		engines->m_implementation->m_isECOMachineFinish();
	}
    
}
