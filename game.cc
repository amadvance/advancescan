/*
 * This file is part of the Advance project.
 *
 * Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Andrea Mazzoleni
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

// ------------------------------------------------------------------------
// Game

game::game() {
	working = true;
	working_tree = true;
}

game::game(const string& Aname) :
	name(Aname) {
	working = true;
	working_tree = true;
}

game::game(const game& A) : 
	rs(A.rs), 
	ss(A.ss), 
	rzs(A.rzs), 
	szs(A.szs),
	name(A.name), 
	romof(A.romof), 
	cloneof(A.cloneof), 
	sampleof(A.sampleof),
	description(A.description), 
	year(A.year), 
	manufacturer(A.manufacturer),
	working(A.working),
	working_tree(A.working_tree),
	rom_son(A.rom_son) {
}

game::~game() {
}

unsigned game::good_romzip_size() const {
	unsigned size = 0;
	for(zippath_container::const_iterator i=rzs_get().begin();i!=rzs_get().end();++i)
		if (i->good_get() && (!size || i->size_get() < size))
			size = i->size_get();
	return size;
}

bool game::good_romzip_has() const {
	for(zippath_container::const_iterator i=rzs_get().begin();i!=rzs_get().end();++i)
		if (i->good_get())
			return true;
	return false;
}

string game::good_romzip_get() const {
	for(zippath_container::const_iterator i=rzs_get().begin();i!=rzs_get().end();++i)
		if (i->good_get())
			return i->file_get();
	return "";
}

bool game::bad_romzip_has() const {
	for(zippath_container::const_iterator i=rzs_get().begin();i!=rzs_get().end();++i)
		if (!i->good_get())
			return true;
	return false;
}

bool game::usable_romzip_has() const {
	for(zippath_container::const_iterator i=rzs_get().begin();i!=rzs_get().end();++i) {
		if (i->good_get())
			return true;
		if (!i->good_get() && !i->readonly_get())
			return true;
	}
	return false;
}

bool game::good_samplezip_has() const {
	for(zippath_container::const_iterator i=szs_get().begin();i!=szs_get().end();++i)
		if (i->good_get())
			return true;
	return false;
}

unsigned game::good_samplezip_size() const {
	unsigned size = 0;
	for(zippath_container::const_iterator i=szs_get().begin();i!=szs_get().end();++i)
		if (i->good_get() && (!size || i->size_get() < size))
			size = i->size_get();
	return size;
}

bool game::bad_samplezip_has() const {
	for(zippath_container::const_iterator i=szs_get().begin();i!=szs_get().end();++i)
		if (!i->good_get())
			return true;
	return false;
}

void game::rzs_add(const zippath& Azip) const {
	rzs.insert( rzs.end(), Azip );
}

void game::szs_add(const zippath& Azip) const {
	szs.insert( szs.end(), Azip );
}

void game::working_set(bool Aworking) const {
	working = Aworking;
}

void game::working_tree_set(bool Aworking) const {
	working_tree = Aworking;
}

void game::name_set(const string& Aname) {
	name = Aname;
}

void game::cloneof_set(const string& Acloneof) {
	cloneof = Acloneof;
}

void game::romof_set(const string& Aromof) {
	romof = Aromof;
}

void game::sampleof_set(const string& Asampleof) {
	sampleof = Asampleof;
}

void game::description_set(const string& Adescription) {
	description = Adescription;
}

void game::year_set(const string& Ayear) {
	year = Ayear;
}

void game::manufacturer_set(const string& Amanufacturer) {
	manufacturer = Amanufacturer;
}

// Remove roms by name
void game::rs_remove_name(const rom_by_name_set& A) const {
	rom_by_name_set B;
	for(rom_by_name_set::const_iterator i=rs_get().begin();i!=rs_get().end();++i) {
		rom_by_name_set::const_iterator j = A.find( *i );
		if (j == A.end() || j->crc_get()!=i->crc_get() || j->size_get()!=i->size_get()) {
			B.insert( *i );
		}
	}
	rs = B;
}

// Remove roms by crc
void game::rs_remove_crc(const rom_by_crc_set& A) const {
	rom_by_name_set B;
	for(rom_by_name_set::const_iterator i=rs_get().begin();i!=rs_get().end();++i) {
		rom_by_crc_set::const_iterator j = A.find( *i );
		if (j == A.end()) {
			B.insert( *i );
		}
	}
	rs = B;
}

// Remove samples by name
void game::ss_remove_name(const sample_by_name_set& A) const {
	sample_by_name_set B;
	for(sample_by_name_set::const_iterator i=ss_get().begin();i!=ss_get().end();++i) {
		sample_by_name_set::const_iterator j = A.find( *i );
		if (j == A.end()) {
			B.insert( *i );
		}
	}
	ss = B;
}

// Return the size of the romlist
unsigned game::size_get() const {
	unsigned size = 0;
	for(rom_by_name_set::const_iterator i=rs_get().begin();i!=rs_get().end();++i) {
		size += i->size_get();
	}
	return size;
}

// ------------------------------------------------------------------------
// Game archive

bool is_game_working_tree(const game& g, const gamearchive& gar) {
	if (g.working_get())
		return true;
	for(string_container::const_iterator i=g.rom_son_get().begin();i!=g.rom_son_get().end();++i) {
		gamearchive::iterator j = gar.find( *i );
		if (j!=gar.end()) {
			if (is_game_working_tree(*j, gar))
				return true;
		}
	}
	return false;
}

gamearchive::gamearchive() {
}

gamearchive::~gamearchive() {
}

gamearchive::const_iterator gamearchive::find(const game& A) const {
	return map.find( A );
}

void gamearchive::load(istream& f) {
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
		// rom
		string romof = i->romof_get();
		while (romof.length()) {
			iterator j = find( game( romof ) );
			if (j!=end()) {
				rom_by_crc_set A;
				for(rom_by_name_set::const_iterator k=j->rs_get().begin();k!=j->rs_get().end();++k) {
					A.insert( *k );
				}
				i->rs_remove_crc( A );
				romof = j->romof_get();
			} else
				break;
		}
		// sample
		string sampleof = i->sampleof_get();
		while (sampleof.length()) {
			iterator j = find( game( sampleof ) );
			if (j!=end()) {
				i->ss_remove_name( j->ss_get() );
				sampleof = j->sampleof_get();
			} else
				break;
		}
	}

	// test romof relationship and compute the parent
	for(iterator i=begin();i!=end();++i) {
		i->rom_son_erase();

		if (i->romof_get().length() != 0) {
			const_iterator j = find( i->romof_get() );
			if (j == end()) {
				cerr << "Missing definition of romof '" << i->romof_get() << "' for game '" << i->name_get() << "'." << endl;
				(const_cast<game*>((&*i)))->romof_set(string());
			} else {
				while (j != end()) {
					if (&*j == &*i) {
						cerr << "Circular romof reference for game '" << i->name_get() << "'." << endl;
						(const_cast<game*>((&*i)))->romof_set(string());
						break;
					}
					j = find( j->romof_get() );
				}
			}
		}
	}

	// compute the romof list
	for(iterator i=begin();i!=end();++i) {
		iterator j = find( game( i->romof_get() ) );
		if (j!=end()) {
			j->rom_son_get().insert(j->rom_son_get().end(), i->name_get());
		}
	}

	// compute the working tree info
	for(iterator i=begin();i!=end();++i) {
		i->working_tree_set( is_game_working_tree(*i, *this));
	}
}

void gamearchive::filter(filter_proc* p) {
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

