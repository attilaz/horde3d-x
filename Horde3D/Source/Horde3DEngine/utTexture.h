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

#ifndef _utTexture_H_
#define _utTexture_H_

namespace Horde3D {

struct utTextureTypes
{
	enum List
	{
		Tex2D,
		Tex3D,
		TexCube,
	};
};

struct utTextureFormats
{
	enum List
	{
		Unknown,
		RGBA8,
		RGB8,
		RGBX8,
		BGRA8,
		BGR8,
		BGRX8,

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

		Count,
	};
};

struct utTextureSurfaceInfo
{
	unsigned int	_mip;
	unsigned int	_slice;	// cubemap slice
	void*			_data;
	unsigned int	_size;	// in bytes
};

struct utTextureInfo
{
	unsigned int _width,_height,_depth;
	unsigned int _mipCount;
	unsigned int _sliceCount;

	utTextureTypes::List	_type;
	utTextureFormats::List	_format;

	utTextureSurfaceInfo*	_surfaces;	
	unsigned int			_surfaceCount;
};

bool utTextureCheck( const char* data, unsigned int	size );	//returns true if fileformat is supported
bool utTextureLoad( const char* data, unsigned int size, utTextureInfo* info );
void utTextureFree( utTextureInfo* info );

}

#endif	//_utTexture_H_
