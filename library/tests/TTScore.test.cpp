/* 
 * Unit tests for the Score framework
 * Copyright © 2013, Théo de la Hogue
 * 
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#include "TTScore.test.h"

#define thisTTClass			TTScoreTest
#define thisTTClassName		"score.test"
#define thisTTClassTags		"test, score"

TT_OBJECT_CONSTRUCTOR
{;}

TTScoreTest::~TTScoreTest()
{;}

void TTScoreTestMain(int& errorCount, int& testAssertionCount)
{
	TTTestLog("\n");
	TTTestLog("Testing Score");
	
	    
    TTTestAssertion("TTScoreTest",
                    YES,
					testAssertionCount,
					errorCount);
}

// TODO: Benchmarking

TTErr TTScoreTest::test(TTValue& returnedTestInfo)
{
	int	errorCount = 0;
	int testAssertionCount = 0;
	
	TTScoreTestMain(errorCount, testAssertionCount);
	
	return TTTestFinish(testAssertionCount, errorCount, returnedTestInfo);
}

