/*
 * TTScore Library
 * Copyright © 2012, Théo de la Hogue
 *
 * License: This code is licensed under the terms of the "CeCILL-C"
 * http://www.cecill.info
 */


#include "TTScore.h"

// Statics and Globals
static bool TTScoreHasInitialized = false;

/****************************************************************************************************/

void TTScoreInit()
{	
	// Initialized Foundation framework
	TTFoundationInit();

	if (!TTScoreHasInitialized) {
		
		TTScoreHasInitialized = true;
		
		// register classes -- both internal and external
		TTScoreTest::registerClass();
		

#ifdef TT_DEBUG
		TTLogMessage("Score -- Version %s -- Debugging Enabled\n", TTSCORE_VERSION_STRING);
#else
		TTLogMessage("Score -- Version %s\n", TTSCORE_VERSION_STRING);
#endif

	}
}

#ifdef TT_PLATFORM_LINUX
int main(void)
{
	// TODO: should we call TTScoreInit() here?
	return 0;
}
#endif