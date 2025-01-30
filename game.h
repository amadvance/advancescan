/*
 * This file is part of the Advance project.
 *
 * Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004 Andrea Mazzoleni
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
#include "sample.h"
#include "disk.h"

#include <list>

class game;

typedef std::list<std::string> string_container;

class game {
	mutable rom_by_name_set rs; // rom set
	mutable sample_by_name_set ss; // sample set
	mutable disk_by_name_set ds; // disk set

	mutable zippath_container rzs; // set of rom zip for the game
	mutable zippath_container szs; // set of sample zip for the game
	mutable zippath_container dzs; // set of disk chd for the game

	// game information
	std::string name;
	std::string romof;
	std::string cloneof;
	std::string sampleof;
	std::string description;
	std::string year;
	std::string manufacturer;
	bool resource;

	mutable bool working;
	mutable bool working_subset;
	mutable bool working_parent_subset;

	mutable string_container rom_son;
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
	void resource_set(bool Aresource);

	const std::string& name_get() const { return name; }
	const std::string& cloneof_get() const { return cloneof; }
	const std::string& romof_get() const { return romof; }
	const std::string& sampleof_get() const { return sampleof; }
	const std::string& description_get() const { return description; }
	const std::string& year_get() const { return year; }
	const std::string& manufacturer_get() const { return manufacturer; }
	bool resource_get() const { return resource; }

	void working_set(bool Aworking) const;
	bool working_get() const { return working; }
	void working_subset_set(bool Aworking) const;
	bool working_subset_get() const { return working_subset; }
	void working_parent_subset_set(bool Aworking) const;
	bool working_parent_subset_get() const { return working_parent_subset; }

	const rom_by_name_set& rs_get() const { return rs; }
	const sample_by_name_set& ss_get() const { return ss; }
	const disk_by_name_set& ds_get() const { return ds; }
	rom_by_name_set& rs_get() { return rs; }
	sample_by_name_set& ss_get() { return ss; }
	disk_by_name_set& ds_get() { return ds; }
	void rs_remove_name(const rom_by_name_set& A) const;
	void rs_remove_crc(const rom_by_crc_set& A) const;
	void ss_remove_name(const sample_by_name_set& A) const;
	void ds_remove_name(const disk_by_name_set& A) const;

	void rzs_add(const infopath& Azip) const;
	void szs_add(const infopath& Azip) const;
	void dzs_add(const infopath& Azip) const;
	const zippath_container& rzs_get() const { return rzs; }
	const zippath_container& szs_get() const { return szs; }
	const zippath_container& dzs_get() const { return dzs; }

	string_container& rom_son_get() const { return rom_son; }

	bool is_romset_required() const;
	bool is_sampleset_required() const { return ss_get().begin() != ss_get().end(); }
	bool is_diskset_required() const { return ds_get().begin() != ds_get().end(); }

	bool has_good_rom() const;
	unsigned good_rom_size() const;
	std::string good_rom_get() const;
        bool has_bad_rom() const;
	bool has_usable_rom() const;

	bool has_good_sample() const;
	unsigned good_sample_size() const;
        bool has_bad_sample() const;

	bool has_good_disk() const;
	unsigned good_disk_size() const;
	bool has_bad_disk() const;

	unsigned size_get() const;
};

struct game_by_name_less {
	bool operator()(const game& A, const game& B) const {
		return A.name_get() < B.name_get();
	}
};

struct game_by_output_less {
	bool operator()(const game& A, const game& B) const {
		int r = A.cloneof_get().compare(B.cloneof_get());
		if (r<0) return true;
		if (r>0) return false;
		return A.size_get() < B.size_get();
	}
};

typedef std::set<game, game_by_name_less> game_by_name_set;
typedef std::set<game, game_by_output_less> game_by_output_set;

class gamearchive;

typedef bool filter_proc(const game& g);

class gamearchive {
	game_by_name_set map;

	// abstract
	gamearchive(const gamearchive&);
	gamearchive& operator=(const gamearchive&);

	bool load_info_internal();
	void load_info(std::istream& f);
	void load_xml(std::istream& f);

	bool is_game_parent(const game& g);
	bool game_working_subset_compute(const game& g);
	bool game_working_parent_subset_compute(const game& g);

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
	string_container find_working_clones(const game& A) const;
	bool has_working_clone_with_rom(const game& A) const;

	void load(std::istream& f);
	void filter(filter_proc* p);
};

#endif

