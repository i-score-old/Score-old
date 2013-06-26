/** @file
 *
 * @ingroup scoreLibrary
 *
 * @brief the Score library
 *
 * @see TTTimeEvent, TTTimeProcess
 *
 * @authors Théo de la Hogue & Clément Bossut
 *
 * @copyright Copyright © 2013, Théo de la Hogue & Clément Bossut @n
 * This code is licensed under the terms of the "CeCILL-C" @n
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
        TTTimeEvent::registerClass();
        TTTimeProcess::registerClass();
        
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