#include "format/nds.hpp"
using namespace nre;
using namespace oic;

bool NDS::invalid() const {

	return
		bannerOffset + sizeof(NDSBanner) >= romSize ||
		romHeaderSize < sizeof(NDS) ||
		(fntSize && fntOffset + fntSize > romSize) ||
		(fatSize && fatOffset + fatSize > romSize) ||
		(arm9OverlaySize && arm9OverlayOffset + arm9OverlaySize > romSize) ||
		(arm7OverlaySize && arm7OverlayOffset + arm7OverlaySize > romSize) ||
		(dRomOff && dRomOff + dRomSize > romSize) ||
		(
			arm9Size && 
			arm9Offset + arm9Size > romSize
		) || (
			arm7Size && 
			arm7Offset + arm7Size > romSize
		);
}

NDS *NDS::get(u8 *romPtr, usz romSize) {

	if (romSize < sizeof(NDS))
		return nullptr;

	NDS* const nds = (NDS*) romPtr;

	if(nds->invalid())
		return nullptr;

	return nds;
}

NDSFileSystem::NDSFileSystem(NDS *nds) : FileSystem({ FileAccess::NONE, FileAccess::READ_WRITE }) {

	if (!nds->fntSize)
		throw std::runtime_error("NDS file doesn't include a file system");

	u8 *ptr = (u8*) nds;
	NDSFolder *root = (NDSFolder*)(ptr + nds->fntOffset);

	//Check if it has a parent (root node doesn't)
	if (root->relation & 0xF000)		//TODO: Do folders start at 0xF000 or do they just set the upper nibble to 0xF?
		throw std::runtime_error("Root folder not found in NDS file system");

	//Get all files 

	u16 folderCount = root->relation;
	List<NDSFile> nfiles(folderCount);

	//Init root node

	nfiles[0] = {
		nullptr,
		0, 0,
		0, 0,
		0,
		0, true
	};

	//Folders

	for (u16 i = 1; i < folderCount; ++i)
		nfiles[i] = {
			nullptr,
			0, 0,
			0, 0,
			FileHandle(root[i].relation & 0xFFF),
			0, true
		};

	//TODO: Overlays, located at i < rootFolders; i * 8

	//Get names of folders and get files

	u8 *nameDat = (u8*)(root + folderCount);
	u32 *fat = (u32*)(ptr + nds->fatOffset);
	u16 firstFilePos = root->firstFilePosition;

	u16 i = 1;

	for (u16 j = 0, l = firstFilePos; ; ++i) {

		u8 spec = *nameDat;
		++nameDat;

		if (!spec) {
			spec = *nameDat;
			++nameDat;
			++j;

			if (j == folderCount)
				break;
		}

		const u8 nameLen = spec & 0x7F;
		const c8* const name = (const c8*) nameDat;
		nameDat += nameLen;

		if (spec & 0x80) {

			NDSFile &nf = nfiles[*(u16*)nameDat & 0xFFF];
			nf.name = name;
			nf.nameLen = nameLen;
			nameDat += 2;

			++nfiles[nf.parent].folders;

		} else {

			u32 *siz = fat + (l++ << 1);
			++nfiles[j].files;

			NDSFile nf {
				name,
				siz[0], siz[1] - siz[0],
				0, 0,
				j,
				nameLen, false
			};

			nfiles.push_back(nf);
		}
	}

	//Convert to independent FileSystem and change the order of files

	List<FileInfo> &fs = files(true) = List<FileInfo>(i);
	List<u16> mappings(i);

	u32 nextFile{};

	fs[0] = FileInfo {
		".", ".",
		0, 0,
		root,
		0,
		0, 0,
		++nextFile, nextFile += nfiles[0].folders, nextFile += nfiles[0].files,
		FileAccess::READ_WRITE,
		true
	};

	mappings[0] = 0;

	struct FolderFileCounter { u16 folders{}, files{}; };
	List<FolderFileCounter> folders(folderCount);

	for (u16 j = 1; j < i; ++j) {

		NDSFile &nf = nfiles[j];
		String name = String(nf.name, nf.name + nf.nameLen);

		u16 parentId = mappings[nf.parent];
		const FileInfo &parent = fs[parentId];

		u16 placeId = u16(
			nf.isFolder
			? parent.folderHint + folders[nf.parent].folders++ 
			: parent.fileHint + folders[nf.parent].files++
		);

		mappings[j] = placeId;

		if (nf.isFolder)
			fs[placeId] = FileInfo {
				parent.path + "/" + name, name,
				0, 0,
				root + j,
				0,
				parentId, placeId,
				nextFile, nextFile += nf.folders, nextFile += nf.files,
				FileAccess::READ_WRITE,
				true
			};
		else
			fs[placeId] = FileInfo {
				parent.path + "/" + name, name,
				nf.size, 0,
				ptr + nf.beg,
				*(u32*)(ptr + nf.beg),
				parentId, placeId,
				0, 0, 0,
				FileAccess::READ_WRITE,
				false
			};
	}

	initLut();
}

bool NDSFileSystem::read(const FileInfo&, void*, usz, usz) const {
	//TODO:
	return false;
}

bool NDSFileSystem::write(FileInfo&, const Buffer&, usz, usz, usz) {
	//TODO:
	//size, bufferOffset, fileOffset can be 0
	return false;
}
