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

#ifndef __DISK_H
#define __DISK_H

#include "file.h"

#include <set>
#include <iostream>

class sha1 {
	unsigned char hash[20];

	friend std::ostream& operator<<(std::ostream& os, const sha1& A);
public:
	sha1();
	sha1(const sha1& A);
	sha1(const unsigned char* A);
	sha1(const std::string& A);

	bool operator==(const sha1& A) const;
	sha1& operator=(const sha1& A);
};

std::ostream& operator<<(std::ostream& os, const sha1& A);

class disk {
	std::string name;
	sha1 hash;
public:
	disk();
	disk(const std::string& Aname);
	disk(const disk& A);

	const sha1& sha1_get() const { return hash; }
	void sha1_set(sha1 Asha1);

	const std::string& name_get() const { return name; }
	void name_set(const std::string& Aname);
};

struct disk_by_name_less {
	bool operator()(const disk& A, const disk& B) const {
		return file_compare(A.name_get(), B.name_get()) < 0;
	}
};

typedef std::set<disk, disk_by_name_less> disk_by_name_set;

sha1 disk_sha1(const std::string& file);

#endif

