/*
 * This file is part of the AdvanceSCAN project.
 *
 * Copyright (C) 1999-2002 Andrea Mazzoleni
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details. 
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "portable.h"

#include "lib/endianrw.h"
#include "lib/mng.h"

#include "pngex.h"

#include <iostream>
#include <iomanip>

using namespace std;

static bool png_compress_raw(shrink_t level, unsigned char* z_ptr, unsigned& z_size, const unsigned char* fil_ptr, unsigned fil_size) {
#if defined(USE_7Z)
	if (level != shrink_none && level != shrink_normal) {
		unsigned sz_passes;
		unsigned sz_fastbytes;

		switch (level) {
		case shrink_extra :
			sz_passes = 1;
			sz_fastbytes = 64;
			break;
		case shrink_extreme :
			sz_passes = 3;
			sz_fastbytes = 128;
			break;
		case shrink_normal :
		case shrink_none :
			assert(0);
		}

		if (!compress_rfc1950_7z(fil_ptr,fil_size,z_ptr,z_size,sz_passes,sz_fastbytes)) {
			return false;
		}

		return true;
	}
#endif

	int libz_level = Z_BEST_COMPRESSION;

	switch (level) {
	case shrink_none :
		libz_level = Z_NO_COMPRESSION;
		break;
	case shrink_normal :
		libz_level = Z_DEFAULT_COMPRESSION;
		break;
	case shrink_extra :
		libz_level = Z_BEST_COMPRESSION;
		break;
	case shrink_extreme :
		libz_level = Z_BEST_COMPRESSION;
		break;
	}

	if (!compress_rfc1950_zlib(fil_ptr,fil_size,z_ptr,z_size,libz_level,Z_DEFAULT_STRATEGY,MAX_MEM_LEVEL)) {
		return false;
	}

	return true;
}

void png_compress(shrink_t level, data_ptr& out_ptr, unsigned& out_size, const unsigned char* img_ptr, unsigned img_scanline, unsigned img_pixel, unsigned x, unsigned y, unsigned dx, unsigned dy) {
	data_ptr fil_ptr;
	unsigned fil_size;
	unsigned fil_scanline;
	data_ptr z_ptr;
	unsigned z_size;
	unsigned i;
	unsigned char* p0;

	fil_scanline = dx * img_pixel + 1;
	fil_size = dy * fil_scanline;
	z_size = fil_size * 11 / 10 + 12;

	fil_ptr = data_alloc(fil_size);
	z_ptr = data_alloc(z_size);

	p0 = fil_ptr;

	for(i=0;i<dy;++i) {
		const unsigned char* p1 = &img_ptr[x * img_pixel + (i+y) * img_scanline];
		*p0++ = 0;
		memcpy(p0, p1, dx * img_pixel);
		p0 += dx * img_pixel;
	}

	assert(p0 == fil_ptr + fil_size);

	if (!png_compress_raw(level, z_ptr, z_size, fil_ptr, fil_size)) {
		throw error() << "Failed compression";
	}

	out_ptr = z_ptr;
	out_size = z_size;
}

void png_compress_delta(shrink_t level, data_ptr& out_ptr, unsigned& out_size, const unsigned char* img_ptr, unsigned img_scanline, unsigned img_pixel, const unsigned char* prev_ptr, unsigned prev_scanline,unsigned x, unsigned y, unsigned dx, unsigned dy) {
	data_ptr fil_ptr;
	unsigned fil_size;
	unsigned fil_scanline;
	data_ptr z_ptr;
	unsigned z_size;
	unsigned i;
	unsigned char* p0;

	fil_scanline = dx * img_pixel + 1;
	fil_size = dy * fil_scanline;
	z_size = fil_size * 11 / 10 + 12;

	fil_ptr = data_alloc(fil_size);
	z_ptr = data_alloc(z_size);

	p0 = fil_ptr;

	for(i=0;i<dy;++i) {
		unsigned j;
		const unsigned char* p1 = &img_ptr[x * img_pixel + (i+y) * img_scanline];
		const unsigned char* p2 = &prev_ptr[x * img_pixel + (i+y) * prev_scanline];

		*p0++ = 0;
		for(j=0;j<dx*img_pixel;++j)
			*p0++ = *p1++ - *p2++;
	}

	assert(p0 == fil_ptr + fil_size);

	if (!png_compress_raw(level, z_ptr, z_size, fil_ptr, fil_size)) {
		throw error() << "Failed compression";
	}

	out_ptr = z_ptr;
	out_size = z_size;
}

void png_compress_palette_delta(data_ptr& out_ptr, unsigned& out_size, const unsigned char* pal_ptr, unsigned pal_size, const unsigned char* prev_ptr) {
	unsigned i;
	unsigned char* dst_ptr;
	unsigned dst_size;

	dst_ptr = data_alloc(pal_size * 2);
	dst_size = 0;

	dst_ptr[dst_size++] = 0; /* replacement */

	i = 0;
	while (i<pal_size) {
		unsigned j;

		while (i < pal_size && prev_ptr[i] == pal_ptr[i] && prev_ptr[i+1] == pal_ptr[i+1] && prev_ptr[i+2] == pal_ptr[i+2])
			i += 3;

		if (i == pal_size)
			break;

		j = i;

		while (j < pal_size && (prev_ptr[j] != pal_ptr[j] || prev_ptr[j+1] != pal_ptr[j+1] || prev_ptr[j+2] != pal_ptr[j+2]))
			j += 3;

		dst_ptr[dst_size++] = i / 3; /* first index */
		dst_ptr[dst_size++] = (j / 3) - 1; /* last index */

		while (i < j) {
			dst_ptr[dst_size++] = pal_ptr[i++];
			dst_ptr[dst_size++] = pal_ptr[i++];
			dst_ptr[dst_size++] = pal_ptr[i++];
		}
	}

	if (dst_size == 1) {
		out_ptr = 0;
		out_size = 0;
		free(dst_ptr);
	} else {
		out_ptr = dst_ptr;
		out_size = dst_size;
	}
}

void png_print_chunk(unsigned type, unsigned char* data, unsigned size) {
	char tag[5];
	unsigned i;

	be_uint32_write(tag, type);
	tag[4] = 0;

	cout << tag << setw(8) << size;

	switch (type) {
		case MNG_CN_MHDR :
			cout << " width:" << be_uint32_read(data+0) << " height:" << be_uint32_read(data+4) << " frequency:" << be_uint32_read(data+8) << " simplicity:" << be_uint32_read(data+24);
		break;
		case MNG_CN_DHDR :
			cout << " id:" << be_uint16_read(data+0);
			switch (data[2]) {
				case 0 : cout << " img:unspecified"; break;
				case 1 : cout << " img:png"; break;
				case 2 : cout << " img:jng"; break;
				default: cout << " img:?"; break;
			}
			switch (data[3]) {
				case 0 : cout << " delta:entire_replacement"; break;
				case 1 : cout << " delta:block_addition"; break;
				case 2 : cout << " delta:block_alpha_addition"; break;
				case 3 : cout << " delta:block_color_addition"; break;
				case 4 : cout << " delta:block_replacement"; break;
				case 5 : cout << " delta:block_alpha_replacement"; break;
				case 6 : cout << " delta:block_color_replacement"; break;
				case 7 : cout << " delta:no_change"; break;
				default: cout << " delta:?"; break;
			}
			if (size >= 12) {
				cout << " width:" << be_uint32_read(data + 4) << " height:" << be_uint32_read(data + 8);
			}
			if (size >= 20) {
				cout << " x:" << (int)be_uint32_read(data + 12) << " y:" << (int)be_uint32_read(data + 16);
			}
		break;
		case MNG_CN_FRAM :
			if (size >= 1) {
				cout << " mode:" << (unsigned)data[0];
			}
			if (size > 1) {
				i = 1;
				while (i < size && data[i]!=0)
					++i;
				cout << " len:" << i-1;

				if (size >= i+2) {
					cout << " delay_mode:" << (unsigned)data[i+1];
				}

				if (size >= i+3) {
					cout << " timeout:" << (unsigned)data[i+2];
				}

				if (size >= i+4) {
					cout << " clip:" << (unsigned)data[i+3];
				}

				if (size >= i+5) {
					cout << " syncid:" << (unsigned)data[i+4];
				}

				if (size >= i+9) {
					cout << " tick:" << be_uint32_read(data+i+5);
				}

				if (size >= i+13) {
					cout << " timeout:" << be_uint32_read(data+i+9);
				}

				if (size >= i+14) {
					cout << " dt:" << (unsigned)data[i+10];
				}

				if (size >= i+15) {
					cout << " ...";
				}
			}
			break;
		case MNG_CN_DEFI :
			cout << " id:" << be_uint16_read(data+0);
			if (size >= 3) {
				switch (data[2]) {
					case 0 : cout << " visible:yes"; break;
					case 1 : cout << " visible:no"; break;
					default : cout << " visible:?"; break;
				}
			}
			if (size >= 4) {
				switch (data[3]) {
					case 0 : cout << " concrete:abstract"; break;
					case 1 : cout << " concrete:concrete"; break;
					default : cout << " concrete:?"; break;
				}
			}
			if (size >= 12) {
				cout << " x:" << (int)be_uint32_read(data + 4) << " y:" << (int)be_uint32_read(data + 8);
			}
			if (size >= 28) {
				cout << " left:" << be_uint32_read(data + 12) << " right:" << be_uint32_read(data + 16) << " top:" << be_uint32_read(data + 20) << " bottom:" << be_uint32_read(data + 24);
			}
		break;
		case MNG_CN_MOVE :
			cout << " id_from:" << be_uint16_read(data+0) << " id_to:" << be_uint16_read(data+2);
			switch (data[4]) {
				case 0 : cout << " type:replace"; break;
				case 1 : cout << " type:add"; break;
				default : cout << " type:?"; break;
			}
			cout << " x:" << (int)be_uint32_read(data + 5) << " y:" << (int)be_uint32_read(data + 9);
			break;
		case MNG_CN_PPLT :
			switch (data[0]) {
				case 0 : cout << " type:replacement_rgb"; break;
				case 1 : cout << " type:delta_rgb"; break;
				case 2 : cout << " type:replacement_alpha"; break;
				case 3 : cout << " type:delta_alpha"; break;
				case 4 : cout << " type:replacement_rgba"; break;
				case 5 : cout << " type:delta_rgba"; break;
				default : cout << " type:?"; break;
			}
			i = 1;
			while (i<size) {
				unsigned ssize;
				cout << " " << (unsigned)data[i] << ":" << (unsigned)data[i+1];
				if (data[0] == 0 || data[1] == 1)
					ssize = 3;
				else if (data[0] == 2 || data[1] == 3)
					ssize = 1;
				else
					ssize = 4;
				i += 2 + (data[i+1] - data[i] + 1) * ssize;
			}
			break;
		case PNG_CN_IHDR :
			cout << " width:" << be_uint32_read(data) << " height:" << be_uint32_read(data + 4);
			cout << " depth:" << (unsigned)data[8];
			cout << " color_type:" << (unsigned)data[9];
			cout << " compression:" << (unsigned)data[10];
			cout << " filter:" << (unsigned)data[11];
			cout << " interlace:" << (unsigned)data[12];
		break;
	}

	cout << endl;
}

