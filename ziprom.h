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

#ifndef __ZIPROM_H
#define __ZIPROM_H

#include "zip.h"
#include "rom.h"

// ------------------------------------------------------------------------
// Zip archive

enum zip_type {
	zip_own, // roms part of the set
	zip_import, // roms of other sets used for importing
	zip_unknow // roms unknow
};

// Zip with readonly flag
class ziprom : public zip {
	zip_type type;
	bool readonly;

	void move(const std::string& zipintname_src, ziprom& dst, const std::string& zipintname_dst);

	// abstract;
	ziprom();
public:
	ziprom(const std::string& Apath, zip_type Atype, bool Areadonly);
	ziprom(const ziprom& A);
	~ziprom();

	bool is_readonly() const { return readonly; }
	zip_type type_get() const { return type; }

	void open();

	ziprom::iterator find(const std::string& name);
	void load();
	void unload();
	void save();

	void crc(const std::string& zipintname, crc_t& crc);

	void remove(const std::string& zipintname);
	void remove(const std::string& zipintname, ziprom& reject);
	void add(const ziprom::const_iterator& entry_src, const std::string& zipintname_dst);
	void add(const ziprom::const_iterator& entry_src, const std::string& zipintname_dst, ziprom& reject);
	void add(const std::string& zipintname_src, const std::string& zipintname_dst);
	void add(const std::string& zipintname_src, const std::string& zipintname_dst, ziprom& reject);
	void rename(const std::string& zipintname_src, const std::string& zipintname_dst, ziprom& reject);
	void swap(const std::string& zipintname, ziprom& reject, ziprom::iterator& reject_entry);
};

struct ziprom_by_file_less : std::binary_function<ziprom,ziprom,bool> {
	bool operator()(const ziprom& A, const ziprom& B) const {
		// don't use file_compare, do exact compare
		return A.file_get() < B.file_get();
	}
};

typedef std::list<ziprom> zipromcontainer;

// Store crc and size pair
class ziparchive_crcsize {
	crc_t crc;
	unsigned size;
public:
	ziparchive_crcsize();
	ziparchive_crcsize(const ziparchive_crcsize& A);
	ziparchive_crcsize(unsigned size, crc_t crc);
	~ziparchive_crcsize();

	bool operator==(const ziparchive_crcsize& A) const { return crc==A.crc && size==A.size; }
	bool operator<(const ziparchive_crcsize& A) const { return crc<A.crc || (crc==A.crc && size<A.size); }
};

typedef std::set<ziparchive_crcsize> ziparchive_crcsizeset;

class ziparchive {
public:
	typedef zipromcontainer::const_iterator const_iterator;
	typedef zipromcontainer::iterator iterator;

private:
	zipromcontainer data; // std::list of zip
	mutable ziparchive_crcsizeset index; // fast exist test in data

	// abstract
	ziparchive(const ziparchive&);

	const_iterator find_iter(unsigned size, crc_t crc, ziprom::const_iterator& k) const;
	const_iterator find_iter(unsigned size, crc_t crc, zip_type type, ziprom::const_iterator& k) const;
	const_iterator find_exclude_iter(const ziprom& exclude, unsigned size, crc_t crc, ziprom::const_iterator& k) const;

public:
	ziparchive();
	~ziparchive();

	unsigned size() const { return data.size(); }
	iterator open_and_insert(const ziprom& A);
	void update(const ziprom& A);
	void erase(iterator A);

	iterator begin() { return data.begin(); }
	iterator end() { return data.end(); }

	const_iterator begin() const { return data.begin(); }
	const_iterator end() const { return data.end(); }

	const_iterator find(const std::string& zipfile) const;
	iterator find(const std::string& zipfile);

	const_iterator find(unsigned size, crc_t crc, ziprom::const_iterator& k) const;
	const_iterator find(unsigned size, crc_t crc, zip_type type, ziprom::const_iterator& k) const;
	const_iterator find_exclude(const ziprom& exclude, unsigned size, crc_t crc, ziprom::const_iterator& k) const;
};

#endif
