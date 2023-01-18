# Sleaping

## Fadso
Fadso is a tool that downsamples fastq data using sleaping, an efficient approximate reservoir sampling algorithm  It takes gzipped or plain fastq files as input and outputs gzipped or plain fastq files. [This](http://www.aaa.bbb.ccc) describes the sleaping method in detail.  Fadso was implemented in C.

## Installation of Fadso
This section shows an installation procedure using cmake. 

0. Install [zlib-devel](https://www.zlib.net/) as fadso uses the zlib API.
1. Download sleaping to SLEAPING_HOME.
2. Create a directory FADSO_BIN_DIR to house the fadso executable.
3. Go to FADSO_BIN_DIR, and do cmake SLEAPING_HOME, and do cmake --build .	
	

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

	-o <output-file>
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

###  example 1
Downsample gzipped fastq files for paired-end reads (in1.fastq.gz and in2.fatsq.gz) using sleaping with random seed 12345 to have 1 million read-pairs and output gzipped fastq files (out1.fastq.gz and out2.fastq.gz). There are two ways to do this: one using the single task twice, each with the same random seed and the other using the pair task. With single:

	fadso single -r 12345 -k 1000000 -i in1.fastq.gz -m sleaping -o out1.fastq.gz -z
	fadso single -r 12345 -k 1000000 -i in2.fastq.gz -m sleaping -o out2.fastq.gz -z

And with pair:
	
	fadso pair -r 12345 -k 1000000 -1 in1.fastq.gz -2 in2.fastq.gz -m sleaping -a out1.fastq.gz -b out2.fastq.gz -z
 	
		
###  example 2
Take input fastq data (either gzipped or plain) for single-end reads from standard input, downsample that to have 2 million reads using Algorithm R with random seed 123, and output a plain fastq file called out.fastq

	fadso single -r 123 -k 2000000 -i stdin -m r -o out.fastq
	 	    


