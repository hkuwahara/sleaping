#if !defined(HAVE_FASTQ_READER)
#define HAVE_FASTQ_READER

#include "type.h"

READER *createZippedFastqReader(FILE *file, UINT readLen);
READER *createZippedFastqReader2(FILE *file);

/*
READER *createPlainFastqReader(FILE *file);
*/

#endif
