#include "stdafx.h"

FILE _iob2[] = { *stdin, *stdout, *stderr };
extern "C" FILE* __cdecl __iob_func(void) { return _iob2; }