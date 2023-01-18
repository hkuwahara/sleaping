
#include <stdlib.h>
#include <string.h>

#include "strconv.h"


int strToINT(INT *val, char *str) {
	char *endp = NULL;
	
	*val = strtoll( str, &endp, 0 );

	if( *endp != '\0' ) {
		return -1; 
	}
	return 0;
}


int strToUINT(UINT *val, char *str) {
	char *endp = NULL;
	
	*val = strtoull( str, &endp, 0 );

	if( *endp != '\0' ) {
		return -1; 
	}
	return 0;
}

