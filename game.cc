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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "game.h"
#include "readinfo.h"

using namespace std;

// ------------------------------------------------------------------------
// Game

game::game() {
	working = true;
}

game::game(const string& Aname) :
	name(Aname) {
	working = true;
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
	working(A.working) {
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

void game::working_set(bool Aworking) {
	working = Aworking;
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

// Check if a game is a neogeo game (use the neogeo BIOS)
bool is_game_neogeo(const game& g, const gamearchive& gar) {
	gamearchive::iterator romof = gar.find( g.romof_get() );
	while (romof != gar.end()) {
		if (romof->name_get() == "neogeo")
			return true;
		romof = gar.find( romof->romof_get() );
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

bool gamearchive::load(FILE* f) {
	info_init();
	bool r = load_internal(f);
	info_done();
	return r;
}

bool gamearchive::load_internal(FILE* f) {
	info_t token = info_token_get(f);
	while (token!=info_eof) {
		if (token != info_symbol) return false;
		if (strcmp(info_text_get(),"game")==0 || strcmp(info_text_get(),"resource")==0)	{
			if (strcmp(info_text_get(),"resource")==0) {
				int i = info_open;
				++i;
			}
			if (info_token_get(f) != info_open) return false;
			game g;
			token =	info_token_get(f);
			while (token != info_close) {
				if (token != info_symbol)
					return false;
				if (strcmp(info_text_get(),"name")==0) {
					if (info_token_get(f) != info_symbol) return false;
					g.name_set( info_text_get() );
				} else if (strcmp(info_text_get(),"description")==0) {
					if (info_token_get(f) != info_string) return false;
					g.description_set( info_text_get() );
				} else if (strcmp(info_text_get(),"manufacturer")==0) {
					if (info_token_get(f) != info_string) return false;
					g.manufacturer_set( info_text_get() );
				} else if (strcmp(info_text_get(),"year")==0) {
					if (info_token_get(f) != info_symbol) return false;
					g.year_set( info_text_get() );
				} else if (strcmp(info_text_get(),"cloneof")==0) {
					if (info_token_get(f) != info_symbol) return false;
					g.cloneof_set( info_text_get() );
				} else if (strcmp(info_text_get(),"romof")==0) {
					if (info_token_get(f) != info_symbol) return false;
					g.romof_set( info_text_get() );
				} else if (strcmp(info_text_get(),"sampleof")==0) {
					if (info_token_get(f) != info_symbol) return false;
					g.sampleof_set( info_text_get() );
				} else if (strcmp(info_text_get(),"rom")==0) {
					if (info_token_get(f) != info_open)  return false;
					rom r;
					token =	info_token_get(f);
					while (token != info_close) {
						if (token != info_symbol) return false;
						if (strcmp(info_text_get(),"name")==0) {
							if (info_token_get(f) != info_symbol) return false;
							r.name_set( info_text_get() );
						} else if (strcmp(info_text_get(),"size")==0) {
							if (info_token_get(f) != info_symbol) return false;
							r.size_set( strdec( info_text_get() ) );
						} else if (strcmp(info_text_get(),"crc")==0) {
							if (info_token_get(f) != info_symbol) return false;
							r.crc_set( strhex( info_text_get() ) );
						} else {
							if (info_skip_value(f) == info_error) return false;
						}
						token = info_token_get(f);
					}
					g.rs_get().insert( r );
				} else if (strcmp(info_text_get(),"driver")==0) {
					if (info_token_get(f) != info_open)  return false;
					token =	info_token_get(f);
					while (token != info_close) {
						if (token != info_symbol) return false;
						if (strcmp(info_text_get(),"status")==0) {
							if (info_token_get(f) != info_symbol) return false;
							g.working_set( strcmp( info_text_get(), "good") == 0 );
						} else {
							if (info_skip_value(f) == info_error) return false;
						}
						token = info_token_get(f);
					}
				} else if (strcmp(info_text_get(),"sample")==0) {
					if (info_token_get(f) != info_symbol) return false;
					sample s( info_text_get() );
					g.ss_get().insert( s );
				} else {
					if (info_skip_value(f) == info_error) return false;
				}
				token = info_token_get(f);
			}
			map.insert( g );
		} else {
			if (info_skip_value(f) == info_error) return false;
		}
		token = info_token_get(f);
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

	return true;
}
