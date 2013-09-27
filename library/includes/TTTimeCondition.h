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

/**	a class to define a condition and a set of different cases
 
 The TTTimeCondition class allows to ...
 
 @see TTTimeEvent
 */
class TTSCORE_EXPORT TTTimeCondition : public TTObjectBase {
    
    TTCLASS_SETUP(TTTimeCondition)
    
    friend class TTTimeEvent;
    friend class TTTimeContainer;
    
    TTObjectBasePtr                 mContainer;                     ///< the container which handles the condition
    
protected :
    
    TTSymbol                        mName;                          ///< the name of the condition
    
    TTBoolean                       mReady;                         ///< is the condition ready to be tested ?
    
    TTHash                          mReceivers;                     ///< a table of receivers stored by address
    TTHash                          mCases;                         ///< a table of events stored by expression ("address operator value"). (it is possible to have more than one event per expression)
 
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
    
    /**  Add a case to test using an expression 
     @param	inputValue      an expression value or symbol
     @param	outputValue     nothing
     @return                an error code if the operation fails */
    TTErr           CaseAdd(const TTValue& inputValue, TTValue& outputValue);
    
    /**  Remove a case to test using an expression
     @param	inputValue      an expression value or symbol
     @param	outputValue     nothing
     @return                an error code if the operation fails */
    TTErr           CaseRemove(const TTValue& inputValue, TTValue& outputValue);
    
    /**  Link an event to a case
     @param	inputValue      an expression symbol, an event
     @param	outputValue     nothing
     @return                an error code if the operation fails */
    TTErr           CaseLinkEvent(const TTValue& inputValue, TTValue& outputValue);
    
    /**  Unlink an event to a case
     @param	inputValue      an expression symbol, an event
     @param	outputValue     nothing
     @return                an error code if the operation fails */
    TTErr           CaseUnlinkEvent(const TTValue& inputValue, TTValue& outputValue);
    
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






/** Define an unordered map to store and retreive a value relative to a TTTimeConditionPtr */
#ifdef TT_PLATFORM_WIN
    #include <hash_map>
    using namespace stdext;	// Visual Studio 2008 puts the hash_map in this namespace
    typedef hash_map<TTTimeConditionPtr,TTValuePtr>    TTTimeConditionMap;
#else
//	#ifdef TT_PLATFORM_LINUX
//  at least for GCC 4.6 on the BeagleBoard, the unordered map is standard
    #include <unordered_map>
//	#else
//		#include "boost/unordered_map.hpp"
//		using namespace boost;
//	#endif
    typedef std::unordered_map<TTTimeConditionPtr,TTValuePtr>	TTTimeConditionMap;
#endif

typedef	TTTimeConditionMap*                 TTTimeConditionMapPtr;
typedef TTTimeConditionMap::const_iterator	TTTimeConditionMapIterator;

#endif // __TT_TIME_CONDITION_H__