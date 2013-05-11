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

#ifndef _egRendererBaseD3D11_H_
#define _egRendererBaseD3D11_H_

#include <string>


namespace Horde3D {

const uint32 MaxNumVertexLayouts = 16;


// =================================================================================================
// GPUTimer
// =================================================================================================

class GPUTimer	//INTERFACE
{
public:
	GPUTimer();
	~GPUTimer();
	
	void beginQuery( uint32 frameID );
	void endQuery();
	bool updateResults();
	
	void reset();
	float getTimeMS() { return _time; }

private:
	std::vector < uint32 >  _queryPool;
	uint32                  _numQueries;
	uint32                  _queryFrame;
	float                   _time;
	bool                    _activeQuery;
};

// ---------------------------------------------------------
// Vertex layout
// ---------------------------------------------------------

struct RDIVertexLayout
{
	uint32              numAttribs;
	VertexLayoutAttrib  attribs[16];
};


// ---------------------------------------------------------
// Buffers
// ---------------------------------------------------------

struct RDIBuffer
{
	uint32  type;
	ID3D11Buffer* d3dObj;
	uint32  size;
	bool dynamic;
};

struct RDIVertBufSlot
{
	uint32  vbObj;
	uint32  offset;
	uint32  stride;

	RDIVertBufSlot() : vbObj( 0 ), offset( 0 ), stride( 0 ) {}
	RDIVertBufSlot( uint32 vbObj, uint32 offset, uint32 stride ) :
		vbObj( vbObj ), offset( offset ), stride( stride ) {}
};


// ---------------------------------------------------------
// Textures
// ---------------------------------------------------------

struct RDITexture
{
	union
	{
		ID3D11Resource*		d3dTexture;	
		ID3D11Texture2D*	d3dTexture2D;	
		ID3D11Texture3D*	d3dTexture3D;	
	};
	ID3D11ShaderResourceView* d3dResourceView;

	DXGI_FORMAT           d3dFmt;
	int                   type;
	TextureFormats::List  format;
	int                   width, height, depth;
	int                   memSize;
	bool                  sRGB;
	bool                  hasMips, genMips;
};

struct RDITexSlot
{
	uint32  texObj;
	uint32  samplerState;

	RDITexSlot() : texObj( 0 ), samplerState( 0 ) {}
	RDITexSlot( uint32 texObj, uint32 samplerState ) :
		texObj( texObj ), samplerState( samplerState ) {}
};


// ---------------------------------------------------------
// Shaders
// ---------------------------------------------------------

struct RDIShaderType
{
	enum List
	{
		Vertex,
		Pixel,
		Count,
	};
};

struct RDIUniform
{
	std::string name;
	struct Desc 
	{ 
		Desc() { offset = -1; size = 0; }
		int		offset;
		uint32	size; 
	};
	Desc desc[RDIShaderType::Count];
};

struct RDISampler
{
	std::string name;
	struct Desc
	{
		Desc() { samplerBindPoint = -1; textureBindPoint = -1; }
		int samplerBindPoint;
		int textureBindPoint;
	};
	int slot;	//index in RenderDevice _texSlots
	Desc desc[RDIShaderType::Count];
};

struct RDIShader
{
	ID3D11InputLayout* inputLayouts[MaxNumVertexLayouts];
	ID3D11VertexShader* vertexShader;
	ID3D11PixelShader* pixelShader;

		// constants in $Globals cbuffer
	struct GlobalCBuffer
	{
		uint32	bufferId;
		int8*	bufferData;
		int     bindPoint;	
	};
	GlobalCBuffer	globalCBuffers[RDIShaderType::Count];
	std::vector<RDIUniform> uniforms;
	std::vector<RDISampler> samplers;
};

// ---------------------------------------------------------
// Render buffers
// ---------------------------------------------------------

struct RDIRenderBuffer
{
	static const uint32 MaxColorAttachmentCount = 4;

	uint32  fbo, fboMS;  // fboMS: Multisampled FBO used when samples > 0
	uint32  width, height;
	uint32  samples;

	uint32  depthTex, colTexs[MaxColorAttachmentCount];
	uint32  depthBuf, colBufs[MaxColorAttachmentCount];  // Used for multisampling

	RDIRenderBuffer() : fbo( 0 ), fboMS( 0 ), width( 0 ), height( 0 ), depthTex( 0 ), depthBuf( 0 )
	{
		for( uint32 i = 0; i < MaxColorAttachmentCount; ++i ) colTexs[i] = colBufs[i] = 0;
	}
};


// ---------------------------------------------------------
// Render states
// ---------------------------------------------------------

struct RDIRasterState
{
	union
	{
		uint32  hash;
		struct
		{
			uint32  fillMode : 1;  // RDIFillMode
			uint32  cullMode : 2;  // RDICullMode
			uint32  scissorEnable : 1;
			uint32  multisampleEnable : 1;
		};
	};
	static const uint32 stateMask = 0x0000001f;
	static const uint32 numStates = stateMask + 1;
};

struct RDIBlendState
{
	union
	{
		uint32  hash;
		struct
		{
			uint32  alphaToCoverageEnable : 1;
			uint32  blendEnable : 1;
			uint32  srcBlendFunc : 4;
			uint32  destBlendFunc : 4;
			uint32  renderTargetWriteMask : 1;
		};
	};
	static const uint32 stateMask = 0x000007ff;
	static const uint32 numStates = stateMask + 1;
};


struct RDIDepthStencilState
{
	union
	{
		uint32  hash;
		struct
		{
			uint32  depthWriteMask : 1;
			uint32  depthEnable : 1;
			uint32  depthFunc : 4;  // RDIDepthFunc
		};
	};
	static const uint32 stateMask = 0x0000003f;
	static const uint32 numStates = stateMask + 1;
};

// =================================================================================================

class RenderDevice
{
public:

	RenderDevice(void* device);
	~RenderDevice();
	
	void initStates();
	bool init();

	void beginRendering();
	void finishRendering();


// -----------------------------------------------------------------------------
// Resources
// -----------------------------------------------------------------------------

	// Vertex layouts
	uint32 registerVertexLayout( uint32 numAttribs, VertexLayoutAttrib *attribs );
	
	// Buffers
	uint32 createVertexBuffer( uint32 size, const void *data, bool dynamic = false );
	uint32 createIndexBuffer( uint32 size, const void *data, bool dynamic = false );
	uint32 createUniformBuffer( uint32 size, const void *data );
	void destroyBuffer( uint32 bufObj );
	void updateBufferData( uint32 bufObj, uint32 offset, uint32 size, void *data );
	uint32 getBufferMem() { return _bufferMem; }

	// Textures
	uint32 calcTextureSize( TextureFormats::List format, int width, int height, int depth );
	uint32 createTexture( TextureTypes::List type, int width, int height, int depth, TextureFormats::List format,
	                      bool hasMips, bool genMips, bool compress, bool sRGB );
	void uploadTextureData( uint32 texObj, int slice, int mipLevel, const void *pixels );
	void destroyTexture( uint32 texObj );
	void updateTextureData( uint32 texObj, int slice, int mipLevel, const void *pixels );
	bool getTextureData( uint32 texObj, int slice, int mipLevel, void *buffer );
	uint32 getTextureMem() { return _textureMem; }

	// Shaders
	uint32 createShader( const char *vertexShaderSrc, const char *fragmentShaderSrc );
	void destroyShader( uint32 shaderId );
	void bindShader( uint32 shaderId );
	std::string &getShaderLog() { return _shaderLog; }
	int getShaderConstLoc( uint32 shaderId, const char *name );
	int getShaderSamplerLoc( uint32 shaderId, const char *name );
	void setShaderConst( int loc, RDIShaderConstType type, void *values, uint32 count = 1 );
	void setShaderSampler( int loc, uint32 texUnit );
	const char *getDefaultVSCode();
	const char *getDefaultFSCode();

	// Renderbuffers
	uint32 createRenderBuffer( uint32 width, uint32 height, TextureFormats::List format,
	                           bool depth, uint32 numColBufs, uint32 samples );
	void destroyRenderBuffer( uint32 rbObj );
	uint32 getRenderBufferTex( uint32 rbObj, uint32 bufIndex );
	void setRenderBuffer( uint32 rbObj );
	void getRenderBufferSize( uint32 rbObj, int *width, int *height );
	bool getRenderBufferData( uint32 rbObj, int bufIndex, int *width, int *height,
	                          int *compCount, void *dataBuffer, int bufferSize );

	// Queries
	uint32 createOcclusionQuery();
	void destroyQuery( uint32 queryObj );
	void beginQuery( uint32 queryObj );
	void endQuery( uint32 queryObj );
	uint32 getQueryResult( uint32 queryObj );

// -----------------------------------------------------------------------------
// Commands
// -----------------------------------------------------------------------------
	
	void setViewport( int x, int y, int width, int height )
		{ _vpX = x; _vpY = y; _vpWidth = width; _vpHeight = height; _pendingMask |= PM_VIEWPORT; }
	void setScissorRect( int x, int y, int width, int height )
		{ _scX = x; _scY = y; _scWidth = width; _scHeight = height; _pendingMask |= PM_SCISSOR; }
	void setIndexBuffer( uint32 bufObj, RDIIndexFormat idxFmt )
		{ _indexFormat = (uint32)idxFmt; _newIndexBuf = bufObj; _pendingMask |= PM_INDEXBUF; }
	void setVertexBuffer( uint32 slot, uint32 vbObj, uint32 offset, uint32 stride )
		{ ASSERT( slot < 16 ); _vertBufSlots[slot] = RDIVertBufSlot( vbObj, offset, stride );
	      _pendingMask |= PM_VERTLAYOUT; }
	void setVertexLayout( uint32 vlObj )
		{ _newVertLayout = vlObj; }
	void setTexture( uint32 slot, uint32 texObj, uint16 samplerState )
		{ ASSERT( slot < 16 ); _texSlots[slot] = RDITexSlot( texObj, samplerState );
	      _pendingMask |= PM_TEXTURES; }
	
	// Render states
	void setColorWriteMask( bool enabled )
		{ _newBlendState.renderTargetWriteMask = enabled; _pendingMask |= PM_RENDERSTATES; }
	void getColorWriteMask( bool &enabled )
		{ enabled = _newBlendState.renderTargetWriteMask; }
	void setFillMode( RDIFillMode fillMode )
		{ _newRasterState.fillMode = fillMode; _pendingMask |= PM_RENDERSTATES; }
	void getFillMode( RDIFillMode &fillMode )
		{ fillMode = (RDIFillMode)_newRasterState.fillMode; }
	void setCullMode( RDICullMode cullMode )
		{ _newRasterState.cullMode = cullMode; _pendingMask |= PM_RENDERSTATES; }
	void getCullMode( RDICullMode &cullMode )
		{ cullMode = (RDICullMode)_newRasterState.cullMode; }
	void setScissorTest( bool enabled )
		{ _newRasterState.scissorEnable = enabled; _pendingMask |= PM_RENDERSTATES; }
	void getScissorTest( bool &enabled )
		{ enabled = _newRasterState.scissorEnable; }
	void setMulisampling( bool enabled )
		{ _newRasterState.multisampleEnable = enabled; _pendingMask |= PM_RENDERSTATES; }
	void getMulisampling( bool &enabled )
		{ enabled = _newRasterState.multisampleEnable; }
	void setAlphaToCoverage( bool enabled )
		{ _newBlendState.alphaToCoverageEnable = enabled; _pendingMask |= PM_RENDERSTATES; }
	void getAlphaToCoverage( bool &enabled )
		{ enabled = _newBlendState.alphaToCoverageEnable; }
	void setBlendMode( bool enabled, RDIBlendFunc srcBlendFunc = BS_BLEND_ZERO, RDIBlendFunc destBlendFunc = BS_BLEND_ZERO )
		{ _newBlendState.blendEnable = enabled; _newBlendState.srcBlendFunc = srcBlendFunc;
		  _newBlendState.destBlendFunc = destBlendFunc; _pendingMask |= PM_RENDERSTATES; }
	void getBlendMode( bool &enabled, RDIBlendFunc &srcBlendFunc, RDIBlendFunc &destBlendFunc )
		{ enabled = _newBlendState.blendEnable; srcBlendFunc = (RDIBlendFunc)_newBlendState.srcBlendFunc;
		  destBlendFunc = (RDIBlendFunc)_newBlendState.destBlendFunc; }
	void setDepthMask( bool enabled )
		{ _newDepthStencilState.depthWriteMask = enabled; _pendingMask |= PM_RENDERSTATES; }
	void getDepthMask( bool &enabled )
		{ enabled = _newDepthStencilState.depthWriteMask; }
	void setDepthTest( bool enabled )
		{ _newDepthStencilState.depthEnable = enabled; _pendingMask |= PM_RENDERSTATES; }
	void getDepthTest( bool &enabled )
		{ enabled = _newDepthStencilState.depthEnable; }
	void setDepthFunc( RDIDepthFunc depthFunc )
		{ _newDepthStencilState.depthFunc = depthFunc; _pendingMask |= PM_RENDERSTATES; }
	void getDepthFunc( RDIDepthFunc &depthFunc )
		{ depthFunc = (RDIDepthFunc)_newDepthStencilState.depthFunc; }

	bool commitStates( uint32 filter = 0xFFFFFFFF );
	void resetStates();
	
	// Draw calls and clears
	void clear( uint32 flags, float *colorRGBA = 0x0, float depth = 1.0f );
	void draw( RDIPrimType primType, uint32 firstVert, uint32 numVerts );
	void drawIndexed( RDIPrimType primType, uint32 firstIndex, uint32 numIndices,
	                  uint32 firstVert, uint32 numVerts );

// -----------------------------------------------------------------------------
// Getters
// -----------------------------------------------------------------------------

	const DeviceCaps &getCaps() { return _caps; }
	const RDIBuffer &getBuffer( uint32 bufObj ) { return _buffers.getRef( bufObj ); }
	const RDITexture &getTexture( uint32 texObj ) { return _textures.getRef( texObj ); }
	const RDIRenderBuffer &getRenderBuffer( uint32 rbObj ) { return _rendBufs.getRef( rbObj ); }

	friend class Renderer;

protected:

	enum RDIPendingMask
	{
		PM_VIEWPORT      = 0x00000001,
		PM_INDEXBUF      = 0x00000002,
		PM_VERTLAYOUT    = 0x00000004,
		PM_TEXTURES      = 0x00000008,
		PM_SCISSOR       = 0x00000010,
		PM_RENDERSTATES  = 0x00000020
	};

protected:

	void resolveRenderBuffer( uint32 rbObj );

	bool applyVertexLayout();
	ID3D11SamplerState* getSamplerState( uint32 samplerState );
	void applyRenderStates();

	uint32 createBuffer( uint32 size, const void *data, bool dynamic, uint32 bindFlags );

protected:

	void parseShaderBlob(uint32 shaderId, LPD3D10BLOB blob, RDIShaderType::List type);

	ID3D11Device* _d3dDevice;
	ID3D11DeviceContext* _d3dContext;
	DeviceCaps    _caps;
	
	DXGI_FORMAT   _depthFormat;
	int           _vpX, _vpY, _vpWidth, _vpHeight;
	int           _scX, _scY, _scWidth, _scHeight;
	int           _fbWidth, _fbHeight;
	std::string   _shaderLog;
	uint32        _curRendBuf;
	int           _outputBufferIndex;  // Left and right eye for stereo rendering
	uint32        _textureMem, _bufferMem;

	ID3D11RenderTargetView*			_defaultRenderTargetViews[RDIRenderBuffer::MaxColorAttachmentCount];
	ID3D11DepthStencilView*			_defaultDepthStencilView;
	uint32                         _numVertexLayouts;
	RDIVertexLayout                _vertexLayouts[MaxNumVertexLayouts];
	RDIObjects< RDIBuffer >        _buffers;
	RDIObjects< RDITexture >       _textures;
	RDIObjects< RDIShader >        _shaders;
	RDIObjects< RDIRenderBuffer >  _rendBufs;

	ID3D11RasterizerState*		_rasterizerStates[RDIRasterState::numStates];
	ID3D11BlendState*			_blendStates[RDIBlendState::numStates];
	ID3D11DepthStencilState*	_depthStencilStates[RDIDepthStencilState::numStates];
	ID3D11SamplerState*			_samplerStates[RDISamplerNumStates];

	RDIVertBufSlot        _vertBufSlots[16];
	RDITexSlot            _texSlots[16];
	RDIRasterState        _curRasterState, _newRasterState;
	RDIBlendState         _curBlendState, _newBlendState;
	RDIDepthStencilState  _curDepthStencilState, _newDepthStencilState;
	uint32                _prevShaderId, _curShaderId;
	uint32                _curVertLayout, _newVertLayout;
	uint32                _curIndexBuf, _newIndexBuf;
	uint32                _indexFormat;
	uint32                _activeVertexAttribsMask;
	uint32                _pendingMask;
};

}
#endif // _egRendererBaseD3D11_H_
