#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <inttypes.h>

#include "common.h"
#include "log.h"

#include "fastq_writer.h"
#include "fastq_item_manager.h"

static INT _writeKeyItemToBinaryFile(WRITER *writer, ITEM *item);
static INT _writeKeyItemsToBinaryFile(WRITER *writer, ITEM **items, UINT size);
static INT _closeBinaryKeyWriter(WRITER *writer);


static INT _writeFastqItemToPlainFile(WRITER *writer, ITEM *item);
static INT _writeFastqItemsToPlainFile(WRITER *writer, ITEM **items, UINT size);
static INT _closePlainFastqWriter(WRITER *writer);



WRITER *createZippedFastqWriter(FILE *file) {
	gzFile gzfp = Z_NULL;
	WRITER *writer = NULL;
	
	if((writer = (WRITER*)MALLOC(sizeof(WRITER))) == NULL){
		return NULL;
	}
	
	if((gzfp = gzdopen(file->_fileno, "w")) != Z_NULL) {
		writer->fp = (CADDR_T)gzfp;
		writer->writeItem = _writeKeyItemToBinaryFile;
		writer->writeItems = _writeKeyItemsToBinaryFile;
		writer->closeWriter = _closeBinaryKeyWriter;
		return writer;
	}

	return NULL;
}

static INT _writeKeyItemToBinaryFile(WRITER *writer, ITEM *item) {
	INT ret = 0;
	INT len = 0;
	FASTQ_ITEM *thisItem = (FASTQ_ITEM*)item;
	gzFile gzfp = (gzFile)writer->fp;
	char buf[WRITE_BUFFER_SIZE];
	
	ret = sprintf(buf, "%s\n%s\n%s\n%s\n", 
		thisItem->data->lines[0], thisItem->data->lines[1],
		thisItem->data->lines[2], thisItem->data->lines[3]);
	if( ret <= 0 ) {
		TRACE_1("error in creating a buffer for item %" PRIu64 "\n", thisItem->key); 
		return 0;
	}
	len = strlen(buf);
	ret = gzwrite(gzfp, buf, len);
	if( ret != len ) {
		TRACE_3("error in gzwrite item %" PRIu64 ": item length = %" PRIi64 " vs written = %" PRIi64 "\n", thisItem->key, len, ret); 
		return -1;
	}
	return ret;	
}

static INT _writeKeyItemsToBinaryFile(WRITER *writer, ITEM **items, UINT size) {
	INT ret = 0;
	UINT i = 0;	
	INT total = 0;
	ITEM *item = NULL;
	
	for(; i < size; i++){
		item = items[i];
		TRACE_1("Writing %" PRIi64 "-th item\n", i);
		if( (ret = _writeKeyItemToBinaryFile(writer, item)) <= 0 ) {
			return -1;			
		}
		total += ret;
	}
	return total;
}

static INT _closeBinaryKeyWriter(WRITER *writer) {
	gzFile fp = (gzFile)(writer->fp);
	gzflush(fp, Z_FINISH);
	gzclose(fp);
	FREE(writer);
	
	return 0;
}







WRITER *createPlainFastqWriter(FILE *file) {
	WRITER *writer = NULL;
	
	if((writer = (WRITER*)MALLOC(sizeof(WRITER))) == NULL){
		return NULL;
	}
		
	writer->fp = (CADDR_T)file;
	writer->writeItem = _writeFastqItemToPlainFile;
	writer->writeItems = _writeFastqItemsToPlainFile;
	writer->closeWriter = _closePlainFastqWriter;

	return writer;
}

static INT _writeFastqItemToPlainFile(WRITER *writer, ITEM *item) {
	INT ret = 0;
	INT len = 0;
	FILE *file = (FILE*)(writer->fp);
	FASTQ_ITEM *thisItem = (FASTQ_ITEM*)item;
	
	ret = fprintf(file, "%s\n%s\n%s\n%s\n", 
		thisItem->data->lines[0], thisItem->data->lines[1],
		thisItem->data->lines[2], thisItem->data->lines[3]);
	if( ret <= 0 ) {
		TRACE_1("error in writing item %" PRIu64 "\n", thisItem->key); 
		return 0;
	}
	return ret;	
}

static INT _writeFastqItemsToPlainFile(WRITER *writer, ITEM **items, UINT size) {
	INT ret = 0;
	UINT i = 0;	
	INT total = 0;
	ITEM *item = NULL;
	
	for(; i < size; i++){
		item = items[i];
		TRACE_1("Writing %" PRIi64 "-th item\n", i);
		if( (ret = _writeFastqItemToPlainFile(writer, item)) <= 0 ) {
			return -1;			
		}
		total += ret;
	}
	return total;
}

static INT _closePlainFastqWriter(WRITER *writer) {
	FILE *file = (FILE*)(writer->fp);
	fflush(file);
	fclose(file);
	FREE(writer);
	
	return 0;
}



