/** @file
 *
 * @ingroup scoreLibrary
 *
 * @brief a class to define a condition and a set of different cases
 *
 * @details The TTTimeCondition class allows to ... @n@n
 *
 * @see TTTimeEvent
 *
 * @authors Théo de la Hogue & Clément Bossut
 *
 * @copyright Copyright © 2013, Théo de la Hogue & Clément Bossut @n
 * This code is licensed under the terms of the "CeCILL-C" @n
 * http://www.cecill.info
 */

#ifndef __TT_TIME_CONDITION_H__
#define __TT_TIME_CONDITION_H__

#include "TTScore.h"
#include "Expression.h"
#include "TTTimeEvent.h"




/** Define an unordered map to store and retreive an expression relative to a TTTimeEventPtr */
#ifdef TT_PLATFORM_WIN
    #include <hash_map>
    using namespace stdext;	// Visual Studio 2008 puts the hash_map in this namespace
    typedef hash_map<TTTimeEventPtr,Expression>    TTCaseMap;
#else
//	#ifdef TT_PLATFORM_LINUX
//  at least for GCC 4.6 on the BeagleBoard, the unordered map is standard
    #include <unordered_map>
//	#else
//		#include "boost/unordered_map.hpp"
//		using namespace boost;
//	#endif
    typedef std::unordered_map<TTTimeEventPtr,Expression>	TTCaseMap;
#endif

typedef	TTCaseMap*                  TTCaseMapPtr;
typedef TTCaseMap::const_iterator   TTCaseMapIterator;

#endif // __TT_TIME_CONDITION_H__



/**	a class to define a condition and a set of different cases
 
 The TTTimeCondition class allows to ...
 
 @see TTTimeEvent
 */
class TTSCORE_EXPORT TTTimeCondition : public TTObjectBase {
    
    TTCLASS_SETUP(TTTimeCondition)
    
    friend class TTTimeEvent;
    
    TTObjectBasePtr                 mContainer;                     ///< the container which handles the condition
    
protected :
    
    TTSymbol                        mName;                          ///< the name of the condition
    
    TTBoolean                       mReady;                         ///< is the condition ready to be tested ?
    
    TTHash                          mReceivers;                     ///< a table of receivers stored by address
    TTCaseMap                       mCases;                         ///< a map linking an event to its expression
 
private :
    
    TTAttributePtr                  readyAttribute;                 ///< cache ready attribute for observer notification
    
    /** Enable or disable the time condition to allow it to be tested
     @param	value           a boolean
     @return                kTTErrNone */
    TTErr           setReady(const TTValue& value);
    
    /** get all cases expressions symbol
     @param	value           a value containing one expression per case
     @return                kTTErrNone */
    TTErr           getCases(TTValue& value);

    /** get all the events associated to the condition
     @param value           the time events objects
     @return                kTTErrNone */
    TTErr           getEvents(TTValue& value);
    
    /**  Add an event to the condition
     @param	inputValue      an event
     @param	outputValue     nothing
     @return                an error code if the operation fails */
    TTErr           EventAdd(const TTValue& inputValue, TTValue& outputValue);
    
    /**  Remove an event from the condition
     @param	inputValue      an event
     @param	outputValue     nothing
     @return                an error code if the operation fails */
    TTErr           EventRemove(const TTValue& inputValue, TTValue& outputValue);
    
    /**  Link an expression to an event
     @param	inputValue      an event and the expression to link
     @param	outputValue     nothing
     @return                an error code if the operation fails */
    TTErr           EventExpression(const TTValue& inputValue, TTValue& outputValue);

    /**  Helper functions to manage receivers */
    void            cleanReceiver(TTAddress addr);
    void            addReceiver(TTAddress addr);
    
    /**  Find a case related to an event
     @param	inputValue      an event
     @param	outputValue     an expression symbol
     @return                an error code if the operation fails */
    TTErr           CaseFind(const TTValue& inputValue, TTValue& outputValue);
    
    /** Test the case
     @param inputvalue      an expression value or symbol
     @param outputvalue     the result as a boolean
     @return                an error code if the operation fails */
    TTErr           CaseTest(const TTValue& inputValue, TTValue& outputValue);
    
    /**  needed to be handled by a TTXmlHandler
     @param	inputValue      ..
     @param	outputValue     ..
     @return                .. */
	TTErr           WriteAsXml(const TTValue& inputValue, TTValue& outputValue);
	TTErr           ReadFromXml(const TTValue& inputValue, TTValue& outputValue);
    
    friend TTErr TTSCORE_EXPORT TTTimeConditionReceiverReturnValueCallback(TTPtr baton, TTValue& data);
    
};

typedef TTTimeCondition* TTTimeConditionPtr;

/** The case receiver callback return back the value of observed address
 @param	baton               a time condition instance, an address
 @param	data                a value to test
 @return					an error code */
TTErr TTSCORE_EXPORT TTTimeConditionReceiverReturnValueCallback(TTPtr baton, TTValue& data);
