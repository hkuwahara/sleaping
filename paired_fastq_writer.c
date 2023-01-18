#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <inttypes.h>

#include "common.h"
#include "log.h"
#include "paired_fastq_writer.h"
#include "paired_fastq_item_manager.h"

static INT _writeKeyItemToBinaryFile(WRITER *writer, ITEM *item);
static INT _writeKeyItemsToBinaryFile(WRITER *writer, ITEM **items, UINT size);
static INT _closeBinaryKeyWriter(WRITER *writer);


static INT _writeFastqItemToPlainFile(WRITER *writer, ITEM *item);
static INT _writeFastqItemsToPlainFile(WRITER *writer, ITEM **items, UINT size);
static INT _closePlainFastqWriter(WRITER *writer);

typedef struct {
	gzFile fp1;
	gzFile fp2;
} PAIRED_FP_GZ;


WRITER *createZippedPairedFastqWriter(FILE *file1, FILE *file2) {
	gzFile gzfp1 = Z_NULL;
	gzFile gzfp2 = Z_NULL;
	WRITER *writer = NULL;
	PAIRED_FP_GZ *pairedFp = NULL;
	
	if((pairedFp = (PAIRED_FP_GZ*)MALLOC(sizeof(PAIRED_FP_GZ))) == NULL){
		return NULL;
	}
	if((writer = (WRITER*)MALLOC(sizeof(WRITER))) == NULL){
		return NULL;
	}
	
	if( ((gzfp1 = gzdopen(file1->_fileno, "w")) != Z_NULL) && ((gzfp2 = gzdopen(file2->_fileno, "w")) != Z_NULL) ) {
		pairedFp->fp1 = gzfp1;
		pairedFp->fp2 = gzfp2;
		writer->writeItem = _writeKeyItemToBinaryFile;
		writer->writeItems = _writeKeyItemsToBinaryFile;
		writer->closeWriter = _closeBinaryKeyWriter;
		writer->fp = (CADDR_T)pairedFp;
		return writer;
	}

	return NULL;
}

static INT _writeKeyItemToBinaryFile(WRITER *writer, ITEM *item) {
	INT ret = 0;
	INT len = 0;
	PAIRED_FASTQ_ITEM *thisItem = (PAIRED_FASTQ_ITEM*)item;
	gzFile gzfp1 = ((PAIRED_FP_GZ*)writer->fp)->fp1;
	gzFile gzfp2 = ((PAIRED_FP_GZ*)writer->fp)->fp2;
	char buf[WRITE_BUFFER_SIZE];
	
	ret = sprintf(buf, "%s\n%s\n%s\n%s\n", 
		thisItem->data1->lines[0], thisItem->data1->lines[1],
		thisItem->data1->lines[2], thisItem->data1->lines[3]);
	if( ret <= 0 ) {
		TRACE_1("error in creating a buffer for item %" PRIu64 "\n", thisItem->key); 
		return 0;
	}
	len = strlen(buf);
	ret = gzwrite(gzfp1, buf, len);
	if( ret != len ) {
		TRACE_3("error in gzwrite item %" PRIu64 ": item length = %" PRIi64 " vs written = %" PRIi64 "\n", thisItem->key, len, ret); 
		return -1;
	}
	
	ret = sprintf(buf, "%s\n%s\n%s\n%s\n", 
		thisItem->data2->lines[0], thisItem->data2->lines[1],
		thisItem->data2->lines[2], thisItem->data2->lines[3]);
	if( ret <= 0 ) {
		TRACE_1("error in creating a buffer for item %" PRIu64 "\n", thisItem->key); 
		return 0;
	}
	len = strlen(buf);
	ret = gzwrite(gzfp2, buf, len);
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
	PAIRED_FP_GZ *pairedFp = (PAIRED_FP_GZ*)(writer->fp);	
	gzFile fp = pairedFp->fp1;
	gzflush(fp, Z_FINISH);
	gzclose(fp);

	fp = pairedFp->fp2;
	gzflush(fp, Z_FINISH);
	gzclose(fp);

	FREE(pairedFp);
	FREE(writer);
	
	return 0;
}




typedef struct {
	FILE *fp1;
	FILE *fp2;
} PAIRED_FP_PLAIN;


WRITER *createPlainPairedFastqWriter(FILE *file1, FILE *file2) {
	WRITER *writer = NULL;
	PAIRED_FP_PLAIN *pairedFp = NULL;
	
	if((pairedFp = (PAIRED_FP_PLAIN*)MALLOC(sizeof(PAIRED_FP_PLAIN))) == NULL){
		return NULL;
	}
	
	if((writer = (WRITER*)MALLOC(sizeof(WRITER))) == NULL){
		return NULL;
	}
		
	pairedFp->fp1 = file1;
	pairedFp->fp2 = file2;
	
	writer->writeItem = _writeFastqItemToPlainFile;
	writer->writeItems = _writeFastqItemsToPlainFile;
	writer->closeWriter = _closePlainFastqWriter;
	writer->fp = (CADDR_T)pairedFp;

	return writer;
}

static INT _writeFastqItemToPlainFile(WRITER *writer, ITEM *item) {
	INT ret = 0;
	INT len = 0;
	FILE *file = NULL;
	PAIRED_FP_PLAIN *pairedFp = (PAIRED_FP_PLAIN*)(writer->fp);
	PAIRED_FASTQ_ITEM *thisItem = (PAIRED_FASTQ_ITEM*)item;
	
	file = pairedFp->fp1;
	ret = fprintf(file, "%s\n%s\n%s\n%s\n", 
		thisItem->data1->lines[0], thisItem->data1->lines[1],
		thisItem->data1->lines[2], thisItem->data1->lines[3]);
	if( ret <= 0 ) {
		TRACE_1("error in writing item %" PRIu64 "\n", thisItem->key); 
		return 0;
	}

	file = pairedFp->fp2;
	ret = fprintf(file, "%s\n%s\n%s\n%s\n", 
		thisItem->data2->lines[0], thisItem->data2->lines[1],
		thisItem->data2->lines[2], thisItem->data2->lines[3]);
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
	FILE *file = NULL;
	PAIRED_FP_PLAIN *pairedFp = (PAIRED_FP_PLAIN*)(writer->fp);

	file = pairedFp->fp1;
	fflush(file);
	fclose(file);

	file = pairedFp->fp2;
	fflush(file);
	fclose(file);

	FREE(pairedFp);
	FREE(writer);
	
	return 0;
}



