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

#define _CRT_SECURE_NO_WARNINGS
#include "utOpenGLES2.h"
#include <cstdlib>
#include <cstring>
#include <string>

#if defined(PLATFORM_WIN)
#include "EGL/egl.h"
#endif

#if defined(PLATFORM_ANDROID)
#include <EGL/egl.h>
#endif
#if defined (PLATFORM_IOS)
#include <CoreFoundation/CoreFoundation.h>
#endif

namespace glExt
{
	bool EXT_multisampled_render_to_texture = false;
	bool ANGLE_framebuffer_blit = false;
	bool ANGLE_framebuffer_multisample = false;
	bool OES_rgb8_rgba8 = false;

	bool EXT_texture_filter_anisotropic = false;
	bool EXT_disjoint_timer_query = false;
	bool EXT_occlusion_query_boolean = false;

	bool OES_texture_3D = false;

	bool EXT_texture_compression_s3tc = false;
	bool EXT_texture_compression_dxt1 = false;
	bool ANGLE_texture_compression_dxt3 = false;
	bool ANGLE_texture_compression_dxt5 = false;

	bool IMG_texture_compression_pvrtc = false;
	bool OES_compressed_ETC1_RGB8_texture = false;

	bool OES_depth_texture = false;
	bool ANGLE_depth_texture = false;

	bool EXT_shadow_samplers = false;

	int	majorVersion = 1, minorVersion = 0;
}



namespace h3dGL
{
#ifdef H3DGL_OES_texture_3D
PFNGLTEXIMAGE3DOESPROC glTexImage3DOES = 0x0;
PFNGLTEXSUBIMAGE3DOESPROC glTexSubImage3DOES = 0x0;
PFNGLCOPYTEXSUBIMAGE3DOESPROC glCopyTexSubImage3DOES = 0x0;
PFNGLCOMPRESSEDTEXIMAGE3DOESPROC glCompressedTexImage3DOES = 0x0;
PFNGLCOMPRESSEDTEXSUBIMAGE3DOESPROC glCompressedTexSubImage3DOES = 0x0;
#endif

#ifdef H3DGL_EXT_multisampled_render_to_texture
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC glRenderbufferStorageMultisampleEXT = 0x0;
PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC glFramebufferTexture2DMultisampleEXT = 0x0;
#endif

#ifdef H3DGL_ANGLE_framebuffer_blit
PFNGLBLITFRAMEBUFFERANGLEPROC glBlitFramebufferANGLE = 0x0;
#endif

#ifdef H3DGL_ANGLE_framebuffer_multisample
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEANGLEPROC glRenderbufferStorageMultisampleANGLE = 0x0;
#endif

#ifdef H3DGL_EXT_occlusion_query_boolean
PFNGLGENQUERIESEXTPROC glGenQueriesEXT = 0x0;
PFNGLDELETEQUERIESEXTPROC glDeleteQueriesEXT = 0x0;
PFNGLISQUERYEXTPROC glIsQueryEXT = 0x0;
PFNGLBEGINQUERYEXTPROC glBeginQueryEXT = 0x0;
PFNGLENDQUERYEXTPROC glEndQueryEXT = 0x0;
PFNGLGETQUERYIVEXTPROC glGetQueryivEXT = 0x0;
PFNGLGETQUERYOBJECTUIVEXTPROC glGetQueryObjectuivEXT = 0x0;
#endif

}  // namespace h3dGL


bool isExtensionSupported( const char *extName )
{
	const char *extensions = (char *)glGetString( GL_EXTENSIONS );
	size_t nameLen = strlen( extName );
	const char *pos;
	while( ( pos = strstr( extensions, extName ) ) != 0x0 )
	{
		char c = pos[nameLen];
		if( c == ' ' || c == '\0' ) return true;
		extensions = pos + nameLen;
	}

	return false;
}


void getOpenGLVersion()
{
    std::string version = (char *)glGetString( GL_VERSION );
	
	size_t pos1 = version.find( "." );
	size_t pos2 = version.find( ".", pos1 + 1 );
	if( pos2 == std::string::npos ) pos2 = version.find( " ", pos1 + 1 );
	if( pos2 == std::string::npos ) pos2 = version.length();
	size_t pos0 = version.rfind(" ", pos1);
	if( pos0 == std::string::npos ) pos0 = -1;
	
	glExt::majorVersion = atoi( version.substr( pos0 + 1, pos1 ).c_str() );
	glExt::minorVersion = atoi( version.substr( pos1 + 1, pos2 ).c_str() );
}

#ifdef GL_GLEXT_PROTOTYPES

#define platGetProcAddress(funcName) ((void*)0x0)

#else

void *platGetProcAddress( const char *funcName )
{
#if defined( PLATFORM_WIN )
	return (void *)eglGetProcAddress( funcName );
#else
	return (void*)0x0;
#endif
}

#endif

bool initOpenGLExtensions()
{
	bool r = true;
	
	getOpenGLVersion();

	// Extensions
    glExt::OES_texture_3D = isExtensionSupported( "GL_OES_texture_3D" );
#ifdef H3DGL_OES_texture_3D
	if ( glExt::OES_texture_3D )
	{
		bool v = true;
		v &= (glTexImage3DOES = (PFNGLTEXIMAGE3DOESPROC) platGetProcAddress( "glTexImage3DOES" )) != 0x0;
		v &= (glTexSubImage3DOES = (PFNGLTEXSUBIMAGE3DOESPROC) platGetProcAddress( "glTexSubImage3DOES" )) != 0x0;
		v &= (glCopyTexSubImage3DOES = (PFNGLCOPYTEXSUBIMAGE3DOESPROC) platGetProcAddress( "glCopyTexSubImage3DOES" )) != 0x0;
		v &= (glCompressedTexImage3DOES = (PFNGLCOMPRESSEDTEXIMAGE3DOESPROC) platGetProcAddress( "glCompressedTexImage3DOES" )) != 0x0;
		v &= (glCompressedTexSubImage3DOES = (PFNGLCOMPRESSEDTEXSUBIMAGE3DOESPROC) platGetProcAddress( "glCompressedTexSubImage3DOES" )) != 0x0;
		glExt::OES_texture_3D = v;
	}
#endif
    
	glExt::EXT_multisampled_render_to_texture = isExtensionSupported( "GL_EXT_multisampled_render_to_texture" );
#ifdef H3DGL_EXT_multisampled_render_to_texture
	if ( glExt::EXT_multisampled_render_to_texture )
	{
		bool v = true;
		v &= (glFramebufferTexture2DMultisampleEXT = (PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC) platGetProcAddress( "glFramebufferTexture2DMultisampleEXT" )) != 0x0;
		v &= (glRenderbufferStorageMultisampleEXT = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC) platGetProcAddress( "glRenderbufferStorageMultisampleEXT" )) != 0x0;
		glExt::EXT_multisampled_render_to_texture = v;
	}
#endif

		// ANGLE_framebuffer_blit and ANGLE_framebuffer_multisample
	glExt::ANGLE_framebuffer_blit = glExt::ANGLE_framebuffer_multisample = 
		isExtensionSupported( "GL_ANGLE_framebuffer_multisample" ) && isExtensionSupported( "GL_ANGLE_framebuffer_blit" );

#if defined(H3DGL_ANGLE_framebuffer_blit) || defined(H3DGL_ANGLE_framebuffer_multisample)
	if( glExt::ANGLE_framebuffer_multisample )
	{
		bool v = true;

		// From GL_ANGLE_framebuffer_blit
		v &= (glBlitFramebufferANGLE = (PFNGLBLITFRAMEBUFFERANGLEPROC) platGetProcAddress( "glBlitFramebufferANGLE" )) != 0x0;
		// From GL_ANGLE_framebuffer_multisample
		v &= (glRenderbufferStorageMultisampleANGLE = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEANGLEPROC) platGetProcAddress( "glRenderbufferStorageMultisampleANGLE" )) != 0x0;

		glExt::ANGLE_framebuffer_multisample = v;
	}
#endif

	glExt::EXT_occlusion_query_boolean = isExtensionSupported( "GL_EXT_occlusion_query_boolean" );
#ifdef H3DGL_EXT_occlusion_query_boolean
	if ( glExt::EXT_occlusion_query_boolean )
	{
		bool v = true;
		v &= (glGenQueriesEXT = (PFNGLGENQUERIESEXTPROC) platGetProcAddress( "glGenQueriesEXT" )) != 0x0;
		v &= (glDeleteQueriesEXT = (PFNGLDELETEQUERIESEXTPROC) platGetProcAddress( "glDeleteQueriesEXT" )) != 0x0;
		v &= (glIsQueryEXT = (PFNGLISQUERYEXTPROC) platGetProcAddress( "glIsQueryEXT" )) != 0x0;
		v &= (glBeginQueryEXT = (PFNGLBEGINQUERYEXTPROC) platGetProcAddress( "glBeginQueryEXT" )) != 0x0;
		v &= (glEndQueryEXT = (PFNGLENDQUERYEXTPROC) platGetProcAddress( "glEndQueryEXT" )) != 0x0;
		v &= (glGetQueryivEXT = (PFNGLGETQUERYIVEXTPROC) platGetProcAddress( "glGetQueryivEXT" )) != 0x0;
		v &= (glGetQueryObjectuivEXT = (PFNGLGETQUERYOBJECTUIVEXTPROC) platGetProcAddress( "glGetQueryObjectuivEXT" )) != 0x0;
		glExt::EXT_occlusion_query_boolean = v;
	}
#endif
    
	glExt::OES_rgb8_rgba8 = isExtensionSupported( "GL_OES_rgb8_rgba8" );

	glExt::EXT_texture_filter_anisotropic = isExtensionSupported( "GL_EXT_texture_filter_anisotropic" );

	glExt::IMG_texture_compression_pvrtc = isExtensionSupported( "GL_IMG_texture_compression_pvrtc" );

	glExt::EXT_texture_compression_s3tc = isExtensionSupported( "GL_EXT_texture_compression_s3tc" );
	glExt::EXT_texture_compression_dxt1 = isExtensionSupported( "GL_EXT_texture_compression_dxt1" );
	glExt::ANGLE_texture_compression_dxt3 = isExtensionSupported( "GL_ANGLE_texture_compression_dxt3" );
	glExt::ANGLE_texture_compression_dxt5 = isExtensionSupported( "GL_ANGLE_texture_compression_dxt5" );

	glExt::OES_compressed_ETC1_RGB8_texture = isExtensionSupported( "GL_OES_compressed_ETC1_RGB8_texture" );

	glExt::EXT_shadow_samplers = isExtensionSupported( "GL_EXT_shadow_samplers" );

    glExt::OES_depth_texture = isExtensionSupported( "GL_OES_depth_texture" );
    glExt::ANGLE_depth_texture = isExtensionSupported("GL_ANGLE_depth_texture");

	return r;
}
