/*
 * A CSP container
 * Copyright © 2013, Théo de la Hogue, Clément Bossut
 *
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

/*!
 * \class CSP
 *
 *  The CSP class is a container for the constraint satisfaction problem which ask solutions to the solver
 *
 */

#ifndef __CSP_H__
#define __CSP_H__

#include "solver.hpp"

using namespace std;

/** A type to define an unordered map to store and retreive CSP element IDs relative to any object pointer */
#ifdef TT_PLATFORM_WIN
    #include <hash_map>
    using namespace stdext;	// Visual Studio 2008 puts the hash_map in this namespace
    typedef hash_map<void*, int*>    CSPElementMap;
#else
    //	#ifdef TT_PLATFORM_LINUX
    // at least for GCC 4.6 on the BeagleBoard, the unordered map is standard
    #include <unordered_map>
    //	#else
    //		#include "boost/unordered_map.hpp"
    //		using namespace boost;
    //	#endif
    typedef std::unordered_map<void*, int*>	CSPElementMap;
#endif

typedef	CSPElementMap*	CSPElementMapPtr;
typedef CSPElementMap::const_iterator	CSPElementMapIterator;



/* a type to define values handled by the CSP   */
typedef unsigned int CSPValue;

/* a type to define a function type to get report back from CSP.     */
typedef void(*CSPReportFunction)(void*, CSPValue);

class CSP
{
    
public :
    
    CSP(void(*CSPReportFunction)(void*, CSPValue));
    
    ~CSP();
    
    CSPError addProcess(void *pProcessObject, void *pStartObject, void *pEndObject, CSPValue start, CSPValue end, CSPValue max, CSPValue minBound = 0, CSPValue maxBound = 0); // by default, rigid, move to change
    
    CSPError removeProcess(void *pProcessObject, void *pStartObject, void *pEndObject);
    
    CSPError moveProcess(void *pStartObject, void *pEndObject, CSPValue newStart, CSPValue newEnd);
    
    CSPError addInterval(void *pProcessObject, void *pStartObject, void *pEndObject, CSPValue minBound = 0, CSPValue maxBound = 0);
    
    CSPError removeInterval(void *pProcessObject);
    
    CSPError moveInterval(void *pStartObject, void *pEndObject, CSPValue newStart, CSPValue newEnd);
    
private :
    
    Solver                          mSolver;
    
    CSPReportFunction               mCallback;
    
    CSPElementMap                   mVariablesMap;                    // unordered because wo don't have to iterate on the elements
    
    CSPElementMap                   mProcessConstraintsMap;           // unordered because wo don't have to iterate on the elements
    
    CSPElementMap                   mIntervalConstraintsMap;           // unordered because wo don't have to iterate on the elements
};

#endif // __CSP_H__
