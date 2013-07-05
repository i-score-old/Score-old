/** @file
 *
 * @ingroup scoreLibrary
 *
 * @brief a class to define an event
 *
 * @details The TTTimeEvent class allows to ... @n@n
 *
 * @see TTTimeProcess
 *
 * @authors Théo de la Hogue & Clément Bossut
 *
 * @copyright Copyright © 2013, Théo de la Hogue & Clément Bossut @n
 * This code is licensed under the terms of the "CeCILL-C" @n
 * http://www.cecill.info
 */

#ifndef __TT_TIME_EVENT_H__
#define __TT_TIME_EVENT_H__

#include "TTScore.h"

/**	a class to define an event
 
 The TTTimeEvent class allows to ...
 
 @see TTTimeProcess
 */
class TTSCORE_EXPORT TTTimeEvent : public TTObjectBase {
    
    TTCLASS_SETUP(TTTimeEvent)
    
    TTObjectBasePtr                 mContainer;                     ///< the container which handles the time event
    
public :

    TTUInt32                        mDate;                          ///< the date of the event
    
    TTObjectBasePtr                 mState;                         ///< a state handled by the event
    
    TTBoolean                       mInteractive;                   ///< is the time event interactive ?
    
    TTBoolean                       mReady;                         ///< is the time event ready to happen ?
 
private :
    
    TTAttributePtr                  dateAttribute;                  ///< cache date attribute for observer notification
    TTAttributePtr                  readyAttribute;                 ///< cache active attribute for observer notification
    TTMessagePtr                    happenMessage;                  ///< cache happen message for observer notification
    
    /** Set the date of the time event
     @param	value           a date
     @return                an error code if the date is wrong */
    TTErr           setDate(const TTValue& value);
    
    /** Enable or disable the interactive behaviour of the time event
     @param	value           a boolean
     @return                kTTErrNone */
    TTErr           setInteractive(const TTValue& value);
    
    /** Enable or disable the time event to allow it to happen
     @param	value           a boolean
     @return                kTTErrNone */
    TTErr           setReady(const TTValue& value);
    
    /** Try to make the event happen (possibility to use the scenario to check event validity)
     @return                an error code returned by the trigger method */
    TTErr           Trigger();
    
    /** Make the event happen
     @return                an error code returned by the happen method */
    TTErr           Happen();
    
    /**  needed to be handled by a TTXmlHandler
     @param	inputValue      ..
     @param	outputValue     ..
     @return                .. */
	TTErr           WriteAsXml(const TTValue& inputValue, TTValue& outputValue);
	TTErr           ReadFromXml(const TTValue& inputValue, TTValue& outputValue);

    /** Get a line value of the state for an address
        this method eases the access of one state value
     @param	inputValue      an address
     @param	outputValue     the value of the state for an address
     @return                kTTErrNone */
    TTErr           StateAddressGetValue(const TTValue& inputValue, TTValue& outputValue);
};

typedef TTTimeEvent* TTTimeEventPtr;

/** Define an unordered map to store and retreive a value relative to a TTTimeEventPtr */
#ifdef TT_PLATFORM_WIN
    #include <hash_map>
    using namespace stdext;	// Visual Studio 2008 puts the hash_map in this namespace
    typedef hash_map<TTTimeEventPtr,TTValuePtr>    TTTimeEventMap;
#else
//	#ifdef TT_PLATFORM_LINUX
//  at least for GCC 4.6 on the BeagleBoard, the unordered map is standard
    #include <unordered_map>
//	#else
//		#include "boost/unordered_map.hpp"
//		using namespace boost;
//	#endif
    typedef std::unordered_map<TTTimeEventPtr,TTValuePtr>	TTTimeEventMap;
#endif

typedef	TTTimeEventMap*                   TTTimeEventMapPtr;
typedef TTTimeEventMap::const_iterator	TTTimeEventMapIterator;

#endif // __TIME_EVENT_H__