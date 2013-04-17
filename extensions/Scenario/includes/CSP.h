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

/**	\ingroup enums
 CSP Error Codes
 Enumeration of error codes that might be returned by any of CSP operations.	*/
enum CSPError {
	CSPErrorNone = 0,		///< No Error.
    CSPErrorOutOfBounds		///< TODO example of error CSP can return
};

/**	\ingroup enums
 CSP Change Codes
 Enumeration of error codes that might be returned by any of CSP operations.	*/
enum CSPReport {
	CSPReportNone = 0,      ///< No change.
    CSPReportChange         ///< TODO example of report CSP can return
};

// the callback type to get report from CSP
typedef void(*CSPReportCallback)(void*, CSPReport);

class CSP
{
    
public :
    
    CSP(void(*aCSPReportCallback)(void*, CSPReport));
    
    ~CSP();
    
    CSPError addProcess(void *pProcess, int start, int end, int max, int minBound = 0, int maxBound = 0); // by default, rigid, move to change
    
    CSPError removeProcess(void *pProcess);
    
    CSPError moveProcess(); // TODO : remember to check min < max when supple
    
    CSPError addInterval(void *pInterval); // by default, rigid, move to change
    
    CSPError removeInterval(void *pInterval);
    
    CSPError moveInterval(); // TODO : remember to check min < max when supple
    
private :
    
    Solver solver;
    
    CSPReportCallback *pCallback;
    
    unordered_map < void *, int * > varsMap; // unordered because wo don't have to iterate on the elements
};

#endif // __CSP_H__
