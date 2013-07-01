/** @file
 *
 * @ingroup scoreLibrary
 *
 * @brief A symbol cache for Score library
 *
 * @see TTImeEvent, TTTimeProcess
 *
 * @authors Théo de la Hogue & Clément Bossut
 *
 * @copyright Copyright © 2013, Théo de la Hogue & Clément Bossut @n
 * This code is licensed under the terms of the "CeCILL-C" @n
 * http://www.cecill.info
 */

#include "TTValueCache.h"
#include "TTSymbolTable.h"
#include "TTScore.h"
#include "TTScoreSymbolCache.h"

// object classe name
TTSCORE_EXPORT TTSymbol       kTTSym_TimeEvent                  (("TimeEvent"));

// message name
TTSCORE_EXPORT TTSymbol       kTTSym_Happen                     (("Happen"));
TTSCORE_EXPORT TTSymbol       kTTSym_Trigger                    (("Trigger"));
