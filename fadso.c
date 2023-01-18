#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <unistd.h>
#include <zlib.h>

#include "fadso.h"
#include "fadso_single.h"
#include "fadso_pair.h"


int main( int argc, char *argv[] )
{
	int ret = 0;	
	char *specifiedTask = NULL;

	if(argc <= 1){
		fprintf(stderr, FADSO_USAGE_MESSAGE);
	}
	
	specifiedTask = argv[1];
	if( strcmp(specifiedTask, "single") == 0 ){
		ret = fadsoSingleMain(argc - 1, &argv[1]);
	}
	else if(strcmp(specifiedTask, "pair") == 0 ){
		ret = fadsoPairMain(argc - 1, &argv[1]);
	}
	else {
		fprintf(stderr, "invalid task\n");
		fprintf(stderr, FADSO_USAGE_MESSAGE);
	}
	return ret;
}


