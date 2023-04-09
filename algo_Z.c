#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "algo_Z.h"
#include "random_num_gen.h"
#include "log.h"


static INT _initSubsampler(SUBSAMPLER *subsampler, UINT subsampleSize, UINT totalSize);
static INT _downsample(SUBSAMPLER *subsampler, READER *reader, ITEM_MANAGER *itemManager, WRITER *writer);
static INT _sortSubsamples(SUBSAMPLER *subsampler);
static ITEM** _getSubsamples(SUBSAMPLER *subsampler);
static INT _closeSubsampler(SUBSAMPLER *subsampler);

static INT _fillResevoir(SUBSAMPLER *subsampler, READER *reader, ITEM_MANAGER *itemManager);


typedef struct {
	UINT thresholdFactor;
} ALGO_Z_INFO;


#define DEFAULT_ALGO_Z_THRESHOLD_FACTOR ((UINT)22)


SUBSAMPLER *createAlgoZ() {
	return createAlgoZWithOptions(DEFAULT_ALGO_Z_THRESHOLD_FACTOR);	
}

SUBSAMPLER *createAlgoZWithOptions(float thresholdFactor) {
	SUBSAMPLER *subsampler = NULL;
	ALGO_Z_INFO *info = NULL;
	
	TRACE_0("create fadso downsampler\n");
	
	if((info = (ALGO_Z_INFO*)MALLOC(sizeof(ALGO_Z_INFO))) == NULL) {
		fprintf(stderr, "couldn't allocate space for algo Z info\n");
		return NULL;
	}
	info->thresholdFactor = thresholdFactor;
	
	if((subsampler = (SUBSAMPLER*)CALLOC(1, sizeof(SUBSAMPLER))) == NULL){
		fprintf(stderr, "couldn't allocate space for subsampler\n");
		return NULL;
	}
	subsampler->initSubsampler = _initSubsampler;
	subsampler->downSample = _downsample;
	subsampler->sortSubsamples = _sortSubsamples;
	subsampler->getSubsamples = _getSubsamples;
	subsampler->closeSubsampler = _closeSubsampler;
	subsampler->rfu1 = (CADDR_T)info;
		
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
	UINT nextSteps = 0;
	UINT reservoirIndex = 0;
	UINT reservoirSize = subsampler->subsampleSize;
	UINT j = 0;
	UINT threshold = 0;
	UINT num = 0;
	UINT term = 0;
	UINT denom = 0;
	UINT numerLim = 0;
	
	double v = 0.0; 
	double quot = 0.0;
	double skipRan = 0.0;
	double w = 0.0;
	double lhs = 0.0;
	double rhs = 0.0;
	double y = 0.0;

	ALGO_Z_INFO *info = (ALGO_Z_INFO*)(subsampler->rfu1);
	DATA_HOLDER *data = NULL;
	ITEM **reservoir = subsampler->reservoir;
	
	
	threshold = (UINT)(info->thresholdFactor * reservoirSize); 
	
	fprintf(stderr, "Algo Z downsampler\n");

	if((i = _fillResevoir(subsampler, reader, itemManager)) != reservoirSize) {
		fprintf(stderr, "error in the initialization of reservoir with  %" PRIu64 " items: only %" PRIu64 " items were read\n", reservoirSize, i);
		return -1;
	}	

	TRACE_1("done with first filling of the reservoir with i = %" PRIu64 "\n", i);
	num = 0;	
	while(!(reader->isEOF(reader))){
		while(i <= threshold) {
			nextSteps = 1;
			v = getNextUnitUniformRandomExcl();
			i += 1;
			num += 1;
			quot = (double)num/i;
			while(quot > v){
				i += 1;
				num += 1;
				nextSteps += 1;
				quot = (quot * num)/i;
			}
			if((data = reader->skipAndReadItem(reader, nextSteps)) == NULL){
				TRACE_1("no more item at %" PRIu64 "\n", i); 
				goto LABEL_END_DOWNSAMPLING;
			}
			reservoirIndex = (UINT)(getNextUnitUniformRandomExcl() * (reservoirSize));
			itemManager->copyContent(itemManager, reservoir[reservoirIndex], data);
		}
	}
	
	w = exp(-log(getNextUnitUniformRandomExcl())/(double)reservoirSize);
	term = i - reservoirSize + 1;
	while(!(reader->isEOF(reader))){
		for(;;){
			v = getNextUnitUniformRandomExcl();
			skipRan = i * (w - 1.0);
			nextSteps = (UINT)skipRan;
			lhs = exp(log(((v*pow((i + 1)/term, 2.0)) * (term + nextSteps))/(i + skipRan))/reservoirSize);
			rhs = (((i + skipRan)/(term + nextSteps)) * term)/i;
			if(lhs <= rhs){
				w = rhs/lhs;
				break;
			}
			y = (((v*(i+1))/term) * (i + nextSteps + 1))/(i + skipRan);
			if(reservoirSize < nextSteps){
				denom = i;
				numerLim = term + nextSteps;
			}
			else{
				denom = i - reservoirSize + nextSteps;
				numerLim = i + 1;				
			}
			for(j = i + nextSteps; j >= numerLim; j--){
				y = (y*j)/denom;
				denom -= 1;
			}
			w = exp(-log(getNextUnitUniformRandomExcl())/(double)reservoirSize);
			if(exp(log(y)/reservoirSize) <= (i + skipRan)/i){
				break;				
			}
		}
		nextSteps += 1;
		if((data = reader->skipAndReadItem(reader, nextSteps)) == NULL){
			TRACE_1("no more item at %" PRIu64 "\n", i); 
			goto LABEL_END_DOWNSAMPLING;
		}
		reservoirIndex = (UINT)(getNextUnitUniformRandomExcl() * (reservoirSize));
		itemManager->copyContent(itemManager, reservoir[reservoirIndex], data);
		i += nextSteps;
		term += nextSteps;
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

