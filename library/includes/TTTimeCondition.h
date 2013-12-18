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
    
    /** Enable or disable the time condition to allow it to be tested
     @param	value           a boolean
     @return                kTTErrNone */
    TTErr           setReady(const TTValue& value);
    
    /** get all expressions symbol associated to each event
     @param	value           a value containing one expression per event
     @return                kTTErrNone */
    TTErr           getExpressions(TTValue& value);

    /** get all the events associated to the condition
     @param value           the time events objects
     @return                kTTErrNone */
    TTErr           getEvents(TTValue& value);
    
    /**  Add an event to the condition
     @param	inputValue      an event and optionnally the expression associated
     @param	outputValue     nothing
     @return                an error code if the operation fails */
    TTErr           EventAdd(const TTValue& inputValue, TTValue& outputValue);
    
    /**  Remove an event from the condition
     @param	inputValue      an event
     @param	outputValue     nothing
     @return                an error code if the operation fails */
    TTErr           EventRemove(const TTValue& inputValue, TTValue& outputValue);
    
    /**  Edit the expression associated to an event
     @param	inputValue      an event and the expression
     @param	outputValue     nothing
     @return                an error code if the operation fails */
    TTErr           EventExpression(const TTValue& inputValue, TTValue& outputValue);

    /**  Find an expression associated to an event
     @param	inputValue      an event
     @param	outputValue     an expression symbol
     @return                an error code if the operation fails */
    TTErr           ExpressionFind(const TTValue& inputValue, TTValue& outputValue);
    
    /** Test an expression
     @param inputvalue      an expression value or symbol
     @param outputvalue     the result as a boolean
     @return                an error code if the operation fails */
    TTErr           ExpressionTest(const TTValue& inputValue, TTValue& outputValue);
    
    /**  needed to be handled by a TTXmlHandler
     @param	inputValue      ..
     @param	outputValue     ..
     @return                .. */
	TTErr           WriteAsXml(const TTValue& inputValue, TTValue& outputValue);
	TTErr           ReadFromXml(const TTValue& inputValue, TTValue& outputValue);
    
    /** To be notified when an event date changed
     @param inputValue      the event which have changed his date
     @param outputValue     nothing
     @return                kTTErrNone */
    TTErr           EventDateChanged(const TTValue& inputValue, TTValue& outputValue);
    
    /** To be notified when an event status changed
     @param inputValue      the event which have changed his status
     @param outputValue     nothing
     @return                kTTErrNone */
    TTErr           EventStatusChanged(const TTValue& inputValue, TTValue& outputValue);
    
    /**  Helper functions to manage receivers : add a receiver for to the address if no receiver already exists
     @param	anAddress      an address to observe */
    void            addReceiver(TTAddress anAddress);
    
    /**  Helper functions to manage receivers : clean the receiver associated to the address if no other cases needs the address
     @param	anAddress      an observed address */
    void            cleanReceiver(TTAddress anAddress);
    
    friend TTErr TTSCORE_EXPORT TTTimeConditionReceiverReturnValueCallback(TTPtr baton, TTValue& data);
    
};

typedef TTTimeCondition* TTTimeConditionPtr;

/** The case receiver callback return back the value of observed address
 @param	baton               a time condition instance, an address
 @param	data                a value to test
 @return					an error code */
TTErr TTSCORE_EXPORT TTTimeConditionReceiverReturnValueCallback(TTPtr baton, TTValue& data);

#endif // __TT_TIME_CONDITION_H__
