#include "main.hpp"
#include "helper/color.hpp"
#include <system/local_file_system.hpp>
#include <iostream>
#include <codecvt>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

using namespace oic;
using namespace nre;

//Processing invalid flags and valid flags

inline int help() {

	using namespace std;

	cout << "Invalid usage of command" << endl;
	cout << "Use the ROM paths relative to the working directory (./) in combination with one of the following:" << endl;

	for (auto &flag : flags)
		cout << '-' << flag.name << ' ' << flag.desc << endl;

	return 1;
}

void setupConsole();

int main(int argc, char *argv[]) {

	setupConsole();

	if (argc == 1)
		return help();

	List<String> paths;
	u64 flagValue{};

	for (int i = 1; i < argc; ++i) {

		if (argv[i][0] == '-') {

			bool isFlag{};

			for (auto &flag : flags)
				if (flag.name == String(argv[i]).substr(1)) {
					flagValue |= flag.value;
					isFlag = true;
					break;
				}

			if (!isFlag)
				return help();

		} else {

			String path = argv[i];

			if (!System::files()->regionExists(path, 1, 0))
				return help();

			paths.push_back(path);
		}
	}

	for (const String &str : paths) {

		using namespace std;

		Buffer rom;
		if (!System::files()->read(str, rom)) {
			cout << "WARNING: Couldn't read ROM at path \"" << str << "\"" << endl;
			continue;
		}

		u8* const romPtr = rom.data();
		const usz romSize = rom.size();

		NDS *nds = NDS::get(romPtr, romSize);

		if(!nds) {
			cout << "WARNING: File at \"" << str << "\" is not a valid NDS file" << endl;
			continue;
		}

		try {

			NDSFileSystem fs(nds);

			cout << "-------\t" << str << "\t--------" << endl;

			for (auto &flag : flags)
				if (auto *routine = flag.routine)
					if ((flagValue & flag.value) == flag.value)
						if (routine(str, nds, &fs)) {
							cout << "WARNING: File at \"" << str << "\" had a flag routine interrupt the execution process" << endl;
							continue;
						}

			cout << endl;

		} catch (std::runtime_error e) {
			cout << "WARNING: File at \"" << str << "\" doesn't have a valid file system" << endl;
			cout << e.what() << endl;
		}
	}

	return 0;
}

inline String toUTF8(const WString &wstr) {
	using namespace std;
	return wstring_convert<codecvt_utf8<wchar_t>>().to_bytes(wstr);
}

//Helper functions for adding files and encoding data

inline int makeFile(const String &path, String &file, bool isFolder = false) {

	using namespace std;

	const String &base = path.substr(0, path.find_last_of('.'));
	file = base + "/" + file;

	if (!System::files()->add(file, isFolder)) {
		cout << "ERROR: Couldn't add subdir \"" << base << "\"" << endl;
		return 1;
	}

	return 0;
}

inline Buffer encodePng(const List<r8> &colors, u16 w, u16 h) {
	int len{};
	u8 *dat = stbi_write_png_to_mem(colors.data(), 0, w, h, 1, &len);
	Buffer res(dat, dat + len);
	free(dat);
	return res;
}

inline Buffer encodePng(const List<rgba8> &colors, u16 w, u16 h) {
	int len{};
	u8 *dat = stbi_write_png_to_mem((u8*)colors.data(), 0, w, h, 4, &len);
	Buffer res(dat, dat + len);
	free(dat);
	return res;
}

//Implementations of flags

int infoBasics(const String&, NDS *nds, FileSystem*) {

	using namespace std;

	cout
		<< "-------\tROM header base\t--------" << endl
		<< "Game title: " << nds->title << endl
		<< "Game code: " << String(nds->gameCode, nds->gameCode + 4) << endl
		<< "Maker code: " << String(nds->makerCode, nds->makerCode + 2) << endl
		<< "Version: " << u32(nds->version) << endl
		<< "Unit code: " << u32(nds->unitCode) << endl
		<< "Localized names: " << endl;

	NDSBanner *banner = NDSBanner::get(nds);

	const char *languages[] = {
		"Japanese",
		"English",
		"French",
		"German",
		"Italian",
		"Spanish",
	};

	for(u8 l = NDSBanner::LANGUAGE_START; l != NDSBanner::LANGUAGE_END; ++l)
		if (banner->hasTitle(NDSBanner::Language(l)))
			cout << "\t" << languages[l] << ": " << toUTF8(banner->getTitle(NDSBanner::Language(l))) << endl;

	cout << endl;

	cout
		<< "-------\tROM header advanced\t--------" << endl
		<< "Encryption seed: " << u32(nds->encryptionSeed) << endl
		<< "Capacity: " << u32(nds->capacity) << endl
		<< "Card control: 0x" << Log::num<16>(nds->cardControl) << endl
		<< "Secure card control: 0x" << Log::num<16>(nds->sCardControl) << endl
		<< "Secure area checksum: 0x" << Log::num<16>(nds->sAC) << endl
		<< "Secure area loading timeout: 0x" << Log::num<16>(nds->sALT) << endl
		<< "Logo checksum: 0x" << Log::num<16>(nds->nLC) << endl
		<< "Header checksum: 0x" << Log::num<16>(nds->nHC) << endl
		<< endl;

	return 0;
}

int infoLocations(const String&, NDS *nds, FileSystem*) {

	using namespace std;

	cout
		<< "-------\tROM header locations\t--------" << endl

		<< "ARM9 Binary: [0x" << Log::num<16>(nds->arm9Load) << ", 0x" << Log::num<16>(nds->arm9Load + nds->arm9Size) << ">" << endl
		<< "ARM9 Entry: 0x" << Log::num<16>(nds->arm9Entry) << endl
		<< "ARM9 ALLRA: 0x" << Log::num<16>(nds->arm9ALLRA) << endl
		<<  "ARM9 Overlay: [0x" 
			<< Log::num<16>(nds->arm9OverlayOffset) << ", 0x"
			<< Log::num<16>(nds->arm9OverlayOffset + nds->arm9OverlaySize) << ">" << endl

		<< "ARM7 Binary: [0x" << Log::num<16>(nds->arm7Load) << ", 0x" << Log::num<16>(nds->arm7Load + nds->arm7Size) << ">" << endl
		<< "ARM7 Entry: 0x" << Log::num<16>(nds->arm7Entry) << endl
		<< "ARM7 ALLRA: 0x" << Log::num<16>(nds->arm7ALLRA) << endl
		<<  "ARM7 Overlay: [0x" 
			<< Log::num<16>(nds->arm7OverlayOffset) << ", 0x"
			<< Log::num<16>(nds->arm7OverlayOffset + nds->arm7OverlaySize) << ">" << endl

		<< "File Name Table: [0x" 
			<< Log::num<16>(nds->fntOffset) << ", 0x" << Log::num<16>(nds->fntOffset + nds->fntSize) << ">" << endl
		<< "File Allocation Table: [0x" 
			<< Log::num<16>(nds->fatOffset) << ", 0x" << Log::num<16>(nds->fatOffset + nds->fatSize) << ">" << endl

		<< "Debug ROM: [0x" 
			<< Log::num<16>(nds->dRomOff) << ", 0x" << Log::num<16>(nds->dRomOff + nds->dRomSize) << ">" << endl

		<< endl;

	return 0;
}

int exportIcon(const String &path, NDS *nds, FileSystem*) {

	String file = "icon.png";
	if (int ret = makeFile(path, file)) return ret;

	NDSBanner *banner = NDSBanner::get(nds);

	List<rgba8> col(32 * 32);
	R4_8::toRGBA8Image<true, true>(banner->icon, col.data(), 32, 32, banner->palette);

	List<u8> buffer = encodePng(col, 32, 32);

	if (!System::files()->write(file, buffer))
		return 2;

	return 0;
}

int exportIconPalette(const String &path, NDS *nds, FileSystem*) {

	String file = "icon_palette.png";
	if (int ret = makeFile(path, file)) return ret;

	NDSBanner *banner = NDSBanner::get(nds);

	List<rgba8> col(16);
	BGR5::toRGBA8Image(banner->palette, col.data(), 16);

	List<u8> buffer = encodePng(col, 16, 1);

	if (!System::files()->write(file, buffer))
		return 2;

	return 0;
}

int exportIconTilemap(const String &path, NDS *nds, FileSystem*) {

	String file = "icon_tilemap.png";
	if (int ret = makeFile(path, file)) return ret;

	NDSBanner *banner = NDSBanner::get(nds);

	List<r8> col(32 * 32);
	R4_8::toR8Image<true, true>(banner->icon, col.data(), 32, 32);

	List<u8> buffer = encodePng(col, 32, 32);

	if (!System::files()->write(file, buffer))
		return 2;

	return 0;
}

int exportArm9Bin(const String &path, NDS *nds, FileSystem*) {

	if(nds->arm9Size){

		String file = "arm9.bin";
		if (int ret = makeFile(path, file)) return ret;

		System::files()->write(
			file, Buffer((u8*)nds + nds->arm9Offset, (u8*)nds + nds->arm9Offset + nds->arm9Size)
		);
	}

	return 0;
}

int exportArm7Bin(const String &path, NDS *nds, FileSystem*) {

	if(nds->arm7Size){

		String file = "arm7.bin";
		if (int ret = makeFile(path, file)) return ret;

		System::files()->write(
			file, Buffer((u8*)nds + nds->arm7Offset, (u8*)nds + nds->arm7Offset + nds->arm7Size)
		);
	}

	return 0;
}

int exportArm9Overlay(const String &path, NDS *nds, FileSystem*) {

	if(nds->arm9OverlaySize){

		String file = "arm9_overlay.bin";
		if (int ret = makeFile(path, file)) return ret;

		System::files()->write(
			file,  Buffer((u8*)nds + nds->arm9OverlayOffset, (u8*)nds + nds->arm9OverlayOffset + nds->arm9OverlaySize)
		);
	}

	return 0;
}

int exportArm7Overlay(const String &path, NDS *nds, FileSystem*) {

	if (nds->arm7OverlaySize) {

		String file = "arm7_overlay.bin";
		if (int ret = makeFile(path, file)) return ret;

		System::files()->write(
			file, Buffer((u8*)nds + nds->arm7OverlayOffset, (u8*)nds + nds->arm7OverlayOffset + nds->arm7OverlaySize)
		);
	}

	return 0;
}

int exportDebug(const String &path, NDS *nds, FileSystem*) {

	if (nds->dRomSize) {

		String file = "debug.bin";
		if (int ret = makeFile(path, file)) return ret;

		System::files()->write(
			file, Buffer((u8*)nds + nds->dRomOff, (u8*)nds + nds->dRomOff + nds->dRomSize)
		);
	}

	return 0;
}

int exportFiles(const String &path, NDS*, FileSystem *fs) {

	for (auto &f : fs->getFiles(true)) {

		if (f.id == 0) continue;

		String file = f.path.substr(2);
		if (int ret = makeFile(path, file, f.isFolder)) return ret;

		Buffer buf;
		fs->read(f, buf);

		if(f.fileSize)
			System::files()->write(file, buf);
	}

	return 0;
}

inline void logFile(u8 *ptr, const FileInfo &f) {

	using namespace std;

	cout << f.path << " ";

	if (f.getFileObjects() || f.fileSize)
		cout << "with ";

	if (auto folders = f.getFolders()) {

		cout << folders << " folder" << (folders == 1 ? "" : "s");

		if(auto files = f.getFiles())
			cout << ", " << files << " file" << (files == 1 ? " " : "s ");

	} else if(auto files = f.getFiles())
		cout << files << " file" << (files == 1 ? "" : "s");

	if (f.fileSize) {

		if (f.getFileObjects())
			cout << ", ";

		cout
			<< "offset 0x" << Log::num<16>(u32((u8*)f.dataExt - ptr))
			<< " and size " << f.fileSize << " (0x" << Log::num<16>(u32(f.fileSize))
			<< ") ";

		String magicNum = String((u8*)&f.magicNumber, (u8*)(&f.magicNumber + 1));

		bool isValidMagicNum = true;

		for (const c8 c : magicNum)
			if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))) {
				isValidMagicNum = false;
				break;
			}

		if (isValidMagicNum)
			cout << "and magic number \"" << magicNum << "\"";
	}

	cout << endl;
}

int infoFiles(const String&, NDS *nds, FileSystem *fs) {

	u8 *ptr = (u8*)nds;

	for (auto &f : fs->getFiles(true))
		logFile(ptr, f);

	return 0;
}

int infoFolders(const String&, NDS *nds, FileSystem *fs) {

	u8 *ptr = (u8*)nds;

	for (auto &f : fs->getFiles(true))
		if(f.isFolder)
			logFile(ptr, f);

	return 0;
}

#ifdef _WIN32

#include <Windows.h>

void setupConsole() {
	SetConsoleOutputCP(CP_UTF8); 
}

#endif
