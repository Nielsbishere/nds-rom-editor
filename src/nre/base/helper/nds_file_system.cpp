#include "helper/nds_file_system.hpp"

using namespace oic;

namespace nre {

	bool NDSFile::read(void *v, FileSize size, FileSize offset) const {

		if (offset + size > f.fileSize) {
			System::log()->fatal("File read is out of bounds");
			return false;
		}

		std::memcpy(v, (u8*) f.dataExt + offset, size);
		return true;
	}

	bool NDSFile::resize(FileSize size) {

		//We can at least read or write it now, but our fileSize doesn't have to change, because that one won't be able to change

		if (size <= f.fileSize)
			return true;

		//We're already aligned to a KiB, so we can't allocate the padding

		if (!(f.fileSize & 0x3FF))
			return false;

		//Our file system allocates in KiBs, so a 128 byte file is actually 1 KiB.
		//We allow that portion to be reallocated

		if ((f.fileSize & ~0x3FF) + 0x400 < size)
			return false;

		f.fileSize = size;
		return true;
	}

	bool NDSFile::write(const void *v, FileSize size, FileSize offset) {

		//Files are allocated by the KiB, so we can only expand this file to the next size in KiB

		usz maxSize = f.fileSize & 0x3FF ? (f.fileSize & ~0x3FF) + 0x400 : f.fileSize;

		if (offset + size > maxSize) {
			System::log()->fatal("File write out of bounds");
			return false;
		}

		if (offset + size > f.fileSize)
			f.fileSize = offset + size;

		std::memcpy((u8*) f.dataExt + offset, v, size);
		return true;
	}

	File *NDSFileSystem::open(const FileInfo &f, ns, ns) {

		if (f.isFolder()) {
			System::log()->fatal("Can't open a folder");
			return nullptr;
		}

		if (f.isLocal()) {
			System::log()->fatal("Can't open a virtual file");
			return nullptr;
		}

		return new NDSFile(this, f);
	}

	NDSFileSystem::NDSFileSystem(NDS *nds) : FileSystem(FileAccess::READ_WRITE) {

		if (!nds)
			return;		//We don't have files

		if (!nds->fntSize)
			throw std::runtime_error("NDS file doesn't include a file system");

		u8 *ptr = (u8*)nds;
		FNTFolder *root = (FNTFolder*)(ptr + nds->fntOffset);

		//Check if it has a parent (root node doesn't)
		if (root->relation & 0xF000)		//TODO: Do folders start at 0xF000 or do they just set the upper nibble to 0xF?
			throw std::runtime_error("Root folder not found in NDS file system");

		//Get all files 

		u16 folderCount = root->relation;
		List<FNTFile> nfiles(folderCount);

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
			FileHandle(root[i].relation - 0xF000),
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
			const c8 *const name = (const c8*)nameDat;
			nameDat += nameLen;

			if (spec & 0x80) {

				FNTFile &nf = nfiles[*(u16*)nameDat - 0xF000];
				nf.name = name;
				nf.nameLen = nameLen;
				nameDat += 2;

				++nfiles[nf.parent].folders;

			} else {

				u32 *siz = fat + (l++ << 1);
				++nfiles[j].files;

				FNTFile nf {
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

		List<FileInfo> &fs = virtualFiles = List<FileInfo>(i);
		List<u16> mappings(i);

		u32 nextFile{};

		fs[0] = FileInfo {
			"~", "~",
			0,
			root,
			0,
			0,
			++nextFile, nextFile += nfiles[0].folders, nextFile += nfiles[0].files,
			FileFlags::VIRTUAL_FOLDER
		};

		mappings[0] = 0;

		struct FolderFileCounter { u16 folders{}, files{}; };
		List<FolderFileCounter> folders(folderCount);

		for (u16 j = 1; j < i; ++j) {

			FNTFile &nf = nfiles[j];
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
					0,
					root + j,
					0,
					parentId,
					nextFile, nextFile += nf.folders, nextFile += nf.files,
					FileFlags::VIRTUAL_FOLDER
				};
			else
				fs[placeId] = FileInfo{
					parent.path + "/" + name, name,
					0,
					ptr + nf.beg,
					nf.size,
					parentId,
					0, 0, 0,
					FileFlags::VIRTUAL_FILE_WRITE
				};
		}

		initLut();
	}

}