/** @file
 *
 * @ingroup scoreLibrary
 *
 * @brief Score library
 *
 * @details
 *
 * @authors Théo de la Hogue & Clément Bossut
 *
 * @copyright Copyright © 2013, Théo de la Hogue & Clément Bossut @n
 * This code is licensed under the terms of the "CeCILL-C" @n
 * http://www.cecill.info
 */

#ifndef __TT_SCORE_INCLUDES_H__
#define __TT_SCORE_INCLUDES_H__

#define TTSCORE_VERSION_STRING "0.2"
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

#include "TTScore.h"

#include "Expression.h"
#include "TTTimeEvent.h"
#include "TTTimeProcess.h"
#include "TTTimeCondition.h"
#include "TTTimeContainer.h"


#include "TTScore.test.h"

#endif // __TT_SCORE_INCLUDES_H__
