#pragma once
#include <types/types.hpp>

namespace nre {

	enum ResourceType : u32 {
		RESOURCE_NCLR = 0x4E434C52,
		RESOURCE_NCGR = 0x4E434752,
		RESOURCE_NCSR = 0x4E534352,
		RESOURCE_NARC = 0x4352414E,
		RESOURCE_BMD0 = 0x30444D42
	};

	struct GenericHeader {
		ResourceType type;
		u32 constant;
		u32 size;
		u16 headerSize;
		u16 sections;
	};

	enum SectionType : u32 {
		SECTION_TTLP = 0x504C5454,
		SECTION_PMCP = 0x50434D50,
		SECTION_RAHC = 0x43484152,
		SECTION_SOPC = 0x43504F53,
		SECTION_NRCS = 0x5343524E,
		SECTION_BTAF = 0x46415442,
		SECTION_BTNF = 0x464E5442,
		SECTION_GMIF = 0x46494D47,
		SECTION_MDL0 = 0x304C444D,
		SECTION_TEX0 = 0x30584554
	};

	template<SectionType sectionType, typename DataType>
	struct GenericSection {

		SectionType type;
		u32 size;

		static constexpr SectionType getSectionType() { return sectionType; }
	};

}