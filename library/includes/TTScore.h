/* 
 * TTScore Library
 * Copyright © 2012, Théo de la Hogue
 * 
 * License: This code is licensed under the terms of the "CeCILL-C"
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

#include "TTFoundationAPI.h"

#include "TTScoreSymbolCache.h"

#include "CSPTypes.hpp"

#include "TTEngine.h"

// Macros

#define TT_SCORE_CONSTRUCTOR \
TTObjectPtr thisTTClass :: instantiate (TTSymbol& name, TTValue& arguments) {return new thisTTClass (arguments);} \
\
extern "C" void thisTTClass :: registerClass () {TTClassRegister( TT(thisTTClassName), thisTTClassTags, thisTTClass :: instantiate );} \
\
thisTTClass :: thisTTClass (TTValue& arguments) : TTDataObject(arguments)



#define TT_SCORE_CONSTRUCTOR_EXPORT \
	\
	extern "C" TT_EXTENSION_EXPORT TTErr loadTTExtension(void);\
	TTErr loadTTExtension(void)\
	{\
		TTSCOREInit();\
		thisTTClass :: registerClass(); \
		return kTTErrNone;\
	}\
	\
	TT_SCORE_CONSTRUCTOR

// Global

/** The main objects of TTScore */


// Prototypes

/** Init the Score library, and the Modular if needed */
void	TTSCORE_EXPORT	TTScoreInit();

#endif // __TT_SCORE_H__
