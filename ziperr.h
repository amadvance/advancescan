/*
 * This file is part of the AdvanceSCAN project.
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

#ifndef __ZIPERROR_H
#define __ZIPERROR_H

#include <string>
#include <sstream>

class error {
	std::string function;
	std::string file;
	unsigned line;
	std::string desc;
public:
	error() : line(0)
	{
	}

	error(const char* Afunction, const char* Afile, unsigned Aline) : function(Afunction), file(Afile), line(Aline)
	{
	}

	const std::string& desc_get() const
	{
		return desc;
	}

	const std::string& function_get() const
	{
		return function;
	}

	const std::string& file_get() const
	{
		return file;
	}

	unsigned line_get() const
	{
		return line;
	}

	error& operator<<(const char* A)
	{
		desc += A;
		return *this;
	}

	error& operator<<(const std::string& A)
	{
		desc += A;
		return *this;
	}

	error& operator<<(const unsigned A)
	{
		std::ostringstream s;
		s << A;
		desc += s.str();
		return *this;
	}

};

#define error() \
	error(__PRETTY_FUNCTION__,__FILE__,__LINE__)

static inline std::ostream& operator<<(std::ostream& os, const error& e) {
	os << e.desc_get();
#ifndef NDEBUG
	os << " [at " << e.function_get() << ":" << e.file_get() << ":" << e.line_get() << "]";
#endif

	return os;
}

#endif
