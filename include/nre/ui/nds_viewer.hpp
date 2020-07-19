#pragma once
#include <types/nds.hpp>
#include <utils/inflect.hpp>
#include <gui/ui_value.hpp>

namespace nre {

	using namespace igx::ui;

	struct NDSViewer {

		//NDS Header

		struct _Header {

			c8 Title[12];
			c8 Game_code[4];
			c8 Maker_code[2];

			u8 Unit_code;
			u8 Capacity;
			u8 Encryption_seed;
			u8 Reserved[7];
			u16h Unknown;
			u8 Version;
			u8 Flags;

			Inflect(Title, Version, Flags, Game_code, Maker_code, Unit_code, Capacity, Encryption_seed);

		} header;

		//NDS important offsets

		struct _Offsets {

			//A block of arm assembly

			struct CodeBlock {

				const u32h offset;	
				u32h entry, load;
				const u32h size;

				inline void exportToFile() const {
					//TODO:
				}

				inline void importFromFile() const {
					//TODO:
				}

				InflectWithName(
					{ "Offset", "Entry", "Load", "Size", "Export to file", "Import from file" },
					offset, entry, load, size
					/*,		TODO:
					igx::ui::Button<CodeBlock, &CodeBlock::exportToFile>{},
					igx::ui::Button<CodeBlock, &CodeBlock::importFromFile>{}*/
				);

			} 
			ARM9, ARM7;

			//The file tables

			struct FileTable {

				const u32h File_name_table_offset, File_name_table_size;

				const u32h File_allocation_table_offset, File_allocation_table_size;

				Inflect(File_name_table_offset, File_name_table_size, File_allocation_table_offset, File_allocation_table_size);

				//TODO: Show FNT and FAT

			} 
			File_table;

			//The ARM assembly overlays

			struct Overlay {

				const u32h offset, size;

				inline void exportToFile() const {
					//TODO:
				}

				inline void importFromFile() const {
					//TODO:
				}

				InflectWithName(
					{ "Offset", "Size", "Export to file", "Import from file" }, 
					offset, size
					/*, 
					igx::ui::Button<Overlay, &Overlay::exportToFile>{},
					igx::ui::Button<Overlay, &Overlay::importFromFile>{}*/
				);

			}
			ARM9_Overlay, ARM7_Overlay;

			Inflect(File_table, ARM9, ARM7, ARM9_Overlay, ARM7_Overlay);

		}
		offsets;

		//The extended header section

		struct HeaderExt {

			u32h Card_control;
			u32h Safe_card_control;

			const u32h Banner_offset;

			u16h Secure_area_checksum;
			u16 Secure_area_loading_timeout;

			struct ALLRA {
				const u32h ARM9, ARM7;
				Inflect(ARM9, ARM7);
			} 
			Auto_load_list_ram_addresses;

			u64h Secure_area_disable;

			const u32h Rom_size;
			const u32h Rom_header_size;

			u8 Reserved_1[56];

			u8 Nintendo_logo[156];

			u16h Nintendo_logo_checksum;
			u16h Header_checksum;

			const u32h Debug_rom_offset;
			const u32h Debug_rom_size;

			u32h Reserved_2;
			u8 Reserved_3[144];

			Inflect(
				Rom_size, Rom_header_size,
				Debug_rom_offset, Debug_rom_size,
				Secure_area_checksum, Secure_area_loading_timeout,
				Secure_area_disable,
				Auto_load_list_ram_addresses,
				Nintendo_logo_checksum, Header_checksum,
				Card_control, Safe_card_control,
				Banner_offset
			);

		}
		headerExt;

		//The NDS banner with icon, etc.

		inline NDSBanner *getBanner() { return (NDSBanner*)((u8*)this + headerExt.Banner_offset); }

		InflectWithName({ "Header", "Header extended", "Header banner", "Offsets" }, header, headerExt, *getBanner(), offsets);

	};

}