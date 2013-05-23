#ifndef _egRendererBase_H_
#define _egRendererBase_H_

#include "egPrerequisites.h"
#include "utMath.h"
#include <vector>

#ifdef HORDE3D_D3D11
#define WIN32_LEAN_AND_MEAN 1
#ifndef NOMINMAX
#	define NOMINMAX
#endif
#include <d3d11.h>
#elif HORDE3D_GLES2
#include "GLES2/utOpenGLES2.h"
#elif HORDE3D_GL
#include "GL/utOpenGL.h"
#endif


namespace Horde3D {

// ---------------------------------------------------------
//	Render Device Interface
// ---------------------------------------------------------

struct DeviceCaps
{
		//compressed formats
	bool texDXT;	//dxt 1,3,5 support (bc1-3 in d3d11)
	bool texPVRTCI;	
	bool texETC1;

		//other texture caps
	bool texFloat;
	bool texDepth;
	bool texShadowCompare;
	bool tex3D;
	bool texNPOT;	// non power of two textures

		//rendertarget caps
	bool rtMultisampling;
	uint32 rtMaxColBufs;

		//queries
	bool occQuery;
	bool timerQuery;
};

// ---------------------------------------------------------
// Vertex layout
// ---------------------------------------------------------

struct VertexLayoutAttrib
{
	std::string  semanticName;
	uint32       vbSlot;
	uint32       size;
	uint32       offset;
};


struct TextureTypes
{
	enum List
	{
#if defined(HORDE3D_GL)
		Tex2D = GL_TEXTURE_2D,
		Tex3D = GL_TEXTURE_3D,
		TexCube = GL_TEXTURE_CUBE_MAP
#elif defined(HORDE3D_GLES2)
		Tex2D = GL_TEXTURE_2D,
		Tex3D = GL_TEXTURE_3D_OES,
		TexCube = GL_TEXTURE_CUBE_MAP
#else
		Tex2D,
		Tex3D,
		TexCube
#endif
	};
};

struct TextureFormats
{
	enum List
	{
		Unknown,
		RGBA8,
		DXT1,
		DXT3,
		DXT5,
		RGBA16F,
		RGBA32F,
		DEPTH,

		PVRTCI_2BPP,
		PVRTCI_A2BPP,
		PVRTCI_4BPP,
		PVRTCI_A4BPP,
		ETC1,
	};
};

enum RDIShaderConstType
{
	CONST_FLOAT,
	CONST_FLOAT2,
	CONST_FLOAT3,
	CONST_FLOAT4,
	CONST_FLOAT44,
	CONST_FLOAT33
};

// ---------------------------------------------------------
// Render states
// ---------------------------------------------------------

// Note: Render states use unions to provide a hash value. Writing to and reading from different members of a
//       union is not guaranteed to work by the C++ standard but is common practice and supported by many compilers.

enum RDISamplerState
{
	SS_FILTER_BILINEAR   = 0x0,
	SS_FILTER_TRILINEAR  = 0x0001,
	SS_FILTER_POINT      = 0x0002,
	SS_ANISO1            = 0x0,
	SS_ANISO2            = 0x0004,
	SS_ANISO4            = 0x0008,
	SS_ANISO8            = 0x000c,
	SS_ANISO16           = 0x0010,
	SS_ADDRU_CLAMP       = 0x0,
	SS_ADDRU_WRAP        = 0x0020,
	SS_ADDRU_CLAMPCOL    = 0x0040,
	SS_ADDRV_CLAMP       = 0x0,
	SS_ADDRV_WRAP        = 0x0080,
	SS_ADDRV_CLAMPCOL    = 0x0100,
	SS_ADDRW_CLAMP       = 0x0,
	SS_ADDRW_WRAP        = 0x0200,
	SS_ADDRW_CLAMPCOL    = 0x0400,
	SS_ADDR_CLAMP        = SS_ADDRU_CLAMP | SS_ADDRV_CLAMP | SS_ADDRW_CLAMP,
	SS_ADDR_WRAP         = SS_ADDRU_WRAP | SS_ADDRV_WRAP | SS_ADDRW_WRAP,
	SS_ADDR_CLAMPCOL     = SS_ADDRU_CLAMPCOL | SS_ADDRV_CLAMPCOL | SS_ADDRW_CLAMPCOL,
	SS_COMP_LEQUAL       = 0x800
};

const uint32 SS_FILTER_START = 0;
const uint32 SS_FILTER_MASK = SS_FILTER_BILINEAR | SS_FILTER_TRILINEAR | SS_FILTER_POINT;
const uint32 SS_ANISO_START = 2;
const uint32 SS_ANISO_MASK = SS_ANISO1 | SS_ANISO2 | SS_ANISO4 | SS_ANISO8 | SS_ANISO16;
const uint32 SS_ADDRU_START = 5;
const uint32 SS_ADDRU_MASK = SS_ADDRU_CLAMP | SS_ADDRU_WRAP | SS_ADDRU_CLAMPCOL;
const uint32 SS_ADDRV_START = 7;
const uint32 SS_ADDRV_MASK = SS_ADDRV_CLAMP | SS_ADDRV_WRAP | SS_ADDRV_CLAMPCOL;
const uint32 SS_ADDRW_START = 9;
const uint32 SS_ADDRW_MASK = SS_ADDRW_CLAMP | SS_ADDRW_WRAP | SS_ADDRW_CLAMPCOL;
const uint32 SS_ADDR_START = 5;
const uint32 SS_ADDR_MASK = SS_ADDR_CLAMP | SS_ADDR_WRAP | SS_ADDR_CLAMPCOL;

const uint32 RDISamplerStateMask = 0x00000fff;
const uint32 RDISamplerNumStates = RDISamplerStateMask + 1;

enum RDIFillMode
{
	RS_FILL_SOLID = 0,
	RS_FILL_WIREFRAME = 1
};

enum RDICullMode
{
	RS_CULL_BACK = 0,
	RS_CULL_FRONT,
	RS_CULL_NONE,
};

enum RDIBlendFunc
{
	BS_BLEND_ZERO = 0,
	BS_BLEND_ONE,
	BS_BLEND_SRC_ALPHA,
	BS_BLEND_INV_SRC_ALPHA,
	BS_BLEND_DEST_COLOR
};


enum RDIDepthFunc
{
	DSS_DEPTHFUNC_LESS_EQUAL = 0,
	DSS_DEPTHFUNC_LESS,
	DSS_DEPTHFUNC_EQUAL,
	DSS_DEPTHFUNC_GREATER,
	DSS_DEPTHFUNC_GREATER_EQUAL,
	DSS_DEPTHFUNC_ALWAYS
};

// ---------------------------------------------------------
// draw calls and clears
// ---------------------------------------------------------

enum RDIIndexFormat
{
#if defined(HORDE3D_GL) || defined(HORDE3D_GLES2)
	IDXFMT_16 = GL_UNSIGNED_SHORT,
	IDXFMT_32 = GL_UNSIGNED_INT
#elif defined(HORDE3D_D3D11)
	IDXFMT_16 = DXGI_FORMAT_R16_UINT,
	IDXFMT_32 = DXGI_FORMAT_R32_UINT
#else
	IDXFMT_16,
	IDXFMT_32
#endif
};

enum RDIPrimType
{
#if defined(HORDE3D_GL) || defined(HORDE3D_GLES2)
	PRIM_TRILIST = GL_TRIANGLES,
	PRIM_TRISTRIP = GL_TRIANGLE_STRIP
#elif defined(HORDE3D_D3D11)
	PRIM_TRILIST = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
	PRIM_TRISTRIP = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP
#else
	PRIM_TRILIST,
	PRIM_TRISTRIP
#endif
};


enum RDIClearFlags
{
	CLR_COLOR_RT0 = 0x00000001,
	CLR_COLOR_RT1 = 0x00000002,
	CLR_COLOR_RT2 = 0x00000004,
	CLR_COLOR_RT3 = 0x00000008,
	CLR_DEPTH = 0x00000010
};

// ---------------------------------------------------------
//	Common helper
// ---------------------------------------------------------

template< class T > class RDIObjects
{
public:

	uint32 add( const T &obj )
	{
		if( !_freeList.empty() )
		{
			uint32 index = _freeList.back();
			_freeList.pop_back();
			_objects[index] = obj;
			return index + 1;
		}
		else
		{
			_objects.push_back( obj );
			return (uint32)_objects.size();
		}
	}

	void remove( uint32 handle )
	{
		ASSERT( handle > 0 && handle <= _objects.size() );
		
		_objects[handle - 1] = T();  // Destruct and replace with default object
		_freeList.push_back( handle - 1 );
	}

	T &getRef( uint32 handle )
	{
		ASSERT( handle > 0 && handle <= _objects.size() );
		
		return _objects[handle - 1];
	}

	T *get( uint32 idx )
	{
		if ( idx < _objects.size() )
			return &_objects[idx];
		
		return NULL;
	}

	friend class RenderDevice;

private:
	std::vector< T >       _objects;
	std::vector< uint32 >  _freeList;
};


}


#ifdef HORDE3D_D3D11
#include "D3D11/egRendererBaseD3D11.h"
#elif HORDE3D_GLES2
#include "GLES2/egRendererBaseGLES2.h"
#elif HORDE3D_GL
#include "GL/egRendererBaseGL.h"
#endif


#endif // _egRendererBase_H_
