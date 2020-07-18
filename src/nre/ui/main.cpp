#include "ui.hpp"
#include "system/viewport_manager.hpp"
#include "system/local_file_system.hpp"

using namespace oic;
using namespace ignis;
using namespace nre;

int main(int argc, char *argv[]) {

	NDS *nds{};
	Buffer buffer;

	if (argc == 2 && oic::System::files()->exists(argv[1])) {
		oic::System::files()->read(argv[1], buffer);
		nds = NDS::get(buffer.data(), buffer.size());
	}

	Graphics g(nds ? "NRE - " + String(nds->title) : "NRE", 1, "Igx", 1);
	nre::Explorer ui(g, nds);

	g.pause();

	System::viewportManager()->create(
		ViewportInfo(
			g.appName, {}, {}, 0, &ui, ViewportInfo::HANDLE_INPUT
		)
	);

	System::viewportManager()->waitForExit();
	return 0;
}