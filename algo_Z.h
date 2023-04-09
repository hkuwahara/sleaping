#if !defined(HAVE_ALGO_Z)
#	define HAVE_ALGO_Z
	
#include "fadso.h"

SUBSAMPLER *createAlgoZ();
SUBSAMPLER *createAlgoZWithOptions(float thresholdFactor);

#endif
