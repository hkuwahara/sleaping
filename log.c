#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "log.h"



void printToStderr(char *format, ... ) {
    va_list args;
        
    va_start( args, format );
	vfprintf( stderr, format, args );
    va_end(args);
	fflush(stderr);
}


