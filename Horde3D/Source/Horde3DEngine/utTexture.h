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

#include "egRendererBase.h"

namespace Horde3D {

struct SurfaceInfo
{
	uint32	_mip;
	uint32	_slice;	// cubemap slice
	void*	_data;
	uint32	_size;	// in bytes
	bool	_deleteData;
};

struct TextureInfo
{
	uint32 _width,_height,_depth;
	uint32 _mipCount;
	uint32 _sliceCount;

	TextureTypes::List		_type;
	TextureFormats::List	_format;

	SurfaceInfo*	_surfaces;	
	uint32			_surfaceCount;
};

bool utexCheck( const char* data, uint32 size );	//returns true if format is supported
bool utexLoad( const char* data, uint32 size, TextureInfo* info );
void utexFree( TextureInfo* info );

}

#endif	//_utTexture_H_
