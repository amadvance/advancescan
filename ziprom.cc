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

#include "portable.h"

#include "ziprom.h"

#include <unistd.h>

using namespace std;

// ------------------------------------------------------------------------
// ziprom

ziprom::ziprom(const string& Apath, zip_type Atype, bool Areadonly) : zip(Apath), type(Atype), readonly(Areadonly) {
}

ziprom::ziprom(const ziprom& A) : zip(A), type(A.type), readonly(A.readonly) {
}

ziprom::~ziprom() {
}

void ziprom::open() {

	try {
		zip::open();
	} catch (error& e) {
		readonly = true;
		throw;
	}

	if (is_load()) {
		// if it's loaded means that the file is just created
		readonly = false;
	} else {
		// check for read/write access
		readonly = access(file_get().c_str(), F_OK | R_OK | W_OK) != 0;
	}
}

ziprom::iterator ziprom::find(const string& name) {

	for(ziprom::iterator i=begin();i!=end();++i) {
		if (file_compare( i->name_get(), name)==0) {
			return i;
		}
	}

	return end();
}

void ziprom::load()
{
	if (!is_load()) {
		cmessage << MESSAGE << "load " << file_get() << endl;
		try {
			zip::load();
		} catch (error& e) {
			throw e << " loading " << file_get();
		}
	}
}

void ziprom::unload()
{
	if (is_load()) {
		try {
			if (is_modify()) {
				zip::reopen();
			} else {
				zip::unload();
			}
		} catch (error& e) {
			throw e << " unloading " << file_get();
		}
	}
}

void ziprom::save()
{
	if (is_load() && is_modify()) {
		if (size_not_zero() > 0) {
			cmessage << MESSAGE << "save " << file_get() << endl;
			try {
				zip::save();
			} catch (error& e) {
				throw e << " saving " << file_get();
			}
		} else {
			cmessage << MESSAGE << "delete " << file_get() << endl;
			try {
				zip::save();
			} catch (error& e) {
				throw e << " deleting " << file_get();
			}
		}
	}
}

void ziprom::remove(const string& zipintname)
{
	load();

	ziprom::iterator i = find(zipintname);
	if (i!=end()) {
		cmessage << MESSAGE << "remove " << file_get() << "/" << zipintname << endl;

		erase(i);
	}
}


void ziprom::remove(const string& zipintname, ziprom& reject)
{
	load();

	ziprom::iterator i = find(zipintname);
	if (i!=end()) {
		move(zipintname, reject, zipintname);
	}
}

void ziprom::move(const string& zipintname_src, ziprom& reject, const string& zipintname_dst)
{
	load();
	reject.load();

	reject.remove(zipintname_dst);

	cmessage << MESSAGE << "move " << file_get() << "/" << zipintname_src << " to " << reject.file_get() << "/" << zipintname_dst << endl;

	ziprom::iterator i = find(zipintname_src);
	if (i==end())
		throw error() << "Failed move of " << zipintname_src << " in zip " << file_get();

	ziprom::iterator j = reject.insert(*i, zipintname_dst);
	if (j==end())
		throw error() << "Failed add of " << zipintname_dst << " in zip " << reject.file_get();

	erase(i);
}

void ziprom::add(const ziprom::const_iterator& entry_src, const string& zipintname_dst, ziprom& reject)
{
	load();

	remove(zipintname_dst, reject);

	cmessage << MESSAGE << "add " << entry_src->parentname_get() << "/" << entry_src->name_get() << " to " << file_get() << "/" << zipintname_dst << endl;

	// insert
	ziprom::iterator k = insert(*entry_src, zipintname_dst);
	if (k==end())
		throw error() << "Failed add of " << zipintname_dst << " in zip " << file_get();
}

void ziprom::add(const ziprom::const_iterator& entry_src, const string& zipintname_dst)
{
	load();

	remove(zipintname_dst);

	cmessage << MESSAGE << "add " << entry_src->parentname_get() << "/" << entry_src->name_get() << " to " << file_get() << "/" << zipintname_dst << endl;

	// insert
	ziprom::iterator k = insert(*entry_src, zipintname_dst);
	if (k==end())
		throw error() << "Failed add of " << zipintname_dst << " in zip " << file_get();
}

void ziprom::add(const string& zipintname_src, const string& zipintname_dst, ziprom& reject)
{
	load();

	ziprom::iterator i = find(zipintname_src);
	if (i==end())
		throw error() << "File " << zipintname_src << " not found in zip " << file_get();

	add(i,zipintname_dst,reject);
}

void ziprom::add(const string& zipintname_src, const string& zipintname_dst)
{
	load();

	ziprom::iterator i = find(zipintname_src);
	if (i==end())
		throw error() << "File " << zipintname_src << " not found in zip " << file_get();

	add(i,zipintname_dst);
}

void ziprom::rename(const string& zipintname_src, const string& zipintname_dst, ziprom& reject)
{
	load();

	remove(zipintname_dst, reject);

	cmessage << MESSAGE << "rename " << file_get() << "/" << zipintname_src << " to " << zipintname_dst << endl;

	ziprom::iterator i = find(zipintname_src);
	if (i==end())
		throw error() << "File " << zipintname_src << " not found in zip " << file_get();

	zip::rename(i, zipintname_dst.c_str());
}

void ziprom::swap(const string& zipintname, ziprom& reject, ziprom::iterator& reject_entry)
{
	load();
	reject.load();

	ziprom::iterator i = find(zipintname);
	if (i==end()) {
		// not present in the zip, it isn't a swap
		cmessage << MESSAGE << "move " << reject.file_get() << "/" << reject_entry->name_get() << " to " << file_get() << "/" << zipintname << endl;

		insert(*reject_entry, reject_entry->name_get());
		reject.erase(reject_entry);
	} else {
		cmessage << MESSAGE << "swap " << reject.file_get() << reject_entry->name_get() << " and " << file_get() << "/" << zipintname << endl;

		insert(*reject_entry, reject_entry->name_get());
		reject.insert(*i, i->name_get());

		erase(i);
		reject.erase(reject_entry);
	}
}

// ------------------------------------------------------------------------
// ziparchive

ziparchive_crcsize::ziparchive_crcsize() {
}

ziparchive_crcsize::ziparchive_crcsize(const ziparchive_crcsize& A) : crc(A.crc), size(A.size) {
}

ziparchive_crcsize::ziparchive_crcsize(unsigned Asize, crc_t Acrc) : crc(Acrc), size(Asize) {
}

ziparchive_crcsize::~ziparchive_crcsize() {
}

ziparchive::ziparchive() {
}

ziparchive::~ziparchive() {
	for(iterator i=begin();i!=end();++i) {
		i->close();
	}
}

ziparchive::const_iterator ziparchive::find(const string& zipfile) const {
	for(const_iterator i=begin();i!=end();++i) {
		if (zipfile == i->file_get()) {
			return i;
		}
	}

	return end();
}

ziparchive::iterator ziparchive::find(const string& zipfile) {
	for(iterator i=begin();i!=end();++i) {
		if (zipfile == i->file_get()) {
			return i;
		}
	}

	return end();
}

ziparchive::iterator ziparchive::open_and_insert(const ziprom& A) {
	assert( !A.is_open() );

	ziparchive::iterator i = data.insert(data.end(), A);

	try {
		i->open();
	} catch (...) {
		data.erase(i);
		throw;
	}

	assert( i->is_open() );

	// update the index
	for(ziprom::const_iterator j=i->begin();j!=i->end();++j) {
		index.insert( ziparchive_crcsize( j->uncompressed_size_get(), j->crc_get() ) );
	}

	return i;
}

void ziparchive::update(const ziprom& A) {
	assert( A.is_open() );

	// remove if already present
	ziparchive::iterator i = find( A.file_get() );
	if (i != end()) {
		i->close();
		erase(i);
	}

	// insert only if not empty
	if (A.size()) {
		ziparchive::iterator i = data.insert(data.end(), A);

		assert( i->is_open() );

		// update the index
		for(ziprom::const_iterator j=A.begin();j!=A.end();++j) {
			index.insert( ziparchive_crcsize( j->uncompressed_size_get(), j->crc_get() ) );
		}
	}
}

void ziparchive::erase(ziparchive::iterator A) {
	data.erase(A);
}

ziparchive::const_iterator ziparchive::find_iter(unsigned size, crc_t crc, ziprom::const_iterator& k) const {
	for(const_iterator i=begin();i!=end();++i) {
		for(ziprom::const_iterator j=i->begin();j!=i->end();++j) {
			if (crc==j->crc_get() && size==j->uncompressed_size_get()) {
				k = j;
				return i;
			}
		}
	}
	return end();
}

ziparchive::const_iterator ziparchive::find_iter(unsigned size, crc_t crc, zip_type type, ziprom::const_iterator& k) const {
	for(const_iterator i=begin();i!=end();++i) {
		if (type==i->type_get()) {
			for(ziprom::const_iterator j=i->begin();j!=i->end();++j) {
				if (crc==j->crc_get() && size==j->uncompressed_size_get()) {
					k = j;
					return i;
				}
			}
		}
	}
	return end();
}

ziparchive::const_iterator ziparchive::find(unsigned size, crc_t crc, ziprom::const_iterator& k) const {
	ziparchive_crcsizeset::const_iterator i = index.find( ziparchive_crcsize( size, crc ));
	if (i!=index.end())
		return find_iter(size,crc,k);
	return end();

}

ziparchive::const_iterator ziparchive::find(unsigned size, crc_t crc, zip_type type, ziprom::const_iterator& k) const {
	return find_iter(size,crc,type,k);
}

ziparchive::const_iterator ziparchive::find_exclude_iter(const ziprom& exclude, unsigned size, crc_t crc, ziprom::const_iterator& k) const {
	for(const_iterator i=begin();i!=end();++i) {
		if (&*i != &exclude) {
			for(ziprom::const_iterator j=i->begin();j!=i->end();++j) {
				if (crc==j->crc_get() && size==j->uncompressed_size_get()) {
					k = j;
					return i;
				}
			}
		}
	}
	return end();
}

ziparchive::const_iterator ziparchive::find_exclude(const ziprom& exclude, unsigned size, crc_t crc, ziprom::const_iterator& k) const {
	ziparchive_crcsizeset::const_iterator i = index.find( ziparchive_crcsize( size, crc ));
	if (i!=index.end())
		return find_exclude_iter(exclude,size,crc,k);
	return end();
}

