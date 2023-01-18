#if !defined(HAVE_PAIRED_FASTQ_READER)
#define HAVE_PAIRED_FASTQ_READER

#include "common.h"
#include "type.h"

READER *createZippedPairedFastqReader(FILE *file1, FILE *file2, UINT readLen);
READER *createZippedPairedFastqReader2(FILE *file1, FILE *file2n);


#endif
