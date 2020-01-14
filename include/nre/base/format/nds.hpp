#pragma once
#include <system/log.hpp>
#include <system/file_system.hpp>

namespace nre {

	//NDS file format
	struct NDS {

		c8 title[12];
		c8 gameCode[4];
		c8 makerCode[2];

		u8 unitCode;
		u8 encryptionSeed;
		u8 capacity;
		u8 reserved[7];
		u8 unknown[2];			//Used by DSi titles
		u8 version;
		u8 flags;				//Flags such as auto start (Bit2)

		u32 arm9Offset;			//Rom offset for ARM9
		u32 arm9Entry;			//Entry address for ARM9
		u32 arm9Load;			//Load address for ARM9
		u32 arm9Size;			//Size for ARM9 binary

		u32 arm7Offset;			//Rom offset for ARM7
		u32 arm7Entry;			//Entry address for ARM7
		u32 arm7Load;			//Load address for ARM7
		u32 arm7Size;			//Size for ARM7

		u32 fntOffset;			//File name table offset
		u32 fntSize;			//File name table length

		u32 fatOffset;			//File allocation table offset
		u32 fatSize;			//File allocation table length

		u32 arm9OverlayOffset;	//Overlay offset for ARM9
		u32 arm9OverlaySize;	//Overlay length for ARM9
		u32 arm7OverlayOffset;	//Overlay offset for ARM7
		u32 arm7OverlaySize;	//Overlay length for ARM7

		u32 cardControl;		//Normal card control register settings
		u32 sCardControl;		//Safe card control register settings

		u32 bannerOffset;		//See NDSBanner

		u16 sAC;				//Secure area checksum
		u16 sALT;				//Secure area loading timeout

		u32 arm9ALLRA;			//Auto load list RAM address
		u32 arm7ALLRA;			//Auto load list RAM address

		u64 sAD;				//Secure area disable

		u32 romSize;
		u32 romHeaderSize;

		u8 reserved1[56];

		u8 nLogo[156];

		u16 nLC;				//Logo Checksum
		u16 nHC;				//Header Checksum

		u32 dRomOff;			//Debug rom offset
		u32 dRomSize;			//Debug rom size

		u32 reserved2;
		u8 reserved3[144];

		//Only get NDS ROM if the header is valid

		static NDS *get(u8 *romPtr, usz romSize);
		bool invalid() const;
	};

	//Implementation of oic FileSystem to support NDS
	class NDSFileSystem : public oic::FileSystem {

	public:

		NDSFileSystem(NDS*) noexcept(false);

		bool read(const oic::FileInfo &file, void *address, usz size, usz offset) const final override;
		bool write(oic::FileInfo &file, const Buffer &buffer, usz size = 0, usz bufferOffset = 0, usz fileOffset = 0) final override;
		void initFiles() final override {}
	};

	//Array of NDSFolder located at fntOffset
	struct NDSFolder {
		u32 offset;
		u16 firstFilePosition;
		u16 relation;
	};

	//A representation of a NDSFile
	struct NDSFile {

		const c8 *name;

		u32 beg;
		u32 size;

		u16 files, folders;
		u16 parent; u8 nameLen; bool isFolder;

	};

	//A banner located at NDS::iconOffset
	//Contains the game screen titles and icon
	struct NDSBanner {

		static inline NDSBanner *get(NDS *nds) {
			oicAssert("NDS* is required", nds);
			return (NDSBanner*)((u8*)nds + nds->bannerOffset);
		}

		//Header

		u16 version;
		u16 checksum;
		u8 reserved[28];

		//BGR5 4-bit encoded images

		u8 icon[32 * 32 / 2];			//NDS icon data 4-bit 32x32 (palette[i])
		u16 palette[16];				//NDS icon palette (BGR5)
		
		//Not every game has titles for every language
		//If it doesn't, the title at the index will be empty
		enum Language : u8 {
			JAPANESE,
			ENGLISH,
			FRENCH,
			GERMAN,
			ITALIAN,
			SPANISH,
			LANGUAGE_START = JAPANESE,
			LANGUAGE_END = SPANISH + 1
		};

		c16 titles[LANGUAGE_END][128];		//Title of game in 6 languages (unicode)

		inline bool hasTitle(Language lang) const { return titles[lang][0]; }

		inline WString getTitle(Language lang = ENGLISH) const { 
			WString str = WString(titles[lang]);
			std::replace(str.begin(), str.end(), L'\n', L' ');
			return str;
		}
	};
}