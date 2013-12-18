/** @file
 *
 * @ingroup scoreExtension
 *
 * @brief Scenario Graph file defines specific methods to handle the PetriNet execution engine
 *
 * @details The Scenario Graph allows to ... @n@n
 *
 * @see Scenario, PetriNet
 *
 * @authors Théo de la Hogue & Clément Bossut
 *
 * @copyright Copyright © 2013, Théo de la Hogue & Clément Bossut @n
 * This code is licensed under the terms of the "CeCILL-C" @n
 * http://www.cecill.info
 */

#ifndef NO_EXECUTION_GRAPH

#ifndef __SCENARIO_GRAPH_H__
#define __SCENARIO_GRAPH_H__

#include "PetriNet.hpp"

typedef PetriNet* GraphPtr;
typedef Transition* TransitionPtr;

using namespace std;

/** A type to define an unordered map to store and retreive Solver objects */
#ifdef TT_PLATFORM_WIN
    #include <hash_map>
    using namespace stdext;	// Visual Studio 2008 puts the hash_map in this namespace
    typedef hash_map<void*, void*>              GraphObjectMap;
#else
    //	#ifdef TT_PLATFORM_LINUX
    // at least for GCC 4.6 on the BeagleBoard, the unordered map is standard
    #include <unordered_map>
    //	#else
    //		#include "boost/unordered_map.hpp"
    //		using namespace boost;
    //	#endif
    typedef std::unordered_map<void*, void*>	GraphObjectMap;
#endif

typedef	GraphObjectMap*                    GraphObjectMapPtr;
typedef GraphObjectMap::const_iterator     GraphObjectMapIterator;

#endif // __SCENARIO_GRAPH_H__

#endif // NO_EXECUTION_GRAPH
