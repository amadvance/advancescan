/*
 * This file is part of the AdvanceSCAN project.
 *
 * Copyright (C) 1998-2002 Andrea Mazzoleni
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

#ifndef __TYPES_H
#define __TYPES_H

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

// -------------------------------------------------------------------------
// Little Endian

inline uint8 le_uint8_read(const void* ptr) {
	const uint8* ptr8 = (const uint8*)ptr;
	return (uint8)ptr8[0];
}

inline uint16 le_uint16_read(const void* ptr) {
	const uint8* ptr8 = (const uint8*)ptr;
	return (uint16)ptr8[0] | (uint16)ptr8[1] << 8;
}

inline uint32 le_uint32_read(const void* ptr) {
	const uint8* ptr8 = (const uint8*)ptr;
	return (uint32)ptr8[0] | (uint32)ptr8[1] << 8 |	(uint32)ptr8[2] << 16 | (uint32)ptr8[3] << 24;
}

inline void le_uint8_write(void* ptr, uint8 v) {
	uint8* ptr8 = (uint8*)ptr;
	ptr8[0] = v;
}

inline void le_uint16_write(void* ptr, uint16 v) {
	uint8* ptr8 = (uint8*)ptr;
	ptr8[0] = v & 0xFF;
	ptr8[1] = (v >> 8) & 0xFF;
}

inline void le_uint32_write(void* ptr, uint32 v) {
	uint8* ptr8 = (uint8*)ptr;
	ptr8[0] = v & 0xFF;
	ptr8[1] = (v >> 8) & 0xFF;
	ptr8[2] = (v >> 16) & 0xFF;
	ptr8[3] = (v >> 24) & 0xFF;
}

// -------------------------------------------------------------------------
// Big Endian

#endif

