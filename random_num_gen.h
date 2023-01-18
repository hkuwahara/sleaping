#if !defined(HAVE_RANDOM_NUM_GEN)
#define HAVE_RANDOM_NUM_GEN

#include "common.h"

void setRandomSeed(UINT seed);
double getNextUniformRandomDouble( double minUniform, double maxUniform );
UINT getNextUniformRandomUINT( UINT minUniform, UINT maxUniform );
double getNextUnitUniformRandomIncl( );
double getNextUnitUniformRandomExcl( );
double getNextNormalRandom( double mean, double stdDeviation );
double getNextUnitNormalRandom( );
double getNextExponentialRandom( double lambda );
UINT getNextGeometricRandom( double p );

#endif
