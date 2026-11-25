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

#include <algorithm>
#include <map>
#include <set>
#include <vector>

using namespace std;

game::game()
{
	resource = false;
	order = 0;
	merge_root = false;
	working = true;
	working_subset = false;
	working_parent_subset = false;
}

game::game(const string& Aname)
	: name(Aname)
{
	resource = false;
	order = 0;
	merge_root = false;
	working = true;
	working_subset = false;
	working_parent_subset = false;
}

game::game(const game& A)
	: rs(A.rs),
	mrs(A.mrs),
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
	order(A.order),
	merge_root(A.merge_root),
	merge_parent(A.merge_parent),
	merge_son(A.merge_son),
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

bool game::is_zipset_required() const
{
	// in the merged format the roms of a clone are in the zip of the parent
	if (merge_parent.length())
		return false;

	for(rom_by_name_set::const_iterator i=zs_get().begin();i!=zs_get().end();++i)
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

void game::order_set(unsigned Aorder)
{
	order = Aorder;
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
 * Remove disk by name and sha1.
 */
void game::ds_remove_name(const disk_by_name_set& A) const {
	disk_by_name_set B;
	for(disk_by_name_set::const_iterator i=ds_get().begin();i!=ds_get().end();++i) {
		disk_by_name_set::const_iterator j = A.find(*i);
		if (j == A.end() || !(j->sha1_get() == i->sha1_get())) {
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

	for(string_container::const_iterator i=g.rom_son_get().begin();i!=g.rom_son_get().end();++i) {
		gamearchive::iterator j = find(*i);
		if (j!=end()) {
			// a game with romof but without cloneof is not a real clone,
			// like "gngknight", and it's always processed as a parent
			if (!result || j->cloneof_get().length() == 0) {
				if (game_working_parent_subset_compute(*j))
					result = true;
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
#if 0 // recent MAME have many pf them, like "genpin", but also others
				cerr << "Missing definition of sampleof '" << i->sampleof_get() << "' for game '" << i->name_get() << "'." << endl;
#endif
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

/**
 * Filter the game list.
 * \param p Filter function. Games for which it returns false are removed.
 * \param keep_parent If set, the parents of all the clones kept are also kept,
 * even if removed by the filter. Used in the merged format where the roms
 * of the clones are stored in the zip of the parent.
 */
void gamearchive::filter(filter_proc* p, bool keep_parent)
{
	set<string> keep;

	for(const_iterator i=begin();i!=end();++i) {
		if (p(*i))
			keep.insert(i->name_get());
	}

	if (keep_parent) {
		set<string> parent;
		for(set<string>::const_iterator i=keep.begin();i!=keep.end();++i) {
			const_iterator g = find(*i);
			set<string> visited;
			visited.insert(g->name_get());
			while (g->cloneof_get().length()) {
				const_iterator j = find(g->cloneof_get());
				if (j == end() || j->resource_get() || visited.count(j->name_get()))
					break;
				visited.insert(j->name_get());
				parent.insert(j->name_get());
				g = j;
			}
		}
		keep.insert(parent.begin(), parent.end());
	}

	iterator i = map.begin();
	while (i != map.end()) {
		iterator j = i;
		++i;
		if (keep.find(j->name_get()) == keep.end()) {
			map.erase(j);
		}
	}
}

static bool game_by_order_less(const game* A, const game* B)
{
	if (A->order_get() != B->order_get())
		return A->order_get() < B->order_get();
	return A->name_get() < B->name_get();
}

static bool rom_by_order_less(const rom& A, const rom& B)
{
	if (A.order_get() != B.order_get())
		return A.order_get() < B.order_get();
	return file_compare(A.name_get(), B.name_get()) < 0;
}

/**
 * Convert the rom sets to the merged format.
 *
 * The roms of the clones are stored in the zip of the parent, in a
 * subdirectory with the same name of the clone. The parent roms are stored
 * without subdirectory.
 * A clone rom with the same crc and size of a rom already present in the parent,
 * or in a previous clone, isn't stored again. The clones are processed
 * in the order of the information file.
 *
 * Must be called after load() and filter().
 */
void gamearchive::merge()
{
	typedef std::map<string, vector<const game*> > family_map;
	family_map family;

	// find the root parent of every clone
	for(const_iterator i=begin();i!=end();++i) {
		if (i->resource_get())
			continue;

		const game* root = &*i;
		set<string> visited;
		visited.insert(root->name_get());
		while (root->cloneof_get().length()) {
			const_iterator j = find(root->cloneof_get());
			if (j == end()) {
				if (root == &*i)
					cerr << "Missing definition of cloneof '" << root->cloneof_get() << "' for game '" << root->name_get() << "'." << endl;
				break;
			}
			if (j->resource_get())
				break;
			if (visited.count(j->name_get())) {
				cerr << "Circular definition of cloneof '" << root->cloneof_get() << "' for game '" << root->name_get() << "'." << endl;
				break;
			}
			visited.insert(j->name_get());
			root = &*j;
		}

		if (root != &*i)
			family[root->name_get()].push_back(&*i);
	}

	// every game not merged in another has its own zip
	for(const_iterator i=begin();i!=end();++i) {
		i->merge_root = false;
		i->merge_parent = string();
		i->merge_son.clear();
		i->mrs.clear();
	}

	for(family_map::iterator f=family.begin();f!=family.end();++f) {
		for(vector<const game*>::const_iterator c=f->second.begin();c!=f->second.end();++c) {
			(*c)->merge_parent = f->first;
		}
	}

	for(const_iterator i=begin();i!=end();++i) {
		if (i->merge_parent.length())
			continue;

		i->merge_root = true;
		i->mrs = i->rs;

		family_map::iterator f = family.find(i->name_get());
		if (f == family.end())
			continue;

		// roms already stored
		rom_by_crc_set stored;
		for(rom_by_name_set::const_iterator j=i->rs.begin();j!=i->rs.end();++j) {
			if (!j->nodump_get())
				stored.insert(*j);
		}

		vector<const game*>& clone = f->second;
		sort(clone.begin(), clone.end(), game_by_order_less);

		for(vector<const game*>::const_iterator c=clone.begin();c!=clone.end();++c) {
			i->merge_son.insert(i->merge_son.end(), (*c)->name_get());

			vector<rom> rl((*c)->rs.begin(), (*c)->rs.end());
			sort(rl.begin(), rl.end(), rom_by_order_less);

			for(vector<rom>::const_iterator j=rl.begin();j!=rl.end();++j) {
				rom r = *j;
				r.name_set((*c)->name_get() + "/" + j->name_get());

				if (!j->nodump_get()) {
					if (stored.find(*j) != stored.end())
						continue;
					stored.insert(*j);
				}

				i->mrs.insert(r);
			}
		}
	}
}

