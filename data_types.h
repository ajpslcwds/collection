/** 
 * @file
 * @brief		Data type definition for different platforms
 * @author		Chunfeng Shen
 * @date		2009/03/15
 * @version		Initial Version
 *
 * Define basic data types on different platforms. e.g., uint64 means unsigned
 * int occupying 64 bits
 */
#ifndef DATA_TYPE_H
#define DATA_TYPE_H

#include <limits.h>

/* boolean type and constant variable */
#ifndef __cplusplus
typedef signed char bool;
#define true	1
#define false	0
#endif

#define SIZEOF_CHAR 1

// number of bytes in a short
#if (USHRT_MAX) == 255U
#	define SIZEOF_SHORT 1
#elif (USHRT_MAX) == 65535U
#	define SIZEOF_SHORT 2
#elif (USHRT_MAX) == 4294967295U
#	define SIZEOF_SHORT 4
#elif (USHRT_MAX) == 18446744073709551615U
#	define SIZEOF_SHORT 8
#else
#	error: unsupported short size, must be updated for this platform!
#endif

// number of bytes in an int
#if (UINT_MAX) == 65535U
#	define SIZEOF_INT 2
#elif (UINT_MAX) == 4294967295U
#	define SIZEOF_INT 4
#elif (UINT_MAX) == 18446744073709551615U
#	define SIZEOF_INT 8
#else
#	error: unsupported int size, must be updated for this platform!
#endif

// number of bytes in a long
#if (ULONG_MAX) == 65535UL
#	define SIZEOF_LONG 2
#elif ((ULONG_MAX) == 4294967295UL)
#	define SIZEOF_LONG 4
#elif ((ULONG_MAX) == 18446744073709551615UL)
#	define SIZEOF_LONG 8
#else
#	error: unsupported long size, must be updated for this platform!
#endif

#if _MSC_VER <= 1200  //ULL can not be supported by vc6.0
#else
	#if defined (ULLONG_MAX)
	#	if ((ULLONG_MAX) == 4294967295ULL)
	#		define SIZEOF_LONG_LONG 4
	#	elif ((ULLONG_MAX) == 18446744073709551615ULL)
	#		define SIZEOF_LONG_LONG 8
	#	endif
	#elif defined (ULONGLONG_MAX)
	#	if ((ULONGLONG_MAX) == 4294967295ULL)
	#		define SIZEOF_LONG_LONG 4
	#	elif ((ULONGLONG_MAX) == 18446744073709551615ULL)
	#		define SIZEOF_LONG_LONG 8
	#	endif
	#endif
#endif



#ifndef AIX
typedef char					int8;
#endif
typedef unsigned char 			uint8;

#if SIZEOF_SHORT == 2
	typedef short				int16;
#elif SIZEOF_INT == 2
	typedef int					int16;
#endif

#if SIZEOF_SHORT == 2
	typedef unsigned short		uint16;
#elif SIZEOF_INT == 2
	typedef unsigned int		uint16;
#endif

#if SIZEOF_INT == 4
	typedef int					int32;
#elif SIZEOF_LONG == 4
	typedef long				int32;
#endif

#if SIZEOF_INT == 4
	typedef unsigned int		uint32;
#elif SIZEOF_LONG == 4
	typedef unsigned long		uint32;
#endif

#if defined (__MINGW32__)
	typedef signed long long	int64;
#elif SIZEOF_LONG == 8
	typedef long				int64;
#elif SIZEOF_LONG_LONG == 8
	typedef signed long long	int64;
#elif defined WIN32
	typedef signed __int64		int64;
#else
	typedef signed long long	int64;
#endif

#if defined (__MINGW32__)
	typedef unsigned long long	uint64;
#elif SIZEOF_LONG == 8
	typedef unsigned long		uint64;
#elif SIZEOF_LONG_LONG == 8
	typedef unsigned long long	uint64;
#elif defined WIN32
	typedef unsigned __int64	uint64;
#else
	typedef unsigned long long	uint64;
#endif

//#define float32 float 
//#define float64 double

typedef double float64;
typedef float float32;

#endif	// DATA_TYPE_H
