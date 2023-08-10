
#include <stdio.h>
#include <string.h>
#include <zlib.h>

#include "common.h"
#include "log.h"

#include "paired_fastq_item_manager.h"



static ITEM_MANAGER *_createFastqItemManager();
static ITEM *_createItem(ITEM_MANAGER *manager, DATA_HOLDER *src);
static INT _copyContent(ITEM_MANAGER *manager, ITEM *dest, DATA_HOLDER *src);
static void _freeItem(ITEM_MANAGER *manager, ITEM *item);


ITEM_MANAGER *createPairedFastqItemManager() {
	ITEM_MANAGER *itemManager = NULL;
	
	if((itemManager = _createFastqItemManager()) != NULL) {
		return itemManager;		
	}
	return NULL;	
}



static ITEM_MANAGER *_createFastqItemManager() {
	ITEM_MANAGER *itemManager = NULL;
	
	if((itemManager = (ITEM_MANAGER*)MALLOC(sizeof(ITEM_MANAGER))) == NULL){
		fprintf(stderr, "error in allocating item manager\n");
		return NULL;
	}
	
	itemManager->createItem = _createItem;
	itemManager->copyContent = _copyContent;
	itemManager->freeItem = _freeItem;
	itemManager->growthSize = 10;

	return itemManager;
}



static ITEM *_createItem(ITEM_MANAGER *manager, DATA_HOLDER *src) {
	int i = 0;
	int len = 0;
	int growthSize = manager->growthSize;
	
	char *buf = NULL;
	PAIRED_FASTQ_DATA_HOLDER *holder = (PAIRED_FASTQ_DATA_HOLDER*)src;
	PAIRED_FASTQ_ITEM *item = NULL;
	PAIRED_FASTQ_DATA *data1 = NULL;
	PAIRED_FASTQ_DATA *data2 = NULL;
	
	if((data1 = (PAIRED_FASTQ_DATA*)MALLOC(sizeof(PAIRED_FASTQ_DATA))) == NULL){
		return NULL;		
	}
	if((data2 = (PAIRED_FASTQ_DATA*)MALLOC(sizeof(PAIRED_FASTQ_DATA))) == NULL){
		return NULL;		
	}
	if((item = (PAIRED_FASTQ_ITEM*)MALLOC(sizeof(PAIRED_FASTQ_ITEM))) == NULL){
		return NULL;		
	}

	for(i = 0; i < 4; i++){
		buf = holder->content->lines1[i];
		len = strlen(buf) - 1;
		if(buf[len] == '\n'){
			buf[len] = '\0';
		}
		if((data1->lines[i] = (char*)MALLOC(sizeof(char)*(len+growthSize))) == NULL) {
			return NULL;		
		}
		data1->sizes[i] = len+growthSize;
		strcpy(data1->lines[i], buf);
		
		buf = holder->content->lines2[i];
		len = strlen(buf) - 1;
		if(buf[len] == '\n'){
			buf[len] = '\0';
		}
		if((data2->lines[i] = (char*)MALLOC(sizeof(char)*(len+growthSize))) == NULL) {
			return NULL;		
		}
		data2->sizes[i] = len+growthSize;
		strcpy(data2->lines[i], buf);		
	}
	item->key = holder->key;
	item->data1 = data1;
	item->data2 = data2;

	return (ITEM*)item;
}

static INT _copyContent(ITEM_MANAGER *manager, ITEM *dest, DATA_HOLDER *src) {
	int i = 0;
	int len = 0;
	int growthSize = manager->growthSize;
	
	char *buf = NULL;
	PAIRED_FASTQ_DATA_HOLDER *holder = (PAIRED_FASTQ_DATA_HOLDER*)src;
	PAIRED_FASTQ_ITEM *item = (PAIRED_FASTQ_ITEM*)dest;
	PAIRED_FASTQ_DATA *data1 = item->data1;
	PAIRED_FASTQ_DATA *data2 = item->data2;

		
	for(i = 0; i < 4; i++){
		buf = holder->content->lines1[i];
		len = strlen(buf) - 1;
		if(buf[len] == '\n'){
			buf[len] = '\0';
		}
		if(data1->sizes[i] <= len){
			if((data1->lines[i] = (char*)REALLOC(data1->lines[i], sizeof(char)*(len+growthSize))) == NULL) {
				return -1;		
			}
			data1->sizes[i] = len+growthSize;
		}
		strcpy(data1->lines[i], buf);

		buf = holder->content->lines2[i];
		len = strlen(buf) - 1;
		if(buf[len] == '\n'){
			buf[len] = '\0';
		}
		if(data2->sizes[i] <= len){
			if((data2->lines[i] = (char*)REALLOC(data2->lines[i], sizeof(char)*(len+growthSize))) == NULL) {
				return -1;		
			}
			data2->sizes[i] = len+growthSize;
		}
		strcpy(data2->lines[i], buf);
	}
	item->key = holder->key;
	
	return 0;
}

static void _freeItem(ITEM_MANAGER *manager, ITEM *item) {
	PAIRED_FASTQ_ITEM *fastqItem = (PAIRED_FASTQ_ITEM*)item;
	PAIRED_FASTQ_DATA *fastqData = NULL;
	
	if(fastqItem != NULL){
		fastqData = fastqItem->data1;
		if(fastqData != NULL) {
			FREE(fastqData->lines[0]);
			FREE(fastqData->lines[1]);
			FREE(fastqData->lines[2]);
			FREE(fastqData->lines[3]);
			FREE(fastqData);
			FREE(fastqItem);
		}
		fastqData = fastqItem->data2;
		if(fastqData != NULL) {
			FREE(fastqData->lines[0]);
			FREE(fastqData->lines[1]);
			FREE(fastqData->lines[2]);
			FREE(fastqData->lines[3]);
			FREE(fastqData);
			FREE(fastqItem);
		}
	}
	
}
