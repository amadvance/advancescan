/*
 * This file is part of the Advance project.
 *
 * Copyright (C) 1998, 1999, 2000, 2001, 2002 Andrea Mazzoleni
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

#ifndef __SAMPLE_H
#define __SAMPLE_H

#include "file.h"

#include <set>

class sample {
	std::string name;
public:
	sample(const std::string& Aname);
	sample(const sample&);
	~sample();

	const std::string& name_get() const { return name; }
};

struct sample_by_name_less {
	bool operator()(const sample& A, const sample& B) const {
		return file_compare(A.name_get(), B.name_get()) < 0;
	}
};

typedef std::set<sample, sample_by_name_less> sample_by_name_set;

#endif

