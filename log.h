#if !defined(HAVE_LOG)
#define HAVE_LOG

#include "common.h"


void printToStderr(char *format, ... );

#if defined(DEBUG)
#define TRACE(format, ...) printToStderr(format, __VA_ARGS__)
#define TRACE_0(format) printToStderr(format)
#define TRACE_1(format,a1) printToStderr(format,a1)
#define TRACE_2(format,a1,a2) printToStderr(format,a1,a2)
#define TRACE_3(format,a1,a2,a3) printToStderr(format,a1,a2,a3)
#define TRACE_4(format,a1,a2,a3,a4) printToStderr(format,a1,a2,a3,a4)
#define TRACE_5(format,a1,a2,a3,a4,a5) printToStderr(format,a1,a2,a3,a4,a5)
#define TRACE_6(format,a1,a2,a3,a4,a5,a6) printToStderr(format,a1,a2,a3,a4,a5,a6)
#define TRACE_7(format,a1,a2,a3,a4,a5,a6,a7) printToStderr(format,a1,a2,a3,a4,a5,a6,a7)
#define TRACE_8(format,a1,a2,a3,a4,a5,a6,a7,a8) printToStderr(format,a1,a2,a3,a4,a5,a6,a7,a8)
#define TRACE_9(format,a1,a2,a3,a4,a5,a6,a7,a8,a9) printToStderr(format,a1,a2,a3,a4,a5,a6,a7,a8,a9)
#else
#define TRACE(format, ...)  
#define TRACE_0(format) 
#define TRACE_1(format,a1) 
#define TRACE_2(format,a1,a2) 
#define TRACE_3(format,a1,a2,a3) 
#define TRACE_4(format,a1,a2,a3,a4) 
#define TRACE_5(format,a1,a2,a3,a4,a5)
#define TRACE_6(format,a1,a2,a3,a4,a5,a6) 
#define TRACE_7(format,a1,a2,a3,a4,a5,a6,a7)
#define TRACE_8(format,a1,a2,a3,a4,a5,a6,a7,a8) 
#define TRACE_9(format,a1,a2,a3,a4,a5,a6,a7,a8,a9)
#endif



#endif
