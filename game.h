/*
 * This file is part of the Advance project.
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

#ifndef __GAME_H
#define __GAME_H

#include "rom.h"

// ------------------------------------------------------------------------
// Game

#include <cstdio>

class game {
	mutable rom_by_name_set rs; // rom set
	mutable sample_by_name_set ss; // sample set
	
	mutable zippath_container rzs; // set of rom zip for the game
	mutable zippath_container szs; // set of sample zip for the game

	// game information
	std::string name;
	std::string romof;
	std::string cloneof;
	std::string sampleof;
	std::string description;
	std::string year;
	std::string manufacturer;
	bool working;
public:
	game();
	game(const std::string& name);
	game(const game& A);
	~game();

	void name_set(const std::string& Aname);
	void cloneof_set(const std::string& Aromof);
	void romof_set(const std::string& Aromof);
	void sampleof_set(const std::string& Aromof);
	void description_set(const std::string& Ades);
	void year_set(const std::string& Ayear);
	void manufacturer_set(const std::string& Amanufacturer);
	void working_set(bool Aworking);
	bool working_get() const { return working; }

	const rom_by_name_set& rs_get() const { return rs; }
	const sample_by_name_set& ss_get() const { return ss; }
	rom_by_name_set& rs_get() { return rs; }
	sample_by_name_set& ss_get() { return ss; }
	void rs_remove_name(const rom_by_name_set& A) const;
	void rs_remove_crc(const rom_by_crc_set& A) const;
	void ss_remove_name(const sample_by_name_set& A) const;

	void rzs_add(const zippath& Azip) const;
	void szs_add(const zippath& Azip) const;

	const std::string& name_get() const { return name; }
	const std::string& cloneof_get() const { return cloneof; }
	const std::string& romof_get() const { return romof; }
	const std::string& sampleof_get() const { return sampleof; }
	const std::string& description_get() const { return description; }
	const std::string& year_get() const { return year; }
	const std::string& manufacturer_get() const { return manufacturer; }
	const zippath_container& rzs_get() const { return rzs; }
	const zippath_container& szs_get() const { return szs; }

	bool romset_required() const { return rs_get().begin() != rs_get().end(); }
	bool sampleset_required() const { return ss_get().begin() != ss_get().end(); }
	
	bool good_romzip_has() const;
	unsigned good_romzip_size() const;
	std::string good_romzip_get() const;
        bool bad_romzip_has() const;
	bool usable_romzip_has() const;
	
	bool good_samplezip_has() const;
	unsigned good_samplezip_size() const;
        bool bad_samplezip_has() const;

	unsigned size_get() const;
};

struct game_by_name_less : std::binary_function<game,game,bool> {
	bool operator()(const game& A, const game& B) const {
		return A.name_get() < B.name_get();
	}
};

struct game_by_output_less : std::binary_function<game,game,bool> {
	bool operator()(const game& A, const game& B) const {
		int r = A.cloneof_get().compare(B.cloneof_get());
		if (r<0) return true;
		if (r>0) return false;
		return A.size_get() < B.size_get();
	}
};

typedef std::set<game,game_by_name_less> game_by_name_set;
typedef std::set<game,game_by_output_less> game_by_output_set;


// ------------------------------------------------------------------------
// Game archive

class gamearchive {
	game_by_name_set map;

	// abstract
	gamearchive(const gamearchive&);
	gamearchive& operator=(const gamearchive&);

	bool load_internal(FILE* f);

public:
	gamearchive();
	~gamearchive();

	typedef game_by_name_set::iterator iterator;
	typedef game_by_name_set::const_iterator const_iterator;

	iterator begin() { return map.begin(); }
	const_iterator begin() const { return map.begin(); }
	iterator end() { return map.end(); }
	const_iterator end() const { return map.end(); }

	const_iterator find(const game& A) const;

	bool load(FILE* f);
};

bool is_game_neogeo(const game& g, const gamearchive& gar);

#endif
