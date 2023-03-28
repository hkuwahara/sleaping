#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "algo_R.h"
#include "random_num_gen.h"
#include "log.h"


static INT _initSubsampler(SUBSAMPLER *subsampler, UINT subsampleSize, UINT totalSize);
static INT _downsample(SUBSAMPLER *subsampler, READER *reader, ITEM_MANAGER *itemManager, WRITER *writer);
static INT _sortSubsamples(SUBSAMPLER *subsampler);
static ITEM** _getSubsamples(SUBSAMPLER *subsampler);
static INT _closeSubsampler(SUBSAMPLER *subsampler);

static INT _fillResevoir(SUBSAMPLER *subsampler, READER *reader, ITEM_MANAGER *itemManager);


SUBSAMPLER *createAlgoR() {
	SUBSAMPLER *subsampler = NULL;
	TRACE_0("create algo R\n");
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
	UINT j = 0;
	UINT reservoirIndex = 0;
	UINT reservoirSize = subsampler->subsampleSize;
	double p = 0.0;
	DATA_HOLDER *data = NULL;
	ITEM **reservoir = subsampler->reservoir;

	fprintf(stderr, "Algorithm R\n");
	/* first do Algorithm R up to 2 * |reservoir| */
	if((i = _fillResevoir(subsampler, reader, itemManager)) != reservoirSize) {
		fprintf(stderr, "error in the initialization of reservoir with  %" PRIu64 " items: only %" PRIu64 " items were read\n", reservoirSize, i);
		return -1;
	}	
	TRACE_1("done with first filling of the reservoir with i = %" PRIu64 "\n", i);
	while(!(reader->isEOF(reader))) {
		i += 1;
		reservoirIndex = (UINT)(getNextUnitUniformRandomExcl() * i);
		if(reservoirIndex < reservoirSize){
			if((data = reader->readNextItem(reader)) == NULL){
				TRACE_1("no more item at i = %" PRIu64 "\n", i);
				goto LABEL_END_DOWNSAMPLING;
			}
			itemManager->copyContent(itemManager, reservoir[reservoirIndex], data);
		} 
		else{
			if(reader->skipNextItem(reader) < 0){
				TRACE_1("no more item at i = %" PRIu64 "\n", i);
				goto LABEL_END_DOWNSAMPLING;
			}
		}
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

