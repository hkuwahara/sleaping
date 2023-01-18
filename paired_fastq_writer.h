#if !defined(HAVE_PAIRED_FASTQ_WRITER)
#define HAVE_PAIRED_FASTQ_WRITER

#include "type.h"

#if !defined(WRITE_BUFFER_SIZE)	
#define WRITE_BUFFER_SIZE (1<<18)	
#endif	


WRITER *createZippedPairedFastqWriter(FILE *file1, FILE *file2);
WRITER *createPlainPairedFastqWriter(FILE *file1, FILE *file2);


#endif
