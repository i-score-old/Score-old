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
TTSCORE_EXPORT TTSymbol         kTTSym_case                     (("case"));
TTSCORE_EXPORT TTSymbol         kTTSym_color                    (("color"));
TTSCORE_EXPORT TTSymbol         kTTSym_condition                (("condition"));
TTSCORE_EXPORT TTSymbol         kTTSym_date                     (("date"));
TTSCORE_EXPORT TTSymbol         kTTSym_duration                 (("duration"));
TTSCORE_EXPORT TTSymbol         kTTSym_durationMin              (("durationMin"));
TTSCORE_EXPORT TTSymbol         kTTSym_durationMax              (("durationMax"));
TTSCORE_EXPORT TTSymbol         kTTSym_event                    (("event"));
TTSCORE_EXPORT TTSymbol         kTTSym_expression               (("expression"));
TTSCORE_EXPORT TTSymbol         kTTSym_Happen                   (("Happen"));
TTSCORE_EXPORT TTSymbol         kTTSym_Pause                    (("Pause"));
TTSCORE_EXPORT TTSymbol         kTTSym_ready                    (("ready"));
TTSCORE_EXPORT TTSymbol         kTTSym_redundancy               (("redundancy"));
TTSCORE_EXPORT TTSymbol         kTTSym_Resume                   (("Resume"));
TTSCORE_EXPORT TTSymbol         kTTSym_rigid                    (("rigid"));
TTSCORE_EXPORT TTSymbol         kTTSym_Trigger                  (("Trigger"));
TTSCORE_EXPORT TTSymbol         kTTSym_verticalPosition         (("verticalPosition"));
TTSCORE_EXPORT TTSymbol         kTTSym_verticalSize             (("verticalSize"));

