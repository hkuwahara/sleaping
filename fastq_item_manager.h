#if !defined(HAVE_FASTQ_ITEM_MANAGER)
#define HAVE_FASTQ_ITEM_MANAGER

#include "common.h"
#include "type.h"

typedef struct {
	char *lines[4];
	int sizes[4];
} FASTQ_DATA;


typedef struct {
	UINT key;
	FASTQ_DATA *data;	
} FASTQ_ITEM;


typedef struct {
	char *lines[4];
} FASTQ_CONTENT;

typedef struct {
	UINT key;
	FASTQ_CONTENT *content;
} FASTQ_DATA_HOLDER;


ITEM_MANAGER *createFastqItemManager();

#endif
