#include "utTexture.h"
#include "egModules.h"
#include "egCom.h"


namespace Horde3D {

uint32 calcTextureSize( utTextureFormats::List format, int width, int height, int depth )
{
	switch( format )
	{
	case utTextureFormats::RGBA8:
	case utTextureFormats::RGBX8:
	case utTextureFormats::BGRA8:
	case utTextureFormats::BGRX8:
		return width * height * depth * 4;
	case utTextureFormats::RGB8:
	case utTextureFormats::BGR8:
		return width * height * depth * 3;
	case utTextureFormats::DXT1:
		return std::max( width / 4, 1 ) * std::max( height / 4, 1 ) * depth * 8;
	case utTextureFormats::DXT3:
		return std::max( width / 4, 1 ) * std::max( height / 4, 1 ) * depth * 16;
	case utTextureFormats::DXT5:
		return std::max( width / 4, 1 ) * std::max( height / 4, 1 ) * depth * 16;
	case utTextureFormats::RGBA16F:
		return width * height * depth * 8;
	case utTextureFormats::RGBA32F:
		return width * height * depth * 16;
	case utTextureFormats::PVRTCI_2BPP:
	case utTextureFormats::PVRTCI_A2BPP:
		return (std::max( width, 16 ) * std::max( height, 8 ) * 2 + 7) / 8;
	case utTextureFormats::PVRTCI_4BPP:
	case utTextureFormats::PVRTCI_A4BPP:
		return (std::max( width, 8 ) * std::max( height, 8 ) * 4 + 7) / 8;
	case utTextureFormats::ETC1:
		return std::max( width / 4, 1 ) * std::max( height / 4, 1 ) * depth * 8;
	default:
		return 0;
	}
}


// =================================================================================================
// dds
// =================================================================================================

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


bool utTextureCheckDDS( const char *data, int size )
{
	return size > 128 && *((uint32 *)data) == FOURCC( 'D', 'D', 'S', ' ' );
}

bool utTextureLoadDDS( const char* data, uint32 size, utTextureInfo* info)
{
	ASSERT_STATIC( sizeof( DDSHeader ) == 128 );

	info->_surfaces = NULL;

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
	info->_format = utTextureFormats::Unknown;
	info->_mipCount = ddsHeader.dwFlags & DDSD_MIPMAPCOUNT ? ddsHeader.dwMipMapCount : 1;

	// Get texture type
	if( ddsHeader.caps.dwCaps2 == 0 )
	{
		info->_type = utTextureTypes::Tex2D;
	}
	else if( ddsHeader.caps.dwCaps2 & DDSCAPS2_CUBEMAP )
	{
		if( (ddsHeader.caps.dwCaps2 & DDSCAPS2_CM_COMPLETE) != DDSCAPS2_CM_COMPLETE )
			Modules::log().writeError( "DDS cubemap does not contain all cube sides" );
		info->_type = utTextureTypes::TexCube;
	}
	else if( ddsHeader.caps.dwCaps2 & DDSCAPS2_VOLUME )
	{
		info->_depth = ddsHeader.dwDepth;
		info->_type = utTextureTypes::Tex3D;
	}
	else
	{
		Modules::log().writeError( "Unsupported DDS texture type" );
		return false;
	}
	
	// Get pixel format
	if( ddsHeader.pixFormat.dwFlags & DDPF_FOURCC )
	{
		switch( ddsHeader.pixFormat.dwFourCC )
		{
		case FOURCC( 'D', 'X', 'T', '1' ):
			info->_format = utTextureFormats::DXT1;
			break;
		case FOURCC( 'D', 'X', 'T', '3' ):
			info->_format = utTextureFormats::DXT3;
			break;
		case FOURCC( 'D', 'X', 'T', '5' ):
			info->_format = utTextureFormats::DXT5;
			break;
		case D3DFMT_A16B16G16R16F: 
			info->_format = utTextureFormats::RGBA16F;
			break;
		case D3DFMT_A32B32G32R32F: 
			info->_format = utTextureFormats::RGBA32F;
			break;
		}
	}
	else if( ddsHeader.pixFormat.dwFlags & DDPF_RGB )
	{
		if( ddsHeader.pixFormat.dwRBitMask == 0x00ff0000 &&
		    ddsHeader.pixFormat.dwGBitMask == 0x0000ff00 &&
			ddsHeader.pixFormat.dwBBitMask == 0x000000ff ) info->_format = utTextureFormats::BGR8;
		else
		if( ddsHeader.pixFormat.dwRBitMask == 0x000000ff &&
		    ddsHeader.pixFormat.dwGBitMask == 0x0000ff00 &&
		    ddsHeader.pixFormat.dwBBitMask == 0x00ff0000 ) info->_format = utTextureFormats::RGB8;

		if( (info->_format == utTextureFormats::BGR8 || info->_format == utTextureFormats::RGB8) &&
			ddsHeader.pixFormat.dwRGBBitCount == 32)
		{
			if( !(ddsHeader.pixFormat.dwFlags & DDPF_ALPHAPIXELS) ||
			    ddsHeader.pixFormat.dwABitMask == 0x00000000 )
			{
				info->_format = (info->_format == utTextureFormats::BGR8) ? utTextureFormats::BGRX8 : utTextureFormats::RGBX8;
			}
			else
			{	
				info->_format = (info->_format == utTextureFormats::BGR8) ? utTextureFormats::BGRA8 : utTextureFormats::RGBA8;
			}
		}
	}

	if( info->_format == utTextureFormats::Unknown )
	{
		Modules::log().writeError( "Unsupported DDS pixel format" );
		return false;
	}

		// generate surface infos
	info->_sliceCount = info->_type == utTextureTypes::TexCube ? 6 : 1;
	unsigned char *pixels = (unsigned char *)(data + 128);

	uint32 surfaceIndex = 0;
	info->_surfaceCount = info->_sliceCount * info->_mipCount;
	info->_surfaces = new utTextureSurfaceInfo[info->_surfaceCount];

	for( uint32 i = 0; i < info->_sliceCount; ++i )
	{
		int width = info->_width, height = info->_height, depth = info->_depth;

		for( uint32 j = 0; j < info->_mipCount; ++j )
		{
			size_t mipSize =  calcTextureSize( info->_format, width, height, 1 );
			
			if( pixels + mipSize > (unsigned char *)data + size )
			{
				Modules::log().writeError( "Corrupt DDS" );
				return false;
			}

			utTextureSurfaceInfo& surface = info->_surfaces[surfaceIndex++];
			surface._mip = j;
			surface._slice = i;
			surface._data = pixels;
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

// =================================================================================================
// pvr
// =================================================================================================

struct PVRHeader
{
        uint32 dwVersion;
        uint32 dwFlags;
        uint32 dwPixelFormatLow;
		uint32 dwPixelFormatHigh;
        uint32 dwColourSpace;
        uint32 dwChannelType;
        uint32 dwHeight;
        uint32 dwWidth;
        uint32 dwDepth;
        uint32 dwNumSurfaces;
        uint32 dwNumFaces;
        uint32 dwMipMapCount;
        uint32 dwMetaDataSize;
} pvrHeader;

enum EPVRTPixelFormat
{
	ePVRTPF_PVRTCI_2bpp_RGB,
	ePVRTPF_PVRTCI_2bpp_RGBA,
	ePVRTPF_PVRTCI_4bpp_RGB,
	ePVRTPF_PVRTCI_4bpp_RGBA,
	ePVRTPF_PVRTCII_2bpp,
	ePVRTPF_PVRTCII_4bpp,
	ePVRTPF_ETC1,
	ePVRTPF_DXT1,
	ePVRTPF_DXT2,
	ePVRTPF_DXT3,
	ePVRTPF_DXT4,
	ePVRTPF_DXT5,

	//These formats are identical to some DXT formats.
	ePVRTPF_BC1 = ePVRTPF_DXT1,
	ePVRTPF_BC2 = ePVRTPF_DXT3,
	ePVRTPF_BC3 = ePVRTPF_DXT5,
};


bool utTextureCheckPVR( const char* data, uint32 size )
{
	return size > 52 && *((uint32 *)data) == FOURCC( 'P', 'V', 'R', 0x03 );
}

bool utTextureLoadPVR( const char* data, uint32 size, utTextureInfo* info)
{
	// Load pvr
    ASSERT_STATIC( sizeof( PVRHeader ) == 52 );

	info->_surfaces = NULL;

	memcpy( &pvrHeader, data, 52 );

    // Store properties
    info->_width = pvrHeader.dwWidth;
    info->_height = pvrHeader.dwHeight;
	info->_depth = 1;
    info->_format = utTextureFormats::Unknown;
	info->_mipCount = pvrHeader.dwMipMapCount;

    // Get texture type
    if( pvrHeader.dwNumFaces == 6 )
    {
		info->_type = utTextureTypes::TexCube;
    }
    else
    {
		info->_type = utTextureTypes::Tex2D;
    }

    // Get pixel format
	if (pvrHeader.dwPixelFormatHigh == 0 )
	{
		switch(pvrHeader.dwPixelFormatLow)
		{
		case ePVRTPF_PVRTCI_2bpp_RGBA:
			info->_format = utTextureFormats::PVRTCI_A2BPP;
			break;
		case ePVRTPF_PVRTCI_4bpp_RGBA:
			info->_format = utTextureFormats::PVRTCI_A4BPP;
			break;
		case ePVRTPF_DXT1:
			info->_format = utTextureFormats::DXT1;
			break;
		case ePVRTPF_DXT3:
			info->_format = utTextureFormats::DXT3;
			break;
		case ePVRTPF_DXT5:
			info->_format = utTextureFormats::DXT5;
			break;
		case ePVRTPF_PVRTCI_2bpp_RGB:
			info->_format = utTextureFormats::PVRTCI_2BPP;
			break;
		case ePVRTPF_PVRTCI_4bpp_RGB:
			info->_format = utTextureFormats::PVRTCI_4BPP;
			break;
		case ePVRTPF_ETC1:
			info->_format = utTextureFormats::ETC1;
			break;
		}
	}

	if( info->_format == utTextureFormats::Unknown )
	{
		Modules().log().writeError( "Unsupported PVR pixel format" );
		return false;
	}

    // Upload texture subresources
    info->_sliceCount = info->_type == utTextureTypes::TexCube ? 6 : 1;
    unsigned char *pixels = (unsigned char *)(data + 52);

	uint32 surfaceIndex = 0;
	info->_surfaceCount = info->_sliceCount * info->_mipCount;
	info->_surfaces = new utTextureSurfaceInfo[info->_surfaceCount];

    for( uint32 i = 0; i < info->_sliceCount; ++i )
    {
        int width = pvrHeader.dwWidth, height = pvrHeader.dwHeight;

        for( uint32 j = 0; j < info->_mipCount; ++j )
        {
			size_t mipSize =  calcTextureSize( info->_format, width, height, 1 );
            if( pixels + mipSize > (unsigned char *)data + size )
 			{
				Modules::log().writeError( "Corrupt PVR" );
				return false;
			}

			utTextureSurfaceInfo& surface = info->_surfaces[surfaceIndex++];
			surface._mip = j;
			surface._slice = i;
			surface._data = pixels;
			surface._size = (uint32)mipSize;

            pixels += mipSize;
            if( width > 1 ) width >>= 1;
            if( height > 1 ) height >>= 1;
        }
    }

    ASSERT( pixels == (unsigned char *)data + size);
	return true;
}

// =================================================================================================
// ktx
// =================================================================================================

const unsigned char ktxIdentifier[12] = {0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A};

struct KTXHeader
{
	unsigned char identifier[12];
	uint32 endianness;
    uint32 glType;
    uint32 glTypeSize;
    uint32 glFormat;
    uint32 glInternalFormat;
    uint32 glBaseInternalFormat;
    uint32 pixelWidth;
    uint32 pixelHeight;
    uint32 pixelDepth;
    uint32 numberOfArrayElements;
    uint32 numberOfFaces;
    uint32 numberOfMipmapLevels;
    uint32 bytesOfKeyValueData;
} ktxHeader;

#define GL_RGB8                             0x8051
#define GL_RGBA8                            0x8058 
#define GL_BGRA		                        0x80E1 
#define GL_RGBA16F		                    0x881A
#define GL_RGBA32F		                    0x8814
#define GL_COMPRESSED_RGB_S3TC_DXT1			0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1		0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3		0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5		0x83F3
#define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG                      0x8C00
#define GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG                      0x8C01
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG                     0x8C02
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG                     0x8C03
#define GL_ETC1_RGB8_OES                                        0x8D64 

bool utTextureCheckKTX( const char *data, int size )
{
	return size > 64 && memcmp(data, ktxIdentifier, 12)==0;
}

bool utTextureLoadKTX( const char *data, int size, utTextureInfo* info )
{
	ASSERT_STATIC( sizeof( KTXHeader ) == 64 );

	info->_surfaces = NULL;

	memcpy( &ktxHeader, data, 64 );

	// Check header
	if( memcmp(ktxHeader.identifier, ktxIdentifier, 12) !=0 )
	{
		Modules::log().writeError( "Invalid KTX header" );
		return false;
	}
	
	// Store properties
    info->_width = ktxHeader.pixelWidth;
    info->_height = ktxHeader.pixelHeight;
	info->_depth = 1;
    info->_format = utTextureFormats::Unknown;
	info->_mipCount = ktxHeader.numberOfMipmapLevels;

	// Get texture type
	if( ktxHeader.numberOfFaces > 1 )
	{
		if( ktxHeader.numberOfFaces != 6 )
		{
			Modules::log().writeError( "Wrong number of cube texture faces (should be 6)" );
			return false;
		}
		else
		{
			info->_type = utTextureTypes::TexCube;
		}
	}
	else if( ktxHeader.pixelDepth > 1 )
	{
		info->_depth = ktxHeader.pixelDepth;
		info->_type = utTextureTypes::Tex3D;
	}
	else
		info->_type = utTextureTypes::Tex2D;
	
	// Texture arrays are not supported yet
	if( ktxHeader.numberOfArrayElements > 1 )
	{
		Modules::log().writeWarning( "Texture Arrays not supported. using first array element only" );
	}

	// Get pixel format
	switch( ktxHeader.glInternalFormat )
	{
		case GL_RGB8:
			info->_format = utTextureFormats::RGB8;
			break;
		case GL_RGBA8:
			info->_format = utTextureFormats::BGRA8;
			break;
		case GL_BGRA:
			info->_format = utTextureFormats::BGRA8;
			break;
		case GL_RGBA16F:
			info->_format = utTextureFormats::RGBA16F;
			break;
		case GL_RGBA32F:
			info->_format = utTextureFormats::RGBA32F;
			break;
		case GL_COMPRESSED_RGBA_S3TC_DXT1:
			info->_format = utTextureFormats::DXT1;
			break;
		case GL_COMPRESSED_RGBA_S3TC_DXT3:
			info->_format = utTextureFormats::DXT3;
			break;
		case GL_COMPRESSED_RGBA_S3TC_DXT5:
			info->_format = utTextureFormats::DXT5;
			break;
		case GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG:
			info->_format = utTextureFormats::PVRTCI_2BPP;
			break;
		case GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG:
			info->_format = utTextureFormats::PVRTCI_A2BPP;
			break;
		case GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG:
			info->_format = utTextureFormats::PVRTCI_4BPP;
			break;
		case GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG:
			info->_format = utTextureFormats::PVRTCI_A4BPP;
			break;
		case GL_ETC1_RGB8_OES:
			info->_format = utTextureFormats::ETC1;
			break;
	}
	
	if( info->_format == utTextureFormats::Unknown )
	{
		Modules::log().writeError( "Unsupported KTX pixel format" );
		return false;
	}

	info->_sliceCount = info->_type == utTextureTypes::TexCube ? 6 : 1;
	unsigned char *pixels = (unsigned char *)(data + 128);

	uint32 surfaceIndex = 0;
	info->_surfaceCount = info->_sliceCount * info->_mipCount;
	info->_surfaces = new utTextureSurfaceInfo[info->_surfaceCount];

	int width = info->_width, height = info->_height, depth = info->_depth;

	for( uint32 i = 0; i < info->_mipCount; ++i )
	{
		size_t mipSize =  calcTextureSize( info->_format, width, height, depth );
		if( pixels + mipSize > (unsigned char *)data + size )
		{
			Modules::log().writeError( "Corrupt KTX" );
			return false;
		}
		
		for( uint32 j = 0; j < ktxHeader.numberOfArrayElements; ++j )
		{
			for( uint32 k = 0; k < ktxHeader.numberOfFaces; ++k )
			{
				if ( j == 0)
				{	// using only first element of array now
					utTextureSurfaceInfo& surface = info->_surfaces[surfaceIndex++];
					surface._mip = i;
					surface._slice = k;
					surface._data = pixels;
					surface._size = (uint32)mipSize;
				}
					
				pixels += mipSize;
			}
		}
		if( width > 1 ) width >>= 1;
		if( height > 1 ) height >>= 1;
		if( depth > 1 ) depth >>= 1;
	}

	ASSERT( pixels == (unsigned char *)data + size );
	return true;
}

bool utTextureCheck( const char* data, uint32 size )
{
	return utTextureCheckDDS( data, size ) || 
			utTextureCheckPVR( data, size ) ||
			utTextureCheckKTX( data, size );
}

bool utTextureLoad( const char* data, uint32 size, utTextureInfo* info)
{
	if ( utTextureCheckDDS( data, size ) )
		return utTextureLoadDDS( data, size, info );
	else if ( utTextureCheckPVR( data, size ) )
		return utTextureLoadPVR( data, size, info );
	else if ( utTextureCheckKTX( data, size ) )
		return utTextureLoadKTX( data, size, info );
	
	return false;
}

void utTextureFree( utTextureInfo* info )
{
	delete[] info->_surfaces;
}


}