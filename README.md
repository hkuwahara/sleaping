# Sleaping


## Installation


## Usage

	Usage: fadso <task> [task-attributes]
 where implemented tasks are currently 'single' and 'pair'.  
 
### The attributes for 'single'
The task 'single' downsamples fastq data from a single input file. This can be used for downsampling of signle-end or paired-end read data. The attributes for 'single' are as follows

	-r <random-seed>
>This specifies the random seed for a random number generator. This attribute is required.

	-k <subsample-size> 
>This specifies the size of the downsampled data. This attribute is required.

	-i <input-file>
> This specifies the input fastq file. If this option is not specified, then stdin is used as input.

	-o <output-file>.
>This specifies the output fastq filepath. If this option is not specified, then stdout is used as output.

	-m {sleaping | r}
>This specifies which downsampling method to use. Valid method names are 'sleaping' for s-leaping and 'r' for Algorithm R. If this option is not specified, then s-leaping is used.

	-l <line-length>
>This specifies the maximum length of the lines in the input fastq file, which can be treated as the max read length. This option is mainly for long-read data. For fastq files with read lengths smaller than 500, this option is not needed.

	-z
>This option flags gzip compression of the output fastq data.

### The attributes for 'pair'

The task 'pair' downsamples fastq data from a pair of input files. This can be used for downsampling of paired-end read data. The attributes for 'pair' are as follows:

	-r <random-seed>
>This specifies the random seed for a random number generator. This attribute is required.

	-k <subsample-size> 
>This specifies the size of the downsampled data. This attribute is required.

	-1 <read1-input-file>
>This specifies the input fastq file for read 1. This attribute is required.

	-2 <read2-input-file>
>This specifies the input fastq file for read 2. This attribute is required.

	-a <read1-output-file>
>This specifies the output fastq filepath for read 1. This attribute is required.

	-b <read2-output-file>
>This specifies the output fastq filepath for read 2. This attribute is required.

	-m {sleaping | r}
>This specifies which downsampling method to use. Valid method names are 'sleaping' for s-leaping and 'r' for Algorithm R. If this option is not specified, then s-leaping is used.

	-l <line-length>
>This specifies the maximum length of the lines in the input fastq file, which can be treated as the max read length. This option is mainly for long-read data. For fastq files with read lengths smaller than 500, this option is not needed.

	-z
>This option flags gzip compression of the output fastq data.


## Examples


