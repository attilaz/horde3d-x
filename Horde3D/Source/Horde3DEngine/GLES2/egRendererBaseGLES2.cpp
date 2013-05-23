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

#include "../egRendererBase.h"
#include "../egModules.h"
#include "../egCom.h"

#include "utDebug.h"


namespace Horde3D {

#ifdef H3D_VALIDATE_DRAWCALLS
#	define CHECK_GL_ERROR checkGLError();
#else
#	define CHECK_GL_ERROR
#endif

static const char *defaultShaderVS =
	"uniform mat4 viewProjMat;\n"
	"uniform mat4 worldMat;\n"
	"attribute vec3 vertPos;\n"
	"void main() {\n"
	"	gl_Position = viewProjMat * worldMat * vec4( vertPos, 1.0 );\n"
	"}\n";


const char *defaultShaderFS =
	"uniform mediump vec4 color;\n"
	"void main() {\n"
	"	gl_FragColor = color;\n"
	"}\n";

// =================================================================================================
// GPUTimer
// =================================================================================================

GPUTimer::GPUTimer() : _numQueries( 0 ),  _queryFrame( 0 ), _time( 0 ), _activeQuery( false )
{
	reset();
}


GPUTimer::~GPUTimer()
{
	if( !_queryPool.empty() )
		glDeleteQueries( (uint32)_queryPool.size(), &_queryPool[0] );
}


void GPUTimer::beginQuery( uint32 frameID )
{
	if( !glExt::ARB_timer_query ) return;
	ASSERT( !_activeQuery );
	
	if( _queryFrame != frameID )
	{
		if( !updateResults() ) return;

		_queryFrame = frameID;
		_numQueries = 0;
	}
	
	// Create new query pair if necessary
	uint32 queryObjs[2];
	if( _numQueries++ * 2 == _queryPool.size() )
	{
		glGenQueries( 2, queryObjs );
		_queryPool.push_back( queryObjs[0] );
		_queryPool.push_back( queryObjs[1] );
	}
	else
	{
		queryObjs[0] = _queryPool[(_numQueries - 1) * 2];
	}
	
	_activeQuery = true;
	 glQueryCounter( queryObjs[0], GL_TIMESTAMP );
}


void GPUTimer::endQuery()
{
	if( _activeQuery )
	{	
		glQueryCounter( _queryPool[_numQueries * 2 - 1], GL_TIMESTAMP );
		_activeQuery = false;
	}
}


bool GPUTimer::updateResults()
{
	if( !glExt::ARB_timer_query ) return false;
	
	if( _numQueries == 0 )
	{
		_time = 0;
		return true;
	}
	
	// Make sure that last query is available
	GLint available;
	glGetQueryObjectiv( _queryPool[_numQueries * 2 - 1], GL_QUERY_RESULT_AVAILABLE, &available );
	if( !available ) return false;
	
	//  Accumulate time
	GLuint64 timeStart = 0, timeEnd = 0, timeAccum = 0;
	for( uint32 i = 0; i < _numQueries; ++i )
	{
		glGetQueryObjectui64v( _queryPool[i * 2], GL_QUERY_RESULT, &timeStart );
		glGetQueryObjectui64v( _queryPool[i * 2 + 1], GL_QUERY_RESULT, &timeEnd );
		timeAccum += timeEnd - timeStart;
	}
	
	_time = (float)((double)timeAccum / 1000000.0);
	return true;
}


void GPUTimer::reset()
{
	_time = glExt::ARB_timer_query ? 0.f : -1.f;
}


// =================================================================================================
// RenderDevice
// =================================================================================================

RenderDevice::RenderDevice(void*)
{
	_numVertexLayouts = 0;
	
	_vpX = 0; _vpY = 0; _vpWidth = 320; _vpHeight = 240;
	_scX = 0; _scY = 0; _scWidth = 320; _scHeight = 240;
	_prevShaderId = _curShaderId = 0;
	_curRendBuf = 0; _outputBufferIndex = 0;
	_textureMem = 0; _bufferMem = 0;
	_curRasterState.hash = _newRasterState.hash = 0;
	_curBlendState.hash = _newBlendState.hash = 0;
	_curDepthStencilState.hash = _newDepthStencilState.hash = 0;
	_curVertLayout = _newVertLayout = 0;
	_curIndexBuf = _newIndexBuf = 0;
	_defaultFBO = 0;
	_indexFormat = (uint32)IDXFMT_16;
	_pendingMask = 0;
	_maxTextureSize = 64;
	_maxCubeTextureSize = 16;
}


RenderDevice::~RenderDevice()
{
}


void RenderDevice::initStates()
{
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
}


bool RenderDevice::init()
{
	bool failed = false;

	char *vendor = (char *)glGetString( GL_VENDOR );
	char *renderer = (char *)glGetString( GL_RENDERER );
	char *version = (char *)glGetString( GL_VERSION );
	char *extensions = (char *)glGetString( GL_EXTENSIONS );
	
	Modules::log().writeInfo( "Initializing GLES2 backend using OpenGL ES driver '%s' by '%s' on '%s'",
	                          version, vendor, renderer );
	Modules::log().writeInfo( "extensions: %s", extensions);
	
	// Init extensions
	if( !initOpenGLExtensions() )
	{	
		Modules::log().writeError( "Could not find all required OpenGL function entry points" );
		failed = true;
	}

	// Check that OpenGL ES 2.0 is available
	if( glExt::majorVersion * 10 + glExt::minorVersion < 20 )
	{
		Modules::log().writeError( "OpenGL 2.0 not available" );
		failed = true;
	}

	// Check that required extensions are supported
	if( !glExt::EXT_texture_filter_anisotropic )
	{
		Modules::log().writeWarning( "Extension EXT_texture_filter_anisotropic not supported" );
	}
	
	if( failed )
	{
		Modules::log().writeError( "Failed to init renderer backend, debug info following" );
		char *exts = (char *)glGetString( GL_EXTENSIONS );
		Modules::log().writeInfo( "Supported extensions: '%s'", exts );

		return false;
	}
	
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &_maxTextureSize);
	glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &_maxCubeTextureSize);
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &_maxTextureUnits);
	if (_maxTextureUnits > 16) _maxTextureUnits = 16;

    //	Get the currently bound frame buffer object. 
	glGetIntegerv( GL_FRAMEBUFFER_BINDING, &_defaultFBO );
    
	// Get capabilities
	_caps.texDXT = glExt::EXT_texture_compression_s3tc || 
		(glExt::EXT_texture_compression_dxt1 && glExt::ANGLE_texture_compression_dxt3 && glExt::ANGLE_texture_compression_dxt5);
	_caps.texPVRTCI = glExt::IMG_texture_compression_pvrtc;
	_caps.texETC1 = glExt::OES_compressed_ETC1_RGB8_texture;

	_caps.texFloat = glExt::ARB_texture_float;
	_caps.texDepth = glExt::OES_depth_texture;
	_caps.texShadowCompare = glExt::EXT_shadow_samplers;

	_caps.tex3D = glExt::OES_texture_3D;
	_caps.texNPOT = glExt::ARB_texture_non_power_of_two;

	_caps.rtMultisampling = glExt::EXT_framebuffer_multisample || glExt::IMG_multisampled_render_to_texture ? 1 : 0;
	_caps.rtMaxColBufs = 1;

	_caps.occQuery = glExt::EXT_occlusion_query_boolean;
	_caps.timerQuery = glExt::ARB_timer_query;

	_depthFormat = GL_DEPTH_COMPONENT16;
	
	initStates();
	resetStates();

	return true;
}

void RenderDevice::handleContextLost()
{
	RDIBuffer *buffer;
	for(unsigned int i=0; (buffer=_buffers.get(i))!=NULL; ++i)
		buffer->glObj = 0;

	RDITexture *texture;
	for(unsigned int i=0; (texture=_textures.get(i))!=NULL; ++i)
		texture->glObj = 0;

	RDIShader *shader;
	for(unsigned int i=0; (shader=_shaders.get(i))!=NULL; ++i)
		shader->oglProgramObj = 0;

	RDIRenderBuffer *renderBuffer;
	for(unsigned int i=0; (renderBuffer=_rendBufs.get(i))!=NULL; ++i)
	{	
		renderBuffer->fbo = 0;
		renderBuffer->fboMS = 0;
		renderBuffer->depthBuf = 0;
		renderBuffer->depthTex = 0;
		for( uint32 i = 0; i < RDIRenderBuffer::MaxColorAttachmentCount; ++i ) renderBuffer->colTexs[i] = renderBuffer->colBufs[i] = 0;
	}
}

// =================================================================================================
// Vertex layouts
// =================================================================================================

uint32 RenderDevice::registerVertexLayout( uint32 numAttribs, VertexLayoutAttrib *attribs )
{
	if( _numVertexLayouts == MaxNumVertexLayouts )
		return 0;
	
	_vertexLayouts[_numVertexLayouts].numAttribs = numAttribs;

	for( uint32 i = 0; i < numAttribs; ++i )
		_vertexLayouts[_numVertexLayouts].attribs[i] = attribs[i];

	return ++_numVertexLayouts;
}


// =================================================================================================
// Buffers
// =================================================================================================

void RenderDevice::beginRendering()
{	
	//	Get the currently bound frame buffer object. 
	glGetIntegerv( GL_FRAMEBUFFER_BINDING, &_defaultFBO );
	resetStates();
}

void RenderDevice::finishRendering()
{

}

uint32 RenderDevice::createVertexBuffer( uint32 size, const void *data)
{
	RDIBuffer buf;

	buf.type = GL_ARRAY_BUFFER;
	buf.size = size;

	glGenBuffers( 1, &buf.glObj );
	glBindBuffer( buf.type, buf.glObj );
	glBufferData( buf.type, size, data, GL_DYNAMIC_DRAW );
	glBindBuffer( buf.type, 0 );

	_bufferMem += size;
	return _buffers.add( buf );
}


uint32 RenderDevice::createIndexBuffer( uint32 size, const void *data )
{
	RDIBuffer buf;

	buf.type = GL_ELEMENT_ARRAY_BUFFER;
	buf.size = size;
	glGenBuffers( 1, &buf.glObj );
	glBindBuffer( buf.type, buf.glObj );
	glBufferData( buf.type, size, data, GL_DYNAMIC_DRAW );
	glBindBuffer( buf.type, 0 );
	
	_bufferMem += size;
	return _buffers.add( buf );
}


void RenderDevice::destroyBuffer( uint32 bufObj )
{
	if( bufObj == 0 ) return;
	
	RDIBuffer &buf = _buffers.getRef( bufObj );
	if (buf.glObj!=0)
		glDeleteBuffers( 1, &buf.glObj );

	_bufferMem -= buf.size;
	_buffers.remove( bufObj );
}


void RenderDevice::updateBufferData( uint32 bufObj, uint32 offset, uint32 size, void *data )
{
	const RDIBuffer &buf = _buffers.getRef( bufObj );
	ASSERT( offset + size <= buf.size );

	if (buf.glObj!=0)
	{
		glBindBuffer( buf.type, buf.glObj );
		
		if( offset == 0 &&  size == buf.size )
		{
			// Replacing the whole buffer can help the driver to avoid pipeline stalls
			glBufferData( buf.type, size, data, GL_DYNAMIC_DRAW );
			glBindBuffer( buf.type, 0 );
			return;
		}

		glBufferSubData( buf.type, offset, size, data );
		glBindBuffer( buf.type, 0 );
	}
}


// =================================================================================================
// Textures
// =================================================================================================

uint32 RenderDevice::calcTextureSize( TextureFormats::List format, int width, int height, int depth )
{
	switch( format )
	{
	case TextureFormats::RGBA8:
		return width * height * depth * 4;
	case TextureFormats::DXT1:
		return std::max( width / 4, 1 ) * std::max( height / 4, 1 ) * depth * 8;
	case TextureFormats::DXT3:
		return std::max( width / 4, 1 ) * std::max( height / 4, 1 ) * depth * 16;
	case TextureFormats::DXT5:
		return std::max( width / 4, 1 ) * std::max( height / 4, 1 ) * depth * 16;
	case TextureFormats::RGBA16F:
		return width * height * depth * 8;
	case TextureFormats::RGBA32F:
		return width * height * depth * 16;
    case TextureFormats::PVRTCI_2BPP:
	case TextureFormats::PVRTCI_A2BPP:
		return (std::max( width, 16 ) * std::max( height, 8 ) * 2 + 7) / 8;
	case TextureFormats::PVRTCI_4BPP:
	case TextureFormats::PVRTCI_A4BPP:
		return (std::max( width, 8 ) * std::max( height, 8 ) * 4 + 7) / 8;
	case TextureFormats::ETC1:
		return std::max( width / 4, 1 ) * std::max( height / 4, 1 ) * depth * 8;
	default:
		return 0;
	}
}


uint32 RenderDevice::createTexture( TextureTypes::List type, int width, int height, int depth,
                                    TextureFormats::List format,
                                    bool hasMips, bool genMips, bool sRGB )
{
	ASSERT( depth > 0 );

	if( type == TextureTypes::Tex3D && !_caps.tex3D )
	{
		Modules::log().writeWarning( "3D textures are not supported on the GPU" );
		return 0;
	}

	if ( !_caps.texDXT && (format == TextureFormats::DXT1 || format == TextureFormats::DXT3 || format == TextureFormats::DXT5 ) )
	{
		Modules::log().writeWarning( "Unsupported texture formats: DXT1,3,5" );
		return 0;
	}

	if( !_caps.texFloat && (format == TextureFormats::RGBA16F || format == TextureFormats::RGBA32F ) )
	{
		Modules::log().writeWarning( "Unsupported texture formats: RGBA16F and RGBA32F" );
		return 0;
	}
	if( !_caps.texDepth && format == TextureFormats::DEPTH)
	{
		Modules::log().writeWarning( "Depth texture format is unsupported" );
		return 0;
	}
	if( !_caps.texPVRTCI && (format == TextureFormats::PVRTCI_2BPP || format == TextureFormats::PVRTCI_4BPP || 
		format == TextureFormats::PVRTCI_A2BPP || format == TextureFormats::PVRTCI_A4BPP  ) )
	{
		Modules::log().writeWarning( "Unsupported texture format: PVRTCI" );
		return 0;
	}
	if ( !_caps.texETC1 && format == TextureFormats::ETC1 )
	{
		Modules::log().writeWarning( "Unsupported texture format: ETC1" );
		return 0;
	}


	if( !_caps.texNPOT )
	{
		// Check if texture is NPOT	if mipmap is enabled (es2 has conditional nonpow2)
		if( ((width & (width-1)) != 0 || (height & (height-1)) != 0) && (hasMips || genMips) )
		{
			Modules::log().writeWarning( "Texture has non-power-of-two dimensions although NPOT is not supported by GPU" );
		}
	}
	
	RDITexture tex;
	tex.type = type;
	tex.format = format;
	tex.width = width;
	tex.height = height;
	tex.depth = depth;
	tex.sRGB = sRGB && Modules::config().sRGBLinearization;
	tex.genMips = genMips;
	tex.hasMips = hasMips;
	
	switch( format )
	{
	case TextureFormats::RGBA8:
		tex.glFmt = GL_RGBA;
		break;
	case TextureFormats::DXT1:
		tex.glFmt = tex.sRGB ? GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT : GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		break;
	case TextureFormats::DXT3:
		tex.glFmt = tex.sRGB ? GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT : GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		break;
	case TextureFormats::DXT5:
		tex.glFmt = tex.sRGB ? GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT : GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		break;
	case TextureFormats::RGBA16F:
		tex.glFmt = GL_RGBA;
		break;
	case TextureFormats::RGBA32F:
		tex.glFmt = GL_RGBA;
		break;
	case TextureFormats::DEPTH:
		tex.glFmt = GL_DEPTH_COMPONENT;
		break;
	case TextureFormats::PVRTCI_2BPP:
		tex.glFmt = GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
		break;
	case TextureFormats::PVRTCI_4BPP:
		tex.glFmt = GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
		break;
	case TextureFormats::PVRTCI_A2BPP:
		tex.glFmt = GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
		break;
	case TextureFormats::PVRTCI_A4BPP:
		tex.glFmt = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
		break;
	case TextureFormats::ETC1:
		tex.glFmt = GL_ETC1_RGB8_OES;
		break;
	default:
		ASSERT( 0 );
		break;
	};
	
	glGenTextures( 1, &tex.glObj );
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( tex.type, tex.glObj );

	tex.samplerState = 0;
	applySamplerState( tex );
	
	glBindTexture( tex.type, 0 );
	if( _texSlots[15].texObj )
		glBindTexture( _textures.getRef( _texSlots[15].texObj ).type, _textures.getRef( _texSlots[15].texObj ).glObj );

	// Calculate memory requirements
	tex.memSize = calcTextureSize( format, width, height, depth );
	if( hasMips || genMips ) tex.memSize += ftoi_r( tex.memSize * 1.0f / 3.0f );
	if( type == TextureTypes::TexCube ) tex.memSize *= 6;
	_textureMem += tex.memSize;
	
	return _textures.add( tex );
}


void RenderDevice::uploadTextureData( uint32 texObj, int slice, int mipLevel, const void *pixels )
{
	const RDITexture &tex = _textures.getRef( texObj );
	TextureFormats::List format = tex.format;

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( tex.type, tex.glObj );
	
	int inputFormat = GL_RGBA, inputType = GL_UNSIGNED_BYTE;
	bool compressed = (format == TextureFormats::DXT1) || (format == TextureFormats::DXT3) || (format == TextureFormats::DXT5) ||
		(format == TextureFormats::PVRTCI_2BPP) || (format == TextureFormats::PVRTCI_4BPP) ||  (format == TextureFormats::PVRTCI_A2BPP) ||  (format == TextureFormats::PVRTCI_A4BPP) ||
		(format == TextureFormats::ETC1);
	
	switch( format )
	{
	case TextureFormats::RGBA16F:
		inputFormat = GL_RGBA;
		inputType = GL_FLOAT;
		break;
	case TextureFormats::RGBA32F:
		inputFormat = GL_RGBA;
		inputType = GL_FLOAT;
		break;
	case TextureFormats::DEPTH:
		inputFormat = GL_DEPTH_COMPONENT;
		inputType = GL_UNSIGNED_SHORT;
	};
	
	// Calculate size of next mipmap using "floor" convention
	int width = std::max( tex.width >> mipLevel, 1 ), height = std::max( tex.height >> mipLevel, 1 );
	
	if ( (tex.type == TextureTypes::Tex2D && (width>_maxTextureSize || height>_maxTextureSize)) || 
		 (tex.type == TextureTypes::TexCube && (width>_maxCubeTextureSize || height>_maxCubeTextureSize)))
	return;

	if( tex.type == TextureTypes::Tex2D || tex.type == TextureTypes::TexCube )
	{
		int target = (tex.type == TextureTypes::Tex2D) ?
			GL_TEXTURE_2D : (GL_TEXTURE_CUBE_MAP_POSITIVE_X + slice);
		
		if( compressed )
			glCompressedTexImage2D( target, mipLevel, tex.glFmt, width, height, 0,
			                        calcTextureSize( format, width, height, 1 ), pixels );
		else
			glTexImage2D( target, mipLevel, tex.glFmt, width, height, 0, inputFormat, inputType, pixels );
	}
	else if( tex.type == TextureTypes::Tex3D )
	{
		int depth = std::max( tex.depth >> mipLevel, 1 );
		
		if( compressed )
			glCompressedTexImage3DOES( GL_TEXTURE_3D_OES, mipLevel, tex.glFmt, width, height, depth, 0,
			                        calcTextureSize( format, width, height, depth ), pixels );	
		else
			glTexImage3DOES( GL_TEXTURE_3D_OES, mipLevel, tex.glFmt, width, height, depth, 0,
			              inputFormat, inputType, pixels );
	}

	if( tex.genMips && (tex.type != GL_TEXTURE_CUBE_MAP || slice == 5) )
	{
		// Note: for cube maps mips are only generated when the side with the highest index is uploaded
		glGenerateMipmap( tex.type );
	}

	glBindTexture( tex.type, 0 );
	if( _texSlots[15].texObj )
		glBindTexture( _textures.getRef( _texSlots[15].texObj ).type, _textures.getRef( _texSlots[15].texObj ).glObj );
}


void RenderDevice::destroyTexture( uint32 texObj )
{
	if( texObj == 0 ) return;
	
	const RDITexture &tex = _textures.getRef( texObj );
	if (tex.glObj!=0)
		glDeleteTextures( 1, &tex.glObj );

	_textureMem -= tex.memSize;
	_textures.remove( texObj );
}


void RenderDevice::updateTextureData( uint32 texObj, int slice, int mipLevel, const void *pixels )
{
	uploadTextureData( texObj, slice, mipLevel, pixels );
}


bool RenderDevice::getTextureData( uint32 texObj, int slice, int mipLevel, void *buffer )
{
	Modules::log().writeWarning( "getTextureData is not supported with OpenGL ES 2.0 renderer" );
	return false;
}

uint32 RenderDevice::getTextureMem( uint32 texObj )
{
	if( texObj == 0 ) return 0;
	
	const RDITexture &tex = _textures.getRef( texObj );
	return tex.memSize;
}


// =================================================================================================
// Shaders
// =================================================================================================

uint32 RenderDevice::createShaderProgram( const char *vertexShaderSrc, const char *fragmentShaderSrc )
{
	int infologLength = 0;
	int charsWritten = 0;
	char *infoLog = 0x0;
	int status;

	_shaderLog = "";

	// Vertex shader
	uint32 vs = glCreateShader( GL_VERTEX_SHADER );
	glShaderSource( vs, 1, &vertexShaderSrc, 0x0 );
	glCompileShader( vs );
	glGetShaderiv( vs, GL_COMPILE_STATUS, &status );
	if( !status )
	{	
		// Get info
		glGetShaderiv( vs, GL_INFO_LOG_LENGTH, &infologLength );
		if( infologLength > 1 )
		{
			infoLog = new char[infologLength];
			glGetShaderInfoLog( vs, infologLength, &charsWritten, infoLog );
			_shaderLog = _shaderLog + "[Vertex Shader]\n" + infoLog;
			delete[] infoLog; infoLog = 0x0;
		}

		glDeleteShader( vs );
		return 0;
	}

	// Fragment shader
	uint32 fs = glCreateShader( GL_FRAGMENT_SHADER );
	glShaderSource( fs, 1, &fragmentShaderSrc, 0x0 );
	glCompileShader( fs );
	glGetShaderiv( fs, GL_COMPILE_STATUS, &status );
	if( !status )
	{	
		glGetShaderiv( fs, GL_INFO_LOG_LENGTH, &infologLength );
		if( infologLength > 1 )
		{
			infoLog = new char[infologLength];
			glGetShaderInfoLog( fs, infologLength, &charsWritten, infoLog );
			_shaderLog = _shaderLog + "[Fragment Shader]\n" + infoLog;
			delete[] infoLog; infoLog = 0x0;
		}

		glDeleteShader( vs );
		glDeleteShader( fs );
		return 0;
	}

	// Shader program
	uint32 program = glCreateProgram();
	glAttachShader( program, vs );
	glAttachShader( program, fs );
	glDeleteShader( vs );
	glDeleteShader( fs );

	return program;
}


bool RenderDevice::linkShaderProgram( uint32 programObj )
{
	int infologLength = 0;
	int charsWritten = 0;
	char *infoLog = 0x0;
	int status;

	_shaderLog = "";
	
	glLinkProgram( programObj );
	glGetProgramiv( programObj, GL_INFO_LOG_LENGTH, &infologLength );
	if( infologLength > 1 )
	{
		infoLog = new char[infologLength];
		glGetProgramInfoLog( programObj, infologLength, &charsWritten, infoLog );
		_shaderLog = _shaderLog + "[Linking]\n" + infoLog;
		delete[] infoLog; infoLog = 0x0;
	}
	
	glGetProgramiv( programObj, GL_LINK_STATUS, &status );
	if( !status ) return false;

	return true;
}


uint32 RenderDevice::createShader( const char *vertexShaderSrc, const char *fragmentShaderSrc )
{
	// Compile and link shader
	uint32 programObj = createShaderProgram( vertexShaderSrc, fragmentShaderSrc );
	if( programObj == 0 ) return 0;
	if( !linkShaderProgram( programObj ) ) return 0;
	
	uint32 shaderId = _shaders.add( RDIShader() );
	RDIShader &shader = _shaders.getRef( shaderId );
	shader.oglProgramObj = programObj;
	
	int attribCount;
	glGetProgramiv( programObj, GL_ACTIVE_ATTRIBUTES, &attribCount );
	
	for( uint32 i = 0; i < _numVertexLayouts; ++i )
	{
		RDIVertexLayout &vl = _vertexLayouts[i];
		bool allAttribsFound = true;
		
		for( uint32 j = 0; j < 16; ++j )
			shader.inputLayouts[i].attribIndices[j] = -1;
		
		for( int j = 0; j < attribCount; ++j )
		{
			char name[32];
			uint32 size, type;
			glGetActiveAttrib( programObj, j, 32, 0x0, (int *)&size, &type, name );

			bool attribFound = false;
			for( uint32 k = 0; k < vl.numAttribs; ++k )
			{
				if( vl.attribs[k].semanticName.compare(name) == 0 )
				{
					shader.inputLayouts[i].attribIndices[k] = glGetAttribLocation( programObj, name );
					attribFound = true;
				}
			}

			if( !attribFound )
			{
				allAttribsFound = false;
				break;
			}
		}

		shader.inputLayouts[i].valid = allAttribsFound;
	}

	return shaderId;
}


void RenderDevice::destroyShader( uint32 shaderId )
{
	if( shaderId == 0 ) return;

	RDIShader &shader = _shaders.getRef( shaderId );
	if (shader.oglProgramObj!=0)
		glDeleteProgram( shader.oglProgramObj );
	_shaders.remove( shaderId );
}


void RenderDevice::bindShader( uint32 shaderId )
{
	if( shaderId != 0 )
	{
		RDIShader &shader = _shaders.getRef( shaderId );
		glUseProgram( shader.oglProgramObj );
	}
	else
	{
		glUseProgram( 0 );
	}
	
	_curShaderId = shaderId;
	_pendingMask |= PM_VERTLAYOUT;
} 


int RenderDevice::getShaderConstLoc( uint32 shaderId, const char *name )
{
	RDIShader &shader = _shaders.getRef( shaderId );
	return glGetUniformLocation( shader.oglProgramObj, name );
}


int RenderDevice::getShaderSamplerLoc( uint32 shaderId, const char *name )
{
	RDIShader &shader = _shaders.getRef( shaderId );
	return glGetUniformLocation( shader.oglProgramObj, name );
}


void RenderDevice::setShaderConst( int loc, RDIShaderConstType type, void *values, uint32 count )
{
	switch( type )
	{
	case CONST_FLOAT:
		glUniform1fv( loc, count, (float *)values );
		break;
	case CONST_FLOAT2:
		glUniform2fv( loc, count, (float *)values );
		break;
	case CONST_FLOAT3:
		glUniform3fv( loc, count, (float *)values );
		break;
	case CONST_FLOAT4:
		glUniform4fv( loc, count, (float *)values );
		break;
	case CONST_FLOAT44:
		glUniformMatrix4fv( loc, count, false, (float *)values );
		break;
	case CONST_FLOAT33:
		glUniformMatrix3fv( loc, count, false, (float *)values );
		break;
	}
}


void RenderDevice::setShaderSampler( int loc, uint32 texUnit )
{
	glUniform1i( loc, (int)texUnit );
}


const char *RenderDevice::getDefaultVSCode()
{
	return defaultShaderVS;
}


const char *RenderDevice::getDefaultFSCode()
{
	return defaultShaderFS;
}


// =================================================================================================
// Renderbuffers
// =================================================================================================

uint32 RenderDevice::createRenderBuffer( uint32 width, uint32 height, TextureFormats::List format,
                                         bool depth, uint32 numColBufs, uint32 samples )
{
	if( (format == TextureFormats::RGBA16F || format == TextureFormats::RGBA32F) && !_caps.texFloat )
	{
		return 0;
	}

	if( numColBufs > _caps.rtMaxColBufs ) return 0;

	uint32 maxSamples = 0;
	if( _caps.rtMultisampling )
	{
		GLint value;
		glGetIntegerv( glExt::EXT_framebuffer_multisample ? GL_MAX_SAMPLES_EXT : GL_MAX_SAMPLES_IMG, &value );
		maxSamples = (uint32)value;
	}
	if( samples > maxSamples )
	{
		samples = maxSamples;
		Modules::log().writeWarning( "GPU does not support desired multisampling quality for render target" );
	}

	RDIRenderBuffer rb;
	rb.width = width;
	rb.height = height;
	rb.samples = samples;

	// Create framebuffers
	glGenFramebuffers( 1, &rb.fbo );
	if( samples > 0 && !glExt::IMG_multisampled_render_to_texture ) glGenFramebuffers( 1, &rb.fboMS );

	if( numColBufs > 0 )
	{
		// Attach color buffers
		for( uint32 j = 0; j < numColBufs; ++j )
		{
			glBindFramebuffer( GL_FRAMEBUFFER, rb.fbo );
			// Create a color texture
			uint32 texObj = createTexture( TextureTypes::Tex2D, rb.width, rb.height, 1, format, false, false, false );
			ASSERT( texObj != 0 );
			uploadTextureData( texObj, 0, 0, 0x0 );
			rb.colTexs[j] = texObj;
			RDITexture &tex = _textures.getRef( texObj );
			// Attach the texture
			if (samples > 0 && glExt::IMG_multisampled_render_to_texture )
				glFramebufferTexture2DMultisampleIMG( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex.glObj, 0, samples);
			else
				glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + j, GL_TEXTURE_2D, tex.glObj, 0 );

			if( samples > 0 && !glExt::IMG_multisampled_render_to_texture)
			{
				glBindFramebuffer( GL_FRAMEBUFFER, rb.fboMS );
				// Create a multisampled renderbuffer
				glGenRenderbuffers( 1, &rb.colBufs[j] );
				glBindRenderbuffer( GL_RENDERBUFFER, rb.colBufs[j] );
				glRenderbufferStorageMultisampleEXT( GL_RENDERBUFFER, rb.samples, tex.glFmt, rb.width, rb.height );
				// Attach the renderbuffer
				glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + j,
				                              GL_RENDERBUFFER, rb.colBufs[j] );
			}
		}

		glBindFramebuffer( GL_FRAMEBUFFER, rb.fbo );
		
		if( rb.fboMS > 0 )
			glBindFramebuffer( GL_FRAMEBUFFER, rb.fboMS );
	}
	else
	{	
		glBindFramebuffer( GL_FRAMEBUFFER, rb.fbo );
		
		if( samples > 0 )
			glBindFramebuffer( GL_FRAMEBUFFER, rb.fboMS );
	}

	// Attach depth buffer
	if( depth )
	{
		glBindFramebuffer( GL_FRAMEBUFFER, rb.fbo );

		// Create a depth texture
		if (samples > 0 && glExt::IMG_multisampled_render_to_texture )
		{
			glGenRenderbuffers(1, &rb.depthBuf);
			glBindRenderbuffer(GL_RENDERBUFFER, rb.depthBuf);
			glRenderbufferStorageMultisampleIMG(GL_RENDERBUFFER, samples, 
				_depthFormat, rb.width, rb.height);

			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rb.depthBuf);
		}
		else
		{
			uint32 texObj = createTexture( TextureTypes::Tex2D, rb.width, rb.height, 1, TextureFormats::DEPTH, false, false, false );
			ASSERT( texObj != 0 );
			if (glExt::EXT_shadow_samplers)
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_EXT, GL_NONE );
			uploadTextureData( texObj, 0, 0, 0x0 );
			rb.depthTex = texObj;
			RDITexture &tex = _textures.getRef( texObj );
			// Attach the texture
			glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex.glObj, 0 );
		}

		if( samples > 0 && !glExt::IMG_multisampled_render_to_texture)
		{
			glBindFramebuffer( GL_FRAMEBUFFER, rb.fboMS );
			// Create a multisampled renderbuffer
			glGenRenderbuffers( 1, &rb.depthBuf );
			glBindRenderbuffer( GL_RENDERBUFFER, rb.depthBuf );
			glRenderbufferStorageMultisampleEXT( GL_RENDERBUFFER, rb.samples, _depthFormat, rb.width, rb.height );
			// Attach the renderbuffer
			glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
			                              GL_RENDERBUFFER, rb.depthBuf );
		}
	}

	uint32 rbObj = _rendBufs.add( rb );
	
	// Check if FBO is complete
	bool valid = true;
	glBindFramebuffer( GL_FRAMEBUFFER, rb.fbo );
	uint32 status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	if( status != GL_FRAMEBUFFER_COMPLETE ) valid = false;
	
	if(  samples > 0 && !glExt::IMG_multisampled_render_to_texture )
	{
		glBindFramebuffer( GL_FRAMEBUFFER, rb.fboMS );
		status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
		glBindFramebuffer( GL_FRAMEBUFFER, 0 );
		if( status != GL_FRAMEBUFFER_COMPLETE ) valid = false;
	}

	if( !valid )
	{
		destroyRenderBuffer( rbObj );
		return 0;
	}
	
	glBindFramebuffer( GL_FRAMEBUFFER, _defaultFBO );

	return rbObj;
}


void RenderDevice::destroyRenderBuffer( uint32 rbObj )
{
	RDIRenderBuffer &rb = _rendBufs.getRef( rbObj );
	
	glBindFramebuffer( GL_FRAMEBUFFER, _defaultFBO );
	
	if( rb.depthTex != 0 ) destroyTexture( rb.depthTex );
	if( rb.depthBuf != 0 ) glDeleteRenderbuffers( 1, &rb.depthBuf );
	rb.depthTex = rb.depthBuf = 0;
		
	for( uint32 i = 0; i < RDIRenderBuffer::MaxColorAttachmentCount; ++i )
	{
		if( rb.colTexs[i] != 0 ) destroyTexture( rb.colTexs[i] );
		if( rb.colBufs[i] != 0 ) glDeleteRenderbuffers( 1, &rb.colBufs[i] );
		rb.colTexs[i] = rb.colBufs[i] = 0;
	}

	if( rb.fbo != 0 ) glDeleteFramebuffers( 1, &rb.fbo );
	if( rb.fboMS != 0 ) glDeleteFramebuffers( 1, &rb.fboMS );
	rb.fbo = rb.fboMS = 0;

	_rendBufs.remove( rbObj );
}


uint32 RenderDevice::getRenderBufferTex( uint32 rbObj, uint32 bufIndex )
{
	RDIRenderBuffer &rb = _rendBufs.getRef( rbObj );
	
	if( bufIndex < _caps.rtMaxColBufs ) return rb.colTexs[bufIndex];
	else if( bufIndex == 32 ) return rb.depthTex;
	else return 0;
}


void RenderDevice::resolveRenderBuffer( uint32 rbObj )
{
	RDIRenderBuffer &rb = _rendBufs.getRef( rbObj );
	
	if( rb.fboMS == 0 ) return;
	
	glBindFramebuffer( GL_READ_FRAMEBUFFER_EXT, rb.fboMS );
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER_EXT, rb.fbo );

	bool depthResolved = false;
	for( uint32 i = 0; i < RDIRenderBuffer::MaxColorAttachmentCount; ++i )
	{
		if( rb.colBufs[i] != 0 )
		{
			int mask = GL_COLOR_BUFFER_BIT;
			if( !depthResolved && rb.depthBuf != 0 )
			{
				mask |= GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
				depthResolved = true;
			}
			glBlitFramebufferEXT( 0, 0, rb.width, rb.height, 0, 0, rb.width, rb.height, mask, GL_NEAREST );
		}
	}

	if( !depthResolved && rb.depthBuf != 0 )
	{
		glBlitFramebufferEXT( 0, 0, rb.width, rb.height, 0, 0, rb.width, rb.height,
							  GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST );
	}

	glBindFramebuffer( GL_READ_FRAMEBUFFER_EXT, _defaultFBO );
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER_EXT, _defaultFBO );
}


void RenderDevice::setRenderBuffer( uint32 rbObj )
{
	// Resolve render buffer if necessary
	if( _curRendBuf != 0 ) resolveRenderBuffer( _curRendBuf );
	
	// Set new render buffer
	_curRendBuf = rbObj;
	
	if( rbObj == 0 )
	{
		int currentFrameBuffer;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFrameBuffer);

		//check if it is already, because this call can be extremely expensive on some platforms
		if (currentFrameBuffer != _defaultFBO)	
			glBindFramebuffer( GL_FRAMEBUFFER, _defaultFBO );

		_fbWidth = _vpWidth + _vpX;
		_fbHeight = _vpHeight + _vpY;
#ifndef HORDE3D_GLES2
		glDisable( GL_MULTISAMPLE );
#endif
	}
	else
	{
		// Unbind all textures to make sure that no FBO attachment is bound any more
		for( int i = 0; i < _maxTextureUnits; ++i ) setTexture( i, 0, 0 );
		commitStates( PM_TEXTURES );
		
		RDIRenderBuffer &rb = _rendBufs.getRef( rbObj );

		glBindFramebuffer( GL_FRAMEBUFFER, rb.fboMS != 0 ? rb.fboMS : rb.fbo );
		ASSERT( glCheckFramebufferStatus( GL_FRAMEBUFFER ) == GL_FRAMEBUFFER_COMPLETE );
		_fbWidth = rb.width;
		_fbHeight = rb.height;

#ifndef HORDE3D_GLES2
		if( rb.fboMS != 0 ) glEnable( GL_MULTISAMPLE );
		else glDisable( GL_MULTISAMPLE );
#endif
	}
}

void RenderDevice::getRenderBufferSize( uint32 rbObj, int *width, int *height )
{
	if( rbObj == 0 )
	{
		if( width != 0x0 ) *width = _vpWidth;
		if( height != 0x0 ) *height = _vpHeight;
	}
	else
	{
		RDIRenderBuffer &rb = _rendBufs.getRef( rbObj );
		
		if( width != 0x0 ) *width = rb.width;
		if( height != 0x0 ) *height = rb.height;
	}
}


bool RenderDevice::getRenderBufferData( uint32 rbObj, int bufIndex, int *width, int *height,
                                        int *compCount, void *dataBuffer, int bufferSize )
{
	int x, y, w, h;
	int format = GL_RGBA;
	int type = GL_FLOAT;
	beginRendering();
	glPixelStorei( GL_PACK_ALIGNMENT, 4 );
	
	if( rbObj == 0 )
	{
		if( bufIndex != 32 && bufIndex != 0 ) return false;
		if( width != 0x0 ) *width = _vpWidth;
		if( height != 0x0 ) *height = _vpHeight;
		
		x = _vpX; y = _vpY; w = _vpWidth; h = _vpHeight;

		glBindFramebuffer( GL_FRAMEBUFFER, _defaultFBO );
	}
	else
	{
		resolveRenderBuffer( rbObj );
		RDIRenderBuffer &rb = _rendBufs.getRef( rbObj );
		
		if( bufIndex == 32 && rb.depthTex == 0 ) return false;
		if( bufIndex != 32 )
		{
			if( (unsigned)bufIndex >= RDIRenderBuffer::MaxColorAttachmentCount || rb.colTexs[bufIndex] == 0 )
				return false;
		}
		if( width != 0x0 ) *width = rb.width;
		if( height != 0x0 ) *height = rb.height;

		x = 0; y = 0; w = rb.width; h = rb.height;
		
		glBindFramebuffer( GL_FRAMEBUFFER, rb.fbo );
	}

	if( bufIndex == 32 )
	{	
		format = GL_DEPTH_COMPONENT;
		type = GL_FLOAT;
	}
	
	int comps = (bufIndex == 32 ? 1 : 4);
	if( compCount != 0x0 ) *compCount = comps;
	
	bool retVal = false;
	if( dataBuffer != 0x0 &&
	    bufferSize >= w * h * comps * (type == GL_FLOAT ? 4 : 1) ) 
	{
		glFinish();
		glReadPixels( x, y, w, h, format, type, dataBuffer );
		retVal = true;
	}
	glBindFramebuffer( GL_FRAMEBUFFER, _defaultFBO );

	return retVal;
}


// =================================================================================================
// Queries
// =================================================================================================

uint32 RenderDevice::createOcclusionQuery()
{
	uint32 queryObj;
	glGenQueries( 1, &queryObj );
	return queryObj;
}


void RenderDevice::destroyQuery( uint32 queryObj )
{
	if( queryObj == 0 ) return;
	
	glDeleteQueries( 1, &queryObj );
}


void RenderDevice::beginQuery( uint32 queryObj )
{
	glBeginQueryEXT( GL_ANY_SAMPLES_PASSED_EXT, queryObj );
}


void RenderDevice::endQuery( uint32 /*queryObj*/ )
{
	glEndQueryEXT( GL_ANY_SAMPLES_PASSED_EXT );
}


uint32 RenderDevice::getQueryResult( uint32 queryObj )
{
	uint32 samples = 0;
	glGetQueryObjectuivEXT( queryObj, GL_QUERY_RESULT_EXT, &samples );
	return samples;
}


// =================================================================================================
// Internal state management
// =================================================================================================

void RenderDevice::checkGLError()
{
	uint32 error = glGetError();
	ASSERT( error != GL_INVALID_ENUM );
	ASSERT( error != GL_INVALID_VALUE );
	ASSERT( error != GL_INVALID_OPERATION );
	ASSERT( error != GL_OUT_OF_MEMORY );
}


bool RenderDevice::applyVertexLayout()
{
	uint32 newVertexAttribMask = 0;
	
	if( _newVertLayout != 0 )
	{
		if( _curShaderId == 0 ) return false;
		
		RDIVertexLayout &vl = _vertexLayouts[_newVertLayout - 1];
		RDIShader &shader = _shaders.getRef( _curShaderId );
		RDIInputLayout &inputLayout = shader.inputLayouts[_newVertLayout - 1];
		
		if( !inputLayout.valid )
			return false;

		// Set vertex attrib pointers
		for( uint32 i = 0; i < vl.numAttribs; ++i )
		{
			int8 attribIndex = inputLayout.attribIndices[i];
			if( attribIndex >= 0 )
			{
				VertexLayoutAttrib &attrib = vl.attribs[i];
				const RDIVertBufSlot &vbSlot = _vertBufSlots[attrib.vbSlot];
				
				RDIBuffer &buf = _buffers.getRef( _vertBufSlots[attrib.vbSlot].vbObj);
				
				ASSERT( buf.type == GL_ARRAY_BUFFER );
				
				if (buf.glObj!=0)
				{
					glBindBuffer( GL_ARRAY_BUFFER, buf.glObj );
					glVertexAttribPointer( attribIndex, attrib.size, GL_FLOAT, GL_FALSE,
										   vbSlot.stride, (char *)0 + vbSlot.offset + attrib.offset );
				}

				newVertexAttribMask |= 1 << attribIndex;
			}
		}
	}
	
	for( uint32 i = 0; i < 16; ++i )
	{
		uint32 curBit = 1 << i;
		if( (newVertexAttribMask & curBit) != (_activeVertexAttribsMask & curBit) )
		{
			if( newVertexAttribMask & curBit ) glEnableVertexAttribArray( i );
			else glDisableVertexAttribArray( i );
		}
	}
	_activeVertexAttribsMask = newVertexAttribMask;

	return true;
}


void RenderDevice::applySamplerState( RDITexture &tex )
{
	uint32 state = tex.samplerState;
	uint32 target = tex.type;
	
	const uint32 magFilters[] = { GL_LINEAR, GL_LINEAR, GL_NEAREST };
	const uint32 minFiltersMips[] = { GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR, GL_NEAREST_MIPMAP_NEAREST };
	const uint32 maxAniso[] = { 1, 2, 4, 8, 16, 0, 0, 0 };
	const uint32 wrapModes[] = { GL_CLAMP_TO_EDGE, GL_REPEAT, GL_CLAMP_TO_EDGE };//todo: clamp to border is not supported

	if( tex.hasMips )
		glTexParameteri( target, GL_TEXTURE_MIN_FILTER, minFiltersMips[(state & SS_FILTER_MASK) >> SS_FILTER_START] );
	else
		glTexParameteri( target, GL_TEXTURE_MIN_FILTER, magFilters[(state & SS_FILTER_MASK) >> SS_FILTER_START] );

	glTexParameteri( target, GL_TEXTURE_MAG_FILTER, magFilters[(state & SS_FILTER_MASK) >> SS_FILTER_START] );
	if (glExt::EXT_texture_filter_anisotropic)
		glTexParameteri( target, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso[(state & SS_ANISO_MASK) >> SS_ANISO_START] );
	glTexParameteri( target, GL_TEXTURE_WRAP_S, wrapModes[(state & SS_ADDRU_MASK) >> SS_ADDRU_START] );
	glTexParameteri( target, GL_TEXTURE_WRAP_T, wrapModes[(state & SS_ADDRV_MASK) >> SS_ADDRV_START] );
	if (glExt::OES_texture_3D)
		glTexParameteri( target, GL_TEXTURE_WRAP_R_OES, wrapModes[(state & SS_ADDRW_MASK) >> SS_ADDRW_START] );

	if (glExt::EXT_shadow_samplers)
	{
		if( !(state & SS_COMP_LEQUAL) )
		{
			glTexParameteri( target, GL_TEXTURE_COMPARE_MODE_EXT, GL_NONE );
		}
		else
		{
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_EXT, GL_COMPARE_REF_TO_TEXTURE_EXT );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC_EXT, GL_LEQUAL );
		}
	}
}


void RenderDevice::applyRenderStates()
{
	// Rasterizer state
	if( _newRasterState.hash != _curRasterState.hash )
	{
		//TODO: not supported on es2.0
		//if( _newRasterState.fillMode == RS_FILL_SOLID ) glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		//else glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

		if( _newRasterState.cullMode == RS_CULL_BACK )
		{
			glEnable( GL_CULL_FACE );
			glCullFace( GL_BACK );
		}
		else if( _newRasterState.cullMode == RS_CULL_FRONT )
		{
			glEnable( GL_CULL_FACE );
			glCullFace( GL_FRONT );
		}
		else
		{
			glDisable( GL_CULL_FACE );
		}

		if( !_newRasterState.scissorEnable ) glDisable( GL_SCISSOR_TEST );
		else glEnable( GL_SCISSOR_TEST );

		if( _newRasterState.renderTargetWriteMask ) glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
		else glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
		
		_curRasterState.hash = _newRasterState.hash;
	}

	// Blend state
	if( _newBlendState.hash != _curBlendState.hash )
	{
		if( !_newBlendState.alphaToCoverageEnable ) glDisable( GL_SAMPLE_ALPHA_TO_COVERAGE );
		else glEnable( GL_SAMPLE_ALPHA_TO_COVERAGE );

		if( !_newBlendState.blendEnable )
		{
			glDisable( GL_BLEND );
		}
		else
		{
			static const uint32 oglBlendFuncs[8] = { GL_ZERO, GL_ONE, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_DST_COLOR, GL_ZERO, GL_ZERO };
			
			glEnable( GL_BLEND );
			glBlendFunc( oglBlendFuncs[_newBlendState.srcBlendFunc], oglBlendFuncs[_newBlendState.destBlendFunc] );
		}
		
		_curBlendState.hash = _newBlendState.hash;
	}

	// Depth-stencil state
	if( _newDepthStencilState.hash != _curDepthStencilState.hash )
	{
		if( _newDepthStencilState.depthWriteMask ) glDepthMask( GL_TRUE );
		else glDepthMask( GL_FALSE);

		if( _newDepthStencilState.depthEnable )
		{
			static const uint32 oglDepthFuncs[8] = { GL_LEQUAL, GL_LESS, GL_EQUAL, GL_GREATER, GL_GEQUAL, GL_ALWAYS, GL_ALWAYS, GL_ALWAYS };
			
			glEnable( GL_DEPTH_TEST );
			glDepthFunc( oglDepthFuncs[_newDepthStencilState.depthFunc] );
		}
		else
		{
			glDisable( GL_DEPTH_TEST );
		}
		
		_curDepthStencilState.hash = _newDepthStencilState.hash;
	}
}


bool RenderDevice::commitStates( uint32 filter )
{
	if( _pendingMask & filter )
	{
		uint32 mask = _pendingMask & filter;
	
		// Set viewport
		if( mask & PM_VIEWPORT )
		{
			glViewport( _vpX, _vpY, _vpWidth, _vpHeight );
			_pendingMask &= ~PM_VIEWPORT;
		}

		if( mask & PM_RENDERSTATES )
		{
			applyRenderStates();
			_pendingMask &= ~PM_RENDERSTATES;
		}

		// Set scissor rect
		if( mask & PM_SCISSOR )
		{
			glScissor( _scX, _scY, _scWidth, _scHeight );
			_pendingMask &= ~PM_SCISSOR;
		}
		
		// Bind index buffer
		if( mask & PM_INDEXBUF )
		{
			if( _newIndexBuf != _curIndexBuf )
			{
				if( _newIndexBuf != 0 )
					glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, _buffers.getRef( _newIndexBuf ).glObj );
				else
					glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
				
				_curIndexBuf = _newIndexBuf;
				_pendingMask &= ~PM_INDEXBUF;
			}
		}

		// Bind textures and set sampler state
		if ( _pendingTextureMask != 0)
		{
			for( int i = 0; i < _maxTextureUnits; ++i )
			{
				if ( (_pendingTextureMask&(1<<i)) == 0)
					continue;
				glActiveTexture( GL_TEXTURE0 + i );

				if( _texSlots[i].texObj != 0 )
				{
					RDITexture &tex = _textures.getRef( _texSlots[i].texObj );
					glBindTexture( tex.type, tex.glObj );

					// Apply sampler state
					if( tex.samplerState != _texSlots[i].samplerState )
					{
						tex.samplerState = _texSlots[i].samplerState;
						applySamplerState( tex );
					}
				}
				else
				{
					glBindTexture( GL_TEXTURE_CUBE_MAP, 0 );
					if ( glExt::OES_texture_3D )
						glBindTexture( GL_TEXTURE_3D_OES, 0 );
					glBindTexture( GL_TEXTURE_2D, 0 );
				}
				_pendingTextureMask &= ~(1<<i);
				if (_pendingTextureMask == 0)
					break;
			}
			
			_pendingTextureMask = 0;
		}

		// Bind vertex buffers
		if( mask & PM_VERTLAYOUT )
		{
			//if( _newVertLayout != _curVertLayout || _curShader != _prevShader )
			{
				if( !applyVertexLayout() )
					return false;
				_curVertLayout = _newVertLayout;
				_prevShaderId = _curShaderId;
				_pendingMask &= ~PM_VERTLAYOUT;
			}
		}

		CHECK_GL_ERROR
	}

	return true;
}


void RenderDevice::resetStates()
{
	_curIndexBuf = 1; _newIndexBuf = 0;
	_curVertLayout = 1; _newVertLayout = 0;
	_curRasterState.hash = 0xFFFFFFFF; _newRasterState.hash = 0;
	_curBlendState.hash = 0xFFFFFFFF; _newBlendState.hash = 0;
	_curDepthStencilState.hash = 0xFFFFFFFF; _newDepthStencilState.hash = 0;

	for( int i = 0; i < _maxTextureUnits; ++i )
		setTexture( i, 0, 0 );

	setColorWriteMask( true );
	_pendingMask = 0xFFFFFFFF;
	_pendingTextureMask = 0xFFFFFFFF;

	_activeVertexAttribsMask = 0xFFFFFFFF;
	commitStates();

	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glActiveTexture(GL_TEXTURE0);

	glBindFramebuffer( GL_FRAMEBUFFER, _defaultFBO );
}


// =================================================================================================
// Draw calls and clears
// =================================================================================================

void RenderDevice::clear( uint32 flags, float *colorRGBA, float depth )
{
	uint32 prevBuffers[4] = { 0 };

	if( _curRendBuf != 0x0 )
	{
		RDIRenderBuffer &rb = _rendBufs.getRef( _curRendBuf );
		
		if( (flags & CLR_DEPTH) && rb.depthTex == 0 ) flags &= ~CLR_DEPTH;
	}
	
	uint32 oglClearMask = 0;
	
	if( flags & CLR_DEPTH )
	{
		oglClearMask |= GL_DEPTH_BUFFER_BIT;
		glClearDepthf( depth );
	}
	if( flags & (CLR_COLOR_RT0 | CLR_COLOR_RT1 | CLR_COLOR_RT2 | CLR_COLOR_RT3) )
	{
		oglClearMask |= GL_COLOR_BUFFER_BIT;
		if( colorRGBA ) glClearColor( colorRGBA[0], colorRGBA[1], colorRGBA[2], colorRGBA[3] );
		else glClearColor( 0, 0, 0, 0 );
	}
	
	if( oglClearMask )
	{	
		commitStates( PM_VIEWPORT | PM_SCISSOR | PM_RENDERSTATES );
		glClear( oglClearMask );
	}

	CHECK_GL_ERROR
}


void RenderDevice::draw( RDIPrimType primType, uint32 firstVert, uint32 numVerts )
{
	if( commitStates() )
	{
		glDrawArrays( (uint32)primType, firstVert, numVerts );
	}

	CHECK_GL_ERROR
}


void RenderDevice::drawIndexed( RDIPrimType primType, uint32 firstIndex, uint32 numIndices,
                                uint32 firstVert, uint32 numVerts )
{
	if( commitStates() )
	{
		firstIndex *= (_indexFormat == IDXFMT_16) ? sizeof( short ) : sizeof( int );

		glDrawElements((uint32)primType, numIndices, _indexFormat, (char *)0 + firstIndex);
	}

	CHECK_GL_ERROR
}

}  // namespace
