#pragma once
#include "system/viewport_interface.hpp"
#include "graphics/graphics.hpp"
#include "helpers/common.hpp"
#include "gui/gui.hpp"
#include "gui/struct_inspector.hpp"
#include "nds_viewer.hpp"
#include "helper/nds_file_system.hpp"

#include <codecvt>

//Reserve temporary data

namespace nre {

	using namespace oic;
	using namespace igx;
	using namespace ui;

	struct ROMExplorer {

		NDSViewer *nds{};
		FileSystem *fs{};

		ROMExplorer(NDS *_nds = nullptr, FileSystem *_fs = nullptr): 
			nds((NDSViewer*)_nds), fs(_fs) {}

		InflectWithName({ "Game info", "File explorer" }, *nds, fs);
	};

	struct Explorer : public ViewportInterface {

		Graphics &g;
		GUI gui;
		SwapchainRef swapchain;

		NDSFileSystem fileSystem;

		StructInspector<ROMExplorer> romExplorer;

		Explorer(Graphics &g, NDS *rom) : 
			g(g), gui(g, SetClearColor(Vec4f32(0.125f, 0.25f, 0.5f))), 
			fileSystem(rom), romExplorer(ROMExplorer{ rom, rom ? &fileSystem : nullptr }) 
		{
			gui.addWindow(Window("ROM Explorer", 0, {}, { 600, 450 }, &romExplorer, Window::Flags(Window::DEFAULT_SCROLL & ~Window::CLOSE)));
		}

		void init(ViewportInfo *vp) final override {

			if (swapchain.exists())
				oic::System::log()->fatal("Currently only supporting 1 viewport");

			g.resume();		//This thread can now interact with graphics

			swapchain = {
				g, NAME("Swapchain"),
				Swapchain::Info{ vp, false }
			};

		}

		void release(const ViewportInfo*) final override {
			swapchain.release();
		}

		void render(const ViewportInfo *vi) final override {
			gui.render(g, vi->offset, vi->monitors);
			g.present(gui.getFramebuffer(), swapchain, gui.getCommands());
		}

		void resize(const ViewportInfo*, const Vec2u32 &size) final override {
			gui.resize(size);
			swapchain->onResize(size);
		}

		void onInputUpdate(ViewportInfo*, const InputDevice *dvc, InputHandle ih, bool isActive) final override {
			gui.onInputUpdate(dvc, ih, isActive);
		}

	};

}