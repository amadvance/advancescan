/*
 * This file is part of the Advance project.
 *
 * Copyright (C) 1998, 1999, 2000, 2001, 2002 Andrea Mazzoleni
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

#ifndef __ROM_H
#define __ROM_H

#include "file.h"

#include <set>
#include <vector>
#include <list>

class rom {
protected:
	std::string name;
	unsigned size;
	crc_t crc;
	bool nodump;
public:
	rom();
	rom(const std::string& Aname, const unsigned Asize, const crc_t Acrc, bool Anodump);
	rom(const rom&);
	~rom();

	const std::string& name_get() const { return name; }
	void name_set(const std::string& Aname);

	crc_t crc_get() const { return crc; }
	void crc_set(crc_t Acrc);

	unsigned size_get() const { return size; }
	void size_set(unsigned Asize);

	bool nodump_get() const { return nodump; }
	void nodump_set(bool Anodump);

	bool operator==(const rom& A) const;
};

struct rom_by_crc_less : std::binary_function<rom, rom, bool> {
	bool operator()(const rom& A, const rom& B) const {
		if (A.crc_get() < B.crc_get()) return true;
		if (A.crc_get() > B.crc_get()) return false;
		if (A.size_get() < B.size_get()) return true;
		return false;
	}
};

struct rom_by_name_less : std::binary_function<rom, rom, bool> {
	bool operator()(const rom& A, const rom& B) const {
		return file_compare(A.name_get(), B.name_get()) < 0;
	}
};

typedef std::list<rom> rom_container;
typedef std::set<rom, rom_by_name_less> rom_by_name_set;
typedef std::set<rom, rom_by_crc_less> rom_by_crc_set;

inline bool operator==(const rom_by_name_set& A, const rom_by_name_set& B)
{
	return A.size()==B.size() && equal(A.begin(), A.end(), B.begin());
}

class gamerom : public rom {
protected:
	std::string game;
public:
	gamerom();
	gamerom(const std::string& Agame, const std::string& Aname, unsigned Asize, crc_t Acrc, bool Anodump);
	gamerom(const gamerom&);
	~gamerom();

	void game_set(const std::string& Agame);
	const std::string& game_get() const { return game; }
};

struct gamerom_by_crc_less : std::binary_function<gamerom, gamerom, bool> {
	bool operator()(const gamerom& A, const gamerom& B) const {
		if (A.crc_get() < B.crc_get()) return true;
		if (A.crc_get() > B.crc_get()) return false;
		if (A.size_get() < B.size_get()) return true;
		return false;
	}
};

typedef std::multiset<gamerom, gamerom_by_crc_less> gamerom_by_crc_multiset;

#endif

