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

#include "TTSymbolTable.h"
#include "TTScore.h"
#include "TTScoreSymbolCache.h"

// object classe name
TTSCORE_EXPORT TTSymbol         kTTSym_TimeEvent                (("TimeEvent"));

// notifications
TTSCORE_EXPORT TTSymbol         kTTSym_ConditionReadyChanged    (("ConditionReadyChanged"));
TTSCORE_EXPORT TTSymbol         kTTSym_EventDateChanged         (("EventDateChanged"));
TTSCORE_EXPORT TTSymbol         kTTSym_EventDisposed            (("EventDisposed"));
TTSCORE_EXPORT TTSymbol         kTTSym_EventHappened            (("EventHappened"));
TTSCORE_EXPORT TTSymbol         kTTSym_EventReadyChanged        (("EventReadyChanged"));
TTSCORE_EXPORT TTSymbol         kTTSym_ProcessStarted           (("ProcessStarted"));
TTSCORE_EXPORT TTSymbol         kTTSym_ProcessEnded             (("ProcessEnded"));

// message name
TTSCORE_EXPORT TTSymbol         kTTSym_case                     (("case"));
TTSCORE_EXPORT TTSymbol         kTTSym_color                    (("color"));
TTSCORE_EXPORT TTSymbol         kTTSym_Compile                  (("Compile"));
TTSCORE_EXPORT TTSymbol         kTTSym_compiled                 (("compiled"));
TTSCORE_EXPORT TTSymbol         kTTSym_condition                (("condition"));
TTSCORE_EXPORT TTSymbol         kTTSym_date                     (("date"));
TTSCORE_EXPORT TTSymbol         kTTSym_Dispose                  (("Dispose"));
TTSCORE_EXPORT TTSymbol         kTTSym_duration                 (("duration"));
TTSCORE_EXPORT TTSymbol         kTTSym_durationMin              (("durationMin"));
TTSCORE_EXPORT TTSymbol         kTTSym_durationMax              (("durationMax"));
TTSCORE_EXPORT TTSymbol         kTTSym_endDate                  (("endDate"));
TTSCORE_EXPORT TTSymbol         kTTSym_event                    (("event"));
TTSCORE_EXPORT TTSymbol         kTTSym_expression               (("expression"));
TTSCORE_EXPORT TTSymbol         kTTSym_Goto                     (("Goto"));
TTSCORE_EXPORT TTSymbol         kTTSym_Happen                   (("Happen"));
TTSCORE_EXPORT TTSymbol         kTTSym_Limit                    (("Limit"));
TTSCORE_EXPORT TTSymbol         kTTSym_Move                     (("Move"));
TTSCORE_EXPORT TTSymbol         kTTSym_Pause                    (("Pause"));
TTSCORE_EXPORT TTSymbol         kTTSym_Process                  (("Process"));
TTSCORE_EXPORT TTSymbol         kTTSym_ProcessStart             (("ProcessStart"));
TTSCORE_EXPORT TTSymbol         kTTSym_ProcessEnd               (("ProcessEnd"));
TTSCORE_EXPORT TTSymbol         kTTSym_ready                    (("ready"));
TTSCORE_EXPORT TTSymbol         kTTSym_redundancy               (("redundancy"));
TTSCORE_EXPORT TTSymbol         kTTSym_Resume                   (("Resume"));
TTSCORE_EXPORT TTSymbol         kTTSym_recorded                 (("recorded"));
TTSCORE_EXPORT TTSymbol         kTTSym_rigid                    (("rigid"));
TTSCORE_EXPORT TTSymbol         kTTSym_sampled                  (("sampled"));
TTSCORE_EXPORT TTSymbol         kTTSym_samples                  (("samples"));
TTSCORE_EXPORT TTSymbol         kTTSym_speed                    (("speed"));
TTSCORE_EXPORT TTSymbol         kTTSym_startDate                (("startDate"));
TTSCORE_EXPORT TTSymbol         kTTSym_Trigger                  (("Trigger"));
TTSCORE_EXPORT TTSymbol         kTTSym_verticalPosition         (("verticalPosition"));
TTSCORE_EXPORT TTSymbol         kTTSym_verticalSize             (("verticalSize"));
TTSCORE_EXPORT TTSymbol         kTTSym_viewPosition             (("viewPosition"));
TTSCORE_EXPORT TTSymbol         kTTSym_viewZoom                 (("viewZoom"));

