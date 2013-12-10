/** @file
 *
 * @ingroup scoreLibrary
 *
 * @brief the Score library
 *
 * @details The Score library allows to ... @n@n
 *
 * @see TTTimeEvent, TTTimeProcess
 *
 * @authors Théo de la Hogue & Clément Bossut
 *
 * @copyright Copyright © 2013, Théo de la Hogue & Clément Bossut @n
 * This code is licensed under the terms of the "CeCILL-C" @n
 * http://www.cecill.info
 */

#ifndef __TT_SCORE_H__
#define __TT_SCORE_H__

#define TTSCORE_VERSION_STRING "0.1"
#define TTSCORE_XML_ENCODING "ISO-8859-1"

#ifdef TT_PLATFORM_WIN
#include "windows.h"
	#ifdef TTSCORE_EXPORTS
		#define TTSCORE_EXPORT __declspec(dllexport)
	#else
	#ifdef TTSTATIC
		#define TTSCORE_EXPORT
	#else
		#define TTSCORE_EXPORT __declspec(dllimport)
	#endif
	#endif // _DLL_EXPORT

#else // TT_PLATFORM_MAC
	#ifdef TTSCORE_EXPORTS
		#define TTSCORE_EXPORT __attribute__((visibility("default")))
	#else
		#define TTSCORE_EXPORT
	#endif
#endif

#include <math.h>
#include <unistd.h>
#include <map>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlreader.h>

#include "TTFoundationAPI.h"
#include "TTModular.h"

#include "TTScoreSymbolCache.h"

#include "TTScore.test.h"

// Prototypes

/** Initialize Score framework */
void TTScoreInit();

#endif // __TT_SCORE_H__
