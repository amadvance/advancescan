/*
 * This file is part of the Advance project.
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

#include "gamediff.h"

#include "lib/readinfo.h"

#include <iomanip>

// ------------------------------------------------------------------------
// rom

rom::rom() {
}

rom::rom(const rom& A)
	: name(A.name), size(A.size), crc(A.crc) {
}

bool rom::operator<(const rom& A) const {
	if (crc_get() < A.crc_get())
		return true;
	if (crc_get() > A.crc_get())
		return false;
	return size_get() < A.size_get();
}

bool rom::operator==(const rom& A) const {
	return crc_get() == A.crc_get() && size_get() == A.size_get();
}

std::ostream& operator<<(std::ostream& os, const rom& A) {
	os << "rom ( name " << A.name_get() << " size " << std::dec << A.size_get() << " crc " << std::hex << A.crc_get() << " )";
	return os;
}

bool include(const rom_set& A, const rom_set& B) {
	for(rom_set::const_iterator i=B.begin();i!=B.end();++i) {
		rom_set::const_iterator j = A.find(*i);
		if (j==A.end())
			return false;
	}
	return true;
}

bool equal(const rom_set& A, const rom_set& B) {
	if (A.size() != B.size())
		return false;
	rom_set::const_iterator i,j;
	for(i=B.begin(),j=A.begin();i!=B.end();++i) {
		if (*i != *j)
			return false;
	}
	return true;
}

// ------------------------------------------------------------------------
// game

game::game() {
}

game::game(const game& A)
	: name(A.name), roms(A.roms) {
}

bool game::operator<(const game& A) const {
	return name_get() < A.name_get();
}

std::ostream& operator<<(std::ostream &os, const game& A) {
	os << "game (" << std::endl;
	os << "\tname " << A.name_get() << std::endl;
	for(rom_set::const_iterator i=A.roms_get().begin();i!=A.roms_get().end();++i) 
		os << "\t" << *i << std::endl;
	os << ")" << std::endl;
	return os;
}

bool game_set_load::load(FILE* f, bool remove_merge) {
	info_t token = info_token_get(f);
	while (token!=info_eof) {
		if (token != info_symbol) return false;
		if (strcmp(info_text_get(),"game")==0) {
			if (info_token_get(f) != info_open) return false;
			game g;
			token = info_token_get(f);
			while (token != info_close) {
				if (token != info_symbol)
					return false;
				if (strcmp(info_text_get(),"name")==0) {
					if (info_token_get(f) != info_symbol) return false;
					g.name_set( info_text_get() );
				} else if (strcmp(info_text_get(),"rom")==0) {
					if (info_token_get(f) != info_open) return false;
					token = info_token_get(f);
					rom r;
					bool merge = false;
					while (token != info_close) {
						if (token != info_symbol) return false;
						if (strcmp(info_text_get(),"size")==0) {
							if (info_token_get(f) != info_symbol) return false;
							r.size_set(atoi( info_text_get() ));				
						} else if (strcmp(info_text_get(),"crc")==0 || strcmp(info_text_get(),"crc32")==0) {
							if (info_token_get(f) != info_symbol) return false;
							r.crc_set(strtoul(info_text_get(), 0, 16));
						} else if (strcmp(info_text_get(),"name")==0) {
							if (info_token_get(f) != info_symbol) return false;
							r.name_set(info_text_get());
						} else if (strcmp(info_text_get(),"merge")==0) {
							if (info_token_get(f) != info_symbol) return false;
							merge = true;
						} else {
							if (info_skip_value(f) == info_error) return false;
						}
						token = info_token_get(f);
					}
					if (!(remove_merge && merge))
						g.roms_get().insert(r);
				} else {
					if (info_skip_value(f) == info_error) return false;
				}
				token = info_token_get(f);
			}
			insert( g );
		} else {
			if (info_skip_value(f) == info_error)
				return false;
		}
		token = info_token_get(f);
	}

	return true;
}
