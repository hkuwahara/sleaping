#if !defined(HAVE_COMMON)
#define HAVE_COMMON

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>



#	if !defined(AND)
#		define AND &&
#	endif

#	if !defined(OR)
#		define OR ||
#	endif


#	if !defined(BOOL)
#		define BOOL int
#	endif
			
#	if !defined(FALSE)
#		define FALSE 0
#	endif

#	if !defined(TRUE)
#		define TRUE (!FALSE)
#	endif


#	if !defined(BYTE)
#		define BYTE unsigned char
#	endif


#	if !defined(UINT)
#		define UINT uint64_t
#	endif

#	if !defined(INT)
#		define INT int64_t
#	endif


#	if !defined(CADDR_T)
#		define CADDR_T char*
#	endif

#	if !defined(LPSTR)
#		define LPSTR char*
#	endif

#	if !defined(LPCSTR)
#		define LPCSTR const char*
#	endif

#	if !defined(ARG)
#		define ARG char*
#	endif
			

#	if !defined(UINT_MAX)
#		define UINT_MAX UINT64_MAX 
#	endif

#	if !defined(UINT_MIN)
#		define UINT_MIN (UINT)0
#	endif

#	if !defined(INT_MAX)
#		define INT_MAX	INT64_MAX
#	endif

#	if !defined(INT_MIN)
#		define INT_MIN  INT64_MIN
#	endif

/**************************************/



/**************************************/
#	if !defined(GET_MAX)
#		define GET_MAX(a,b) (((a)>(b))?(a):(b))
#	endif 

#	if !defined(GET_MIN)
#		define GET_MIN(a,b) (((a)<(b))?(a):(b))
#	endif 

#	if !defined(ROUND_DOUBLE)
#		define ROUND_DOUBLE(a) (((a)>0)?\
			(double)(INT)((a)+0.5):(double)(INT)((a)-0.5))
#	endif

#	if !defined(ROUND_INT)
#		define ROUND_INT(a) (((a)>0)?\
			(INT)((a)+0.5):(INT)((a)-0.5))
#	endif


#	if !defined(GET_MID_DOUBLE)
#		define GET_MID_DOUBLE(a,b) (((a)+(b))/2)		 
#	endif

#	if !defined(GET_MID_INT)
#		define GET_MID_INT(a,b) (((INT)(a)+(INT)(b))>>1)		 
#	endif

#	if !defined(LG_UINT)
#		define LG_UINT(x,a) { \
		a=0; \
		UINT xx = (UINT)(x); \
		while(xx>>=1) a++; }
#	endif

#	if !defined(IS_POW2)
#		define IS_POW2(x) (((INT)(x)>0)&& !((INT)(x) & (INT)((x)-1))?TRUE:FALSE)
#	endif


#	if !defined(BZERO)
#		define BZERO(b,s) memset((CADDR_T)(b),0,s) 
#	endif

#	if !defined(FILL_ZERO)
#		define FILL_ZERO(b) memset((CADDR_T)(b),0,sizeof(b)) 
#	endif

#	if !defined(BONE)
#		define BONE(b,s) memset((CADDR_T)(b),0XFF,s) 
#	endif

#	if !defined(FILL_ONE)
#		define FILL_ONE(b) memset((CADDR_T)(b),0XFF,sizeof(b)) 
#	endif

/**************************************/


/**************************************/
#	define MALLOC(size) ((size==0)?NULL:malloc(size))
#	define CALLOC(n,size) (((size==0)||(n==0))?NULL:calloc(n,size))
#	define REALLOC(ptr,size) (((size==0)&&(ptr==NULL))?NULL:realloc(ptr,size))

#	define FREE(ptr) \
	{\
		free(ptr);\
	} ptr=NULL

#endif
