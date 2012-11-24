/*
 * This file is part of the Advance project.
 *
 * Copyright (C) 2004 Andrea Mazzoleni
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

#include "disk.h"
#include "strcov.h"

using namespace std;

sha1::sha1()
{
	memset(hash, 0, 20);
}

sha1::sha1(const sha1& A)
{
	memcpy(hash, A.hash, 20);
}

sha1::sha1(const unsigned char* A)
{
	if (!A)
		throw error() << "Null hash";
	memcpy(hash, A, 20);
}

sha1::sha1(const string& A)
{
	if (A.length() != 40)
		throw error() << "Invalid hash length";

	const char* e;
	strvhex(hash, A.c_str(), &e);

	if (*e != 0)
		throw error() << "Invalid hash char '" << *e << "'";
}

bool sha1::operator==(const sha1& A) const
{
	return memcmp(hash, A.hash, 20) == 0;
}

sha1& sha1::operator=(const sha1& A)
{
	if (this != &A)
		memcpy(hash, A.hash, 20);

	return *this;
}

std::ostream& operator<<(std::ostream& os, const sha1& A)
{
	for(unsigned i=0;i<20;++i) {
		unsigned v;
		char c;
		v = (A.hash[i] >> 4) & 0xF;
		if (v < 10)
			c = '0' + v;
		else
			c = 'a' + v - 10;
		os << c;
		v = A.hash[i] & 0xF;
		if (v < 10)
			c = '0' + v;
		else
			c = 'a' + v - 10;
		os << c;
	}
	return os;
}

disk::disk()
{
}

disk::disk(const string& Aname)
	: name(Aname)
{
}

disk::disk(const disk& A)
	: name(A.name), hash(A.hash)
{
}

void disk::name_set(const string& Aname)
{
	name = Aname;
}

void disk::sha1_set(sha1 A)
{
	hash = A;
}

sha1 disk_sha1(const string& file)
{
	unsigned char header[120];
	unsigned version;
	unsigned offset;

	FILE* f = fopen(file.c_str(), "rb");
	if (!f)
		throw error() << "Failed open for read file " << file;

	if (fread(header, 120, 1, f)!=1) {
		fclose(f);
		throw error() << "Failed read file " << file;
	}

	fclose(f);

	if (memcmp(header + 0, "MComprHD", 8) != 0) {
		throw error_invalid() << "Invalid CHD file, missing head tag MComprHD";
	}

	version = header[15] + (header[14] << 8) + (header[13] << 16) + (header[12] << 24);

	switch (version) {
	case 3 : offset = 80; break;
	case 4 : offset = 48; break;
	case 5 : offset = 84; break;
	default : throw error_invalid() << "Unsupported CHD version " << version;
	}

	return sha1(header + offset);
}

