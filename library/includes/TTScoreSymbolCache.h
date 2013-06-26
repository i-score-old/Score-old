/** @file
 *
 * @ingroup scoreLibrary
 *
 * @brief A symbol cache for Score library
 *
 * @details The symbol sache allows to ... @n@n
 *
 * @see TTImeEvent, TTTimeProcess
 *
 * @authors Théo de la Hogue & Clément Bossut
 *
 * @copyright Copyright © 2013, Théo de la Hogue & Clément Bossut @n
 * This code is licensed under the terms of the "CeCILL-C" @n
 * http://www.cecill.info
 */

#ifndef __TT_SCORE_SYMBOL_CACHE_H__
#define __TT_SCORE_SYMBOL_CACHE_H__

#include "TTSymbol.h"

// object classe name
extern TTSCORE_EXPORT TTSymbol  kTTSym_TimeEvent;

// message name
extern TTSCORE_EXPORT TTSymbol	kTTSym_Happen;
extern TTSCORE_EXPORT TTSymbol	kTTSym_Trigger;

#endif // __TT_SCORE_SYMBOL_CACHE_H__

