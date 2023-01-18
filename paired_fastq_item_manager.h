#if !defined(HAVE_PAIRED_FASTQ_ITEM_MANAGER)
#define HAVE_PAIRED_FASTQ_ITEM_MANAGER

#include "common.h"
#include "type.h"

typedef struct {
	char *lines[4];
	int sizes[4];
} PAIRED_FASTQ_DATA;

typedef struct {
	UINT key;
	PAIRED_FASTQ_DATA *data1;	
	PAIRED_FASTQ_DATA *data2;	
} PAIRED_FASTQ_ITEM;


typedef struct {
	char *lines1[4];
	char *lines2[4];
} PAIRED_FASTQ_CONTENT;

typedef struct {
	UINT key;
	PAIRED_FASTQ_CONTENT *content;
} PAIRED_FASTQ_DATA_HOLDER;


ITEM_MANAGER *createPairedFastqItemManager();

#endif
