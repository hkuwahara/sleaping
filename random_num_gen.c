#include <math.h>

#include "random_num_gen.h"
#include "log.h"

static void init_genrand64(UINT seed); 
static UINT genrand64_int64();
static INT genrand64_int63();
/* generates a random number on [0,1]-real-interval */
static double genrand64_real1();

/* generates a random number on [0,1)-real-interval */
static double genrand64_real2();

/* generates a random number on (0,1)-real-interval */
static double genrand64_real3();




void setRandomSeed(UINT seed) {
	init_genrand64(seed);	
}


double getNextUniformRandomDouble( double minUniform, double maxUniform ) {
    double t = genrand64_real1();
    double random = t * ( maxUniform - minUniform ) + minUniform;

    return random;    
}

UINT getNextUniformRandomUINT( UINT minUniform, UINT maxUniform ) {
 
	UINT value = genrand64_int64();
    UINT random = (value % (maxUniform-minUniform+1)) + minUniform;
	
    return random;    
}


double getNextNormalRandom( double mean, double stdDeviation ) {
    double unitNormal = 0.0;
    double normal = 0.0;

    unitNormal = getNextUnitNormalRandom();
    normal = ( unitNormal * stdDeviation ) + mean;
    
    return normal;
}



/*
 * Marsaglia polar method
 */
double getNextUnitNormalRandom( ) {
    static BOOL haveNextNormal = FALSE;
    static double nextNormal = 0.0;
    
    double v1;
    double v2;
    double s;
    double multiplier;
    double unitNormal;

    if( haveNextNormal ) {
        haveNextNormal = FALSE;
        return nextNormal;
    }
    else {
        do {
            v1 = 2 * genrand64_real3() - 1.0;
            v2 = 2 * genrand64_real3() - 1.0;
            s = v1 * v1 + v2 * v2;
        } while( s >= 1.0 || s == 0.0 );
        multiplier = sqrt( -2.0 * log( s ) / s );
        haveNextNormal = TRUE;
        nextNormal = v2 * multiplier;
        unitNormal = v1 * multiplier;
        return unitNormal;
    }
}



/* 
 * generate PRN in [0,1] inclusive.
 * 
 */
double getNextUnitUniformRandomIncl( ) {
    double random = genrand64_real1();
    return random;
}

/* 
 * generate PRN in (0,1) exclusive.
 * 
 */
double getNextUnitUniformRandomExcl( ) {
    double random = genrand64_real3();
    return random;
}



double getNextExponentialRandom( double lambda ) {
	double u = genrand64_real3();
	return -log(u)/lambda;	
}


UINT getNextGeometricRandom( double p ) {
	double u = genrand64_real3();
	UINT g = (UINT)(log(u)/(log(1.0 - p))) + (UINT)1;
	return g;
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


