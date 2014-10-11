/** @file
 *
 * @ingroup scoreExtension
 *
 * @brief Scenario Solver file defines data structure to interface a Gecode constrain solver
 *
 * @details The Scenario Solver allows to ... @n@n
 *
 * @see Scenario, Solver
 *
 * @authors Théo de la Hogue & Clément Bossut
 *
 * @copyright Copyright © 2013, Théo de la Hogue & Clément Bossut @n
 * This code is licensed under the terms of the "CeCILL-C" @n
 * http://www.cecill.info
 */

#ifndef NO_EDITION_SOLVER

#ifndef __SCENARIO_SOLVER_H__
#define __SCENARIO_SOLVER_H__

#include "solver.hpp"
#include "TTTimeEvent.h"

typedef Solver* SolverPtr;

using namespace std;

/** A type to define an unordered map to store and retrieve Solver objects */
#ifdef TT_PLATFORM_WIN
    #include <hash_map>
    using namespace stdext;	// Visual Studio 2008 puts the hash_map in this namespace
    typedef hash_map<void*, void*>              SolverObjectMap;
#else
    //	#ifdef TT_PLATFORM_LINUX
    // at least for GCC 4.6 on the BeagleBoard, the unordered map is standard
    #include <unordered_map>
    //	#else
    //		#include "boost/unordered_map.hpp"
    //		using namespace boost;
    //	#endif
    typedef std::unordered_map<void*, void*>	SolverObjectMap;
#endif

typedef	SolverObjectMap*                    SolverObjectMapPtr;
typedef SolverObjectMap::const_iterator     SolverObjectMapIterator;


/* a type to define values handled by the Solver   */
typedef unsigned int SolverValue;

/**	\ingroup enums
 Solver Error Codes
 Enumeration of error codes that might be returned by any of Solver operations	*/
enum SolverError {
	SolverErrorNone = 0,		///< No Error
    SolverErrorGeneric,         ///< ...
    SolverErrorOutOfBounds		///< ...
};

/**	\ingroup enums
 Solver Relation Types  */
enum SolverVariableType { DATE_VARIABLE = 1, RANGE_VARIABLE = 100 };

/**	\ingroup enums
 Solver Relation Types  */
enum SolverRelationType { EQ_RELATION = 0, NQ_RELATION = 1, LQ_RELATION = 2, LE_RELATION = 3, GQ_RELATION = 4, GR_RELATION = 5 };

/*!
 * \class SolverVariable
 *
 *
 *
 */
class SolverVariable
{
public:
    
    TTObject        event;
    
    SolverPtr       solver;
    int             dateID;
    int             rangeID;
    
    SolverVariable(SolverPtr aSolver, TTObject& anEvent, SolverValue max);
    
    ~SolverVariable();
    
    /** Get the variable value from the solver */
    SolverValue get();
    
    /** Set the range bounds of the variable */
    void limit(SolverValue min, SolverValue max);
    
    /** Update the variable value from the solver */
    void update();
};
typedef SolverVariable* SolverVariablePtr;

/*!
 * \class SolverConstraint
 *
 *
 *
 */
class SolverConstraint
{
public:
    
    SolverPtr           solver;
    int                 ID;
    
    SolverVariablePtr   startVariable;
    SolverVariablePtr   endVariable;
    
    SolverConstraint(SolverPtr aSolver, SolverVariablePtr variableA, SolverVariablePtr variableB, SolverValue durationMin, SolverValue durationMax, SolverValue max);
    
    ~SolverConstraint();
    
    /** Move a constraint into the solver (then each variable needs to be updated)
     
     @newStart              a new start date
     @newEnd                a new end date
     @return                an error code if movement fails */
    SolverError move(SolverValue newStart, SolverValue newEnd);
    
    /** Change duration bounds of the constraint (then each variable needs to be updated)
     
     @newStart              a new durationMin
     @newEnd                a new durationMax
     @return                an error code if update fails */
    SolverError limit(SolverValue newDurationMin, SolverValue newDurationMax);
};
typedef SolverConstraint* SolverConstraintPtr;

/*!
 * \class SolverRelation
 *
 *
 *
 */
class SolverRelation
{
public:
    
    SolverPtr           solver;
    int                 minBoundID;
    int                 maxBoundID;
    
    SolverVariablePtr   startVariable;
    SolverVariablePtr   endVariable;
    
    SolverRelation(SolverPtr aSolver, SolverVariablePtr variableA, SolverVariablePtr variableB, SolverValue durationMin=0, SolverValue durationMax=0);
    
    ~SolverRelation();
    
    /** Move a constraint into the solver (then each variable needs to be updated)
     
     @newStart              a new start date
     @newEnd                a new end date
     @return                an error code movement fails */
    SolverError move(SolverValue newStart, SolverValue newEnd);
    
    /** Change duration bounds of the relation (then each variable needs to be updated)
     
     @newStart              a new durationMin
     @newEnd                a new durationMax
     @return                an error code if update fails */
    SolverError limit(SolverValue newDurationMin, SolverValue newDurationMax);
};
typedef SolverRelation* SolverRelationPtr;


#endif // __SCENARIO_SOLVER_H__

#endif // NO_EDITION_SOLVER
