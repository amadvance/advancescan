/*
 * This file is part of the Advance project.
 *
 * Copyright (C) 2002, 2004 Andrea Mazzoleni
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

#include "token.h"

using namespace std;

string strip_space(const string& s)
{
	string r = s;
	while (r.length() && isspace(r[0]))
		r.erase(0, 1);
	while (r.length() && isspace(r[r.length()-1]))
		r.erase(r.length()-1, 1);
	return r;
}

string token_get(const string& s, unsigned& ptr, const char* sep)
{
	unsigned start = ptr;
	while (ptr < s.length() && strchr(sep, s[ptr])==0)
		++ptr;
	return string(s, start, ptr-start);
}

void token_skip(const string& s, unsigned& ptr, const char* sep)
{
	while (ptr < s.length() && strchr(sep, s[ptr])!=0)
		++ptr;
}

std::string token_get(const std::string& s, unsigned& ptr, char sep)
{
	char sep_string[2];
	sep_string[0] = sep;
	sep_string[1] = 0;
	return token_get(s, ptr, sep_string);
}

void token_skip(const std::string& s, unsigned& ptr, char sep)
{
	char sep_string[2];
	sep_string[0] = sep;
	sep_string[1] = 0;
	token_skip(s, ptr, sep_string);
}


