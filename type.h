#if !defined(HAVE_TYPE)
#define HAVE_TYPE

#include "common.h"

typedef struct {
	UINT key;
	CADDR_T data;	
} ITEM;

typedef struct {
	UINT key;
	CADDR_T data;	
} DATA_HOLDER;


typedef struct _ITEM_MANAGER ITEM_MANAGER;
typedef ITEM *CREATE_ITEM(ITEM_MANAGER *manager, DATA_HOLDER *data);
typedef INT COPY_CONTENT(ITEM_MANAGER *manager, ITEM *item, DATA_HOLDER *data);
typedef void FREE_ITEM(ITEM_MANAGER *manager, ITEM *item);

struct _ITEM_MANAGER {
    CREATE_ITEM *createItem;
	COPY_CONTENT *copyContent;
	FREE_ITEM *freeItem;
	int growthSize;
	CADDR_T rfu;	
};



typedef struct _READER READER;

typedef DATA_HOLDER *READ_ITEM(READER *reader);
typedef INT SKIP_ITEM(READER *reader);
typedef INT SKIP_ITEMS(READER *reader, UINT steps);
typedef DATA_HOLDER *SKIP_AND_READ(READER *reader, UINT steps);
typedef INT IS_EOF(READER *reader);
typedef UINT GET_CURRENT_INDEX(READER *reader);
typedef INT CLOSE_READER(READER *reader);

struct _READER {
	CADDR_T fp;
	UINT index;
    READ_ITEM *readNextItem;
	SKIP_ITEMS *skipItems;
	SKIP_AND_READ *skipAndReadItem;
	SKIP_ITEM *skipNextItem;
	GET_CURRENT_INDEX *getCurrentIndex;
	IS_EOF *isEOF;
	CLOSE_READER *closeReader;
	UINT bufSize;
	DATA_HOLDER *itemData;
	CADDR_T rfu;	
};


typedef struct _WRITER WRITER;
typedef INT WRITE_ITEM(WRITER *writer, ITEM *item);
typedef INT WRITE_ITEMS(WRITER *writer, ITEM **items, UINT size);
typedef INT CLOSE_WRITER(WRITER *writer);

struct _WRITER{
	CADDR_T fp;

	WRITE_ITEM *writeItem;
	WRITE_ITEMS *writeItems;
	CLOSE_WRITER *closeWriter;
	CADDR_T rfu;	
};


typedef struct _SUBSAMPLER SUBSAMPLER;
typedef INT INIT_SUBSAMPLER(SUBSAMPLER *subsampler, UINT subsampleSize, UINT totalSize);
typedef INT DOWNSAMPLE(SUBSAMPLER *subsampler, READER *reader, ITEM_MANAGER *itemManager, WRITER *writer);
typedef INT SORT_SUBSAMPLES(SUBSAMPLER *subsampler);
typedef ITEM** GET_SUBSAMPLES(SUBSAMPLER *subsampler);
typedef INT CLOSE_SUBSAMPLER(SUBSAMPLER *subsampler);

struct _SUBSAMPLER {
	UINT subsampleSize;
	UINT totalSize;
	ITEM **reservoir;
	
	INIT_SUBSAMPLER *initSubsampler;
	DOWNSAMPLE *downSample;
	SORT_SUBSAMPLES *sortSubsamples;
	GET_SUBSAMPLES *getSubsamples;
	CLOSE_SUBSAMPLER *closeSubsampler;
			
	CADDR_T rfu1;
	CADDR_T rfu2;
	CADDR_T rfu3;	
};


typedef struct _RECORD RECORD;


#define TASK_SINGLE_DOWNSAMPLING 0x0001
#define TASK_PAIR_DOWNSAMPLING 0x0002
#define TASK_MULTIPLE_DOWNSAMPLING 0x0003
#define TASK_COUNTING 0x0004
#define TASK_INDEXING 0x0005
#define TASK_EXTRACTING 0x0006

#define TASK_MASK 0x000F

#define CONSTRAINT_UNKNOWN_N 0x0010
#define CONSTRAINT_LONG_READS 0x0020


struct _RECORD {
    READER *reader;
	WRITER *writer; 
	ITEM_MANAGER *itemManager;
	SUBSAMPLER *subsampler;
	UINT subsampleSize;
	UINT totalSize;
	UINT readLen;
	int task;
	CADDR_T rfu1;
	CADDR_T rfu2;
	CADDR_T rfu3;	
};

#define FADSO_USAGE_MESSAGE \
"Usage: fadso <task> [task-attributes],\n" \
"where implemented tasks are currently 'single' and 'pair'.\n" \
"\tThe task 'single' downsamples fastq data from a single input file. This can be used for downsampling of signle-end or paired-end read data.\n" \
"\tThe attributes for 'single' are as follows:\n" \
"\t\t-r <random-seed>. This specifies the random seed for a random number generator. This attribute is required.\n" \
"\t\t-k <subsample-size>. This specifies the size of the downsampled data. This attribute is required.\n" \
"\t\t-i <input-file>. This specifies the input fastq file. If this option is not specified, then stdin is used as input.\n" \
"\t\t-o <output-file>. This specifies the output fastq filepath. If this option is not specified, then stdout is used as output.\n" \
"\t\t-m <downsampling-method>. This specifies which downsampling method to use.\n" \
"\t\t\tValid method names are 'sleaping' for s-leaping, 'l' for Algorithm L, 'r' for Algorithm R, and 'z' for Algorithm Z.\n" \
"\t\t\tIf this option is not specified, then s-leaping is used.\n" \
"\t\t-l <line-length>. This specifies the maximum length of the lines in the input fastq file,\n"\
"\t\t\twhich can be treated as the max read length. This option is mainly for long-read data.\n"\
"\t\t\tFor fastq files with read lengths smaller than 500, this option is not needed.\n" \
"\t\t-z. This option flags gzip compression of the output fastq data.\n" \
"\n" \
"\tThe task 'pair' downsamples fastq data from a pair of input files. This can be used for downsampling of paired-end read data.\n" \
"\tThe attributes for 'pair' are as follows:\n" \
"\t\t-r <random-seed>. This specifies the random seed for a random number generator. This attribute is required.\n" \
"\t\t-k <subsample-size>. This specifies the size of the downsampled data. This attribute is required.\n" \
"\t\t-1 <read1-input-file>. This specifies the input fastq file for read 1. This attribute is required.\n" \
"\t\t-2 <read2-input-file>. This specifies the input fastq file for read 2. This attribute is required.\n" \
"\t\t-a <read1-output-file>. This specifies the output fastq filepath for read 1. This attribute is required.\n" \
"\t\t-b <read2-output-file>. This specifies the output fastq filepath for read 2. This attribute is required.\n" \
"\t\t-m <downsampling-method>. This specifies which downsampling method to use.\n" \
"\t\t\tValid method names are 'sleaping' for s-leaping, 'l' for Algorithm L, 'r' for Algorithm R, and 'z' for Algorithm Z.\n" \
"\t\t\tIf this option is not specified, then s-leaping is used.\n" \
"\t\t-l <line-length>. This specifies the maximum length of the lines in the input fastq file,\n"\
"\t\t\twhich can be treated as the max read length. This option is mainly for long-read data.\n"\
"\t\t\tFor fastq files with read lengths smaller than 500, this option is not needed.\n" \
"\t\t-z. This option flags gzip compression of the output fastq data.\n" \
"\n"

#endif
