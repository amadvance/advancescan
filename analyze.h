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

#ifndef __ANALYZE_H
#define __ANALYZE_H

#include <set>
#include <string>

#include "game.h"

struct analyze_entry_static {
	const char* name;
	unsigned size;
	unsigned crc;
};

struct analyze_entry {
	std::string name;
	unsigned size;
	unsigned crc;

	analyze_entry(const std::string& Aname, unsigned size, unsigned crc);
	analyze_entry(const analyze_entry& A);
	analyze_entry(const analyze_entry_static& A);
	~analyze_entry();
};


bool operator<(const analyze_entry& A, const analyze_entry& B);

enum analyze_type {
	analyze_text,
	analyze_binary,
	analyze_garbage
};

typedef std::set<analyze_entry> analyze_set;

class analyze {
	analyze_set garbage;
public:
	analyze(const gamearchive& gar);

	analyze_type operator()(const std::string& name, unsigned size, unsigned crc) const;
};

#endif
