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

// object classe names
extern TTSCORE_EXPORT TTSymbol  kTTSym_TimeEvent;

// notifications
extern TTSCORE_EXPORT TTSymbol  kTTSym_ConditionReadyChanged;
extern TTSCORE_EXPORT TTSymbol	kTTSym_EventDateChanged;
extern TTSCORE_EXPORT TTSymbol	kTTSym_EventStatusChanged;
extern TTSCORE_EXPORT TTSymbol	kTTSym_ProcessStarted;
extern TTSCORE_EXPORT TTSymbol	kTTSym_ProcessEnded;

// attribute, message or any names
extern TTSCORE_EXPORT TTSymbol	kTTSym_case;
extern TTSCORE_EXPORT TTSymbol	kTTSym_color;
extern TTSCORE_EXPORT TTSymbol	kTTSym_Compile;
extern TTSCORE_EXPORT TTSymbol	kTTSym_compiled;
extern TTSCORE_EXPORT TTSymbol	kTTSym_condition;
extern TTSCORE_EXPORT TTSymbol	kTTSym_date;
extern TTSCORE_EXPORT TTSymbol	kTTSym_Dispose;
extern TTSCORE_EXPORT TTSymbol	kTTSym_duration;
extern TTSCORE_EXPORT TTSymbol	kTTSym_durationMin;
extern TTSCORE_EXPORT TTSymbol	kTTSym_durationMax;
extern TTSCORE_EXPORT TTSymbol	kTTSym_endDate;
extern TTSCORE_EXPORT TTSymbol	kTTSym_event;
extern TTSCORE_EXPORT TTSymbol	kTTSym_eventDisposed;
extern TTSCORE_EXPORT TTSymbol	kTTSym_eventHappened;
extern TTSCORE_EXPORT TTSymbol	kTTSym_eventPending;
extern TTSCORE_EXPORT TTSymbol	kTTSym_eventWaiting;
extern TTSCORE_EXPORT TTSymbol	kTTSym_expression;
extern TTSCORE_EXPORT TTSymbol	kTTSym_Goto;
extern TTSCORE_EXPORT TTSymbol	kTTSym_Happen;
extern TTSCORE_EXPORT TTSymbol	kTTSym_Limit;
extern TTSCORE_EXPORT TTSymbol	kTTSym_Move;
extern TTSCORE_EXPORT TTSymbol	kTTSym_Pause;
extern TTSCORE_EXPORT TTSymbol	kTTSym_Process;
extern TTSCORE_EXPORT TTSymbol	kTTSym_ProcessStart;
extern TTSCORE_EXPORT TTSymbol	kTTSym_ProcessEnd;
extern TTSCORE_EXPORT TTSymbol  kTTSym_redundancy;
extern TTSCORE_EXPORT TTSymbol	kTTSym_Resume;
extern TTSCORE_EXPORT TTSymbol	kTTSym_recorded;
extern TTSCORE_EXPORT TTSymbol	kTTSym_rigid;
extern TTSCORE_EXPORT TTSymbol	kTTSym_sampled;
extern TTSCORE_EXPORT TTSymbol	kTTSym_samples;
extern TTSCORE_EXPORT TTSymbol	kTTSym_speed;
extern TTSCORE_EXPORT TTSymbol	kTTSym_startDate;
extern TTSCORE_EXPORT TTSymbol	kTTSym_status;
extern TTSCORE_EXPORT TTSymbol	kTTSym_Trigger;
extern TTSCORE_EXPORT TTSymbol	kTTSym_verticalPosition;
extern TTSCORE_EXPORT TTSymbol	kTTSym_verticalSize;
extern TTSCORE_EXPORT TTSymbol  kTTSym_viewPosition;
extern TTSCORE_EXPORT TTSymbol  kTTSym_viewZoom;

#endif // __TT_SCORE_SYMBOL_CACHE_H__

