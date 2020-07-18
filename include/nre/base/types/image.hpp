#pragma once
#include "generic_resource.hpp"
#include "helper/color.hpp"

namespace nre {

	//Palette data

	struct TTLP : GenericSection<SECTION_TTLP, bgr5> {
		u32 bitDepth;						//3 = 4 bits, 4 = 8 bits
		u32 padding;						//0x00000000
		u32 dataSize;						//size of palette data in bytes; if(size > 0x200) size = 0x200 - size;
		u32 colors;							//0x00000010
	};

	using PaletteId = u16;

	struct PMCP : GenericSection<SECTION_PMCP, PaletteId> {
		u16 count;							//Count of palettes in file
		u16 constant0;						//0x00EF
		u32 constant1;						//0xBE080000
	};

	//Palette ("Color") resource
	typedef GenericResource<RESOURCE_NCLR, TTLP, PMCP> NCLR;

	//The contents of RAHC can be "encrypted";
	//This means that the image needs to be XORed with a magic texture
	//u32 seed = texture[end()];
	//for(i32 i = end(); i >= begin(); --i) { magic[i] = seed; seed = CompressionHelper::generateRandom(seed); }
	struct RAHC : GenericSection<SECTION_RAHC, r4_8> {
		u16 tileHeight;
		u16 tileWidth;
		u16 tileDepth;						//1 << (tileDepth - 1) = bit depth
		u16 unknown0;						//0x0A or 0x00
		u16 unknown1;						//0x10 or 0x00 might be a size hint
		u16 unknown2;						//0x10 or 0x00 might be a size hint
		u8 isEncrypted;						//Only saw this affect encryption and linear image
		u8 specialTiling;					//Seems to be set when images uses different tiling
		u16 padding;						//0x0000
		u32 tileDataSize;					//tileDataSize / 1024 = tileCount; tileDataSize * (2 - (tileDepth - 3)) = pixels
		u32 unknown3;						//0x00000018
	};

	struct SOPC : GenericSection<SECTION_SOPC, void> {
		u32 c_padding;						//0x00000000
		u16 tileWidth;						//= tileCount in RAHC
		u16 tileHeight;						//= RAHC tileCount
	};

	typedef GenericResource<RESOURCE_NCGR, false, RAHC, SOPC> NCGR;				//Graphics resource

	struct NRCS : GenericSection {												//Screen resource
		u16 screenWidth;					//Width of screen (pixels)
		u16 screenHeight;					//Height of screen (pixels)
		u32 c_padding;						//unknown
		u32 screenDataSize;					//Size of screen data buffer
	};

	typedef GenericResource<0x4E534352, false, NRCS> NSCR;	

}