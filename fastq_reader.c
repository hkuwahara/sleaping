
#include <stdio.h>
#include <string.h>
#include <zlib.h>

#include "common.h"
#include "log.h"

#include "fastq_item_manager.h"
#include "fastq_reader.h"

#if !defined(READ_BUFFER_SIZE)	
/*#define READ_BUFFER_SIZE 2048*/	
#define READ_BUFFER_SIZE 1024	
#endif	



static READER *_createZippedFastqReader(gzFile fp, UINT readLen);
static DATA_HOLDER *_readZippedFastqItem(READER *reader);
static DATA_HOLDER *_skipAndReadItem(READER *reader, UINT steps);
static INT _skipItems(READER *reader, UINT steps);
static INT _skipNextItem(READER *reader);
static UINT _getCurrentIndex(READER *reader);
static INT _isEOF(READER *reader);
static INT _closeZippedFastqReader(READER *reader);


#define FASTQ_ZIP_READ_BUFFER_SIZE (UINT)(0x01<<21)


READER *createZippedFastqReader(FILE *file, UINT readLen) {
	gzFile gzfp = Z_NULL;
		
	if((gzfp = gzdopen(file->_fileno, "r")) != Z_NULL) {
		return _createZippedFastqReader(gzfp, readLen);
	}
	return NULL;	
}

READER *createZippedFastqReader2(FILE *file) {
	gzFile gzfp = Z_NULL;
		
	if((gzfp = gzdopen(file->_fileno, "r")) != Z_NULL) {
		return _createZippedFastqReader(gzfp, READ_BUFFER_SIZE);
	}
	return NULL;	
}


static READER *_createZippedFastqReader(gzFile fp, UINT readLen) {
	int i = 0;
	UINT len = 0;
	READER *reader = NULL;
	FASTQ_CONTENT *content = NULL;
	FASTQ_DATA_HOLDER *holder = NULL;
	
	if((reader = (READER*)MALLOC(sizeof(READER))) != NULL){
		reader->fp = (CADDR_T)fp;
		reader->index = 0;
		reader->readNextItem = _readZippedFastqItem;
		reader->skipNextItem = _skipNextItem;
		reader->skipItems = _skipItems;
		reader->skipAndReadItem = _skipAndReadItem;
		reader->getCurrentIndex = _getCurrentIndex;
		reader->isEOF = _isEOF;
		reader->closeReader = _closeZippedFastqReader;
		
		if(gzbuffer(fp, FASTQ_ZIP_READ_BUFFER_SIZE) < 0){
			fprintf(stderr, "error in allocating the gz buffer for gzread\n");
			return NULL;
		}
		
		if((content = (FASTQ_CONTENT*)MALLOC(sizeof(FASTQ_CONTENT))) == NULL){
			fprintf(stderr, "error in allocating internal data structure for fastq reader\n");
			return NULL;
		}
		if((holder = (FASTQ_DATA_HOLDER*)MALLOC(sizeof(FASTQ_DATA_HOLDER))) == NULL){
			fprintf(stderr, "error in allocating internal data structure for fastq reader\n");
			return NULL;
		}
		len = readLen + (UINT)4;
		for(i = 0; i < 4; i++) {
			if((content->lines[i] = (char*)MALLOC(sizeof(char)*len)) == NULL){
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
	UINT bufSize = reader->bufSize;
	gzFile fp = (gzFile)(reader->fp);
	FASTQ_DATA_HOLDER *holder = (FASTQ_DATA_HOLDER*)(reader->itemData);
	FASTQ_CONTENT *content = holder->content;
	
	if(_isEOF(reader)){
		return NULL;
	}
	
	for(i = 0; i < (UINT)4; i++){
		buf = content->lines[i];
		if(gzgets(fp, buf, bufSize) == Z_NULL) {
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
	UINT bufSize = reader->bufSize;
	gzFile fp = (gzFile)(reader->fp);
	FASTQ_DATA_HOLDER *holder = (FASTQ_DATA_HOLDER*)(reader->itemData);
	FASTQ_CONTENT *content = holder->content;

	for( skippedSteps = 0; skippedSteps < steps; skippedSteps++){
		for(i = 0; i < (UINT)4; i++){
			buf = content->lines[i];
			if(gzgets(fp, buf, bufSize) == Z_NULL) {
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
	UINT bufSize = reader->bufSize;
	gzFile fp = (gzFile)(reader->fp);
	FASTQ_DATA_HOLDER *holder = (FASTQ_DATA_HOLDER*)(reader->itemData);
	FASTQ_CONTENT *content = holder->content;
	
	/* steps give the number of trial up to the next item >= 1 */
	for( skippedSteps = 1; skippedSteps < steps; skippedSteps++){
		for(i = 0; i < (UINT)4; i++){
			buf = content->lines[i];
			if(gzgets(fp, buf, bufSize) == Z_NULL) {
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
	UINT bufSize = reader->bufSize;
	gzFile fp = (gzFile)(reader->fp);
	FASTQ_DATA_HOLDER *holder = (FASTQ_DATA_HOLDER*)(reader->itemData);
	FASTQ_CONTENT *content = holder->content;
	
	for(i = 0; i < (UINT)4; i++){
		buf = content->lines[i];
		if(gzgets(fp, buf, bufSize) == Z_NULL) {
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
	gzFile fp = (gzFile)(reader->fp);
		
/*	return (INT)gzeof(fp);*/	
	return (gzeof(fp) ? (INT)1: (INT)0);	
}


static INT _closeZippedFastqReader(READER *reader) {
	gzFile fp = (gzFile)(reader->fp);
	gzclose(fp);
	FREE(reader);
	
	return 0;
}

