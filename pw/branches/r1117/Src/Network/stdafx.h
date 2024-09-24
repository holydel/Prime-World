#ifndef STDAFX_H_C3814A10_F8C5_4212_8EC9
#define STDAFX_H_C3814A10_F8C5_4212_8EC9

#include "System/systemStdAfx.h"
#include "specific.h"

FILE _iob[] = { *stdin, *stdout, *stderr };
extern "C" FILE* __cdecl __iob_func(void) { return _iob; }

#endif //#define STDAFX_H_C3814A10_F8C5_4212_8EC9
