#pragma once
#include <types/vec.hpp>

namespace nre {

	using r8 = u8;

	using bgr5 = u16;

	union rgba8 { 
		u32 v;
		u8 data[4];
		struct { u8 r, g, b, a; };
	};

	//Conversion functions for bgr5 type
	struct BGR5 {

		static inline constexpr u8 to8Bit(u8 bit5) {
			return u8(u16(bit5) * 0xFF / 0x1F);
		}

		static inline constexpr u16 from8Bit(u8 bit8) {
			return u16(bit8) * 0x1F / 0xFF;
		}

		static inline constexpr Vec3u8 components(bgr5 v) {
			return Vec3u8(u8(v), u8(v >> 5), u8(v >> 10)) & 0x1F;
		}

		static inline constexpr bgr5 fromRGBA8(const rgba8 *col) {
			return from8Bit(col->b) | (from8Bit(col->g) << 5) | (from8Bit(col->r) << 10);
		}

		static inline constexpr void toRGBA8(bgr5 in, rgba8 *out) {
			const Vec3u8 c = components(in);
			out->r = to8Bit(c.x);
			out->g = to8Bit(c.y);
			out->b = to8Bit(c.z);
			out->a = 0xFF;
		}

		static inline void toRGBA8Image(const bgr5 *in, rgba8 *out, usz len) {
			for (usz i = 0; i < len; ++i)
				toRGBA8(in[i], out + i);
		}

		static inline void toBGR5Image(const rgba8 *in, bgr5 *out, usz len) {
			for (usz i = 0; i < len; ++i)
				out[i] = BGR5::fromRGBA8(in + i);
		}
	};

	//Conversion functions for r4/r8 types
	struct R4_8 {

		static constexpr inline u8 sample4Bit(const u8 *beg, usz offset) {
			return (beg[offset >> 1] >> ((offset & 1) << 2)) & 0xF;
		}

		static constexpr inline u8 sample4BitNormalized(const u8 *beg, usz offset) {
			return u8(u16(sample4Bit(beg, offset)) * 0xFF / 0xF);
		}

		//i = tile pixel
		//j = tile index
		//w = width in pixels
		static inline usz toLinearSpace(usz i, usz j, usz w) {
			const usz x = (i & 7) + ((j % (w >> 3)) << 3);
			const usz y = ((i >> 3) + ((j / (w >> 3)) << 3));
			return x + y * w;
		}

		template<bool is4Bit, bool isTiled>
		static inline bool toR8Image(const u8 *icon, r8 *out, u16 w, u16 h) {

			if constexpr (!isTiled) {
				if constexpr (!is4Bit)
					std::memcpy(out, icon, w * h);
				else
					for (usz i = 0, j = usz(w) * h; i < j; ++i)
						out[i] = sample4Bit(icon, i);
			} else {

				if (w % 8 || h % 8)
					return false;

				usz tiles = (w >> 3) * (h >> 3);

				if constexpr (!is4Bit)
					for (usz j = 0; j < tiles; ++j)
						for (usz i = 0; i < 64; ++i)
							out[toLinearSpace(i, j, w)] = icon[i + (j << 6)];
				else
					for (usz j = 0; j < tiles; ++j)
						for (usz i = 0; i < 64; ++i)
							out[toLinearSpace(i, j, w)] = sample4Bit(icon, i + (j << 6));
			}

			return true;
		}

		template<bool is4Bit, bool isTiled>
		static inline bool toBGR5Image(const u8 *icon, bgr5 *out, u16 w, u16 h, const bgr5 *palette) {

			if constexpr (!isTiled) {
				if constexpr (!is4Bit)
					for (usz i = 0, j = usz(w) * h; i < j; ++i)
						out[i] = palette[icon[i]];
				else
					for (usz i = 0, j = usz(w) * h; i < j; ++i)
						out[i] = palette[sample4Bit(icon, i)];
			} else {

				if (w % 8 || h % 8)
					return false;

				u16 tiles = (w >> 3) * (h >> 3);

				if constexpr (!is4Bit)
					for (usz j = 0; j < tiles; ++j)
						for (usz i = 0; i < 64; ++i)
							out[toLinearSpace(i, j, w)] = palette[icon[i + (j << 6)]];
				else
					for (usz j = 0; j < tiles; ++j)
						for (usz i = 0; i < 64; ++i)
							out[toLinearSpace(i, j, w)] = palette[sample4Bit(icon, i + (j << 6))];
			}

			return true;
		}

		template<bool is4Bit, bool isTiled>
		static inline bool toRGBA8Image(const u8 *icon, rgba8 *out, u16 w, u16 h, const bgr5 *palette) {

			if constexpr (!isTiled) {
				if constexpr (!is4Bit)
					for (usz i = 0, j = usz(w) * h; i < j; ++i) {
						if(auto pid = icon[i])
							BGR5::toRGBA8(palette[pid], out + i);
					}
				else
					for (usz i = 0, j = usz(w) * h; i < j; ++i)
						if(auto pid = sample4Bit(icon, i))
							BGR5::toRGBA8(palette[pid], out + i);
			} else {

				if (w % 8 || h % 8)
					return false;

				u16 tiles = (w >> 3) * (h >> 3);

				if constexpr (!is4Bit)
					for (usz j = 0; j < tiles; ++j)
						for (usz i = 0; i < 64; ++i) {
							if (auto pid = icon[i + (j << 6)])
								BGR5::toRGBA8(palette[pid], out + toLinearSpace(i, j, w));
						}
				else
					for (usz j = 0; j < tiles; ++j)
						for (usz i = 0; i < 64; ++i)
							if(auto pid = sample4Bit(icon, i + (j << 6)))
								BGR5::toRGBA8(palette[pid], out + toLinearSpace(i, j, w));
			}

			return true;
		}

	};

}