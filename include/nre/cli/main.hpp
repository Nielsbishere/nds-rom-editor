#pragma once
#include "format/nds.hpp"

namespace oic { class FileSystem; }

struct EFlag { 

	static constexpr u64 
		infoBasic			= 1 << 0,
		infoLocations		= 1 << 1,
		info				= infoBasic | infoLocations,
		exportIcon			= 1 << 2,
		exportIconPalette	= 1 << 3,
		exportIconTilemap	= 1 << 4,
		exportArm9Bin		= 1 << 5,
		exportArm7Bin		= 1 << 6,
		exportArm9Overlay	= 1 << 7,
		exportArm7Overlay	= 1 << 8,
		exportDebug			= 1 << 9,
		exportArm9			= exportArm9Bin | exportArm9Overlay,
		exportArm7			= exportArm7Bin | exportArm7Overlay,
		exportCode			= exportArm9 | exportArm7,
		exportFiles			= 1 << 10,
		infoFiles			= 1 << 11,
		infoFolders			= 1 << 12;

};

//A routine that is called if the flag is set
using FlagRoutine = int (*)(const String&, nre::NDS*, oic::FileSystem*);

//The data of a cli flag
struct Flag {
	u64 value;
	String name, desc;
	FlagRoutine routine;
};

//All functions for flags

int infoBasics(const String&, nre::NDS*, oic::FileSystem*);
int infoLocations(const String&, nre::NDS*, oic::FileSystem*);
int exportIcon(const String&, nre::NDS*, oic::FileSystem*);
int exportIconPalette(const String&, nre::NDS*, oic::FileSystem*);
int exportIconTilemap(const String&, nre::NDS*, oic::FileSystem*);
int exportArm9Bin(const String&, nre::NDS*, oic::FileSystem*);
int exportArm7Bin(const String&, nre::NDS*, oic::FileSystem*);
int exportArm9Overlay(const String&, nre::NDS*, oic::FileSystem*);
int exportArm7Overlay(const String&, nre::NDS*, oic::FileSystem*);
int exportDebug(const String&, nre::NDS*, oic::FileSystem*);
int exportFiles(const String&, nre::NDS*, oic::FileSystem*);
int infoFiles(const String&, nre::NDS*, oic::FileSystem*);
int infoFolders(const String&, nre::NDS*, oic::FileSystem*);

//All flags
const std::initializer_list<Flag> flags {

	Flag{
		EFlag::infoBasic,
		"info-basic",
		"Displays the title data and important keys and checksums",
		infoBasics
	},

	Flag{
		EFlag::infoLocations,
		"info-locations",
		"Displays the important memory locations",
		infoLocations
	},

	Flag{
		EFlag::info,
		"info",
		"Shortcut for -info-basic -info-locations",
		nullptr
	},

	Flag{
		EFlag::exportIcon,
		"export-icon",
		"Exports the converted rom icon (./rom.nds -> ./rom/icon.png)",
		exportIcon
	},

	Flag{
		EFlag::exportIconPalette,
		"export-icon-palette",
		"Exports the rom icon palette (./rom.nds -> ./rom/icon_palette.png)",
		exportIconPalette
	},

	Flag{
		EFlag::exportIconTilemap,
		"export-icon-tilemap",
		"Exports the rom icon's tilemap (which palette index for each pixel) (./rom.nds -> ./rom/icon_palette_tilemap.png)",
		exportIconTilemap
	},

	Flag{
		EFlag::exportArm9Bin,
		"export-arm9-bin",
		"Exports the arm9 binary from the rom (./rom.nds -> ./rom/arm9.bin)",
		exportArm9Bin
	},

	Flag{
		EFlag::exportArm7Bin,
		"export-arm7-bin",
		"Exports the arm7 binary from the rom (./rom.nds -> ./rom/arm7.bin)",
		exportArm7Bin
	},

	Flag{
		EFlag::exportArm9Overlay,
		"export-arm9-overlay",
		"Exports the arm9 overlay from the rom (./rom.nds -> ./rom/arm9_overlay.bin)",
		exportArm9Overlay
	},

	Flag{
		EFlag::exportArm7Overlay,
		"export-arm7-overlay",
		"Exports the arm7 overlay from the rom (./rom.nds -> ./rom/arm7_overlay.bin)",
		exportArm7Overlay
	},

	Flag{
		EFlag::exportDebug,
		"export-debug",
		"Exports the debug rom (./rom.nds -> ./rom/debug.bin)",
		exportDebug
	},

	Flag{
		EFlag::exportArm9,
		"export-arm9",
		"Shortcut for -export-arm9-bin -export-arm9-overlay",
		nullptr
	},

	Flag{
		EFlag::exportArm7,
		"export-arm7",
		"Shortcut for -export-arm7-bin -export-arm7-overlay",
		nullptr
	},

	Flag{
		EFlag::exportCode,
		"export-code",
		"Shortcut for -export-arm9 -export-arm7",
		nullptr
	},

	Flag{
		EFlag::exportFiles,
		"export-files",
		"Exports all files from the rom to inputRomPath/romName",
		exportFiles
	},

	Flag{
		EFlag::infoFiles,
		"info-files",
		"Shows a list of all files from the rom and the locations",
		infoFiles
	},

	Flag{
		EFlag::infoFolders,
		"info-folders",
		"Shows a list of all folders from the rom",
		infoFolders
	}

};