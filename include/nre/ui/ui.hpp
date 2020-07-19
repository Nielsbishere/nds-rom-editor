#pragma once
#include "system/viewport_interface.hpp"
#include "graphics/graphics.hpp"
#include "helpers/common.hpp"
#include "gui/gui.hpp"
#include "gui/struct_inspector.hpp"
#include "types/nds.hpp"
#include "helper/nds_file_system.hpp"

#include <codecvt>

//Reserve temporary data

namespace nre {

	using namespace oic;
	using namespace igx;
	using namespace ui;

	struct GameInfo {

		inline String toUTF8(const WString &wstr) {
			return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(wstr);
		}

		String gameTitle, gameCode, makerCode;
		u32 version, unitCode;

		HashMap<String, String> titles;

		GameInfo(NDS *nds) :
			gameTitle(nds->title), gameCode(nds->gameCode, nds->gameCode + 4),
			makerCode(nds->makerCode, nds->makerCode + 2),
			version(nds->version), unitCode(nds->unitCode)
		{
			NDSBanner *banner = NDSBanner::get(nds);

			static constexpr c8 languages[NDSBanner::LANGUAGE_END][9] = {
				"Japanese",
				"English",
				"French",
				"German",
				"Italian",
				"Spanish",
			};

			for(u8 l = NDSBanner::LANGUAGE_START; l != NDSBanner::LANGUAGE_END; ++l)
				if (banner->hasTitle(NDSBanner::Language(l)))
					titles[languages[l]] = toUTF8(banner->getTitle(NDSBanner::Language(l)));
		}

		template<typename T, typename T2>	
		void inflect(T &inflector, const T2*) {
			inflector.inflect(
				this, 
				{ "Game title", "Game code", "Maker code", "Version", "Unit code", "Titles" }, 
				gameTitle, gameCode, makerCode, 
				version, unitCode, titles
			);
		}

		template<typename T, typename T2>	
		void inflect(T &inflector, const T2*) const {
			inflector.inflect(
				this, 
				{ "Game title", "Game code", "Maker code", "Version", "Unit code", "Titles" }, 
				gameTitle, gameCode, makerCode, 
				version, unitCode, titles
			);
		}

	};

	struct ROMExplorer {

		FileSystem *fs{};
		NDS *nds{};

		ROMExplorer(NDS *_nds = nullptr, FileSystem *_fs = nullptr): 
			nds(_nds), fs(_fs) {}

		template<typename T, typename T2>	
		void inflect(T &inflector, const T2*) {
			inflector.inflect(
				this, 
				{ "Game info", "File explorer" }, 
				GameInfo(nds),
				fs
			);
		}

		template<typename T, typename T2>	
		void inflect(T &inflector, const T2*) const {
			inflector.inflect(
				this, 
				{ "Game info", "File explorer" }, 
				GameInfo(nds),
				fs
			);
		}

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