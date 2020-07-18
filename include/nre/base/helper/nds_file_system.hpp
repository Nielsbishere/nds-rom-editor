#pragma once
#include "../types/nds.hpp"
#include <system/file_system.hpp>

namespace nre {

	class NDSFile : public oic::File {

		virtual ~NDSFile() {}

	public:

		NDSFile(oic::FileSystem *fs, const oic::FileInfo &f): oic::File(fs, f) {}

		bool read(void *v, oic::FileSize size, oic::FileSize offset) const final override;
		bool write(const void *v, oic::FileSize size, oic::FileSize offset) final override;

		bool resize(oic::FileSize size) final override;

	};

	//Implementation of oic FileSystem to support NDS
	//
	class NDSFileSystem : public oic::FileSystem {

	public:

		NDSFileSystem(NDS*) noexcept(false);

		oic::File *open(const oic::FileInfo &inf, ns, ns) final override;

		const oic::FileInfo local(const String&) const final override { return {}; }
		bool hasLocal(const String&) const final override { return false; }
		bool hasLocalRegion(const String&, oic::FileSize, oic::FileSize) const final override { return false; }

		List<String> localDirectories(const String&) const final override { return {}; }
		List<String> localFileObjects(const String&) const final override { return {}; }
		List<String> localFiles(const String&) const final override { return {}; }

	protected:

		//Only writes have to be reported, but currently there's no real use for this

		void startFileWatcher(const String&) final override {}
		void endFileWatcher(const String&) final override {}

		//

		bool makeLocal(const String&, bool) final override { return false; }
		bool delLocal(const String&) final override { return false; }
		void initFiles() final override {}
	};

}