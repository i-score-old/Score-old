/** @file
 *
 * @ingroup scoreExtension
 *
 * @brief Automation time process class manage interpolation between the start event state and end event state depending on the scheduler progression
 *
 * @see TimePluginLib, TTTimeProcess
 *
 * @authors Théo de la Hogue & Clément Bossut
 *
 * @copyright Copyright © 2013, Théo de la Hogue & Clément Bossut @n
 * This code is licensed under the terms of the "CeCILL-C" @n
 * http://www.cecill.info
 */

#include "Automation.h"

#define thisTTClass          Automation
#define thisTTClassName      "Automation"
#define thisTTClassTags      "time, process, automation"

extern "C" TT_EXTENSION_EXPORT TTErr TTLoadJamomaExtension_Automation(void)
{
	TTFoundationInit();
	Automation::registerClass();
    Curve::registerClass();
	return kTTErrNone;
}

TIME_PROCESS_CONSTRUCTOR
{
    TIME_PLUGIN_INITIALIZE
    
	TT_ASSERT("Correct number of args to create Automation", arguments.size() == 0);
    
    registerAttribute(TTSymbol("curveAddresses"), kTypeLocalValue, NULL, (TTGetterMethod)& Automation::getCurveAddresses);
    
    addMessageWithArguments(CurveAdd);
    addMessageWithArguments(CurveGet);
    addMessageWithArguments(CurveUpdate);
    addMessageWithArguments(CurveRemove);
    addMessage(Clear);
    addMessageWithArguments(CurveRecord);
    
    mScheduler->setAttributeValue(TTSymbol("granularity"), TTFloat64(1.));
}

Automation::~Automation()
{
    Clear();
}

TTErr Automation::getParameterNames(TTValue& value)
{
    value.clear();
	//value.append(TTSymbol("aParameterName"));
	
	return kTTErrNone;
}

TTErr Automation::getCurveAddresses(TTValue& value)
{
    return mCurves.getKeys(value);
}

TTErr Automation::Compile()
{
    TTValue         duration, keys, objects, none;
    TTSymbol        key;
    TTObjectBasePtr curve;
    TTUInt32        i, j;
    
    // get the current duration
    getAttributeValue(kTTSym_duration, duration);
    
    // sample all curves for the current duration
    mCurves.getKeys(keys);
    
    for (i = 0; i < keys.size(); i++) {
        
        key = keys[i];
        mCurves.lookup(key, objects);
        
        // sample each indexed curves
        for (j = 0; j < objects.size(); j++) {
            
            curve = objects[j];
            curve->sendMessage(TTSymbol("Sample"), duration, none);
        }
    }
    
    // compilation done
    mCompiled = YES;
    
    return kTTErrNone;
}

TTErr Automation::ProcessStart()
{
    TTValue         v, keys, objects, vStart, none;
    TTSymbol        key;
    TTObjectBasePtr curve;
    TTObjectBasePtr aReceiver;
    TTUInt32        i, j;
    TTErr           err;
    
    // set curves on the first sample and prepare new curves to record the address value
    mCurves.getKeys(keys);

    for (i = 0; i < keys.size(); i++) {
        
        key = keys[i];
        mCurves.lookup(key, objects);
        
        // set each indexed curves on the first sample
        for (j = 0; j < objects.size(); j++) {
            
            curve = objects[j];
            CurvePtr(curve)->begin();
        }
        
        // a curve with a receiver is recording
        if (!mReceivers.lookup(key, v)) {
            
            aReceiver = v[0];
            
            // delete all indexed curves
            for (j = 0; j < objects.size(); j++) {
                
                curve = objects[i];
                
                TTObjectBaseRelease(&curve);
            }
            
            // get the current value to update the start state
            if (!aReceiver->sendMessage(TTSymbol("Grab"), v, vStart)) {
                
                // set the start event state value for this address
                v = key;
                v.append(TTPtr(&vStart));
                getStartEvent()->sendMessage(TTSymbol("StateAddressSetValue"), v, none);
                
                // create as many indexed curves as the vStart size
                err = kTTErrNone;
                objects.resize(vStart.size());
                
                for (j = 0; j < vStart.size(); j++) {
                    
                    curve = NULL;
                    err = TTObjectBaseInstantiate(TTSymbol("Curve"), &curve, none);
                    
                    if (!err) {
                        
                        // store the first point
                        CurvePtr(curve)->append(TTValue(0., vStart[j]));
                        
                        // index the curve
                        objects[j] = curve;
                    }
                    else
                        break;
                }
                
                if (!err) {
                    
                    // register all the curves for this address
                    mCurves.remove(key);
                    mCurves.append(key, objects);
                }
            }
        }
    }
    
    return kTTErrNone;
}

TTErr Automation::ProcessEnd()
{
    TTValue         v, keys, objects, vEnd, out;
    TTSymbol        key;
    TTObjectBasePtr curve;
    TTObjectBasePtr aReceiver;
    TTUInt32        i, j;
    TTBoolean       change = NO;
    
    // edit last point of each recording curves
    mReceivers.getKeys(keys);
    for (i = 0; i < keys.size(); i++) {
        
        key = keys[i];
        mReceivers.lookup(key, v);
        aReceiver = v[0];
        
        // get the current value to update the end state
        if (!aReceiver->sendMessage(TTSymbol("Grab"), v, vEnd)) {
            
            // set the end event state value for this address
            v = key;
            v.append(TTPtr(&vEnd));
            getEndEvent()->sendMessage(TTSymbol("StateAddressSetValue"), v, out);
            
            // update each indexed curves
            if (!mCurves.lookup(key, objects)) {
            
                for (j = 0; j < objects.size(); j++) {
                    
                    curve = objects[j];
                    
                    // store the last point
                    CurvePtr(curve)->append(TTValue(1., vEnd[j]));
                    
                    // set the curve in record mode
                    curve->setAttributeValue(kTTSym_recorded, YES);
                    
                    // set the curve as already sampled
                    curve->setAttributeValue(kTTSym_sampled, YES);
                    
                    // some parameters have changed
                    change = YES;
                }
            }
        }
    }
    
    // is the last compilation still valid ?
    mCompiled = !change && mCompiled;

    return kTTErrNone;
}

TTErr Automation::Process(const TTValue& inputValue, TTValue& outputValue)
{
    TTFloat64       progression, realTime, sample;
    TTValue         v, keys, objects, valueToSend, none;
    TTSymbol        key;
    TTAddress       address;
    TTObjectBasePtr curve, sender;
    TTUInt32        i, j;
    TTBoolean       redundancy;
	TTErr			err;
    
    if (inputValue.size() == 2) {
        
        if (inputValue[0].type() == kTypeFloat64 && inputValue[1].type() == kTypeFloat64) {
            
            progression = inputValue[0];
            realTime = inputValue[1];
            
            // store current progression for recording
            mCurrentProgression = progression;
            
            // don't process for 0. or 1. to not send the same value twice
            if (progression == 0. || progression == 1.)
                return kTTErrGeneric;
            
            // calculate the curves
            mCurves.getKeys(keys);
            for (i = 0; i < keys.size(); i++) {
                
                key = keys[i];
                                
                // a curve is processed only if it is not recording
                if (!mReceivers.lookup(key, objects))
                    continue;
                
                mCurves.lookup(key, objects);

                // process each indexed curve to fill the value to send
                valueToSend.clear();
                valueToSend.resize(objects.size());
                err = kTTErrNone;
                redundancy = YES;
                for (j = 0; j < objects.size(); j++) {
                    
                    curve = objects[j];
                    
                    err = CurvePtr(curve)->nextSampleAt(progression, sample);
                    
                    // if no value
                    if (err == kTTErrValueNotFound)
                        break;
                    
                    redundancy &= err == kTTErrGeneric;
                    
                    valueToSend[j] = sample;
                }
                
                // if no value
                if (err == kTTErrValueNotFound || redundancy)
                    continue;
                
                // look for the sender at the address
                if (!mSenders.lookup(key, objects)) {
                    
                    sender = objects[0];
                    sender->sendMessage(kTTSym_Send, valueToSend, none);
                }
            }
        }
    }

    return kTTErrGeneric;
}

TTErr Automation::ProcessPaused(const TTValue& inputValue, TTValue& outputValue)
{
    // théo : what do do on pause/resume ?
    return kTTErrNone;
}

TTErr Automation::Goto(const TTValue& inputValue, TTValue& outputValue)
{
    TTUInt32        duration, timeOffset;
    TTFloat64       progression, realTime;
    TTValue         v, keys, objects, none;
    TTSymbol        key;
    TTObjectBasePtr curve;
    TTUInt32        i, j;
    TTBoolean       mute = NO;
    
    if (inputValue.size() >= 1) {
        
        if (inputValue[0].type() == kTypeUInt32) {
            
            this->getAttributeValue(kTTSym_duration, v);
            
            // TODO : TTTimeProcess should extend Scheduler class
            duration = v[0];
            mScheduler->setAttributeValue(kTTSym_duration, TTFloat64(duration));
            
            timeOffset = inputValue[0];
            mScheduler->setAttributeValue(kTTSym_offset, TTFloat64(timeOffset));
            
            // is the automation is temporary muted ?
            if (inputValue.size() == 2) {
                
                if (inputValue[1].type() == kTypeBoolean) {
                    
                    mute = inputValue[1];
                }
            }
            
            if (!mute && !mMute) {
                
                // get scheduler progression and realTime
                mScheduler->getAttributeValue(TTSymbol("progression"), v);
                progression = TTFloat64(v[0]);
                
                mScheduler->getAttributeValue(TTSymbol("realTime"), v);
                realTime = TTFloat64(v[0]);
                
                // reset each curves on its first sample
                mCurves.getKeys(keys);
                
                for (i = 0; i < keys.size(); i++) {
                    
                    key = keys[i];
                    mCurves.lookup(key, objects);
                    
                    for (j = 0; j < objects.size(); j++) {
                        
                        curve = objects[j];
                        
                        CurvePtr(curve)->begin();
                    }
                }
                
                v = progression;
                v.append(realTime);
                
                return Process(v, none);
            }
            else
                return kTTErrNone;
        }
    }
    
    return kTTErrGeneric;
}

TTErr Automation::WriteAsXml(const TTValue& inputValue, TTValue& outputValue)
{
	TTXmlHandlerPtr	aXmlHandler = NULL;
    TTValue         v, keys;
    TTSymbol        name, key;
    TTUInt32        i;
	
	aXmlHandler = TTXmlHandlerPtr((TTObjectBasePtr)inputValue[0]);
    
    // Write the curves
    mCurves.getKeys(keys);
    for (i = 0; i < keys.size(); i++) {
        
        key = keys[i];
        mCurves.lookup(key, v);
        
        xmlTextWriterStartElement((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "indexedCurves");
        
        // Write the curve's address
        xmlTextWriterWriteAttribute((xmlTextWriterPtr)aXmlHandler->mWriter, BAD_CAST "address", BAD_CAST key.c_str());
        
        // write each indexed curve
        aXmlHandler->setAttributeValue(kTTSym_object, v);
        aXmlHandler->sendMessage(TTSymbol("Write"));
        
        xmlTextWriterEndElement((xmlTextWriterPtr)aXmlHandler->mWriter);
    }
    	
	return kTTErrNone;
}

TTErr Automation::ReadFromXml(const TTValue& inputValue, TTValue& outputValue)
{
	TTXmlHandlerPtr	aXmlHandler = NULL;
    TTObjectBasePtr curve = NULL;
    TTAddress       address;
    TTValue         v, none, duration;
    TTErr           err;
    
	aXmlHandler = TTXmlHandlerPtr((TTObjectBasePtr)inputValue[0]);
    
    // If there are indexed curves
    if (aXmlHandler->mXmlNodeName == TTSymbol("indexedCurves")) {
        
        if (aXmlHandler->mXmlNodeStart) {
            
            mCurrentObjects.clear();
            
        }
        else {
            
            // Get the address
            if (!aXmlHandler->getXmlAttribute(kTTSym_address, v, YES)) {
                
                if (v.size() == 1) {
                    
                    if (v[0].type() == kTypeSymbol) {
                        
                        address = v[0];
                        
                        // store the curve
                        mCurves.append(address, mCurrentObjects);
                        
                        // add a sender
                        addSender(address);
                    }
                }
            }
        }
    }
    
    if (aXmlHandler->mXmlNodeName == TTSymbol("curve")) {
        
        // get the current duration
        getAttributeValue(kTTSym_duration, duration);
        
        TTObjectBaseInstantiate(TTSymbol("Curve"), TTObjectBaseHandle(&curve), none);
        
        mCurrentObjects.append(curve);
        
        // Pass the xml handler to the current curve to fill his data structure
        v = TTObjectBasePtr(curve);
        aXmlHandler->setAttributeValue(kTTSym_object, v);
        err = aXmlHandler->sendMessage(kTTSym_Read);
        
        // Sample the curve to be ready to process it
        if (!err)
            curve->sendMessage(TTSymbol("Sample"), duration, v);
        
        return err;
    }
	
	return kTTErrGeneric;
}

TTErr Automation::WriteAsText(const TTValue& inputValue, TTValue& outputValue)
{
	TTTextHandlerPtr	aTextHandler;
	
	aTextHandler = TTTextHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
	// TODO : write the curves
	
	return kTTErrGeneric;
}

TTErr Automation::ReadFromText(const TTValue& inputValue, TTValue& outputValue)
{
	TTTextHandlerPtr aTextHandler;
	TTValue	v;
	
	aTextHandler = TTTextHandlerPtr((TTObjectBasePtr)inputValue[0]);
	
    // TODO : parse the curves
	
	return kTTErrGeneric;
}

TTErr Automation::CurveAdd(const TTValue& inputValue, TTValue& outputValue)
{
    TTValue         v, vStart, vEnd, parameters, objects, none;
    TTAddress       address;
    TTObjectBasePtr curve, sender;
    TTBoolean       valid = YES;
    TTErr           err;
    TTUInt32        i;
    
    if (inputValue.size() == 1) {
        
        if (inputValue[0].type() == kTypeSymbol) {
            
            address = inputValue[0];
            
            // if there is no curve at the address
            if (mCurves.lookup(address, v)) {
                
                // get the start event state value for this address
                getStartEvent()->sendMessage(TTSymbol("StateAddressGetValue"), address, vStart);
                
                // get the end event state value for this address
                getEndEvent()->sendMessage(TTSymbol("StateAddressGetValue"), address, vEnd);
                
                // check values size : we can't make curve for none equal sized values
                if (vStart.size() != vEnd.size())
                    return kTTErrGeneric;
                
                // check start and end value validity : we can't make a curve for a symbol
                for (i = 0; i < vStart.size(); i++)
                    valid &= vStart[i].type() != kTypeSymbol;
                
                for (i = 0; i < vEnd.size(); i++)
                    valid &= vEnd[i].type() != kTypeSymbol;
                
                if (!valid)
                    return kTTErrGeneric;

                // create a curve for each index of the value
                err = kTTErrNone;
                objects.resize(vStart.size());
                
                for (i = 0; i < vStart.size(); i++) {
                    
                    curve = NULL;
                    err = TTObjectBaseInstantiate(TTSymbol("Curve"), &curve, none);
                    
                    if (!err) {
                        
                        // prepare curve parameters
                        parameters.resize(6);
                        parameters[0] = TTFloat64(0.);
                        parameters[1] = TTFloat64(vStart[i]);
                        parameters[2] = TTFloat64(1);
                        
                        parameters[3] = TTFloat64(1.);
                        parameters[4] = TTFloat64(vEnd[i]);
                        parameters[5] = TTFloat64(1);
                        
                        curve->setAttributeValue(TTSymbol("functionParameters"), parameters);
                        
                        // index the curve
                        objects[i] = curve;
                    }
                    else
                        break;
                    
                }
                
                if (!err) {
                    
                    // register all the curves for this address
                    mCurves.append(address, objects);
                    
                    // add a sender for the curve
                    addSender(address);
                    
                    // the last compilation is not valid
                    mCompiled = NO;
                }
                
                return err;
            }
        }
    }
    
    return kTTErrGeneric;
}

TTErr Automation::CurveGet(const TTValue& inputValue, TTValue& outputValue)
{
    TTAddress  address;

    if (inputValue.size() == 1) {
        
        if (inputValue[0].type() == kTypeSymbol) {
            
            address = inputValue[0];
            
            mCurves.lookup(address, outputValue);
            
            if (outputValue.size())
                return kTTErrNone;
        }
    }
    
    return kTTErrGeneric;
}

TTErr Automation::CurveUpdate(const TTValue& inputValue, TTValue& outputValue)
{
    TTValue         v, vStart, vEnd, parameters, objects, none;
    TTAddress       address;
    TTObjectBasePtr curve;
    TTUInt32        i;
    TTBoolean       change = NO;
    
    // update all curves
    if (inputValue.size() == 0) {
        
        TTValue         keys;
        TTSymbol        key;
        
        // update all curves
        mCurves.getKeys(keys);
        for (i = 0; i < keys.size(); i++) {
            
            key = keys[i];
            CurveUpdate(key, none);
        }
        
        return kTTErrNone;
    }
    
    if (inputValue.size() == 1) {
        
        if (inputValue[0].type() == kTypeSymbol) {
            
            address = inputValue[0];
            
            // if there is a curve at the address
            if (!mCurves.lookup(address, objects)) {
                
                // get the start event state value for this address
                getStartEvent()->sendMessage(TTSymbol("StateAddressGetValue"), address, vStart);
                
                // get the end event state value for this address
                getEndEvent()->sendMessage(TTSymbol("StateAddressGetValue"), address, vEnd);
                
                // check values size : can't update curves for none equal sized values
                if (vStart.size() != vEnd.size())
                    return kTTErrGeneric;
                
                if (objects.size() != vStart.size())
                    return kTTErrGeneric;
                
                // update each indexed curve
                for (i = 0; i < objects.size(); i++) {
                    
                    curve = objects[i];
                    
                    // get current curve parameters
                    if (!curve->getAttributeValue(TTSymbol("functionParameters"), parameters)) {
                        
                        // change the first point y value : x1 y1 b1 ...
                        parameters[1] = TTFloat64(vStart[i]);
                        
                        // TODO : scale the other y points value ?
                        
                        // change the last point y value : ... xn yn bn
                        parameters[parameters.size() - 2] = TTFloat64(vEnd[i]);
                        
                        // set current curve parameters
                        curve->setAttributeValue(TTSymbol("functionParameters"), parameters);
                        
                        // some parameters have changed
                        change = YES;
                    }
                }
                
                // is the last compilation still valid ?
                mCompiled = !change && mCompiled;
                
                return kTTErrNone;
            }
        }
    }
    
    return kTTErrGeneric;
}

TTErr Automation::CurveRemove(const TTValue& inputValue, TTValue& outputValue)
{
    TTValue         v, objects;
    TTAddress       address;
    TTObjectBasePtr curve;
    
    if (inputValue.size() == 1) {
        
        if (inputValue[0].type() == kTypeSymbol) {
            
            address = inputValue[0];
            
            // if there is a curve at the address
            if (!mCurves.lookup(address, objects)) {
                
                // delete each indexed curve
                for (TTUInt32 i = 0; i < objects.size(); i++) {
                
                    curve = objects[i];

                    TTObjectBaseRelease(&curve);
                }
                
                // unregister the curve
                mCurves.remove(address);
                
                // remove sender
                removeSender(address);
                
                // remove receiver
                removeReceiver(address);

                return kTTErrNone;
            }
        }
    }
    
    return kTTErrGeneric;
}

TTErr Automation::Clear()
{
    TTValue         keys,objects;
    TTSymbol        key;
    TTUInt32        i;
    TTObjectBasePtr curve;
    
    // delete all curves
    mCurves.getKeys(keys);
    for (i = 0; i < keys.size(); i++) {
        
        key = keys[i];
        mCurves.lookup(key, objects);
        
        // delete each indexed curve
        for (TTUInt32 i = 0; i < objects.size(); i++) {
            
            curve = objects[i];
            
            TTObjectBaseRelease(&curve);
        }
        
        // remove sender
        removeSender(TTAddress(key));
        
        // remove receiver
        removeReceiver(TTAddress(key));
    }
    
    mCurves.clear();
    
    return kTTErrNone;
}

TTErr Automation::CurveRecord(const TTValue& inputValue, TTValue& outputValue)
{
    TTValue   v, objects, out;
    TTAddress address;
    TTBoolean record;
    
    if (inputValue.size() == 2) {
        
        if (inputValue[0].type() == kTypeSymbol && inputValue[1].type() == kTypeBoolean) {
            
            address = inputValue[0];
            record = inputValue[1];
            
            // if there is no curve at the address
            if (mCurves.lookup(address, objects)) {
                
                // register the address with no curves
                mCurves.append(address, objects);
                
                // add a sender
                addSender(address);
            }
            
            // add or remove receiver for the address
            if (record) {

                addReceiver(address);
                
                // we also need to clear the start and the end state for this address
                // otherwise its value will be recalled on start or on end
                getStartEvent()->sendMessage(TTSymbol("StateAddressClear"), address, out);
                getEndEvent()->sendMessage(TTSymbol("StateAddressClear"), address, out);
            }
            else
                removeReceiver(address);
            
            return kTTErrNone;
        }
    }
    
    return kTTErrGeneric;
}

void Automation::addSender(TTAddress anAddress)
{
    TTObjectBasePtr aSender;
    TTValue         v, none;
    
    // if there is no sender for the address
    if (mSenders.lookup(anAddress, v)) {
        
        aSender = NULL;
        TTObjectBaseInstantiate(kTTSym_Sender, TTObjectBaseHandle(&aSender), none);
        
        // set the address of the sender
        aSender->setAttributeValue(kTTSym_address, anAddress);
        
        v = TTObjectBasePtr(aSender);
        mSenders.append(anAddress, v);
    }
}

void Automation::removeSender(TTAddress anAddress)
{
    TTObjectBasePtr aSender;
    TTValue         v;
    
    // remove the sender for this address
    if (!mSenders.lookup(anAddress, v)) {
        
        aSender = v[0];
        TTObjectBaseRelease(&aSender);
        
        mSenders.remove(anAddress);
    }
}

void Automation::addReceiver(TTAddress anAddress)
{
    TTObjectBasePtr aReceiver;
    TTObjectBasePtr aReceiverCallback;
    TTValuePtr      aReceiverBaton;
    TTValue         v, none;
    
    // if there is no receiver for the address
    if (mReceivers.lookup(anAddress, v)) {
        
        // No callback for the address
        v = TTValue((TTObjectBasePtr)NULL);
        
        // Create a receiver callback to get the expression address value back
        aReceiverCallback = NULL;
        TTObjectBaseInstantiate(TTSymbol("callback"), &aReceiverCallback, none);
        
        aReceiverBaton = new TTValue(TTObjectBasePtr(this));
        aReceiverBaton->append(anAddress);
        
        aReceiverCallback->setAttributeValue(kTTSym_baton, TTPtr(aReceiverBaton));
        aReceiverCallback->setAttributeValue(kTTSym_function, TTPtr(&AutomationReceiverReturnValueCallback));
        
        v.append(aReceiverCallback);
        
        aReceiver = NULL;
        TTObjectBaseInstantiate(kTTSym_Receiver, TTObjectBaseHandle(&aReceiver), v);
        
        // set the address of the receiver
        aReceiver->setAttributeValue(kTTSym_address, anAddress);
        
        v = TTObjectBasePtr(aReceiver);
        mReceivers.append(anAddress, v);
    }
}

void Automation::removeReceiver(TTAddress anAddress)
{
    TTObjectBasePtr aReceiver;
    TTValue v;
    
    // remove the receiver for this address
    if (!mReceivers.lookup(anAddress, v)) {
        
        aReceiver = v[0];
        TTObjectBaseRelease(&aReceiver);
        
        mReceivers.remove(anAddress);
    }
}

#if 0
#pragma mark -
#pragma mark Some Methods
#endif

TTErr AutomationReceiverReturnValueCallback(TTPtr baton, TTValue& data)
{
    TTValuePtr          b;
    AutomationPtr       anAutomation;
    TTAddress           anAddress;
    TTObjectBasePtr     curve;
    TTValue             v, objects;
    
    // unpack baton (automation, address)
    b = (TTValuePtr)baton;
    anAutomation = AutomationPtr(TTObjectBasePtr((*b)[0]));
    anAddress = (*b)[1];
    
    // if the automation is running
    if (anAutomation->mRunning) {
        
        // don't process when progression is equal to 0. or 1.
        if (anAutomation->mCurrentProgression == 0. || anAutomation->mCurrentProgression == 1.)
            return kTTErrNone;
        
        // for each event's expression matching the incoming address
        if (!anAutomation->mCurves.lookup(anAddress, objects)) {
        
            // for each indexed curves
            for (TTUInt32 i = 0; i < objects.size(); i++) {
                
                curve = objects[i];
                
                // store the next point
                CurvePtr(curve)->append(TTValue(anAutomation->mCurrentProgression, TTFloat64(data[i])));
            }
        }
    }
    
    return kTTErrNone;
}