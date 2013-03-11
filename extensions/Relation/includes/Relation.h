/*
 * Relation time process
 * Copyright © 2013, Théo de la Hogue
 *
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

/*!
 * \class Relation
 *
 *  Relation time process class
 *
 */

#ifndef __RELATION_H__
#define __RELATION_H__

#include "TimeProcess.h"

class Relation : public TimeProcess
{
	TTCLASS_SETUP(Relation)
	
private :
       
	TTObjectBasePtr				mFrom;                  ///< ATTRIBUTE : the time process on which the relation is connected from
    TTObjectBasePtr				mTo;                    ///< ATTRIBUTE : the time process on which the relation is connected to
	
    /** Get parameters names needed by this time process
     @param	value           the returned parameter names
     @return                kTTErrNone */
	TTErr getParameterNames(TTValue& value);
    
    /** Specific process method on start
     @return                an error code returned by the process end method */
    TTErr ProcessStart();
    
    /** Specific process method on end
     @return                an error code returned by the process end method */
    TTErr ProcessEnd();
    
    /** Specific process method
     @return                an error code returned by the process method */
    TTErr Process();
    
    /** Set the the time process on which the relation is connected from
     @param	value           a time process
     @return                an error code if the relation can't be done */
    TTErr	setFrom(const TTValue& value);
    
    /** Set the the time process on which the relation is connected to
     @param	value           a time process
     @return                an error code if the relation can't be done */
    TTErr	setTo(const TTValue& value);
    
	/**  needed to be handled by a TTXmlHandler
     @param	inputValue      ..
     @param	outputValue     ..
     @return                .. */
	TTErr	WriteAsXml(const TTValue& inputValue, TTValue& outputValue);
	TTErr	ReadFromXml(const TTValue& inputValue, TTValue& outputValue);
	
	/**  needed to be handled by a TTTextHandler
     @param	inputValue      ..
     @param	outputValue     ..
     @return                .. */
	TTErr	WriteAsText(const TTValue& inputValue, TTValue& outputValue);
	TTErr	ReadFromText(const TTValue& inputValue, TTValue& outputValue);
};

typedef Relation* RelationPtr;

#endif // __RELATION_H__
