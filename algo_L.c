#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "algo_L.h"
#include "random_num_gen.h"
#include "log.h"


static INT _initSubsampler(SUBSAMPLER *subsampler, UINT subsampleSize, UINT totalSize);
static INT _downsample(SUBSAMPLER *subsampler, READER *reader, ITEM_MANAGER *itemManager, WRITER *writer);
static INT _sortSubsamples(SUBSAMPLER *subsampler);
static ITEM** _getSubsamples(SUBSAMPLER *subsampler);
static INT _closeSubsampler(SUBSAMPLER *subsampler);

static INT _fillResevoir(SUBSAMPLER *subsampler, READER *reader, ITEM_MANAGER *itemManager);


SUBSAMPLER *createAlgoL() {
	SUBSAMPLER *subsampler = NULL;

	TRACE_0("create fadso downsampler\n");
	
	if((subsampler = (SUBSAMPLER*)CALLOC(1, sizeof(SUBSAMPLER))) == NULL){
		fprintf(stderr, "couldn't allocate space for subsampler\n");
		return NULL;
	}
	subsampler->initSubsampler = _initSubsampler;
	subsampler->downSample = _downsample;
	subsampler->sortSubsamples = _sortSubsamples;
	subsampler->getSubsamples = _getSubsamples;
	subsampler->closeSubsampler = _closeSubsampler;
	
	return subsampler;
}


static INT _initSubsampler(SUBSAMPLER *subsampler, UINT subsampleSize, UINT totalSize) {
	ITEM **reservoir = NULL;
	
	subsampler->subsampleSize = subsampleSize;
	
	if((reservoir = (ITEM**)MALLOC( (subsampleSize + 1) * sizeof(ITEM*))) == NULL) {
		fprintf(stderr, "couldn't allocate space for a reservoir with %" PRIu64 "items\n", subsampleSize);
		return -1;
	}
	reservoir[subsampleSize] = NULL;
	subsampler->reservoir = reservoir;
	
	TRACE_1("reservoir size: %" PRIu64  "\n", subsampleSize); 
	
	return 0;
}



static INT _downsample(SUBSAMPLER *subsampler, READER *reader, ITEM_MANAGER *itemManager, WRITER *writer) {
	UINT i = 0;
	UINT ranIndex = 0;
	UINT nextSteps = 0;
	double max = 0.0;
	UINT reservoirIndex = 0;
	UINT reservoirSize = subsampler->subsampleSize;
	double p = 0.0;
	DATA_HOLDER *data = NULL;
	ITEM **reservoir = subsampler->reservoir;

	if((i = _fillResevoir(subsampler, reader, itemManager)) != reservoirSize) {
		fprintf(stderr, "error in the initialization of reservoir with  %" PRIu64 " items: only %" PRIu64 " items were read\n", reservoirSize, i);
		return -1;
	}	
	TRACE_1("done with first filling of the reservoir with i = %" PRIu64 "\n", i);
	max = exp(log(getNextUnitUniformRandomExcl())/(double)reservoirSize);
	while(!(reader->isEOF(reader))){
		nextSteps = (UINT)(getNextExponentialRandom(-log(1.0-max))) + (UINT)1;
		if((data = reader->skipAndReadItem(reader, nextSteps)) == NULL){
			TRACE_1("no more item at %" PRIu64 "\n", i+1); 
			goto LABEL_END_DOWNSAMPLING;
		}
		
		ranIndex = (UINT)(getNextUnitUniformRandomExcl() * (reservoirSize));
		itemManager->copyContent(itemManager, reservoir[reservoirIndex], data);
		max = max * exp(log(getNextUnitUniformRandomExcl())/(double)reservoirSize);
	}

	TRACE_0("EOF is reached\n");
LABEL_END_DOWNSAMPLING:	
	writer->writeItems(writer, reservoir, reservoirSize);	
	
	return 0;	
}


static INT _sortSubsamples(SUBSAMPLER *subsampler) {

	return 0;
}

static ITEM** _getSubsamples(SUBSAMPLER *subsampler) {

	return subsampler->reservoir;
}



static INT _closeSubsampler(SUBSAMPLER *subsampler) {
	FREE(subsampler);
}



static INT _fillResevoir(SUBSAMPLER *subsampler, READER *reader, ITEM_MANAGER *itemManager) {
	UINT i = 0;
	UINT size = subsampler->subsampleSize;	
	ITEM *item = NULL;
	DATA_HOLDER *data = NULL;
	ITEM **reservoir = subsampler->reservoir;
	
	for(i=0; i < size; i++){
		if((data = reader->readNextItem(reader)) == NULL ) {
			fprintf(stderr, "couldn't fill out the resevior with %" PRIu64 " items\n", size);
			break;
		}
		if((item = itemManager->createItem(itemManager, data)) == NULL) {
			fprintf(stderr, "couldn't fill out the resevior with %" PRIu64 " items\n", size);
			break;
		}
		reservoir[i] = item;
	}
	return i;	
}

