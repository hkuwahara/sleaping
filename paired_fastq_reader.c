
#include <stdio.h>
#include <string.h>
#include <zlib.h>

#include "common.h"
#include "log.h"

#include "paired_fastq_item_manager.h"
#include "paired_fastq_reader.h"


#if !defined(READ_BUFFER_SIZE)	
/*#define READ_BUFFER_SIZE 2048*/	
#define READ_BUFFER_SIZE 1024	
#endif	


static READER *_createZippedPairedFastqReader(gzFile fp1, gzFile fp2, UINT readLen);
static DATA_HOLDER *_readZippedFastqItem(READER *reader);
static DATA_HOLDER *_skipAndReadItem(READER *reader, UINT steps);
static INT _skipItems(READER *reader, UINT steps);
static INT _skipNextItem(READER *reader);
static UINT _getCurrentIndex(READER *reader);
static INT _isEOF(READER *reader);
static INT _closeZippedPairedFastqReader(READER *reader);


#define FASTQ_ZIP_READ_BUFFER_SIZE (UINT)(0x01<<21)

typedef struct {
	gzFile fp1;
	gzFile fp2;
} PAIRED_FP;

READER *createZippedPairedFastqReader(FILE *file1, FILE *file2, UINT readLen) {
	gzFile gzfp1 = Z_NULL;
	gzFile gzfp2 = Z_NULL;
	
	if( ((gzfp1 = gzdopen(file1->_fileno, "r")) != Z_NULL) && ((gzfp2 = gzdopen(file2->_fileno, "r")) != Z_NULL)) {
		return _createZippedPairedFastqReader(gzfp1, gzfp2, readLen);
	}
	return NULL;	
}

READER *createZippedPairedFastqReader2(FILE *file1, FILE *file2) {
	gzFile gzfp1 = Z_NULL;
	gzFile gzfp2 = Z_NULL;
	
	if( ((gzfp1 = gzdopen(file1->_fileno, "r")) != Z_NULL) && ((gzfp2 = gzdopen(file2->_fileno, "r")) != Z_NULL)) {
		return _createZippedPairedFastqReader(gzfp1, gzfp2, READ_BUFFER_SIZE);
	}
	return NULL;	
}


static READER *_createZippedPairedFastqReader(gzFile fp1, gzFile fp2, UINT readLen) {
	int i = 0;
	UINT len = 0;
	READER *reader = NULL;
	PAIRED_FP *pairedFp = NULL;
	PAIRED_FASTQ_CONTENT *content = NULL;
	PAIRED_FASTQ_DATA_HOLDER *holder = NULL;

	
	if((pairedFp = (PAIRED_FP*)MALLOC(sizeof(PAIRED_FP))) == NULL){
		return NULL;
	}
	pairedFp->fp1 = fp1;
	pairedFp->fp2 = fp2;
	
	
	if((reader = (READER*)MALLOC(sizeof(READER))) != NULL){
		reader->fp = (CADDR_T)pairedFp;
		reader->index = 0;
		reader->readNextItem = _readZippedFastqItem;
		reader->skipNextItem = _skipNextItem;
		reader->skipItems = _skipItems;
		reader->skipAndReadItem = _skipAndReadItem;
		reader->getCurrentIndex = _getCurrentIndex;
		reader->isEOF = _isEOF;
		reader->closeReader = _closeZippedPairedFastqReader;
		
		if(gzbuffer(fp1, FASTQ_ZIP_READ_BUFFER_SIZE) < 0){
			fprintf(stderr, "error in allocating the gz buffer for gzread\n");
			return NULL;
		}
		if(gzbuffer(fp2, FASTQ_ZIP_READ_BUFFER_SIZE) < 0){
			fprintf(stderr, "error in allocating the gz buffer for gzread\n");
			return NULL;
		}
		
		if((content = (PAIRED_FASTQ_CONTENT*)MALLOC(sizeof(PAIRED_FASTQ_CONTENT))) == NULL){
			fprintf(stderr, "error in allocating internal data structure for fastq reader\n");
			return NULL;
		}
		if((holder = (PAIRED_FASTQ_DATA_HOLDER*)MALLOC(sizeof(PAIRED_FASTQ_DATA_HOLDER))) == NULL){
			fprintf(stderr, "error in allocating internal data structure for fastq reader\n");
			return NULL;
		}
		len = readLen + (UINT)4;
		for(i = 0; i < 4; i++) {
			if((content->lines1[i] = (char*)MALLOC(sizeof(char)*len)) == NULL){
				fprintf(stderr, "error in allocating internal data structure for fastq reader\n");
				return NULL;
			}
			if((content->lines2[i] = (char*)MALLOC(sizeof(char)*len)) == NULL){
				fprintf(stderr, "error in allocating internal data structure for fastq reader\n");
				return NULL;
			}
		}
		holder->content = content;
		reader->itemData = (DATA_HOLDER*)holder;
		reader->bufSize = readLen + 3;		
		
		return reader;
	}
	
	return NULL;
}

static DATA_HOLDER *_readZippedFastqItem(READER *reader) {
	UINT i = 0;
	char *buf = NULL;
	UINT bufLen = reader->bufSize;
	gzFile fp = NULL;
	PAIRED_FASTQ_DATA_HOLDER *holder = (PAIRED_FASTQ_DATA_HOLDER*)(reader->itemData);
	PAIRED_FASTQ_CONTENT *content = holder->content;
	
	if(_isEOF(reader)){
		return NULL;
	}
	
	fp = ((PAIRED_FP*)reader->fp)->fp1;
	for(i = 0; i < (UINT)4; i++){
		buf = content->lines1[i];
		if(gzgets(fp, buf, bufLen) == Z_NULL) {
			return NULL;		
		}
	}
	
	fp = ((PAIRED_FP*)reader->fp)->fp2;
	for(i = 0; i < (UINT)4; i++){
		buf = content->lines2[i];
		if(gzgets(fp, buf, bufLen) == Z_NULL) {
			return NULL;		
		}
	}
	
	reader->index++;
	holder->key = reader->index;
	
	return (DATA_HOLDER*)holder;
}


static INT _skipItems(READER *reader, UINT steps) {
	UINT i = 0;
	UINT skippedSteps;	
	char *buf = NULL;
	UINT bufLen = reader->bufSize;
	PAIRED_FASTQ_DATA_HOLDER *holder = (PAIRED_FASTQ_DATA_HOLDER*)(reader->itemData);
	PAIRED_FASTQ_CONTENT *content = holder->content;
	gzFile fp1 = ((PAIRED_FP*)reader->fp)->fp1;
	gzFile fp2 = ((PAIRED_FP*)reader->fp)->fp2;

	
	for( skippedSteps = 0; skippedSteps < steps; skippedSteps++){
		for(i = 0; i < (UINT)4; i++){
			buf = content->lines1[i];
			if(gzgets(fp1, buf, bufLen) == Z_NULL) {
				return -1;
			}
			buf = content->lines2[i];
			if(gzgets(fp2, buf, bufLen) == Z_NULL) {
				return -1;
			}
		}
		(reader->index)++;
	}
	return skippedSteps;
}


static DATA_HOLDER *_skipAndReadItem(READER *reader, UINT steps) {
	UINT i = 0;
	UINT skippedSteps;	
	char *buf = NULL;
	UINT bufLen = reader->bufSize;
	PAIRED_FASTQ_DATA_HOLDER *holder = (PAIRED_FASTQ_DATA_HOLDER*)(reader->itemData);
	PAIRED_FASTQ_CONTENT *content = holder->content;
	gzFile fp1 = ((PAIRED_FP*)reader->fp)->fp1;
	gzFile fp2 = ((PAIRED_FP*)reader->fp)->fp2;

	/* steps give the number of trial up to the next item >= 1 */
	for( skippedSteps = 1; skippedSteps < steps; skippedSteps++){
		for(i = 0; i < (UINT)4; i++){
			buf = content->lines1[i];
			if(gzgets(fp1, buf, bufLen) == Z_NULL) {
				goto LABEL_SKIP_PREMATURE;
			}
			buf = content->lines2[i];
			if(gzgets(fp2, buf, bufLen) == Z_NULL) {
				goto LABEL_SKIP_PREMATURE;
			}
		}
		(reader->index)++;
	}
	
	return _readZippedFastqItem(reader);
	
LABEL_SKIP_PREMATURE:
	return NULL;	
}


static INT _skipNextItem(READER *reader) {
	UINT i = 0;
	char *buf = NULL;
	UINT bufLen = reader->bufSize;
	PAIRED_FASTQ_DATA_HOLDER *holder = (PAIRED_FASTQ_DATA_HOLDER*)(reader->itemData);
	PAIRED_FASTQ_CONTENT *content = holder->content;
	gzFile fp1 = ((PAIRED_FP*)reader->fp)->fp1;
	gzFile fp2 = ((PAIRED_FP*)reader->fp)->fp2;
	
	for(i = 0; i < (UINT)4; i++){
		buf = content->lines1[i];
		if(gzgets(fp1, buf, bufLen) == Z_NULL) {
			return -1;	
		}
		buf = content->lines2[i];
		if(gzgets(fp2, buf, bufLen) == Z_NULL) {
			return -1;	
		}
	}
	reader->index += 1;
	return 1;	
}


static UINT _getCurrentIndex(READER *reader) {
	return reader->index;	
}


static INT _isEOF(READER *reader) {
	gzFile fp1 = ((PAIRED_FP*)reader->fp)->fp1;
	gzFile fp2 = ((PAIRED_FP*)reader->fp)->fp2;
		
/*	return (INT)gzeof(fp);*/	
	return ((gzeof(fp1) || gzeof(fp2)) ? (INT)1: (INT)0);	
}


static INT _closeZippedPairedFastqReader(READER *reader) {
	gzFile fp1 = ((PAIRED_FP*)reader->fp)->fp1;
	gzFile fp2 = ((PAIRED_FP*)reader->fp)->fp2;

	gzclose(fp1);
	gzclose(fp2);
	
	FREE(reader->fp);
	FREE(reader);
	
	return 0;
}

