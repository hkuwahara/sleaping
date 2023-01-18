#if !defined(HAVE_PAIRED_FASTQ_ITEM)
#define HAVE_PAIRED_FASTQ_ITEM

#include "common.h"
#include "type.h"
#include "fastq_item.h"

typedef struct {
	UINT key;
	FASTQ_DATA *data1;	
	FASTQ_DATA *data2;	
} PAIRED_FASTQ_ITEM;


#endif
