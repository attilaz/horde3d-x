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

#include "egRendererBase.h"

namespace Horde3D {

uint32 calcTextureSize( TextureFormats::List format, int width, int height, int depth )
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


}  // namespace
