#ifndef _egRendererBase_H_
#define _egRendererBase_H_

namespace Horde3D {

// ---------------------------------------------------------
// General
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

	friend class RenderDevice;

private:
	std::vector< T >       _objects;
	std::vector< uint32 >  _freeList;
};


struct DeviceCaps
{
	bool  texFloat;
	bool  texNPOT;
	bool  rtMultisampling;
};

struct TextureTypes
{
	enum List
	{
		Tex2D,
		Tex3D,
		TexCube
	};
};

struct TextureFormats
{
	enum List
	{
		Unknown,
		BGRA8,
		DXT1,
		DXT3,
		DXT5,
		RGBA16F,
		RGBA32F,
		DEPTH
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



}


#ifdef HORDE3D_D3D11
#include "D3D11/egRendererBase.h"
#elif HORDE3D_GLES2
#include "GLES2/egRendererBase.h"
#else
#include "GL/egRendererBase.h"
#endif


#endif // _egRendererBase_H_
