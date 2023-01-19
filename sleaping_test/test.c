#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>
#include <time.h>

#if !defined(UINT)
#define UINT uint64_t
#endif

#if !defined(INT)
#define INT int64_t
#endif


typedef int SAMPLER_FUNC(UINT *reservior, UINT reserviorSize, UINT totalSize);

static int _doAlgoR( UINT *reservior, UINT reserviorSize, UINT totalSize);
static int _doSleap( UINT *reservior, UINT reserviorSize, UINT totalSize);
static int _doAlgoL( UINT *reservior, UINT reserviorSize, UINT totalSize);
static int _doAlgoZ( UINT *reservior, UINT reserviorSize, UINT totalSize);


static void _setRandomSeed(UINT seed);
static double _getNextUniformRandomDouble( double minUniform, double maxUniform );
static UINT _getNextUniformRandomUINT( UINT minUniform, UINT maxUniform );
static double _getNextUnitUniformRandomExcl( );
static double _getNextExponentialRandom( double lambda );
static UINT _getNextGeometricRandom( double p );



int strToUINT(UINT *val, char *str);


static void init_genrand64(UINT seed); 
static UINT genrand64_int64();
static INT genrand64_int63();
/* generates a random number on [0,1]-real-interval */
static double genrand64_real1();

/* generates a random number on [0,1)-real-interval */
static double genrand64_real2();

/* generates a random number on (0,1)-real-interval */
static double genrand64_real3();

#define PMF_BIN_COUNT 1000

typedef struct{
	double mean;
	int pmf[PMF_BIN_COUNT];	
} STAT_T;

static int _computeStats( UINT *reservior, UINT len, UINT max, STAT_T *stat);

#define SLEAP_SIZE_PROP 0.005 

//#define ALGO_R
//#define ALGO_L
#define LEAP

int main(int argc, char *argv[]){
	UINT i = 0;
	char *name = "sleap";
	clock_t startTime;
	clock_t endTime;
	double runTimeInSec = 0.0;
	UINT *reservior = NULL;
	UINT reserviorSize = 50000000;	
	UINT totalSize = 400000000;
	UINT randSeed = 1236553;
	STAT_T s;
	SAMPLER_FUNC *func = _doSleap;;
	
	if(argc < 5){
		fprintf(stderr, "usage: <program> <method> <random_seed> <reservoir_size> <total_size>\n");
		return 1;
	}
	if(strcmp(argv[1], "R") == 0) {
		name =  "R";
		func = _doAlgoR;
	}
	else if( strcmp( argv[1], "L") == 0) {
		name =  "L";
		func = _doAlgoL;
	}
	else if( strcmp(argv[1], "S") == 0){
		name =  "S";
		func = _doSleap;
	}
	else if( strcmp(argv[1], "S_log") == 0){
		name =  "S_log";
		func = _doSleap;
	}
	else if( strcmp(argv[1], "Z") == 0){
		name =  "Z";
		func = _doAlgoZ;
	}
	
	strToUINT(&randSeed, argv[2]);
	strToUINT(&reserviorSize, argv[3]);
	strToUINT(&totalSize, argv[4]);
	
	
	if((reservior = (UINT*)malloc(sizeof(UINT)*reserviorSize)) == NULL){
		fprintf(stderr, "error in creating a reservoir\n");
		return -1;
	}

	
	_setRandomSeed(randSeed);
	
	startTime = clock();	
	func( reservior, reserviorSize,  totalSize);
	endTime = clock();
	
	runTimeInSec = ((double)(endTime - startTime))/CLOCKS_PER_SEC;
	_computeStats(reservior, reserviorSize, totalSize, &s);
	
	fprintf(stderr, "%s\t%f\t%f\n", name, runTimeInSec, s.mean);	

	printf("permille\tcount\n");	
	for(i = 0; i < PMF_BIN_COUNT; i++){
		printf("%lu\t%i\n", i, s.pmf[i]); 
	}
	
	return 0;
}

static int _computeStats( UINT *reservior, UINT len, UINT max, STAT_T *stat){
	UINT i = 1;
	UINT j = 0;
	double m = 0.0;
	double v = 0.0;
	
	for(j = 0; j < PMF_BIN_COUNT; j++){
		stat->pmf[j] = 0;
	}
	
	m = (double)(reservior[0]);
	j = (int)((m/max)*PMF_BIN_COUNT);
	stat->pmf[j]++;
	for(i = 1; i < len; i++) {
		v = (double)reservior[i];
		m = m + (double)(v-m)/(i+1);
		
		j = (int)((v/max)*PMF_BIN_COUNT);
		stat->pmf[j]++;
	}
	stat->mean = m;
	return 0;
}


static int _doAlgoR( UINT *reservior, UINT reserviorSize, UINT totalSize) {
	UINT i = 0;
	UINT j = 0;
	UINT ranIndex = 0;
	
	
	for(i = 0; i < reserviorSize; i++){
		reservior[i] = i;
	}
	
	for(; i < totalSize; i++){
		ranIndex = (UINT)(_getNextUnitUniformRandomExcl() * (i+1));
		if(ranIndex < reserviorSize){
			reservior[ranIndex] = i;
		}
	}
	return 0;
}


static int _doSleap( UINT *reservior, UINT reserviorSize, UINT totalSize) {
	UINT i = 0;
	UINT j = 0;
	UINT k = 0;
	UINT nextStep = 0;
	UINT leapSize = (UINT)reserviorSize*SLEAP_SIZE_PROP;
	UINT ranIndex = 0;
	UINT endR = (UINT)(reserviorSize<<1);	
	double p = 0.0;
	double u = 0.0;
		
	for(i = 0; i < reserviorSize; i++){
		reservior[i] = i;
	}
	if(endR >= totalSize){
		for(; i < totalSize; i++){
			ranIndex = (UINT)(_getNextUnitUniformRandomExcl() * (i+1));
			if(ranIndex < reserviorSize){
				reservior[ranIndex] = i;
			}
		}
		return 0;
	}
	
	for(; i < endR; i++){
		ranIndex = (UINT)(_getNextUnitUniformRandomExcl() * (i+1));
		if(ranIndex < reserviorSize){
			reservior[ranIndex] = i;
		}
	}
#if 1	
	double lambda = 0.0;
	for(i -= 1; i < totalSize; i += leapSize){
		p = (reserviorSize)/(double)(i+(leapSize>>1)+1);
		lambda = log(1.0 - p);
		j = 0;
		while(j < leapSize) {
			j += (UINT)(log(_getNextUnitUniformRandomExcl())/lambda) + (UINT)1;
			if(j >= leapSize){
				break;
			}
			k = i + j;
			if(k >= totalSize){
				return 0;
			}
			ranIndex = (UINT)(_getNextUnitUniformRandomExcl() * (reserviorSize));
			reservior[ranIndex] = k;
		}
	}
#else 
	double q = 0.0;
	double threshold = 0.0;
	for(i -= 1; i < totalSize; i += leapSize){
		p = (reserviorSize)/(double)(i+(leapSize>>1)+1);
		q = 1.0 - p;
		j = 0;
		while(j < leapSize) {
			threshold = p;
			u = _getNextUnitUniformRandomExcl();
			nextStep = 1;
			while(threshold < u) {
				u -= threshold;
				threshold = threshold * q;
				nextStep += 1;
			}
			j += nextStep;
			if(j >= leapSize){
				break;
			}
			k = i + j;
			if(k >= totalSize){
				return 0;
			}
			ranIndex = (UINT)(_getNextUnitUniformRandomExcl() * (reserviorSize));
			reservior[ranIndex] = k;
		}
	}
#endif
	return 0;
}



static int _doAlgoL( UINT *reservior, UINT reserviorSize, UINT totalSize) {
	UINT i = 0;
	UINT j = 0;
	UINT ranIndex = 0;
	double max = 0.0;
		
	for(i = 0; i < reserviorSize; i++){
		reservior[i] = i;
	}
	
	max = exp(log(_getNextUnitUniformRandomExcl())/(double)reserviorSize);
	i -= 1;
	while(i < totalSize){
		i = i + (UINT)(_getNextExponentialRandom(-log(1.0-max))) + (UINT)1;
		//fprintf(stderr, "i = %lu; max = %f\n", i, max);
		if(i < totalSize) {
			ranIndex = (UINT)(_getNextUnitUniformRandomExcl() * (reserviorSize));
			reservior[ranIndex] = i;
			max = max * exp(log(_getNextUnitUniformRandomExcl())/(double)reserviorSize);
		}
	}
	return 0;
}


#define Z_THRESHOLD_FACTOR 22

static int _doAlgoZ( UINT *reservior, UINT reserviorSize, UINT totalSize) {
	UINT i = 0;
	UINT j = 0;
	UINT threshold = 0;
	UINT num = 0;
	UINT skipSize = 0;
	UINT ranIndex = 0;
	UINT term = 0;
	UINT denom = 0;
	UINT numer_lim = 0;

	double v = 0.0; 
	double quot = 0.0;
	double skipRan = 0.0;
	double w = 0.0;
	double lhs = 0.0;
	double rhs = 0.0;
	double y = 0.0;
	
		
	for(i = 0; i < reserviorSize; i++){
		reservior[i] = i;
	}
	i -= 1;
	
/* first algo X */
	num = 0;
	threshold = Z_THRESHOLD_FACTOR * reserviorSize;
	while((i < totalSize) && (i <= threshold)){
		v = _getNextUnitUniformRandomExcl();
		i += 1;
		num += 1;
		quot = (double)num/i;
		while(quot > v){
			i += 1;
			num += 1;
			quot = (quot * num)/i;
		}
		if(i < totalSize){
			ranIndex = (UINT)(_getNextUnitUniformRandomExcl() * (reserviorSize));
			reservior[ranIndex] = i;			
		}
	}

/* fot i > threshold, do the rejection method */
	w = exp(-log(_getNextUnitUniformRandomExcl())/(double)reserviorSize);
	term = i - reserviorSize - 1;
	while(i < totalSize){
		for(;;){
			v = _getNextUnitUniformRandomExcl();
			skipRan = i * (w - 1.0);
			skipSize = (UINT)skipRan;
			lhs = exp(log(((v*pow((i + 1)/term, 2.0)) * (term + skipSize))/(i + skipRan))/reserviorSize);
			rhs = (((i + skipRan)/(term + skipSize)) * term)/i;
			if(lhs <= rhs){
				w = rhs/lhs;
				break;
			}
			y = (((v*(i+1))/term) * (i + skipSize + 1))/(i + skipRan);
			if(reserviorSize < skipSize){
				denom = i;
				numer_lim = term + skipSize;
			}
			else{
				denom = i - reserviorSize + skipSize;
				numer_lim = i + 1;				
			}
			for(j = i + skipSize; j > numer_lim; j--){
				y = (y*j)/denom;
				denom -= 1;
			}
			w = exp(-log(_getNextUnitUniformRandomExcl())/(double)reserviorSize);
			if(exp(log(y)/reserviorSize) <= (i + skipRan)/i){
				break;				
			}
		}
		i += skipSize;		
		if(i < totalSize){
			ranIndex = (UINT)(_getNextUnitUniformRandomExcl() * (reserviorSize));
			reservior[ranIndex] = i;			
		}
		i += 1;
		term += skipSize + 1;
	}
	return 0;
}














static void _setRandomSeed (UINT seed) {
	init_genrand64(seed);	
}


static double _getNextUniformRandomDouble ( double minUniform, double maxUniform ) {
    double t = genrand64_real1();
    double random = t * ( maxUniform - minUniform ) + minUniform;

    return random;    
}

static UINT _getNextUniformRandomUINT ( UINT minUniform, UINT maxUniform ) {
 
	UINT value = genrand64_int64();
    UINT random = (value % (maxUniform-minUniform+1)) + minUniform;
	
    return random;    
}




/* 
 * generate PRN in [0,1] inclusive.
 * 
 */
static double getNextUnitUniformRandomIncl( ) {
    double random = genrand64_real1();
    return random;
}

/* 
 * generate PRN in (0,1) exclusive.
 * 
 */
static double _getNextUnitUniformRandomExcl( ) {
    double random = genrand64_real3();
    return random;
}



static double _getNextExponentialRandom ( double lambda ) {
	double u = genrand64_real3();
	return -log(u)/lambda;	
}


static UINT _getNextGeometricRandom ( double p ) {
	double u = genrand64_real3();
	UINT g = (UINT)(log(u)/(log(1.0 - p)));
	return g;
}





int strToUINT(UINT *val, char *str) {
	char *endp = NULL;
	
	*val = strtoull( str, &endp, 0 );

	if( *endp != '\0' ) {
		return -1; 
	}
	return 0;
}






/* 
 * PRNG algorithms below were taken from MT19937-64: 
 * 
   A C-program for MT19937-64 (2004/9/29 version).
   Coded by Takuji Nishimura and Makoto Matsumoto.

   This is a 64-bit version of Mersenne Twister pseudorandom number
   generator.

   Copyright (C) 2004, Makoto Matsumoto and Takuji Nishimura,
   All rights reserved.                          
*/
#define RAND_NN 312
#define RAND_MM 156
#define RAND_MATRIX_A 0xB5026F5AA96619E9ULL
#define RAND_UM 0xFFFFFFFF80000000ULL /* Most significant 33 bits */
#define RAND_LM 0x7FFFFFFFULL /* Least significant 31 bits */


/* The array for the state vector */
static UINT mt[RAND_NN]; 
/* mti==RAND_NN+1 means mt[RAND_NN] is not initialized */
static INT mti=RAND_NN+1; 

/* initializes mt[RAND_NN] with a seed */
static void init_genrand64(UINT seed)
{
    mt[0] = seed;
    for (mti=1; mti<RAND_NN; mti++) 
        mt[mti] =  (6364136223846793005ULL * (mt[mti-1] ^ (mt[mti-1] >> 62)) + mti);
}


/* generates a random number on [0, 2^64-1]-interval */
static UINT genrand64_int64()
{
    int i;
    UINT x;
    static UINT mag01[2]={0ULL, RAND_MATRIX_A};

    if (mti >= RAND_NN) { /* generate RAND_NN words at one time */

        /* if init_genrand64() has not been called, */
        /* a default initial seed is used     */
        if (mti == RAND_NN+1) 
            init_genrand64(5489ULL); 

        for (i=0;i<RAND_NN-RAND_MM;i++) {
            x = (mt[i]&RAND_UM)|(mt[i+1]&RAND_LM);
            mt[i] = mt[i+RAND_MM] ^ (x>>1) ^ mag01[(int)(x&1ULL)];
        }
        for (;i<RAND_NN-1;i++) {
            x = (mt[i]&RAND_UM)|(mt[i+1]&RAND_LM);
            mt[i] = mt[i+(RAND_MM-RAND_NN)] ^ (x>>1) ^ mag01[(int)(x&1ULL)];
        }
        x = (mt[RAND_NN-1]&RAND_UM)|(mt[0]&RAND_LM);
        mt[RAND_NN-1] = mt[RAND_MM-1] ^ (x>>1) ^ mag01[(int)(x&1ULL)];

        mti = 0;
    }
  
    x = mt[mti++];

    x ^= (x >> 29) & 0x5555555555555555ULL;
    x ^= (x << 17) & 0x71D67FFFEDA60000ULL;
    x ^= (x << 37) & 0xFFF7EEE000000000ULL;
    x ^= (x >> 43);

    return x;
}

/* generates a random number on [0, 2^63-1]-interval */
static INT genrand64_int63()
{
    return (INT)(genrand64_int64() >> 1);
}

/* generates a random number on [0,1]-real-interval */
static double genrand64_real1()
{
    return (genrand64_int64() >> 11) * (1.0/9007199254740991.0);
}

/* generates a random number on [0,1)-real-interval */
static double genrand64_real2()
{
    return (genrand64_int64() >> 11) * (1.0/9007199254740992.0);
}

/* generates a random number on (0,1)-real-interval */
static double genrand64_real3()
{
    return ((genrand64_int64() >> 12) + 0.5) * (1.0/4503599627370496.0);
}




