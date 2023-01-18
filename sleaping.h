#if !defined(HAVE_SLEAPING)
#	define HAVE_SLEAPING
	
#include "fadso.h"

SUBSAMPLER *createSleaping();
SUBSAMPLER *createSleapingWithOptions(double leapSizeProp);

#endif
