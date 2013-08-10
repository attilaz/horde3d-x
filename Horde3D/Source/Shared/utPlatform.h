// *************************************************************************************************
//
// Horde3D
//   Next-Generation Graphics Engine
// --------------------------------------
// Copyright (C) 2006-2011 Nicolas Schulz
//
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/legal/epl-v10.html
//
// *************************************************************************************************

#ifndef _utPlatform_H_
#define _utPlatform_H_

#if defined( _DEBUG )
	#include <assert.h>
#endif

// Detect platform
#if defined( WINCE )
#	if !defined( PLATFORM_WIN_CE )
#		define PLATFORM_WIN_CE
#	endif
#elif defined( WIN32 ) || defined( _WINDOWS ) || defined( _WIN32 )
#	if !defined( PLATFORM_WIN )
#		define PLATFORM_WIN
#	endif
#elif defined( __APPLE__ ) || defined( __APPLE_CC__ )
#include <TargetConditionals.h>
#	if TARGET_OS_IPHONE
#	  if !defined( PLATFORM_IOS )
#		   define PLATFORM_IOS
#		endif
#	else
#	  if !defined( PLATFORM_MAC )
#		   define PLATFORM_MAC
#		endif
#	endif
#elif defined(__ANDROID__)
#   if !defined( PLATFORM_ANDROID )
#      define PLATFORM_ANDROID
#   endif
#else
#	if !defined( PLATFORM_LINUX )
#		define PLATFORM_LINUX
#	endif
#endif

#define H3D_RENDERER_GL		0
#define H3D_RENDERER_GLES2	1
#define H3D_RENDERER_D3D11	2

#ifndef H3D_RENDERER
#	if defined(PLATFORM_WIN) || defined(PLATFORM_WIN_CE) || defined(PLATFORM_MAC) || defined(PLATFORM_LINUX)
#		define	H3D_RENDERER H3D_RENDERER_GL
#	elif defined(PLATFORM_ANDROID) || defined(PLATFORM_IOS)
#		define	H3D_RENDERER H3D_RENDERER_GLES2
#	endif
#endif

#if !H3D_RENDERER
    #error  "Cannot recognize renderer backend type"
#endif 



#ifndef DLLEXP
#	ifdef PLATFORM_WIN
#		define DLLEXP extern "C" __declspec( dllexport )
#	else
#		if defined( __GNUC__ ) && __GNUC__ >= 4
#		  define DLLEXP extern "C" __attribute__ ((visibility("default")))
#   	else
#		  define DLLEXP extern "C"
#   	endif
#	endif
#endif


// Shortcuts for common types
typedef signed char int8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef long long int64;
typedef unsigned long long uint64;


#if !defined( PLATFORM_WIN ) && !defined( PLATFORM_WIN_CE )
#	define _stricmp strcasecmp
#	define _mkdir( name ) mkdir( name, 0755 )
#endif

#ifdef PLATFORM_WIN_CE
#define GetProcessAffinityMask
#define SetThreadAffinityMask
#ifndef vsnprintf
#	define vsnprintf _vsnprintf
#endif
#undef ASSERT
#undef min
#undef max
#endif

#if !defined( _MSC_VER ) || (defined( _MSC_VER ) && (_MSC_VER < 1400))
#	define strncpy_s( dst, dstSize, src, count ) strncpy( dst, src, count < dstSize ? count : dstSize )
#endif
#if defined( _MSC_VER ) && (_MSC_VER < 1400)
#   define vsnprintf _vsnprintf
#endif


// Runtime assertion
#if defined( _DEBUG )
#	define ASSERT( exp ) assert( exp );
#else
#	define ASSERT( exp )
#endif

// Support for MSVC static code analysis
#if defined( _MSC_VER ) && defined( _PREFAST_ )
#	define ASSERT( exp ) __analysis_assume( exp );
#endif

// Static compile-time assertion
namespace StaticAssert
{
	template< bool > struct FAILED;
	template<> struct FAILED< true > { };
}
#define ASSERT_STATIC( exp ) (StaticAssert::FAILED< (exp) != 0 >())

#endif // _utPlatform_H_
