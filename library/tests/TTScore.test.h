/* 
 * Unit tests for the Score framework
 * Copyright © 2013, Théo de la Hogue
 * 
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#ifndef __TT_SCORETEST_H__
#define __TT_SCORETEST_H__

#include "TTDataObjectBase.h"
#include "TTUnitTest.h"

/**	Provide unit tests for Score framework */
class TTScoreTest : public TTDataObjectBase {
	TTCLASS_SETUP(TTScoreTest)
		
	virtual TTErr test(TTValue& returnedTestInfo);
};


#endif // __TT_SCORETEST_H__
