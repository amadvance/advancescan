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

#include "portable.h"

#include "game.h"

using namespace std;

game::game()
{
	resource = false;
	working = true;
	working_subset = false;
	working_parent_subset = false;
}

game::game(const string& Aname)
	: name(Aname)
{
	resource = false;
	working = true;
	working_subset = false;
	working_parent_subset = false;
}

game::game(const game& A)
	: rs(A.rs),
	ss(A.ss),
	ds(A.ds),
	rzs(A.rzs), 
	szs(A.szs),
	dzs(A.dzs),
	name(A.name), 
	romof(A.romof), 
	cloneof(A.cloneof), 
	sampleof(A.sampleof),
	description(A.description), 
	year(A.year), 
	manufacturer(A.manufacturer),
	resource(A.resource),
	working(A.working),
	working_subset(A.working_subset),
	working_parent_subset(A.working_parent_subset),
	rom_son(A.rom_son)
{
}

game::~game()
{
}

bool game::is_romset_required() const
{
	for(rom_by_name_set::const_iterator i=rs_get().begin();i!=rs_get().end();++i)
		if (i->size_get() != 0 && !i->nodump_get())
			return true;
	return false;
}

unsigned game::good_rom_size() const
{
	unsigned size = 0;
	for(zippath_container::const_iterator i=rzs_get().begin();i!=rzs_get().end();++i)
		if (i->good_get() && (!size || i->size_get() < size))
			size = i->size_get();
	return size;
}

bool game::has_good_rom() const
{
	for(zippath_container::const_iterator i=rzs_get().begin();i!=rzs_get().end();++i)
		if (i->good_get())
			return true;
	return false;
}

string game::good_rom_get() const
{
	for(zippath_container::const_iterator i=rzs_get().begin();i!=rzs_get().end();++i)
		if (i->good_get())
			return i->file_get();
	return "";
}

bool game::has_bad_rom() const
{
	for(zippath_container::const_iterator i=rzs_get().begin();i!=rzs_get().end();++i)
		if (!i->good_get())
			return true;
	return false;
}

bool game::has_usable_rom() const
{
	for(zippath_container::const_iterator i=rzs_get().begin();i!=rzs_get().end();++i) {
		if (i->good_get())
			return true;
		if (!i->good_get() && !i->readonly_get())
			return true;
	}
	return false;
}

bool game::has_good_sample() const
{
	for(zippath_container::const_iterator i=szs_get().begin();i!=szs_get().end();++i)
		if (i->good_get())
			return true;
	return false;
}

unsigned game::good_sample_size() const
{
	unsigned size = 0;
	for(zippath_container::const_iterator i=szs_get().begin();i!=szs_get().end();++i)
		if (i->good_get() && (!size || i->size_get() < size))
			size = i->size_get();
	return size;
}

bool game::has_bad_sample() const
{
	for(zippath_container::const_iterator i=szs_get().begin();i!=szs_get().end();++i)
		if (!i->good_get())
			return true;
	return false;
}

bool game::has_good_disk() const
{
	for(disk_by_name_set::const_iterator i=ds_get().begin();i!=ds_get().end();++i) {
		zippath_container::const_iterator j;
		for(j=dzs_get().begin();j!=dzs_get().end();++j) {
			string name = file_basename(j->file_get());
			if (name == i->name_get()) {
				if (j->good_get())
					break;
			}
		}

		if (j == dzs_get().end())
			return false;
	}

	return true;
}

unsigned game::good_disk_size() const
{
	unsigned size = 0;

	for(disk_by_name_set::const_iterator i=ds_get().begin();i!=ds_get().end();++i) {
		zippath_container::const_iterator j;
		for(j=dzs_get().begin();j!=dzs_get().end();++j) {
			string name = file_basename(j->file_get());
			if (name == i->name_get()) {
				if (j->good_get())
					size += j->size_get();
			}
		}
	}

	return size;
}

bool game::has_bad_disk() const
{
	for(zippath_container::const_iterator i=dzs_get().begin();i!=dzs_get().end();++i)
		if (!i->good_get())
			return true;
	return false;
}

void game::rzs_add(const infopath& Azip) const
{
	rzs.insert(rzs.end(), Azip);
}

void game::szs_add(const infopath& Azip) const
{
	szs.insert(szs.end(), Azip);
}

void game::dzs_add(const infopath& Azip) const
{
	dzs.insert(dzs.end(), Azip);
}

void game::working_set(bool Aworking) const
{
	working = Aworking;
}

void game::working_subset_set(bool Aworking) const
{
	working_subset = Aworking;
}

void game::working_parent_subset_set(bool Aworking) const
{
	working_parent_subset = Aworking;
}

void game::name_set(const string& Aname)
{
	name = Aname;
}

void game::cloneof_set(const string& Acloneof)
{
	cloneof = Acloneof;
}

void game::romof_set(const string& Aromof)
{
	romof = Aromof;
}

void game::sampleof_set(const string& Asampleof)
{
	sampleof = Asampleof;
}

void game::description_set(const string& Adescription)
{
	description = Adescription;
}

void game::year_set(const string& Ayear)
{
	year = Ayear;
}

void game::manufacturer_set(const string& Amanufacturer)
{
	manufacturer = Amanufacturer;
}

void game::resource_set(bool Aresource)
{
	resource = Aresource;
}

/**
 * Remove roms by name.
 */
void game::rs_remove_name(const rom_by_name_set& A) const {
	rom_by_name_set B;
	for(rom_by_name_set::const_iterator i=rs_get().begin();i!=rs_get().end();++i) {
		rom_by_name_set::const_iterator j = A.find(*i);
		if (j == A.end() || j->crc_get()!=i->crc_get() || j->size_get()!=i->size_get()) {
			B.insert(*i);
		}
	}
	rs = B;
}

/**
 * Remove roms by crc.
 */
void game::rs_remove_crc(const rom_by_crc_set& A) const {
	rom_by_name_set B;
	for(rom_by_name_set::const_iterator i=rs_get().begin();i!=rs_get().end();++i) {
		rom_by_crc_set::const_iterator j = A.find(*i);
		if (j == A.end()) {
			B.insert(*i);
		}
	}
	rs = B;
}

/**
 * Remove sample by name.
 */
void game::ss_remove_name(const sample_by_name_set& A) const {
	sample_by_name_set B;
	for(sample_by_name_set::const_iterator i=ss_get().begin();i!=ss_get().end();++i) {
		sample_by_name_set::const_iterator j = A.find(*i);
		if (j == A.end()) {
			B.insert(*i);
		}
	}
	ss = B;
}

/**
 * Remove disk by name.
 */
void game::ds_remove_name(const disk_by_name_set& A) const {
	disk_by_name_set B;
	for(disk_by_name_set::const_iterator i=ds_get().begin();i!=ds_get().end();++i) {
		disk_by_name_set::const_iterator j = A.find(*i);
		if (j == A.end()) {
			B.insert(*i);
		}
	}
	ds = B;
}

/**
 * Return the size of the romlist.
 */
unsigned game::size_get() const {
	unsigned size = 0;
	for(rom_by_name_set::const_iterator i=rs_get().begin();i!=rs_get().end();++i) {
		size += i->size_get();
	}
	return size;
}

gamearchive::gamearchive()
{
}

gamearchive::~gamearchive()
{
}

gamearchive::const_iterator gamearchive::find(const game& A) const
{
	return map.find(A);
}

bool gamearchive::is_game_parent(const game& g)
{
	const_iterator j = find(g.romof_get());
	if (j == end())
		return true;
	if (j->resource_get())
		return true;
	return false;
}

/**
 * Compute for the subtree the "working" relationship.
 * Assume to have all the "working" flags set to false and
 * to be called only for parents and not for resources or sons.
 */
bool gamearchive::game_working_subset_compute(const game& g)
{
	bool result = false;

	if (g.working_get())
		result = true;

	for(string_container::const_iterator i=g.rom_son_get().begin();i!=g.rom_son_get().end();++i) {
		gamearchive::iterator j = find(*i);
		if (j!=end()) {
			if (game_working_subset_compute(*j))
				result = true;
		}
	}

	if (result)
		g.working_subset_set(true);

	return result;
}

/**
 * Compute for the subtree the "working_parent" relationship.
 * Assume to have all the "working_parent" flags set to false and
 * to be called only for parents and not for resources or sons.
 */
bool gamearchive::game_working_parent_subset_compute(const game& g)
{
	bool result = false;

	if (g.working_get()) {
		result = true;
	}

	if (!result) {
		for(string_container::const_iterator i=g.rom_son_get().begin();i!=g.rom_son_get().end();++i) {
			gamearchive::iterator j = find(*i);
			if (j!=end()) {
				result = game_working_parent_subset_compute(*j);
				if (result)
					break;
			}
		}
	}

	if (result)
		g.working_parent_subset_set(true);

	return result;
}

string_container gamearchive::find_working_clones(const game& g) const
{
	string_container r;

	if (g.working_get())
		r.insert(r.end(), g.name_get());

	for(string_container::const_iterator i=g.rom_son_get().begin();i!=g.rom_son_get().end();++i) {
		gamearchive::iterator j = find(*i);
		if (j!=end()) {
			string_container s = find_working_clones(*j);
			for(string_container::const_iterator k=s.begin();k!=s.end();++k) {
				r.insert(r.end(), *k);
			}
		}
	}

	return r;
}

bool gamearchive::has_working_clone_with_rom(const game& g) const
{
	for(string_container::const_iterator i=g.rom_son_get().begin();i!=g.rom_son_get().end();++i) {
		gamearchive::iterator j = find(*i);
		if (j!=end()) {
			if (j->working_get() && j->has_good_rom())
				return true;
			if (has_working_clone_with_rom(*j))
				return true;
		}
	}

	return false;
}

void gamearchive::load(istream& f)
{
	int c;

	c = f.get();
	while (c != EOF && isspace(c))
		c = f.get();

	if (c != EOF)
		f.putback(c);

	if (c == '<') {
		load_xml(f);
	} else {
		load_info(f);
	}

	// reduce, eliminate merged rom and sample
	for(iterator i=begin();i!=end();++i) {
		// rom/disk
		string romof = i->romof_get();
		while (romof.length()) {
			iterator j = find(game(romof));
			if (j!=end()) {
				// if itself, breaks now
				if (romof == i->name_get())
					break;

				// remove merged stuff
				rom_by_crc_set A;
				for(rom_by_name_set::const_iterator k=j->rs_get().begin();k!=j->rs_get().end();++k) {
					A.insert(*k);
				}
				i->rs_remove_crc(A);

				disk_by_name_set B;
				for(disk_by_name_set::const_iterator k=j->ds_get().begin();k!=j->ds_get().end();++k) {
					B.insert(*k);
				}
				i->ds_remove_name(B);

				romof = j->romof_get();
				if (romof == j->name_get())
					break;
			} else
				break;
		}

		// sample
		string sampleof = i->sampleof_get();
		while (sampleof.length()) {
			iterator j = find(game(sampleof));
			if (j!=end()) {
				// if itself, breaks now
				if (sampleof == i->name_get())
					break;

				// remove merged stuff
				i->ss_remove_name(j->ss_get());

				sampleof = j->sampleof_get();
				if (sampleof == j->name_get())
					break;
			} else
				break;
		}
	}

	// test romof relationship and adjust it
	for(iterator i=begin();i!=end();++i) {
		if (i->romof_get().length() != 0) {
			const_iterator j = find(i->romof_get());
			if (j == end()) {
				cerr << "Missing definition of romof '" << i->romof_get() << "' for game '" << i->name_get() << "'." << endl;
				(const_cast<game*>((&*i)))->romof_set(string());
			} else if (j->name_get() == i->name_get()) {
				cerr << "Self definition of romof '" << i->romof_get() << "' for game '" << i->name_get() << "'." << endl;
				(const_cast<game*>((&*i)))->romof_set(string());
			}
		}
	}

	// test sampleof relationship and adjust it
	for(iterator i=begin();i!=end();++i) {
		if (i->sampleof_get().length() != 0) {
			const_iterator j = find(i->sampleof_get());
			if (j == end()) {
				cerr << "Missing definition of sampleof '" << i->sampleof_get() << "' for game '" << i->name_get() << "'." << endl;
				(const_cast<game*>((&*i)))->sampleof_set(string());
			} else if (j->name_get() == i->name_get()) {
#if 0 // they are normal
				cerr << "Self definition of romof '" << i->sampleof_get() << "' for game '" << i->name_get() << "'." << endl;
#endif
				(const_cast<game*>((&*i)))->sampleof_set(string());
			}
		}
	}

	// compute the rom_son container
	for(iterator i=begin();i!=end();++i) {
		iterator j = find(game(i->romof_get()));
		if (j!=end()) {
			j->rom_son_get().insert(j->rom_son_get().end(), i->name_get());
		}
	}

	// compute the working subset info
	for(iterator i=begin();i!=end();++i) {
		if (i->resource_get()) {
			i->working_subset_set(true);
		} else if (is_game_parent(*i)) {
			game_working_subset_compute(*i);
		}
	}

	// compute the working parent subset info
	for(iterator i=begin();i!=end();++i) {
		if (i->resource_get()) {
			i->working_parent_subset_set(true);
		} else if (is_game_parent(*i)) {
			game_working_parent_subset_compute(*i);
		}
	}
}

void gamearchive::filter(filter_proc* p)
{
	iterator i;

	i = map.begin();
	while (i != map.end()) {
		iterator j = i;
		++i;
		if (!p(*j)) {
			map.erase(j);
		}
	}
}

