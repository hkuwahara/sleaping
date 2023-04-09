#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <unistd.h>
#include <zlib.h>

#include "common.h"
#include "type.h"
#include "log.h"
#include "strconv.h"
#include "fadso_pair.h"
#include "random_num_gen.h"
#include "sleaping.h"
#include "algo_R.h"
#include "algo_L.h"
#include "algo_Z.h"
#include "paired_fastq_item_manager.h"
#include "paired_fastq_reader.h"
#include "paired_fastq_writer.h"


static int _initWork( int argc, char **argv, RECORD *rec );
static int _doWork( RECORD *rec);
static int _endWork( RECORD *rec);

int fadsoPairMain( int argc, char *argv[] )
{
	static RECORD rec;	

	if(_initWork(argc, argv, &rec) == 0){	
		_doWork(&rec);
	}
	_endWork(&rec);
	
    return 0;
}




static int _initWork( int argc, char **argv, RECORD *rec ) {
	int ret = 0;	
	char c = 0;
	char *inputFilePath1 = NULL;
	char *inputFilePath2 = NULL;
	char *outputFilePath1 = NULL;
	char *outputFilePath2 = NULL;
	int outputZipped = 0;
	UINT readLen = 0;
	UINT subsampleSize = 0;
	UINT totalSize = 0;
	UINT rseed = 0;
	FILE *inputp1 = NULL;
	FILE *inputp2 = NULL;
	FILE *outputp1 = NULL;
	FILE *outputp2 = NULL;
	SUBSAMPLER *subsampler = NULL;

	while((c = getopt(argc, argv, "1:2:a:b:m:l:r:k:n:t:z")) >= 0){
		switch(c){
			case '1':
				inputFilePath1 = optarg;
				break;
			
			case '2':
				inputFilePath2 = optarg;
				break;
			
			case 'a':
				outputFilePath1 = optarg;
				break;
			
			case 'b':
				outputFilePath2 = optarg;
				break;
			
			case 'm':
				if(strcmp(optarg, "sleaping") == 0) {
					if((subsampler = createSleaping()) == NULL) {
						fprintf(stderr, "the creation of subsampler failed\n");
						return -1;
					}
				}
				else if(strcmp(optarg, "r") == 0) {
					if((subsampler = createAlgoR()) == NULL) {
						fprintf(stderr, "the creation of subsampler failed\n");
						return -1;
					}
				}
				else if(strcmp(optarg, "l") == 0) {
					if((subsampler = createAlgoL()) == NULL) {
						fprintf(stderr, "the creation of subsampler failed\n");
						return -1;
					}
				}
				else if(strcmp(optarg, "z") == 0) {
					if((subsampler = createAlgoZ()) == NULL) {
						fprintf(stderr, "the creation of subsampler failed\n");
						return -1;
					}
				}
				else {
					if((subsampler = createSleaping()) == NULL) {
						fprintf(stderr, "the creation of subsampler failed\n");
						return -1;
					}
				}
				break;
			
			case 'l':
				if((strToUINT(&readLen, optarg) < 0) && (readLen == 0)) {
					fprintf(stderr, "read length %s is not valid\n", optarg);
					ret = -1;
					goto LABEL_TOOL_USAGE;
				}
				break;
						
			case 'r':
				if(strToUINT(&rseed, optarg) < 0) {
					fprintf(stderr, "random seed %s is not a valid integer\n", optarg);
					ret = -1;
					goto LABEL_TOOL_USAGE;
				}
				setRandomSeed(rseed);
				
			case 'k':
				if(strToUINT(&subsampleSize, optarg) != 0){
					fprintf(stderr, "subsampling size %s is not a valid integer\n", optarg);
					ret = -1;
					goto LABEL_TOOL_USAGE;
				}
				break;
			case 'n':
				if(strToUINT(&totalSize, optarg) != 0){
					fprintf(stderr, "total size %s is not a valid integer\n", optarg);
					ret = -1;
					goto LABEL_TOOL_USAGE;
				}
				break;
			case 't':
				break;	
			
			case 'z':
				outputZipped = 1;
				break;
		}
		
	}
	if(subsampler == NULL){
		if((subsampler = createSleaping()) == NULL) {
			fprintf(stderr, "the creation of subsampler failed\n");
			return -1;
		}
	}
	if(subsampleSize == 0){
		fprintf(stderr, "subsampling size needs to be specified\n");
		goto LABEL_TOOL_USAGE;
	}
	if(subsampler->initSubsampler(subsampler, subsampleSize, totalSize) != 0) {
		fprintf(stderr, "the initialization of subsampler failed\n");
		return -1;
	}
	rec->subsampleSize = subsampleSize;
	rec->subsampler = subsampler;	
	
	if((inputFilePath1 == NULL) || (inputFilePath2 == NULL) ) {
		fprintf(stderr, "input fastq file paths neede to be specified\n");
		goto LABEL_TOOL_USAGE;
	}
	if((inputp1 = fopen(inputFilePath1, "r")) == NULL){
		fprintf(stderr, "error in reading %s\n", inputFilePath1);
		return -1;
	}
	if((inputp2 = fopen(inputFilePath2, "r")) == NULL){
		fprintf(stderr, "error in reading %s\n", inputFilePath2);
		return -1;
	}
	
	rec->itemManager = createPairedFastqItemManager();
		
	rec->reader = (readLen > 512) ? createZippedPairedFastqReader(inputp1, inputp2, readLen) : createZippedPairedFastqReader2(inputp1, inputp2);		

	if((outputFilePath1 == NULL) || (outputFilePath2 == NULL) ) {
		fprintf(stderr, "output fastq file paths neede to be specified\n");
		goto LABEL_TOOL_USAGE;
	}
	if((outputp1 = fopen(outputFilePath1, "w")) == NULL){
		fprintf(stderr, "error in opening %s\n", outputFilePath1);
		return -1;
	}
	if((outputp2 = fopen(outputFilePath2, "w")) == NULL){
		fprintf(stderr, "error in opening %s\n", outputFilePath2);
		return -1;
	}
	rec->writer = (outputZipped > 0) ? createZippedPairedFastqWriter(outputp1, outputp2) : createPlainPairedFastqWriter(outputp1, outputp2); 
	return 0;
	
	LABEL_TOOL_USAGE:
	
	return ret;
}

static int _doWork( RECORD *rec) {
	READER *reader = rec->reader;
	ITEM_MANAGER *itemManager = rec->itemManager;
	WRITER *writer = rec->writer;
	SUBSAMPLER *subsampler = rec->subsampler;
	
	if(subsampler->downSample(subsampler, reader, itemManager, writer) != 0){
		fprintf(stderr, "error returned from downsampler\n");
		return -1;
	}

	return 0;
}

static int _endWork( RECORD *rec) {
	SUBSAMPLER *subsampler = rec->subsampler; 
	READER *reader = rec->reader;
	WRITER *writer = rec->writer;

	if(subsampler != NULL){
		subsampler->closeSubsampler(subsampler);
	}
	if(reader != NULL){
		reader->closeReader(reader);
	}
	if(writer != NULL){
		writer->closeWriter(writer);
	}
	return 0;
}
