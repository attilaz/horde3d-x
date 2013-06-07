#include "utTexture.h"
#include "egModules.h"
#include "egCom.h"


namespace Horde3D {

bool utexLoad(char* buffer, uint32 len, TextureInfo* info);


	// dds 
#define FOURCC( c0, c1, c2, c3 ) ((c0) | (c1<<8) | (c2<<16) | (c3<<24))

#define DDSD_MIPMAPCOUNT      0x00020000

#define DDPF_ALPHAPIXELS      0x00000001
#define DDPF_FOURCC           0x00000004
#define DDPF_RGB              0x00000040

#define DDSCAPS2_CUBEMAP      0x00000200
#define DDSCAPS2_CM_COMPLETE  (0x00000400 | 0x00000800 | 0x00001000 | 0x00002000 | 0x00004000 | 0x00008000)
#define DDSCAPS2_VOLUME       0x00200000

#define D3DFMT_A16B16G16R16F  113
#define D3DFMT_A32B32G32R32F  116

struct DDSHeader
{
	uint32  dwMagic;
	uint32  dwSize;
	uint32  dwFlags;
	uint32  dwHeight, dwWidth;
	uint32  dwPitchOrLinearSize;
	uint32  dwDepth;
	uint32  dwMipMapCount;
	uint32  dwReserved1[11];

	struct {
		uint32  dwSize;
		uint32  dwFlags;
		uint32  dwFourCC;
		uint32  dwRGBBitCount;
		uint32  dwRBitMask, dwGBitMask, dwBBitMask, dwABitMask;
	} pixFormat;

	struct {
		uint32  dwCaps, dwCaps2, dwCaps3, dwCaps4;
	} caps;

	uint32  dwReserved2;
} ddsHeader;


bool utexCheckDDS( const char *data, int size )
{
	return size > 128 && *((uint32 *)data) == FOURCC( 'D', 'D', 'S', ' ' );
}

bool utexLoadDDS( const char* data, uint32 size, TextureInfo* info)
{
	ASSERT_STATIC( sizeof( DDSHeader ) == 128 );

	memcpy( &ddsHeader, data, 128 );
	
	// Check header
	// There are some flags that are required to be set for every dds but we don't check them
	if( ddsHeader.dwSize != 124 )
	{
		Modules::log().writeError( "Invalid DDS header" );
		return false;
	}

	// Store properties
	info->_width = ddsHeader.dwWidth;
	info->_height = ddsHeader.dwHeight;
	info->_depth = 1;
	info->_format = TextureFormats::Unknown;
	info->_mipCount = ddsHeader.dwFlags & DDSD_MIPMAPCOUNT ? ddsHeader.dwMipMapCount : 1;
	bool hasMipMaps = info->_mipCount > 1 ? true : false;

	// Get texture type
	if( ddsHeader.caps.dwCaps2 == 0 )
	{
		info->_type = TextureTypes::Tex2D;
	}
	else if( ddsHeader.caps.dwCaps2 & DDSCAPS2_CUBEMAP )
	{
		if( (ddsHeader.caps.dwCaps2 & DDSCAPS2_CM_COMPLETE) != DDSCAPS2_CM_COMPLETE )
			Modules::log().writeError( "DDS cubemap does not contain all cube sides" );
		info->_type = TextureTypes::TexCube;
	}
	else if( ddsHeader.caps.dwCaps2 & DDSCAPS2_VOLUME )
	{
		info->_depth = ddsHeader.dwDepth;
		info->_type = TextureTypes::Tex3D;
	}
	else
	{
		Modules::log().writeError( "Unsupported DDS texture type" );
		return false;
	}
	
	// Get pixel format
	int blockSize = 1, bytesPerBlock = 4;
	enum { pfBGRA, pfBGR, pfBGRX, pfRGB, pfRGBX, pfRGBA } pixFmt = pfBGRA;
	
	if( ddsHeader.pixFormat.dwFlags & DDPF_FOURCC )
	{
		switch( ddsHeader.pixFormat.dwFourCC )
		{
		case FOURCC( 'D', 'X', 'T', '1' ):
			info->_format = TextureFormats::DXT1;
			blockSize = 4; bytesPerBlock = 8;
			break;
		case FOURCC( 'D', 'X', 'T', '3' ):
			info->_format = TextureFormats::DXT3;
			blockSize = 4; bytesPerBlock = 16;
			break;
		case FOURCC( 'D', 'X', 'T', '5' ):
			info->_format = TextureFormats::DXT5;
			blockSize = 4; bytesPerBlock = 16;
			break;
		case D3DFMT_A16B16G16R16F: 
			info->_format = TextureFormats::RGBA16F;
			bytesPerBlock = 8;
			break;
		case D3DFMT_A32B32G32R32F: 
			info->_format = TextureFormats::RGBA32F;
			bytesPerBlock = 16;
			break;
		}
	}
	else if( ddsHeader.pixFormat.dwFlags & DDPF_RGB )
	{
		bytesPerBlock = ddsHeader.pixFormat.dwRGBBitCount / 8;
		
		if( ddsHeader.pixFormat.dwRBitMask == 0x00ff0000 &&
		    ddsHeader.pixFormat.dwGBitMask == 0x0000ff00 &&
		    ddsHeader.pixFormat.dwBBitMask == 0x000000ff ) pixFmt = pfBGR;
		else
		if( ddsHeader.pixFormat.dwRBitMask == 0x000000ff &&
		    ddsHeader.pixFormat.dwGBitMask == 0x0000ff00 &&
		    ddsHeader.pixFormat.dwBBitMask == 0x00ff0000 ) pixFmt = pfRGB;

		if( pixFmt == pfBGR || pixFmt == pfRGB )
		{
			if( ddsHeader.pixFormat.dwRGBBitCount == 24 )
			{
				info->_format = TextureFormats::RGBA8;
			}
			else if( ddsHeader.pixFormat.dwRGBBitCount == 32 )
			{
				if( !(ddsHeader.pixFormat.dwFlags & DDPF_ALPHAPIXELS) ||
				    ddsHeader.pixFormat.dwABitMask == 0x00000000 )
				{
					info->_format = TextureFormats::RGBA8;
					pixFmt = pixFmt == pfBGR ? pfBGRX : pfRGBX;
				}
				else
				{	
					info->_format = TextureFormats::RGBA8;
					pixFmt = pixFmt == pfBGR ? pfBGRA : pfRGBA;
				}
			}
		}
	}

	if( info->_format == TextureFormats::Unknown )
	{
		Modules::log().writeError( "Unsupported DDS pixel format" );
		return false;
	}

	// Upload texture subresources
	info->_sliceCount = info->_type == TextureTypes::TexCube ? 6 : 1;
	unsigned char *pixels = (unsigned char *)(data + 128);

	uint32 surfaceIndex = 0;
	info->_surfaceCount = info->_sliceCount * info->_mipCount;
	info->_surfaces = new SurfaceInfo[info->_surfaceCount];

	for( uint32 i = 0; i < info->_sliceCount; ++i )
	{
		int width = info->_width, height = info->_height, depth = info->_depth;

		for( uint32 j = 0; j < info->_mipCount; ++j )
		{
			size_t mipSize = std::max( width / blockSize, 1 ) * std::max( height / blockSize, 1 ) *
			                 depth * bytesPerBlock;
			uint32* dstBuf = 0x0;
			
			if( pixels + mipSize > (unsigned char *)data + size )
			{
				Modules::log().writeError( "Corrupt DDS" );
				return false;

			}

			if( info->_format == TextureFormats::RGBA8 && pixFmt != pfRGBA )
			{
				// Convert 8 bit DDS formats to RGBA
				uint32 pixCount = width * height * depth;
				dstBuf = new uint32[pixCount];
				uint32 *p = dstBuf;

				if( pixFmt == pfRGB )
					for( uint32 k = 0; k < pixCount * 3; k += 3 )
						*p++ = pixels[k+0] | pixels[k+1]<<8 | pixels[k+2]<<16 | 0xFF000000;
				else if( pixFmt == pfRGBX )
					for( uint32 k = 0; k < pixCount * 4; k += 4 )
						*p++ = pixels[k+0] | pixels[k+1]<<8 | pixels[k+2]<<16 | 0xFF000000;
				else if( pixFmt == pfBGR )
					for( uint32 k = 0; k < pixCount * 3; k += 3 )
						*p++ = pixels[k+2] | pixels[k+1]<<8 | pixels[k+0]<<16 | 0xFF000000;
				else if( pixFmt == pfBGRX )
					for( uint32 k = 0; k < pixCount * 4; k += 4 )
						*p++ = pixels[k+2] | pixels[k+1]<<8 | pixels[k+0]<<16 | 0xFF000000;
				else if( pixFmt == pfBGRA )
					for( uint32 k = 0; k < pixCount * 4; k += 4 )
						*p++ = pixels[k+2] | pixels[k+1]<<8 | pixels[k+0]<<16 | pixels[k+3]<<24;
			}

			SurfaceInfo& surface = info->_surfaces[surfaceIndex++];
			surface._mip = j;
			surface._slice = i;
			surface._data = dstBuf!=0x0 ? (void*)dstBuf : (void*)pixels;
			surface._deleteData = dstBuf!=0x0;
			surface._size = (uint32)mipSize;

			pixels += mipSize;
			if( width > 1 ) width >>= 1;
			if( height > 1 ) height >>= 1;
			if( depth > 1 ) depth >>= 1;
		}
	}

	ASSERT( pixels == (unsigned char *)data + size );

	return true;
}

bool utexCheck( const char* buffer, uint32 len )
{
	return utexCheckDDS( buffer, len );
}

bool utexLoad( const char* buffer, uint32 len, TextureInfo* info)
{
	if ( utexCheckDDS( buffer, len ) )
		return utexLoadDDS( buffer, len, info );
	
	return false;
}

void utexFree( TextureInfo* info )
{
	for( uint32 i=0; i< info->_surfaceCount; ++i)
		if (info->_surfaces[i]._deleteData)
			delete[] info->_surfaces[i]._data;
	delete[] info->_surfaces;
}


}