#ifndef CCOMMANDLINEARGS_H
#define CCOMMANDLINEARGS_H

#include "Polycode.h"
#include <vector>

using namespace Polycode;

class CCommandLineArgs;
extern CCommandLineArgs* globalCommandLineArgs;

class CCommandLineArgs
{
public:
	CCommandLineArgs(const String &commandString);
	CCommandLineArgs();

	bool startFullscreen;
	bool showDebugConsole;
};

#endif	// CCOMMANDLINEARGS_H