/*
 * This file is part of the Advance project.
 *
 * Copyright (C) 2002 Andrea Mazzoleni
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

#ifndef __GAME_H
#define __GAME_H

#include <set>
#include <list>
#include <vector>
#include <string>

// ------------------------------------------------------------------------
// Game

class rom {
	std::string name;
	unsigned size;
	unsigned crc;
public:
	rom();
	rom(const rom&);
	
	bool operator<(const rom&) const;
	bool operator==(const rom&) const;
	bool operator!=(const rom& A) const { return !operator==(A); }
	
	const std::string& name_get() const { return name; }
	unsigned size_get() const { return size; }
	unsigned crc_get() const { return crc; }
	void name_set(const std::string& A) { name = A; }
	void size_set(unsigned A) { size = A; }
	void crc_set(unsigned A) { crc = A; }
};

std::ostream& operator<<(std::ostream &os, const rom&);

typedef std::set<rom> rom_set;

bool include(const rom_set& A, const rom_set& B);

class game {
	std::string name;
	rom_set roms;
public:
	game();
	game(const game&);
	
	bool operator<(const game&) const;
	
	const std::string& name_get() const { return name; }
	const rom_set& roms_get() const { return roms; }
	rom_set& roms_get() { return roms; }
	void name_set(const std::string& A) { name = A; }
	void roms_set(const rom_set& A) { roms = A; }
};

std::ostream& operator<<(std::ostream &os, const game&);

typedef std::set<game> game_set;

class game_set_load : public game_set {
public:
	typedef game_set::const_iterator const_iterator;
	typedef game_set::iterator iterator;

	bool load(bool remove_merge);
};

#endif
