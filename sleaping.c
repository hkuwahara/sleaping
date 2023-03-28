#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "sleaping.h"
#include "random_num_gen.h"
#include "log.h"


static INT _initSubsampler(SUBSAMPLER *subsampler, UINT subsampleSize, UINT totalSize);
static INT _downsample(SUBSAMPLER *subsampler, READER *reader, ITEM_MANAGER *itemManager, WRITER *writer);
static INT _sortSubsamples(SUBSAMPLER *subsampler);
static ITEM** _getSubsamples(SUBSAMPLER *subsampler);
static INT _closeSubsampler(SUBSAMPLER *subsampler);

static INT _fillResevoir(SUBSAMPLER *subsampler, READER *reader, ITEM_MANAGER *itemManager);

typedef struct {
	double frac;
	UINT leapSize;
} LEAP_INFO;


/*
 * Assuming that the reference human genome size is 3.2Gbp, the read-pair length is 300, and the target coverage rate is 0.4, 
 * the minimum number of reads required is count ~4.3M.  
 * Given that the minimum downsampling size for fastq data is 5M reads and the default leap size rate is 0.002, leap size is >= 10,000.
 *  The rejection prob for the (10M+1)-th read is 0.5 + 1/10M; the (10M+10,000)-th read, 0.5 + 0.001. estimated to be ~0.001248; the average number, 12.48; the sd, 3.533; 
 *  the lower end (mean - 3*sd), ~1.8845.  
 * 
 */
#define DEFAULT_LEAP_SIZE_PROPORTION ((double)0.005)


/*
static UINT _sampleHitsWithNormal(UINT leapSize, double mean, double sd);
static UINT _sampleHitsWithGeometric(UINT leapSize, double rejectProb);
*/

SUBSAMPLER *createSleaping() {
	return createSleapingWithOptions(DEFAULT_LEAP_SIZE_PROPORTION); 
}


SUBSAMPLER *createSleapingWithOptions(double leapSizeProp) {
	SUBSAMPLER *subsampler = NULL;
	LEAP_INFO *leapInfo = NULL;

	TRACE_0("create fadso downsampler\n");
	
	if((leapInfo = (LEAP_INFO*)MALLOC(sizeof(LEAP_INFO))) == NULL) {
		fprintf(stderr, "couldn't allocate space for leap info\n");
		return NULL;
	}
	leapInfo->frac = leapSizeProp;
	
	if((subsampler = (SUBSAMPLER*)CALLOC(1, sizeof(SUBSAMPLER))) == NULL){
		fprintf(stderr, "couldn't allocate space for subsampler\n");
		return NULL;
	}
	subsampler->initSubsampler = _initSubsampler;
	subsampler->downSample = _downsample;
	subsampler->sortSubsamples = _sortSubsamples;
	subsampler->getSubsamples = _getSubsamples;
	subsampler->closeSubsampler = _closeSubsampler;
	subsampler->rfu1 = (CADDR_T)leapInfo;
		
	return subsampler;
}


static INT _initSubsampler(SUBSAMPLER *subsampler, UINT subsampleSize, UINT totalSize) {
	ITEM **reservoir = NULL;
	LEAP_INFO *leapInfo = NULL;
	
	subsampler->subsampleSize = subsampleSize;
	
	if((reservoir = (ITEM**)MALLOC( (subsampleSize + 1) * sizeof(ITEM*))) == NULL) {
		fprintf(stderr, "couldn't allocate space for a reservoir with %" PRIu64 "items\n", subsampleSize);
		return -1;
	}
	reservoir[subsampleSize] = NULL;
	leapInfo = (LEAP_INFO*)(subsampler->rfu1);
	leapInfo->leapSize = (UINT)(leapInfo->frac * subsampleSize);
	subsampler->reservoir = reservoir;
	
	TRACE_2("reservoir size: %" PRIu64 "; leap size: %" PRIu64 "\n", subsampleSize, leapInfo->leapSize); 
	
	return 0;
}



static INT _downsample(SUBSAMPLER *subsampler, READER *reader, ITEM_MANAGER *itemManager, WRITER *writer) {
	UINT i = 0;
	UINT nextSteps = 0;
	UINT reservoirIndex = 0;
	UINT offset = 0;
	LEAP_INFO *leapInfo = (LEAP_INFO*)(subsampler->rfu1);
	UINT leapSize = leapInfo->leapSize;	
	UINT endLeap = leapSize - 1;
	UINT reservoirSize = subsampler->subsampleSize;
	UINT endAlgoRIndex = (UINT)(reservoirSize<<1);
	double p = 0.0;
	double lambda = 0.0;
	DATA_HOLDER *data = NULL;
	ITEM **reservoir = subsampler->reservoir;

	fprintf(stderr, "fadso downsampler\n");

	/* first do Algorithm R up to 2 * |reservoir| */
	if((i = _fillResevoir(subsampler, reader, itemManager)) != reservoirSize) {
		fprintf(stderr, "error in the initialization of reservoir with  %" PRIu64 " items: only %" PRIu64 " items were read\n", reservoirSize, i);
		return -1;
	}	
	TRACE_1("done with first filling of the reservoir with i = %" PRIu64 "\n", i);
	for(; i < endAlgoRIndex; i++) {
		if(reader->isEOF(reader)){
			goto LABEL_END_DOWNSAMPLING;
		}
		reservoirIndex = (UINT)(getNextUnitUniformRandomExcl() * (i+1.0));
		if(reservoirIndex < reservoirSize){
			if((data = reader->readNextItem(reader)) == NULL){
				TRACE_1("no more item at i = %" PRIu64 "\n", i+1);
				goto LABEL_END_DOWNSAMPLING;
			}
			itemManager->copyContent(itemManager, reservoir[reservoirIndex], data);
		} 
		else{
			if(reader->skipNextItem(reader) < 0){
				TRACE_1("no more item at i = %" PRIu64 "\n", i+1);
				goto LABEL_END_DOWNSAMPLING;
			}
		}
	}
	TRACE_1("done with algorithm R step with i = %" PRIu64 "\n", i);
	
	
	while(!(reader->isEOF(reader))){
		offset = 0;	
		p = (reservoirSize)/(double)(i+(leapSize>>1)+1);
		lambda = log(1.0 - p);
		for(;;){
			nextSteps = (UINT)(log(getNextUnitUniformRandomExcl())/lambda) + (UINT)1;
			TRACE_5("i = %" PRIu64 "; offset = %" PRIu64 "; read index = %" PRIu64 "; p = %f; nextSteps = %" PRIu64 "\n", 
					i, offset, reader->getCurrentIndex(reader), p, nextSteps);
			offset += (nextSteps);
			if(offset > endLeap) {
				if(reader->skipItems(reader, leapSize+nextSteps-offset) < 0){
					TRACE_1("no more item at i = %" PRIu64 "\n", reader->getCurrentIndex(reader));
					goto LABEL_END_DOWNSAMPLING;
				}
				i += leapSize;
				break;
			}
			else {
				reservoirIndex = (UINT)(getNextUnitUniformRandomExcl() * reservoirSize);
				if((data = reader->skipAndReadItem(reader, nextSteps)) == NULL){
					TRACE_1("no more item at %" PRIu64 "\n", i+1); 
					goto LABEL_END_DOWNSAMPLING;
				}
				itemManager->copyContent(itemManager, reservoir[reservoirIndex], data);
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

