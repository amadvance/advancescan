/*
 * This file is part of the Advance project.
 *
 * Copyright (C) 2002, 2003 Andrea Mazzoleni
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

#include "analyze.h"
#include "strcov.h"
#include "file.h"

using namespace std;

analyze_entry_static GARBAGE[] = {

#include "analyze.dat"

{ 0, 0, 0 }
};

analyze_entry::analyze_entry(const string& Aname, unsigned Asize, unsigned Acrc)
	: name(Aname), size(Asize), crc(Acrc) {
}

analyze_entry::analyze_entry(const analyze_entry& A)
	: name(A.name), size(A.size), crc(A.crc) {
}

analyze_entry::analyze_entry(const analyze_entry_static& A)
	: name(A.name), size(A.size), crc(A.crc) {
}

analyze_entry::~analyze_entry()
{
}

bool operator<(const analyze_entry& A, const analyze_entry& B)
{
	if (A.size < B.size)
		return true;
	if (A.size > B.size)
		return false;

	if (A.crc < B.crc)
		return true;
	if (A.crc > B.crc)
		return false;

	return A.name < B.name;;
}

// Check if a file is a text file, heuristic
const char* TEXT_WILD[] = {
"*.txt",
"*.doc",
"*.nfo",
"*.diz",
"readme.*",
0
};

// Check if a file is a binary file, heuristic
const char* BIN_WILD[] = {
"*.sam",
"*.wav",
0
};

static bool heuristic_is_text(const string& name, unsigned size, unsigned crc)
{
	// check name for binary
	for(unsigned i=0;BIN_WILD[i];++i) {
		if (striwildcmp(BIN_WILD[i], name.c_str())==0)
			return false;
	}

	// check name for text
	for(unsigned i=0;TEXT_WILD[i];++i) {
		if (striwildcmp(TEXT_WILD[i], name.c_str())==0)
			return true;
	}

	// check if it's empty
	if (size==0)
		return false;

	// check if the size is a power of 2
	unsigned one = 0;
	while (size) {
		if (size % 2)
			++one;
		size /= 2;
	}
	if (one==1)
		return false;

	// Note: The ZIP ASCII flag is not checked because some readme
	// contain the CTRL-Z char as end-of-file and result as a binary file

	return true;
}

analyze::analyze(const gamearchive& gar)
{
	// insert some standard crc
	for(analyze_entry_static* i=GARBAGE;i->size;++i) {
		garbage.insert(*i);
	}
}

analyze_type analyze::operator()(const string& name, unsigned size, unsigned crc) const
{
	if (!heuristic_is_text(name, size, crc))
		return analyze_binary;

	analyze_entry_static t;
	t.name = name.c_str();
	t.size = size;
	t.crc = crc;

	analyze_set::const_iterator i = garbage.find(t);
	if (i != garbage.end())
		return analyze_garbage;
	else
		return analyze_text;
}

