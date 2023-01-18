#if !defined(HAVE_FASTQ_WRITER)
#define HAVE_FASTQ_WRITER

#include "type.h"

#if !defined(WRITE_BUFFER_SIZE)	
#define WRITE_BUFFER_SIZE (1<<18)	
#endif	


WRITER *createZippedFastqWriter(FILE *file);
WRITER *createPlainFastqWriter(FILE *file);


#endif
