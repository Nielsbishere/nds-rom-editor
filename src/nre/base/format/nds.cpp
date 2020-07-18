#include "types/nds.hpp"
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
		(arm9Size && arm9Offset + arm9Size > romSize) || 
		(arm7Size && arm7Offset + arm7Size > romSize);
}

NDS *NDS::get(u8 *romPtr, usz romSize) {

	if (romSize < sizeof(NDS))
		return nullptr;

	NDS* const nds = (NDS*) romPtr;

	if(nds->invalid())
		return nullptr;

	return nds;
}