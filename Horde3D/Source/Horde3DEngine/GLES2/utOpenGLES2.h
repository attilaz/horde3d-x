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

#ifndef _utOpenGL_H_
#define _utOpenGL_H_

#if defined( __gl_h_ ) || defined( __GL_H__ )
#   error gl.h included before utOpenGL.h
#endif

#include "utPlatform.h"

#if defined( PLATFORM_WIN ) || defined( PLATFORM_WIN_CE )
#   define WIN32_LEAN_AND_MEAN 1
#   ifndef NOMINMAX
#       define NOMINMAX
#   endif
#   include <windows.h>
#   include <stddef.h>
#   define GLAPI __declspec( dllimport )
#   define GLAPIENTRY _stdcall
#   define GLAPIENTRYP _stdcall *
#   ifdef PLATFORM_WIN_CE
extern "C" GLAPI void* eglGetProcAddress(const char *procname);
#   endif
#elif defined(PLATFORM_IOS)
#   include <stddef.h>
#   define GLAPIENTRY
#   define GLAPI extern
#   define GLAPIENTRYP *
#else
#   include <stddef.h>
#	define GLAPI
#	define GLAPIENTRY
#   define GLAPIENTRYP *
#   ifdef PLATFORM_MAC
#      include <Carbon/Carbon.h>
#   else
extern "C" void (*glXGetProcAddressARB( const unsigned char *procName ))( void );
#   endif
#endif


namespace glExt
{
	extern bool EXT_framebuffer_multisample;
	extern bool IMG_multisampled_render_to_texture;
	extern bool EXT_texture_filter_anisotropic;
	extern bool ARB_texture_float;
	extern bool ARB_texture_non_power_of_two;
	extern bool ARB_timer_query;

	extern bool EXT_occlusion_query_boolean; // supported on sgx 543+ and Angle(NaCL,emscripten)

	extern bool OES_texture_3D;
	extern bool EXT_texture_sRGB;
	extern bool EXT_texture_compression_s3tc;
	extern bool EXT_texture_compression_dxt1;
	extern bool ANGLE_texture_compression_dxt3;
	extern bool ANGLE_texture_compression_dxt5;
	extern bool IMG_texture_compression_pvrtc;
	extern bool OES_compressed_ETC1_RGB8_texture;

	extern bool OES_depth_texture;
	extern bool EXT_shadow_samplers;

	extern int  majorVersion, minorVersion;
}

bool initOpenGLExtensions();


#if defined( PLATFORM_WIN ) || defined( PLATFORM_WIN_CE )
#include "GLES2/gl2.h"
#elif defined(PLATFORM_IOS)
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#elif defined(PLATFORM_ANDROID) || defined(PLATFORM_NACL) || defined(PLATFORM_QNX)
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif

// =================================================================================================
// Extensions
// =================================================================================================

namespace h3dGL
{

// GL_OES_texture_3D 
#ifndef GL_OES_texture_3D
#define GL_OES_texture_3D 1

#define GL_TEXTURE_WRAP_R_OES                                   0x8072
#define GL_TEXTURE_3D_OES                                       0x806F
#define GL_TEXTURE_BINDING_3D_OES                               0x806A
#define GL_MAX_3D_TEXTURE_SIZE_OES                              0x8073
#define GL_SAMPLER_3D_OES                                       0x8B5F
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_OES        0x8CD4

typedef void (GLAPIENTRYP PFNGLTEXIMAGE3DOESPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
typedef void (GLAPIENTRYP PFNGLTEXSUBIMAGE3DOESPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels);
typedef void (GLAPIENTRYP PFNGLCOPYTEXSUBIMAGE3DOESPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (GLAPIENTRYP PFNGLCOMPRESSEDTEXIMAGE3DOESPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data);
typedef void (GLAPIENTRYP PFNGLCOMPRESSEDTEXSUBIMAGE3DOESPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data);
extern PFNGLTEXIMAGE3DOESPROC glTexImage3DOES;
extern PFNGLTEXSUBIMAGE3DOESPROC glTexSubImage3DOES;
extern PFNGLCOPYTEXSUBIMAGE3DOESPROC glCopyTexSubImage3DOES;
extern PFNGLCOMPRESSEDTEXIMAGE3DOESPROC glCompressedTexImage3DOES;
extern PFNGLCOMPRESSEDTEXSUBIMAGE3DOESPROC glCompressedTexSubImage3DOES;
#endif

/* GL_EXT_shadow_samplers */
#ifndef GL_EXT_shadow_samplers
#define GL_TEXTURE_COMPARE_MODE_EXT                             0x884C
#define GL_TEXTURE_COMPARE_FUNC_EXT                             0x884D
#define GL_COMPARE_REF_TO_TEXTURE_EXT                           0x884E
#define GL_SAMPLER_2D_SHADOW_EXT                                0x8B62
#endif

// GL_EXT_occlusion_query_boolean
#ifndef GL_EXT_occlusion_query_boolean
#define GL_EXT_occlusion_query_boolean 1

#define GL_ANY_SAMPLES_PASSED_EXT                               0x8C2F
#define GL_ANY_SAMPLES_PASSED_CONSERVATIVE_EXT                  0x8D6A
#define GL_CURRENT_QUERY_EXT                                    0x8865
#define GL_QUERY_RESULT_EXT                                     0x8866
#define GL_QUERY_RESULT_AVAILABLE_EXT                           0x8867

typedef void (GLAPIENTRYP PFNGLGENQUERIESEXTPROC) (GLsizei n, GLuint *ids);
typedef void (GLAPIENTRYP PFNGLDELETEQUERIESEXTPROC) (GLsizei n, const GLuint *ids);
typedef GLboolean (GLAPIENTRYP PFNGLISQUERYEXTPROC) (GLuint id);
typedef void (GLAPIENTRYP PFNGLBEGINQUERYEXTPROC) (GLenum target, GLuint id);
typedef void (GLAPIENTRYP PFNGLENDQUERYEXTPROC) (GLenum target);
typedef void (GLAPIENTRYP PFNGLGETQUERYIVEXTPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (GLAPIENTRYP PFNGLGETQUERYOBJECTUIVEXTPROC) (GLuint id, GLenum pname, GLuint *params);

extern PFNGLGENQUERIESEXTPROC glGenQueriesEXT;
extern PFNGLDELETEQUERIESEXTPROC glDeleteQueriesEXT;
extern PFNGLISQUERYEXTPROC glIsQueryEXT;
extern PFNGLBEGINQUERYEXTPROC glBeginQueryEXT;
extern PFNGLENDQUERYEXTPROC glEndQueryEXT;
extern PFNGLGETQUERYIVEXTPROC glGetQueryivEXT;
extern PFNGLGETQUERYOBJECTUIVEXTPROC glGetQueryObjectuivEXT;

#endif
	
// EXT_texture_filter_anisotropic
#ifndef GL_EXT_texture_filter_anisotropic
#define GL_EXT_texture_filter_anisotropic 1

#define GL_TEXTURE_MAX_ANISOTROPY_EXT       0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT   0x84FF

#endif


// EXT_texture_compression_s3tc
#ifndef GL_EXT_texture_compression_s3tc
#define GL_EXT_texture_compression_s3tc 1

#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT     0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT    0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT    0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT    0x83F3

#endif

// GL_IMG_texture_compression_pvrtc 
#ifndef GL_IMG_texture_compression_pvrtc
#define GL_IMG_texture_compression_pvrtc 1
#define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG                      0x8C00
#define GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG                      0x8C01
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG                     0x8C02
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG                     0x8C03
#endif 

// GL_OES_compressed_ETC1
#ifndef GL_OES_compressed_ETC1_RGB8_texture
#define GL_OES_compressed_ETC1_RGB8_texture
#define GL_ETC1_RGB8_OES                                        0x8D64
#endif 

// EXT_texture_sRGB
#ifndef GL_EXT_texture_sRGB
#define GL_EXT_texture_sRGB 1

#define GL_SRGB_EXT                            0x8C40
#define GL_SRGB8_EXT                           0x8C41
#define GL_SRGB_ALPHA_EXT                      0x8C42
#define GL_SRGB8_ALPHA8_EXT                    0x8C43
#define GL_SLUMINANCE_ALPHA_EXT                 0x8C44
#define GL_SLUMINANCE8_ALPHA8_EXT               0x8C45
#define GL_SLUMINANCE_EXT                       0x8C46
#define GL_SLUMINANCE8_EXT                      0x8C47
#define GL_COMPRESSED_SRGB_EXT                  0x8C48
#define GL_COMPRESSED_SRGB_ALPHA_EXT            0x8C49
#define GL_COMPRESSED_SLUMINANCE_EXT            0x8C4A
#define GL_COMPRESSED_SLUMINANCE_ALPHA_EXT      0x8C4B
#define GL_COMPRESSED_SRGB_S3TC_DXT1_EXT        0x8C4C
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT  0x8C4D
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT  0x8C4E
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT  0x8C4F

#endif


// ARB_texture_float
#ifndef GL_ARB_texture_float
#define GL_ARB_texture_float 1

#define GL_TEXTURE_RED_TYPE_ARB             0x8C10
#define GL_TEXTURE_GREEN_TYPE_ARB           0x8C11
#define GL_TEXTURE_BLUE_TYPE_ARB            0x8C12
#define GL_TEXTURE_ALPHA_TYPE_ARB           0x8C13
#define GL_TEXTURE_LUMINANCE_TYPE_ARB       0x8C14
#define GL_TEXTURE_INTENSITY_TYPE_ARB       0x8C15
#define GL_TEXTURE_DEPTH_TYPE_ARB           0x8C16
#define GL_UNSIGNED_NORMALIZED_ARB          0x8C17
#define GL_RGBA32F_ARB                      0x8814
#define GL_RGB32F_ARB                       0x8815
#define GL_ALPHA32F_ARB                     0x8816
#define GL_INTENSITY32F_ARB                 0x8817
#define GL_LUMINANCE32F_ARB                 0x8818
#define GL_LUMINANCE_ALPHA32F_ARB           0x8819
#define GL_RGBA16F_ARB                      0x881A
#define GL_RGB16F_ARB                       0x881B
#define GL_ALPHA16F_ARB                     0x881C
#define GL_INTENSITY16F_ARB                 0x881D
#define GL_LUMINANCE16F_ARB                 0x881E
#define GL_LUMINANCE_ALPHA16F_ARB           0x881F

#endif


// EXT_framebuffer_blit
#ifndef GL_EXT_framebuffer_blit
#define GL_EXT_framebuffer_blit 1

#define GL_READ_FRAMEBUFFER_EXT             0x8CA8
#define GL_DRAW_FRAMEBUFFER_EXT             0x8CA9
#define GL_READ_FRAMEBUFFER_BINDING_EXT     GL_FRAMEBUFFER_BINDING_EXT
#define GL_DRAW_FRAMEBUFFER_BINDING_EXT     0x8CAA

typedef void (GLAPIENTRYP PFNGLBLITFRAMEBUFFEREXTPROC) (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
extern PFNGLBLITFRAMEBUFFEREXTPROC glBlitFramebufferEXT;

#endif


// EXT_framebuffer_multisample
#ifndef GL_EXT_framebuffer_multisample
#define GL_EXT_framebuffer_multisample 1

#define GL_RENDERBUFFER_SAMPLES_EXT                0x8CAB
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT  0x8D56
#define GL_MAX_SAMPLES_EXT                         0x8D57

typedef void (GLAPIENTRYP PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
extern PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC glRenderbufferStorageMultisampleEXT;

#endif


// IMG_multisampled_render_to_texture
#ifndef GL_IMG_multisampled_render_to_texture
#define GL_IMG_multisampled_render_to_texture 1

#define GL_RENDERBUFFER_SAMPLES_IMG                0x9133
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_IMG  0x9134
#define GL_MAX_SAMPLES_IMG                         0x9135

typedef void (GLAPIENTRYP PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEIMGPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples);
typedef void (GLAPIENTRYP PFNGLRENDERBUFFERSTORAGEMULTISAMPLEIMGPROC) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);

extern PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEIMGPROC glFramebufferTexture2DMultisampleIMG;
extern PFNGLRENDERBUFFERSTORAGEMULTISAMPLEIMGPROC glRenderbufferStorageMultisampleIMG;

#endif

#if defined(PLATFORM_ANDROID) || defined(PLATFORM_NACL)
#define PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEIMGPROC PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEIMG
#define PFNGLRENDERBUFFERSTORAGEMULTISAMPLEIMGPROC PFNGLRENDERBUFFERSTORAGEMULTISAMPLEIMG
extern PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEIMG glFramebufferTexture2DMultisampleIMG;
extern PFNGLRENDERBUFFERSTORAGEMULTISAMPLEIMG glRenderbufferStorageMultisampleIMG;
#elif defined(PLATFORM_QNX)
extern PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEIMGPROC glFramebufferTexture2DMultisampleIMG;
extern PFNGLRENDERBUFFERSTORAGEMULTISAMPLEIMGPROC glRenderbufferStorageMultisampleIMG;
#endif



// ARB_timer_query
#ifndef GL_ARB_timer_query
#define GL_ARB_timer_query 1

#define GL_TIME_ELAPSED  0x88BF
#define GL_TIMESTAMP     0x8E28

typedef int64           GLint64;
typedef uint64          GLuint64;

typedef void (GLAPIENTRYP PFNGLQUERYCOUNTERPROC) (GLuint id, GLenum target);
typedef void (GLAPIENTRYP PFNGLGETQUERYOBJECTI64VPROC) (GLuint id, GLenum pname, GLint64 *params);
typedef void (GLAPIENTRYP PFNGLGETQUERYOBJECTUI64VPROC) (GLuint id, GLenum pname, GLuint64 *params);
extern PFNGLQUERYCOUNTERPROC glQueryCounter; 
extern PFNGLGETQUERYOBJECTI64VPROC glGetQueryObjecti64v;
extern PFNGLGETQUERYOBJECTUI64VPROC glGetQueryObjectui64v;

#ifndef GL_QUERY_RESULT_AVAILABLE
#define GL_QUERY_RESULT_AVAILABLE               0x8867
#endif
#ifndef GL_QUERY_RESULT
#define GL_QUERY_RESULT                         0x8866
#endif

typedef void (GLAPIENTRYP PFNGLGENQUERIESPROC) (GLsizei n, GLuint *ids);
typedef void (GLAPIENTRYP PFNGLDELETEQUERIESPROC) (GLsizei n, const GLuint *ids);
typedef void (GLAPIENTRYP PFNGLGETQUERYOBJECTIVPROC) (GLuint id, GLenum pname, GLint *params);

extern PFNGLGENQUERIESPROC glGenQueries;
extern PFNGLDELETEQUERIESPROC glDeleteQueries;
extern PFNGLGETQUERYOBJECTIVPROC glGetQueryObjectiv;
#endif //ARB_timer_query


}  // namespace h3dGL

using namespace h3dGL;

#endif // _utOpenGL_H_
