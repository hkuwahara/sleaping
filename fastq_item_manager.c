
#include <stdio.h>
#include <string.h>
#include <zlib.h>

#include "common.h"
#include "log.h"

#include "fastq_item_manager.h"



static ITEM_MANAGER *_createFastqItemManager();
static ITEM *_createItem(ITEM_MANAGER *manager, DATA_HOLDER *src);
static INT _copyContent(ITEM_MANAGER *manager, ITEM *dest, DATA_HOLDER *src);
static void _freeItem(ITEM_MANAGER *manager, ITEM *item);


ITEM_MANAGER *createFastqItemManager() {
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
	FASTQ_DATA_HOLDER *holder = (FASTQ_DATA_HOLDER*)src;
	FASTQ_ITEM *item = NULL;
	FASTQ_DATA *data = NULL;
	
	if((data = (FASTQ_DATA*)MALLOC(sizeof(FASTQ_DATA))) == NULL){
		return NULL;		
	}
	if((item = (FASTQ_ITEM*)MALLOC(sizeof(FASTQ_ITEM))) == NULL){
		return NULL;		
	}

	for(i = 0; i < (UINT)4; i++){
		buf = holder->content->lines[i];
		len = strlen(buf)-1;
		if(buf[len] == '\n'){
			buf[len] = '\0';
		}
		TRACE_3("ID = %x\tnew entry = %s; size = %i\n", (int)(&(data->lines[i])), buf, len + growthSize); 
		if((data->lines[i] = (char*)MALLOC(sizeof(char)*(len+growthSize))) == NULL) {
			return NULL;		
		}
		data->sizes[i] = len+growthSize;
		strcpy(data->lines[i], buf);
	}
	item->key = holder->key;
	item->data = data;
	
	return (ITEM*)item;
}

static INT _copyContent(ITEM_MANAGER *manager, ITEM *dest, DATA_HOLDER *src) {
	int i = 0;
	int len = 0;
	int growthSize = manager->growthSize;
	char *buf = NULL;
	FASTQ_DATA_HOLDER *holder = (FASTQ_DATA_HOLDER*)src;
	FASTQ_ITEM *item = (FASTQ_ITEM*)dest;
	FASTQ_DATA *data = item->data;
		
	for(i = 0; i < (UINT)4; i++){
		buf = holder->content->lines[i];
		len = strlen(buf)-1;
		if(buf[len] == '\n'){
			buf[len] = '\0';
		}
		if(data->sizes[i] <= len){
			if((data->lines[i] = (char*)REALLOC(data->lines[i], sizeof(char)*(len+growthSize))) == NULL) {
				return -1;		
			}
			data->sizes[i] = len+growthSize;
		}
		strcpy(data->lines[i], buf);
	}
	item->key = holder->key;
	
	return 0;
	
}

static void _freeItem(ITEM_MANAGER *manager, ITEM *item) {
	FASTQ_ITEM *fastqItem = (FASTQ_ITEM*)item;
	FASTQ_DATA *fastqData = NULL;
	
	if(fastqItem != NULL){
		fastqData = fastqItem->data;
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

