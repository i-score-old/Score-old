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

/**	\ingroup enums
 CSP Error Codes
 Enumeration of error codes that might be returned by any of CSP operations.	*/
enum CSPError {
	CSPErrorNone = 0,		///< No Error
    CSPErrorGeneric,        ///< ...
    CSPErrorOutOfBounds		///< ...
};

/* a type to define values handled by the CSP   */
typedef unsigned int CSPValue;

/* a type to define a function type to get report back from CSP.     */
typedef void(*CSPReportFunction)(void*, CSPValue);

class CSP
{
    
public :
    
    CSP(void(*CSPReportFunction)(void*, CSPValue));
    
    ~CSP();
    
    CSPError addProcess(void *pStartObject, void *pEndObject, CSPValue start, CSPValue end, CSPValue max, CSPValue minBound = 0, CSPValue maxBound = 0); // by default, rigid, move to change
    
    CSPError removeProcess(void *pStartObject, void *pEndObject);
    
    CSPError moveProcess(void *pStartObject, void *pEndObject, CSPValue newStart, CSPValue newEnd); // TODO : remember to check min < max when supple
    
    CSPError addInterval(void *pStartObject, void *pEndObject); // by default, rigid, move to change
    
    CSPError removeInterval(void *pStartObject, void *pEndObject);
    
    CSPError moveInterval(void *pStartObject, void *pEndObject, CSPValue newStart, CSPValue newEnd); // TODO : remember to check min < max when supple
    
private :
    
    Solver                          mSolver;
    
    CSPReportFunction               mCallback;
    
    CSPElementMap                   mVariablesMap;                    // unordered because wo don't have to iterate on the elements
    
    CSPElementMap                   mProcessConstraintsMap;           // unordered because wo don't have to iterate on the elements
    
    CSPElementMap                   mIntervalConstraintsMap;           // unordered because wo don't have to iterate on the elements
};

#endif // __CSP_H__
