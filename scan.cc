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

#include "game.h"
#include "ziprom.h"
#include "conf.h"
#include "operatio.h"
#include "output.h"
#include "analyze.h"

#include "lib/readinfo.h"

#include <fstream>
#include <iomanip>
#include <sstream>

#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

// ------------------------------------------------------------------------
// Scan

bool is_zip_file(const string& file) {
	string ext = file_ext(file);
	return file_compare(ext,".zip") == 0;
}

// Read all file in a directory
void read_dir(const string& path, filepath_container& ds, bool recursive) {
	DIR* dir = opendir(path.c_str());
	if (!dir)
		throw error() << "Failed open on dir " << path;

	struct dirent* ent = readdir(dir);
	while (ent) {
		string subpath = path + "/" + ent->d_name;
		struct stat st;

		if (stat(subpath.c_str(),&st)!=0)
			throw error() << "Failed stat on file " << subpath;

		if (S_ISDIR(st.st_mode)) {
			if (recursive) {
				if (strcmp(ent->d_name,".")!=0 && strcmp(ent->d_name,"..")!=0) {
					read_dir(subpath.c_str(),ds,recursive);
				}
			}
		} else {
			if (is_zip_file(ent->d_name)) {
				ds.insert( ds.end(), filepath( subpath.c_str() ) );
			}
		}
		ent = readdir(dir);
	}

	closedir(dir);
}

// Read all zip in zipset
void read_zip(const string& path, ziparchive& zar, zip_type type, bool ignore_error) {
	filepath_container ds;

	read_dir(path,ds,false);

	for(filepath_container::iterator i=ds.begin();i!=ds.end();++i) {
		ziparchive::iterator j;

		if (file_compare(file_ext(i->file_get()),".zip")==0) {
			try {
				j = zar.open_and_insert( ziprom(i->file_get(), type, true) );
			} catch (error& e) {
				if (ignore_error) {
					cwarning << WARNING << "failed open on zip " << i->file_get() << endl;
					cwarning << WARNING << e << endl;
					cwarning << WARNING << "ignoring zip " << i->file_get() << " and resuming" << endl;
				} else {
					throw e << " opening zip " << i->file_get();
				}
			}
		}
	}
}

// ---------------------------------------------------------------------------
// stat

// Association for a rom and wrong crc
struct rom_bad {
	rom r;
	unsigned bad_size;
	crc_t bad_crc;
	
	unsigned size_get() const { return r.size_get(); }
	crc_t crc_get() const { return r.crc_get(); }
	const string& name_get() const { return r.name_get(); }
	unsigned bad_size_get() const { return bad_size; }
	crc_t bad_crc_get() const { return bad_crc; }
};

typedef list<rom_bad> rom_bad_container;

struct rom_stat_t {
	rom_by_name_set rom_miss; // rom missing
	rom_by_name_set rom_equal; // rom good
	rom_bad_container rom_bad; // rom wrong
	rom_by_name_set unk_binary; // unknown binary file
	rom_by_name_set unk_text; // unknown text file
	rom_by_name_set unk_garbage; // unknown garbage file
	rom_by_name_set nodump_equal; // nodump present
	rom_bad_container nodump_bad; // nodump present bat wrong
	rom_by_name_set nodump_miss; // nodump missing
};

void stat_rom_zip(
	const ziprom& zd,
	const game& gam,
	rom_stat_t& result,
	const analyze& ana)
{
	// setup
	rom_by_name_set b = gam.rs_get();

	for(ziprom::const_iterator z=zd.begin();z!=zd.end();++z) {
		// search for name
		rom_by_name_set::iterator i = b.find( rom( z->name_get(), 0, 0, false) );
		if (i == b.end()) { // if name unknown
			// rom to insert
			rom r( z->name_get(), z->uncompressed_size_get(), z->crc_get(), false);

			analyze_type t = ana(z->name_get(), z->uncompressed_size_get(), z->crc_get());
			switch (t) {
				case analyze_text :
					result.unk_text.insert( r );
					break;
				case analyze_binary :
					result.unk_binary.insert( r );
					break;
				case analyze_garbage :
					result.unk_garbage.insert( r );
					break;
			}
		} else { // if name know
			if (i->nodump_get()) {
				if (z->uncompressed_size_get() == i->size_get() && z->crc_get() == i->crc_get()) {
					result.nodump_equal.insert(*i);
				} else {
					rom_bad r;
					r.r = *i;
					r.bad_size = z->uncompressed_size_get();
					r.bad_crc = z->crc_get();
					result.nodump_bad.insert(result.nodump_bad.end(), r);
				}
			} else {
				if (z->uncompressed_size_get() == i->size_get() && z->crc_get() == i->crc_get()) {
					result.rom_equal.insert( *i );
				} else {
					// rom is wrong
					rom_bad r;
					r.r = *i;
					r.bad_size = z->uncompressed_size_get();
					r.bad_crc = z->crc_get();
					result.rom_bad.insert(result.rom_bad.end(),r);
				}
			}

			// remove from original bag
			b.erase( i );
		}
	}

	for(rom_by_name_set::iterator i=b.begin();i!=b.end();++i) {
		if (i->nodump_get()) {
			result.nodump_miss.insert(*i);
		} else {
			result.rom_miss.insert(*i);
		}
	}
}

struct sample_stat_t {
	sample_by_name_set sample_equal;
	sample_by_name_set sample_miss;
	sample_by_name_set unk_binary;
	sample_by_name_set unk_text;
	sample_by_name_set unk_garbage;
};

void sample_stat(
	const ziprom& zd,
	const game& gam,
	sample_stat_t& result,
	const analyze& ana)
{
	result.sample_miss = gam.ss_get();

	for(ziprom::const_iterator z=zd.begin();z!=zd.end();++z) {
		sample_by_name_set::iterator i = result.sample_miss.find( sample( z->name_get() ) );
		if (i == result.sample_miss.end()) { // if name unknow
			sample s( z->name_get() );

			analyze_type t = ana(z->name_get(), z->uncompressed_size_get(), z->crc_get());
			switch (t) {
				case analyze_text :
					result.unk_text.insert( s );
					break;
				case analyze_binary :
					result.unk_binary.insert( s );
					break;
				case analyze_garbage :
					result.unk_garbage.insert( s );
					break;
			}
		} else { // if name know
			result.sample_equal.insert( *i );
			// remove from original bag
			result.sample_miss.erase( i );
		}
	}
}

// ----------------------------------------------------------------------------
// add

// return:
//   true  success
//   false error
void rom_add(
	const operation& oper,
	ziprom& z,
	const game& g, 
	const ziparchive& zar,
	output& out) 
{
	bool title = false; // true if is printed the zip file name

	rom_by_name_set good;
	rom_by_name_set miss = g.rs_get();

	// for any rom/nodump
	rom_by_name_set tmp_miss;
	for(rom_by_name_set::iterator i=miss.begin();i!=miss.end();++i) {
		bool found = false;
		bool added = false;

		// skip nodump
		if (i->nodump_get())
			continue;

		// check if is in another zip
		if (!found) {
			ziprom::const_iterator k;
			ziparchive::const_iterator j = zar.find_exclude(z,i->size_get(),i->crc_get(),k);
			if (j!=zar.end()) {
				if (oper.active_add() && !z.is_readonly()) {
					z.add(k, i->name_get());
					good.insert(*i);
					added = true;
				} 
				if (oper.output_add()) {
					out.title("rom_zip",title,z.file_get());
					out.cmd_rom("rom_good","add", *i) << " " << k->parentname_get() << "/" << k->name_get() << endl;
				}
				found = true;
			}
		}

		if (!added)
			// if not found insert in missing rom
			tmp_miss.insert( *i );
	}
	miss = tmp_miss;

	if (title)
		out() << endl;

	// if zip created with almost one good rom
	if (good.size()) {

		// update the zip
		z.save();
		z.unload();

		if (!z.empty()) {
			// get file information
			struct stat fst;
			string path = z.file_get();
			if (stat(path.c_str(),&fst) != 0)
				throw error() << "Failed stat file " << z.file_get();

			// if no rom is missing
			if (miss.size()==0) {
				// add zip to directory list of game as good
				g.rzs_add( zippath( z.file_get(), true, fst.st_size, z.is_readonly() ) );
			} else {
				// add zip to directory list of game as bad
				g.rzs_add( zippath( z.file_get(), false, fst.st_size, z.is_readonly() ) );
			}
		}
	}
}

// ----------------------------------------------------------------------------
// scan

bool rom_scan_add(
	const operation& oper,
	const char* s_add,
	const char* s_cmd,
	ziprom& z,
	ziprom& reject,
	bool& found,
	bool& title,
	string s_name,
	unsigned s_size,
	crc_t s_crc,
	rom_by_name_set& rremove,
	const ziparchive& zar,
	output& out)
{
	bool added = false; 
	if (!found) {
		// check if it's present in the remove bag
		for(rom_by_name_set::iterator j=rremove.begin();j!=rremove.end();++j) {
			// compare crc, size
			if (s_crc == j->crc_get() && s_size == j->size_get()) {
				if (oper.active_fix() && !z.is_readonly()) {
					// rename the rom
					z.rename(j->name_get(), s_name, reject);
					added = true;

					// remove from the remove bag
					rremove.erase(j);
				}
				if (oper.output_fix()) {
					out.title("rom_zip",title,z.file_get());
					out.cmd_rom(s_add, s_cmd, s_name, s_size, s_crc) << " " << j->name_get() << endl;
				}
				found = true;

				// interrupt
				break;
			}
		}
	}

	if (!found) {
		// check if it's present in the same zip
		for(zip::iterator j=z.begin();j!=z.end();++j) {
			// compare crc, size
			if (s_crc == j->crc_get() && s_size == j->uncompressed_size_get()) {
				string name = j->name_get();
				if (s_name != name) {
					// the name must be different (it always be)
					if (oper.active_fix() && !z.is_readonly()) {
						// add the rom
						// (this operation is safe, because the zip iterator is a list)
						z.add(j, s_name, reject);
						added = true;
					}
				} else {
					throw error() << "Failed internal check";
				}
				if (oper.output_fix()) {
					out.title("rom_zip",title,z.file_get());
					out.cmd_rom(s_add, s_cmd, s_name, s_size, s_crc) << " " << name << endl;
				}
				found = true;

				// interrupt
				break;
			}
		}
	}

	if (!found) {
		// check if it's present in the reject zip
		for(zip::iterator j=reject.begin();j!=reject.end();++j) {
			// compare crc, size
			if (s_crc == j->crc_get() && s_size == j->uncompressed_size_get()) {
				string name = j->name_get();
				if (s_name != name) {
					// the name must be different (it may not be)
					if (oper.active_fix() && !z.is_readonly()) {
						// add the rom
						// (this operation is safe, because the zip iterator is a list)
						z.add(j, s_name, reject);
						added = true;
					}
				} else {
					if (oper.active_fix() && !z.is_readonly()) {
						// swap the roms
						z.swap(s_name, reject, j);
						added = true;
					}
				}
				if (oper.output_fix()) {
					out.title("rom_zip",title,z.file_get());
					out.cmd_rom(s_add, s_cmd, s_name, s_size, s_crc) << " " << reject.file_get() << "/" << name << endl;
				}
				found = true;

				// interrupt
				break;
			}
		}
	}

	if (!found) {
		// check if it's in another zip with the exclusion of the reject zip
		ziprom::const_iterator k;
		ziparchive::const_iterator j = zar.find_exclude(z,s_size,s_crc,k);
		if (j!=zar.end()) {
			if (oper.active_fix() && !z.is_readonly()) {
				z.add(k, s_name, reject);
				added = true;
			}
			if (oper.output_fix()) {
				out.title("rom_zip",title,z.file_get());
				out.cmd_rom(s_add, s_cmd, s_name, s_size, s_crc) << " " << k->parentname_get() << "/" << k->name_get() << endl;
			}
			found = true;
		}
	}

	return added;
}

void rom_scan(
	const operation& oper,
	ziprom& z,
	ziprom& reject,
	const game& gam,
	const ziparchive& zar,
	output& out,
	const analyze& ana)
{
	bool title = false; // true if is printed the zip file name

	reject.open();

	// get rom status
	rom_stat_t st;
	stat_rom_zip(z,gam,st,ana);

	// for any missing rom
	rom_by_name_set tmp_rom_miss; // missing rom
	for(rom_by_name_set::iterator i=st.rom_miss.begin();i!=st.rom_miss.end();++i) {
		bool found = false;
		bool added = false;

		if (rom_scan_add(
			oper,
			"rom_good","add",
			z, reject, found, title,
			i->name_get(), i->size_get(), i->crc_get(), 
			st.unk_binary, zar, out)) {

			st.rom_equal.insert(*i);
			added = true;
		}

		if (!added) {
			// rom is missing
			tmp_rom_miss.insert( *i );
		}
	}
	st.rom_miss = tmp_rom_miss;

	// for any wrong rom (rom with corret name bat bad crc or size)
	rom_bad_container tmp_rom_bad;
	for(rom_bad_container::iterator i=st.rom_bad.begin();i!=st.rom_bad.end();++i) {
		bool found = false;
		bool added = false;

		if (rom_scan_add(
			oper,
			"rom_good","add",
			z, reject, found, title,
			i->name_get(), i->size_get(), i->crc_get(), 
			st.unk_binary, zar, out)) {

			st.rom_equal.insert(i->r);
			added = true;
		}

		if (!added)
			// if not found insert in wrong rom
			tmp_rom_bad.insert( tmp_rom_bad.end(), *i );
	}
	st.rom_bad = tmp_rom_bad;

	for(rom_by_name_set::iterator i=st.unk_binary.begin();i!=st.unk_binary.end();++i) {
		if (oper.active_remove_binary() && !z.is_readonly()) {
			z.remove(i->name_get(), reject);
		}
		if (oper.output_remove_binary()) {
			out.title("rom_zip",title,z.file_get());
			out.cmd_rom("binary","remove", *i) << endl;
		}
	}

	for(rom_by_name_set::iterator i=st.unk_text.begin();i!=st.unk_text.end();++i) {
		if (oper.active_remove_text() && !z.is_readonly()) {
			z.remove(i->name_get(), reject);
		}
		if (oper.output_remove_text()) {
			out.title("rom_zip",title,z.file_get());
			out.cmd_rom("text","remove", *i) << endl;
		}
	}

	for(rom_by_name_set::iterator i=st.unk_garbage.begin();i!=st.unk_garbage.end();++i) {
		if (oper.active_remove_garbage() && !z.is_readonly()) {
			z.remove(i->name_get());
		}
		if (oper.output_remove_garbage()) {
			out.title("rom_zip",title,z.file_get());
			out.cmd_rom("garbage","remove", *i) << endl;
		}
	}

	if (title)
		out() << endl;

	// update the zip
	z.save();
	z.unload();

	if (!z.empty()) {
		// get file information
		struct stat fst;
		string path = z.file_get();
		if (stat(path.c_str(),&fst) != 0)
			throw error() << "Failed stat file " << z.file_get();

		// if no rom is missing or wrong. Ignore nodump.
		if (st.rom_miss.size()==0
		    && st.rom_bad.size()==0
		    ) {
			// add zip to directory list of game as good
			gam.rzs_add( zippath( z.file_get(), true, fst.st_size, z.is_readonly()) );
		} else {
			// add zip to directory list of game as bad
			gam.rzs_add( zippath( z.file_get(), false, fst.st_size, z.is_readonly()) );
		}
	}

	// update the reject zip
	reject.save();
	reject.unload();
}

void sample_scan(
	const operation& oper,
	ziprom& z,
	ziprom& reject,
	const game& gam, 
	output& out,
	const analyze& ana)
{
	bool title = false; // true if is printed the zip file name

	reject.open();

	// get sample status
	sample_stat_t st;
	sample_stat(z,gam,st,ana);
	
	// for any miss sample
	sample_by_name_set tmp_rom_miss;
	for(sample_by_name_set::iterator i=st.sample_miss.begin();i!=st.sample_miss.end();++i) {
		bool found = false;
		bool added = false;

		if (!found) {
			// check if is present in remove bag with directory
			for(sample_by_name_set::iterator j=st.unk_binary.begin();j!=st.unk_binary.end();++j) {
				// compare name without dir
				if (file_compare( i->name_get(), file_name(j->name_get()) )==0) {
					// found rom with different name
					if (oper.active_fix() && !z.is_readonly()) {
						z.add(j->name_get(), i->name_get());
						st.sample_equal.insert(*i);
						added = true;
					} 
					if (oper.output_fix()) {
						out.title("sample_zip",title,z.file_get());
						out.cmd_sample("sound","add", *i) << " " << j->name_get() << endl;
					}
					found = true;
					break;
				}
			}
		}

		if (!added)
			// if not found insert in missing sample
			tmp_rom_miss.insert( *i );
	}
	st.sample_miss = tmp_rom_miss;

	for(sample_by_name_set::iterator i=st.unk_binary.begin();i!=st.unk_binary.end();++i) {
		if (oper.active_remove_binary() && !z.is_readonly()) {
			z.remove(i->name_get(), reject);
		}
		if (oper.output_remove_binary()) {
			out.title("sample_zip",title,z.file_get());
			out.cmd_sample("binary","remove", *i) << endl;
		}
	}

	for(sample_by_name_set::iterator i=st.unk_text.begin();i!=st.unk_text.end();++i) {
		if (oper.active_remove_text() && !z.is_readonly()) {
			z.remove(i->name_get(), reject);
		}
		if (oper.output_remove_text()) {
			out.title("sample_zip",title,z.file_get());
			out.cmd_sample("text","remove", *i) << endl;
		}
	}

	for(sample_by_name_set::iterator i=st.unk_garbage.begin();i!=st.unk_garbage.end();++i) {
		if (oper.active_remove_garbage() && !z.is_readonly()) {
			z.remove(i->name_get());
		}
		if (oper.output_remove_garbage()) {
			out.title("sample_zip",title,z.file_get());
			out.cmd_sample("garbage","remove", *i) << endl;
		}
	} 

	if (title)
		out() << endl;

	// update the zip
	z.save();
	z.unload();

	if (!z.empty()) {
		// get file information
		struct stat fst;
		string path = z.file_get();
		if (stat(path.c_str(),&fst) != 0)
			throw error() << "Failed stat file " << z.file_get();

		// if no sample is missing
		if (st.sample_miss.size()==0) {
			gam.szs_add( zippath( z.file_get(), true, fst.st_size, z.is_readonly()) );
		} else {
			gam.szs_add( zippath( z.file_get(), false, fst.st_size, z.is_readonly()) );
		}
	}

	// update the reject zip
	reject.save();
	reject.unload();
}

void unknow_scan(
	ziprom& z,
	const ziparchive& zar)
{
	// create the list of all the roms to remove
	rom_by_name_set remove;
	for(ziprom::const_iterator i=z.begin();i!=z.end();++i) {
		rom r( i->name_get(), i->uncompressed_size_get(), i->crc_get(), false);
		remove.insert( r );
	}

	for(rom_by_name_set::iterator i=remove.begin();i!=remove.end();++i) {
		ziprom::const_iterator k;

		// search if the rom is present in a file in the own set
		ziparchive::const_iterator j = zar.find(i->size_get(), i->crc_get(), zip_own, k);
		if (j != zar.end()) {
			// if present remove the duplicate in the unknow set
			z.remove(i->name_get());
		}
	}

	// update the zip
	z.save();
	z.unload();
}

// ----------------------------------------------------------------------------
// move

void rom_move(
	const operation& oper,
	ziprom& z,
	ziprom& reject,
	output& out)
{
	bool title = false; // true if is printed the zip file name

	reject.open();

	// create the list of all the roms to remove
	rom_by_name_set remove;
	for(ziprom::const_iterator i=z.begin();i!=z.end();++i) {
		rom r(i->name_get(), i->uncompressed_size_get(), i->crc_get(), false);
		remove.insert( r );
	}

	for(rom_by_name_set::iterator i=remove.begin();i!=remove.end();++i) {
		if (oper.active_move() && !z.is_readonly()) {
			z.remove(i->name_get(), reject);
		}
		if (oper.output_move()) {
			out.title("rom_zip",title,z.file_get());
			out.cmd_rom("binary","remove", *i) << endl;
		}
	}

	if (title)
		out() << endl;

	// save the reject zip. First save and only after delete the data.
	reject.save();
	reject.unload();

	// delete the empty zip.
	z.save();
	z.unload();
}

void sample_move(
	const operation& oper,
	ziprom& z,
	ziprom& reject,
	output& out)
{
	bool title = false; // true if is printed the zip file name

	reject.open();

	// create the list of all the roms to remove
	rom_by_name_set remove;
	for(ziprom::const_iterator i=z.begin();i!=z.end();++i) {
	rom r(i->name_get(), i->uncompressed_size_get(), i->crc_get(), false);
		remove.insert( r );
	}

	for(rom_by_name_set::iterator i=remove.begin();i!=remove.end();++i) {
		if (oper.active_move() && !z.is_readonly()) {
			z.remove(i->name_get(), reject);
		}
		if (oper.output_move()) {
			out.title("sample_zip",title,z.file_get());
			out.cmd_rom("binary","remove", *i) << endl;
		}
	}

	if (title)
		out() << endl;

	// save the reject zip. First save and only after delete the data.
	reject.save();
	reject.unload();

	// delete the empty zip.
	z.save();
	z.unload();
}

// ----------------------------------------------------------------------------
// report

// Report a zip file
// return:
//   true  success
//   false error
void rom_report(
	const ziprom& z,
	const game& gam,
	output& out,
	bool verbose,
	const analyze& ana)
{
	// if empty don't report
	if (!z.size())
		return;

	// get rom status
	rom_stat_t st;
	stat_rom_zip(z,gam,st,ana);

	// print the report
	if (st.rom_miss.size() || st.rom_bad.size()
		|| (verbose && (st.nodump_miss.size() || st.nodump_bad.size() || st.nodump_equal.size() || st.unk_text.size() || st.unk_binary.size() || st.unk_garbage.size()))
	) {
		bool title = false;

		out.title("rom_zip",title,z.file_get());

		for(rom_by_name_set::iterator i=st.rom_miss.begin();i!=st.rom_miss.end();++i) {
			out.state_rom("rom_miss", *i) << endl;
		}

		for(rom_by_name_set::iterator i=st.nodump_miss.begin();i!=st.nodump_miss.end();++i) {
			out.state_rom("nodump_miss", *i) << endl;
		}

		for(rom_bad_container::iterator i=st.rom_bad.begin();i!=st.rom_bad.end();++i) {
			out.state_rom_real("rom_bad", i->r, i->bad_size_get(), i->bad_crc_get()) << endl;
		}

		for(rom_bad_container::iterator i=st.nodump_bad.begin();i!=st.nodump_bad.end();++i) {
			out.state_rom_real("nodump_bad", i->r, i->bad_size_get(), i->bad_crc_get()) << endl;
		}

		for(rom_by_name_set::iterator i=st.unk_text.begin();i!=st.unk_text.end();++i) {
			out.state_rom("text", *i) << endl;
		}

		for(rom_by_name_set::iterator i=st.unk_binary.begin();i!=st.unk_binary.end();++i) {
			out.state_rom("binary", *i) << endl;
		}

		for(rom_by_name_set::iterator i=st.unk_garbage.begin();i!=st.unk_garbage.end();++i) {
			out.state_rom("garbage", *i) << endl;
		}

		for(rom_by_name_set::iterator i=st.nodump_equal.begin();i!=st.nodump_equal.end();++i) {
			out.state_rom("nodump_good", i->name_get(), i->size_get(), i->crc_get()) << endl;
		}

		for(rom_by_name_set::iterator i=st.rom_equal.begin();i!=st.rom_equal.end();++i) {
			out.state_rom("rom_good", *i) << endl;
		}

		out() << endl;
	}
}

void sample_report(
	const ziprom& z,
	const game& gam,
	output& out,
	bool verbose,
	const analyze& ana)
{
	// if empty don't report
	if (!z.size())
		return;

	// get rom status
	sample_stat_t st;
	sample_stat(z,gam,st,ana);

	// print the report
	if (st.sample_miss.size()
		|| (verbose && (st.unk_text.size() || st.unk_binary.size() || st.unk_garbage.size()))
	) {
		bool title = false;

		out.title("sample_zip",title,z.file_get());

		for(sample_by_name_set::iterator i=st.sample_miss.begin();i!=st.sample_miss.end();++i) {
			out.state_sample("sound_miss", *i) << endl;
		}

		for(sample_by_name_set::iterator i=st.unk_text.begin();i!=st.unk_text.end();++i) {
			out.state_sample("text", *i) << endl;
		}
		
		for(sample_by_name_set::iterator i=st.sample_equal.begin();i!=st.sample_equal.end();++i) {
			out.state_sample("sound_good", *i) << endl;
		}

		out() << endl;
	}
}

// ---------------------------------------------------------------------------
// all_load

void all_rom_load(ziparchive& zar, const config& cfg)
{
	// read own zip
	for(filepath_container::const_iterator i=cfg.rompath_get().begin();i!=cfg.rompath_get().end();++i) {
		read_zip(i->file_get(), zar, zip_own, false);
	}

	// read unknow zip
	read_zip(cfg.romunknowpath_get().file_get(), zar, zip_unknow, false);

	// read import zip
	for(filepath_container::const_iterator i=cfg.romreadonlytree_get().begin();i!=cfg.romreadonlytree_get().end();++i) {
		read_zip(i->file_get(), zar, zip_import, true);
	}
}

void set_rom_load(ziparchive& zar, const config& cfg)
{
	// read own zip
	for(filepath_container::const_iterator i=cfg.rompath_get().begin();i!=cfg.rompath_get().end();++i) {
		read_zip(i->file_get(), zar, zip_own, false);
	}
}

void set_sample_load(filepath_container& zar, const config& cfg)
{
	for(filepath_container::const_iterator i=cfg.samplepath_get().begin();i!=cfg.samplepath_get().end();++i) {
		read_dir(i->file_get(),zar,false);
	}
}

// ----------------------------------------------------------------------------
// all_scan

void all_rom_scan(const operation& oper, ziparchive& zar, gamearchive& gar, const config& cfg, output& out, const analyze& ana) {
	filepath_container unknow; // container of unknow zip

	// scan zips
	for(ziparchive::iterator i=zar.begin();i!=zar.end();++i) {
		assert( i->is_open() );

		if (i->type_get() == zip_own) {
			gamearchive::iterator g = gar.find( game( file_basename( i->file_get() ) ) );

			if (g == gar.end() || !g->romset_required()) {
				// insert in the unknow set, processed later
				unknow.insert( unknow.end(), filepath( i->file_get()) );
			} else {
				ziprom reject( cfg.romunknowpath_get().file_get() + "/" + g->name_get() + ".zip", zip_unknow, false );

				try {
					rom_scan(oper, *i, reject, *g, zar, out, ana);
				} catch (error& e) {
					throw e << " scanning rom " << i->file_get();
				}

				zar.update( reject );
			}
		}
	}

	// add zips
	if (oper.active_add() || oper.output_add()) {
		for(gamearchive::const_iterator i=gar.begin();i!=gar.end();++i) {
			if (i->romset_required() && !i->usable_romzip_has()) {
				string path = cfg.romnewpath_get().file_get() + "/" + i->name_get() + ".zip";

				ziparchive::iterator j;

				// create zip and insert in container
				try {
					j = zar.open_and_insert( ziprom(path, zip_own, false) );
				} catch (error &e) {
					throw e << " creating zip " << path;
				}

				try {
					rom_add(oper, *j, *i, zar, out);
				} catch (error& e) {
					throw e << " scanning rom " << j->file_get();
				}
			}
		}
	}

	// move zips
	if (oper.active_move() || oper.output_move()) {
		for(filepath_container::const_iterator i=unknow.begin();i!=unknow.end();++i) {

			ziparchive::iterator j = zar.find( i->file_get() );
			if (j == zar.end())
				throw error() << "Failed internal check on moving the unknown zip " << i->file_get();

			ziprom reject( cfg.romunknowpath_get().file_get() + "/" + file_name(j->file_get()), zip_unknow, false );

			try {
				rom_move(oper, *j, reject, out);
			} catch (error& e) {
				throw e << " moving zip " << j->file_get() << " to " << reject.file_get();
			}

			zar.update( reject );
		}
	}
}

void set_rom_scan(const operation& oper, ziparchive& zar, gamearchive& gar, const config& cfg, output& out, const analyze& ana) {
	// scan zips
	for(ziparchive::iterator i=zar.begin();i!=zar.end();++i) {
		assert( i->is_open() );

		if (i->type_get() == zip_own) {
			gamearchive::iterator g = gar.find( game( file_basename( i->file_get() ) ) );

			if (g == gar.end() || !g->romset_required()) {
				// ignored
			} else {
				ziprom reject( cfg.romunknowpath_get().file_get() + "/" + g->name_get() + ".zip", zip_unknow, false );

				try {
					rom_scan(oper, *i, reject, *g, zar, out, ana);
				} catch (error& e) {
					throw e << " scanning rom " << i->file_get();
				}

				zar.update( reject );
			}
		}
	}
}

void all_sample_scan(const operation& oper, filepath_container& zar, gamearchive& gar, config& cfg, output& out, const analyze& ana) {
	filepath_container unknow;

	// scan zips
	for(filepath_container::iterator i=zar.begin();i!=zar.end();++i) {
		gamearchive::iterator g = gar.find( game( file_basename( i->file_get() ) ) );
		if (g == gar.end() || !g->sampleset_required()) {
			unknow.insert( unknow.end(), filepath( i->file_get() ) );
		} else {
			ziprom z(i->file_get(), zip_own, false);

			z.open();

			ziprom reject( cfg.sampleunknowpath_get().file_get() + "/" + file_name(i->file_get()), zip_unknow, false );

			try {
				sample_scan(oper,z,reject,*g,out,ana);
			} catch (error& e) {
				throw e << " scanning sample " << i->file_get();
			}
		}
	}

	// move zips
	if (oper.active_move() || oper.output_move()) {
		for(filepath_container::const_iterator i=unknow.begin();i!=unknow.end();++i) {

			ziprom z(i->file_get(), zip_own, false);

			z.open();

			ziprom reject( cfg.sampleunknowpath_get().file_get() + "/" + file_name(i->file_get()), zip_unknow, false );

			try {
				sample_move(oper, z, reject, out);
			} catch (error& e) {
				throw e << " moving zip " << i->file_get() << " to " << reject.file_get();
			}
		}
	}
}

void set_sample_scan(const operation& oper, filepath_container& zar, gamearchive& gar, config& cfg, output& out, const analyze& ana) {
	// scan zips
	for(filepath_container::iterator i=zar.begin();i!=zar.end();++i) {
		gamearchive::iterator g = gar.find( game( file_basename( i->file_get() ) ) );
		if (g == gar.end() || !g->sampleset_required()) {
			// ignore
		} else {
			ziprom z(i->file_get(), zip_own, false);

			z.open();

			ziprom reject( cfg.sampleunknowpath_get().file_get() + "/" + file_name(i->file_get()), zip_unknow, false );

			try {
				sample_scan(oper,z,reject,*g,out,ana);
			} catch (error& e) {
				throw e << " scanning sample " << i->file_get();
			}
		}
	}
}

void all_unknow_scan(ziparchive& zar, const config& cfg, output& out) {

	// scan unknow zips
	for(ziparchive::iterator i=zar.begin();i!=zar.end();++i) {
		assert( i->is_open() );

		if (i->type_get() == zip_unknow && !i->is_readonly()) {
			try {
				unknow_scan(*i, zar);
			} catch (error& e) {
				throw e << " scanning zip " << i->file_get();
			}
		}
	}
}

// ----------------------------------------------------------------------------
// all_report

void report_rom_zip(const ziparchive& zar, gamearchive& gar, output& out, bool verbose, const analyze& ana) {
	for(ziparchive::const_iterator i=zar.begin();i!=zar.end();++i) {
		if (i->type_get() == zip_own) {
			gamearchive::iterator g = gar.find( game( file_basename( i->file_get() ) ) );

			if (g == gar.end() || !g->romset_required()) {
				// ignore
			} else {
				rom_report(*i,*g,out,verbose,ana);
			}
		}
	}
}

void report_sample_zip(const filepath_container& zar, gamearchive& gar, output& out, bool verbose, const analyze& ana) {
	for(filepath_container::const_iterator i=zar.begin();i!=zar.end();++i) {
		gamearchive::iterator g = gar.find( game( file_basename( i->file_get() ) ) );

		if (g == gar.end() || !g->sampleset_required()) {
			// ignore
		} else {
			ziprom z(i->file_get(), zip_own, true);

			z.open();

			sample_report(z, *g, out, verbose, ana);

			z.close();
		}
	}
}

// ----------------------------------------------------------------------------
// all_report

void report_rom_set(const gamearchive& gar, output& out) {
	unsigned duplicate = 0;
	for(gamearchive::const_iterator i=gar.begin();i!=gar.end();++i) {
		if (i->rzs_get().size()>1) {
			++duplicate;
			out.state_gamerom("game_rom_dup", *i, gar);
			for(zippath_container::const_iterator j=i->rzs_get().begin();j!=i->rzs_get().end();++j) {
				if (j->good_get())
					out.ziptag("zip_rom_dup", j->file_get(), "good");
				else
					out.ziptag("zip_rom_dup", j->file_get(), "bad");
			}
			out() << endl;
		}
	}

	out.c("total_game_rom_dup",duplicate);
	out() << endl;

	unsigned ok_parent = 0;
	unsigned long long ok_parent_size = 0;
	unsigned long long ok_parent_size_zip = 0;
	unsigned ok_clone = 0;
	unsigned long long ok_clone_size = 0;
	unsigned long long ok_clone_size_zip = 0;
	for(gamearchive::const_iterator i=gar.begin();i!=gar.end();++i) {
		if (!i->romset_required() || i->good_romzip_has()) {
			// increase counter
			if (i->cloneof_get().length()) {
				++ok_clone;
				if (i->romset_required()) {
					ok_clone_size += i->size_get();
					ok_clone_size_zip += i->good_romzip_size();
				}
			} else {
				++ok_parent;
				if (i->romset_required()) {
					ok_parent_size += i->size_get();
					ok_parent_size_zip += i->good_romzip_size();
				}
			}
			out.state_gamerom("game_rom_good", *i, gar);
		}
	}
	out() << endl;

	out.csz("total_game_rom_good_parent", ok_parent, ok_parent_size, ok_parent_size_zip);
	out.csz("total_game_rom_good_clone", ok_clone, ok_clone_size, ok_clone_size_zip);
	out.csz("total_game_rom_good", ok_parent+ok_clone, ok_clone_size+ok_parent_size, ok_clone_size_zip+ok_parent_size_zip);
	out() << endl;

	unsigned wrong_clone = 0;
	unsigned long long wrong_clone_size = 0;
	unsigned wrong_parent = 0;
	unsigned long long wrong_parent_size = 0;
	for(gamearchive::const_iterator i=gar.begin();i!=gar.end();++i) {
		if (i->romset_required() && !i->good_romzip_has() && i->bad_romzip_has()) {
			// increase counter
			if (i->cloneof_get().length()) {
				++wrong_clone;
				wrong_clone_size += i->size_get();
			} else {
				++wrong_parent;
				wrong_parent_size += i->size_get();
			}
			out.state_gamerom("game_rom_bad", *i, gar);
		}
	}
	out() << endl;

	out.cs("total_game_rom_bad_parent", wrong_parent, wrong_parent_size);
	out.cs("total_game_rom_bad_clone", wrong_clone, wrong_clone_size);
	out.cs("total_game_rom_bad", wrong_parent+wrong_clone, wrong_clone_size+wrong_parent_size);
	out() << endl;

	unsigned miss_parent = 0;
	unsigned miss_clone = 0;
	unsigned long long miss_parent_size = 0;
	unsigned long long miss_clone_size = 0;
	for(gamearchive::const_iterator i=gar.begin();i!=gar.end();++i) {
		// only if have almost on rom and not exist any zip
		if (i->romset_required() && i->rzs_get().size()==0) {
			// increase counter
			if (i->cloneof_get().length()) {
				++miss_clone;
				miss_clone_size += i->size_get();
			} else {
				++miss_parent;
				miss_parent_size += i->size_get();
			}
			out.state_gamerom("game_rom_miss", *i, gar);
		}
	}
	out() << endl;

	out.cs("total_game_rom_miss_parent", miss_parent, miss_parent_size);
	out.cs("total_game_rom_miss_clone", miss_clone, miss_clone_size);
	out.cs("total_game_rom_miss", miss_parent+miss_clone, miss_clone_size+miss_parent_size);
	out() << endl;
}

void report_sample_set(const gamearchive& gar, output& out) {
	unsigned duplicate = 0;
	for(gamearchive::const_iterator i=gar.begin();i!=gar.end();++i) {
		if (i->szs_get().size()>1) {
			++duplicate;
			out.state_gamesample("game_sample_dup", *i);
			for(zippath_container::const_iterator j=i->szs_get().begin();j!=i->szs_get().end();++j) {
				if (j->good_get()) 
					out.ziptag("zip_sample_dup", j->file_get(), "good");
				else
					out.ziptag("zip_sample_dup", j->file_get(), "bad");
			}
			out() << endl;
		}
	}
	
	out.c("total_game_sample_dup", duplicate);
	out() << endl;

	unsigned ok = 0;
	unsigned long long ok_size_zip = 0;
	for(gamearchive::const_iterator i=gar.begin();i!=gar.end();++i) {
		if (i->good_samplezip_has()) {
			++ok;
			ok_size_zip += i->good_samplezip_size();
			out.state_gamesample("game_sample_good", *i);
		}
	}
	out() << endl;

	out.cz("total_game_sample_good", ok, ok_size_zip);
	out() << endl;

	unsigned wrong = 0;
	for(gamearchive::const_iterator i=gar.begin();i!=gar.end();++i) {
		if (!i->good_samplezip_has() && i->bad_samplezip_has()) {
			++wrong;
			out.state_gamesample("game_sample_bad", *i);
		}
	}
	out() << endl;

	out.c("total_game_sample_bad", wrong);
	out() << endl;

	unsigned miss = 0;
	for(gamearchive::const_iterator i=gar.begin();i!=gar.end();++i) {
		// only if have almost on sample and not exist any zip
		if (i->ss_get().size()>0 && i->szs_get().size()==0) {
			// increase counter
			++miss;
			out.state_gamesample("game_sample_miss", *i);
		}
	}
	out() << endl;

	out.c("total_game_sample_miss", miss);

	out() << endl;
}

// ----------------------------------------------------------------------------
// command

void equal(const gamearchive& gar, ostream& out) {
	gamerom_by_crc_multiset rcb;

	// insert rom
	for(gamearchive::const_iterator i=gar.begin();i!=gar.end();++i) {
		for(rom_by_name_set::const_iterator j=i->rs_get().begin();j!=i->rs_get().end();++j) {
			rcb.insert( gamerom( i->name_get(), j->name_get(), j->size_get(), j->crc_get(), j->nodump_get() ) );
		}
	}

	unsigned equal = 0;
	
	gamerom_by_crc_multiset::const_iterator start = rcb.begin();
	while (start != rcb.end()) {
		unsigned count = 1;
		gamerom_by_crc_multiset::const_iterator end = start;
		++end;
		while (end!=rcb.end() && (*start).crc_get()==(*end).crc_get() && (*start).size_get()==(*end).size_get()) {
			++end;
			++count;
		}

		if (count>1) {
			++equal; 
			out << "group " << dec << (*start).size_get();
			out << " " << hex << setw(8) << setfill('0') << (*start).crc_get();
			out << endl;
			while (start != end) {
				out << "rom " << (*start).game_get().c_str() << "/" << (*start).name_get();
				gamearchive::const_iterator g = gar.find( (*start).game_get() );
				// print parent game also if not really exists
				if (g!=gar.end() && (*g).cloneof_get().length()) {
					out << " cloneof " << (*g).cloneof_get();
				}
				out << endl;
				++start;
			}
			out << endl;
		}
		start = end;
	}

	out.setf(ios::right, ios::adjustfield);

	out << "total_group " << setw(8) << setfill(' ') << dec << equal << endl;

	out.setf(ios::left, ios::adjustfield);

	out << endl;
}

void ident_data(const string& file, crc_t crc, unsigned size, const gamearchive& gar, ostream& out) {
	out << file << endl;

	for(gamearchive::const_iterator i=gar.begin();i!=gar.end();++i) {
		for(rom_by_name_set::const_iterator j=i->rs_get().begin();j!=i->rs_get().end();++j) {
			if (j->crc_get()==crc && j->size_get()==size) {
				out << "\t" << setw(8) << i->name_get().c_str() << " " << setw(12) << j->name_get().c_str() << " " << i->description_get() << endl;
			}
		}
	}
}

void ident_zip(const string& file, const gamearchive& gar, ostream& out) {
	zip z(file);

	z.open();

	for(zip::const_iterator i=z.begin();i!=z.end();++i) {
		ident_data(z.file_get() + "/" + i->name_get(), i->crc_get(), i->uncompressed_size_get(), gar, out );
	}
}

void ident_file(const string& file, const gamearchive& gar, ostream& out) {
	struct stat st;
	if (stat(file.c_str(),&st) != 0)
		throw error() << "Failed stat on file " << file;

	if (S_ISDIR(st.st_mode)) {
		DIR* d = opendir(file.c_str());
		if (!d)
			throw error() << "Failed open dir " << file;

		struct dirent* dd;
		while ((dd = readdir(d)) != 0) {
			if (dd->d_name[0] != '.')
				ident_file(file + "/" + dd->d_name, gar, out);
		}

		closedir(d);
	} else if (file_compare(file_ext(file),".zip")==0) {
		ident_zip(file, gar, out);
	} else {
		unsigned size = file_size(file);
		crc_t crc = file_crc(file);
		ident_data(file,crc,size,gar,out);
	}
}

void bbs(const gamearchive& gar, ostream& out) {
	for(gamearchive::const_iterator i=gar.begin();i!=gar.end();++i) {
		out << i->name_get() << ".zip";
		out << " " << i->size_get();
		out << " " << i->description_get();
		gamearchive::const_iterator parent = gar.find( game( i->cloneof_get() ) );
		if (parent != gar.end()) {
			out << " [clone]";
		}
		out << ", " << i->manufacturer_get() << ", " << i->year_get() << endl;
	}
}

void version() {
	cout << PACKAGE " v" VERSION " by Andrea Mazzoleni" << endl;
}

void usage() {
	version();

	cout << "Usage: advscan [options] < info.txt" << endl;
	cout << endl;
	cout << "Modes:" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-r, --rom        ", "-r") "  Operate on rom sets" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-s, --sample     ", "-s") "  Operate on sample sets" << endl;
	cout << "Operations:" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-a, --add-zip    ", "-a") "  Add missing zips" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-b, --add-bin    ", "-b") "  Add missing files in zips" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-d, --del-zip    ", "-d") "  Delete unknown zips" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-u, --del-unknown", "-u") "  Delete unknown files in zips" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-g, --del-garbage", "-g") "  Delete garbage files" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-t, --del-text   ", "-t") "  Delete unused text files" << endl;
	cout << "Shortcuts:" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-R, --rom-std    ", "-R") "  Shortcut for -rabdug" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-S, --sample-std ", "-S") "  Shortcut for -sabdug" << endl;
	cout << "Commands:" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-e, --equal      ", "-e") "  Print list of roms with equal crc+size" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-i, --ident file ", "-i") "  Identify roms by crc and size" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-b, --bbs        ", "-b") "  Output a 'files.bbs' for roms .zip" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-h, --help       ", "-h") "  Help of the program" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-V, --version    ", "-V") "  Version of the program" << endl;
	cout << "Options:" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-p, --report     ", "-p") "  Write an extensive report" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-n, --print-only ", "-n") "  Only print operations, do nothing" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-v, --verbose    ", "-v") "  Verbose output" << endl;
}

#ifdef HAVE_GETOPT_LONG
struct option long_options[] = {
	{"rom", 0, 0, 'r'},
	{"rom-std", 0, 0, 'R'},
	{"sample", 0, 0, 's'},
	{"sample-std", 0, 0, 'S'},

	{"add-zip", 0, 0, 'a'},
	{"add-bin", 0, 0, 'b'},
	{"del-zip", 0, 0, 'd'},
	{"del-unknown", 0, 0, 'u'},
	{"del-text", 0, 0, 't'},
	{"del-garbage", 0, 0, 'g'},

	{"cfg", 1, 0, 'c'},

	{"bbs", 0, 0, 'l'},
	{"equal", 0, 0, 'e'},
	{"ident", 0, 0, 'i'},

	{"report", 0, 0, 'p'},

	{"print-only", 0, 0, 'n'},

	{"verbose", 0, 0, 'v'},
	{"help", 0, 0, 'h'},
	{"version", 0, 0, 'V'},
	{0, 0, 0, 0}
};
#endif

#define OPTIONS "rRsSabdutgc:leipnvhV"

void run(int argc, char* argv[]) {
	if (argc<=1) {
		usage();
		exit(EXIT_FAILURE);
	}

	// read options
	bool flag_rom = false;
	bool flag_sample = false;
	bool flag_equal = false;
	bool flag_bbs = false;
	bool flag_report = false;
	bool flag_print_only = false;
	bool flag_verbose = false;
	bool flag_move = false;
	bool flag_add = false;
	bool flag_fix = false;
	bool flag_remove_binary = false;
	bool flag_remove_text = false;
	bool flag_remove_garbage = false;
	bool flag_ident = false;
	operation oper;
	string cfg_file;

	int c = 0;

	opterr = 0; // don't print errors

	while ((c =
#ifdef HAVE_GETOPT_LONG
		getopt_long(argc, argv, OPTIONS, long_options, 0))
#else
		getopt(argc, argv, OPTIONS))
#endif
	!= EOF) {
		switch (c) {
			case 'r' :
				flag_rom = true;
				break;
			case 's' :
				flag_sample = true;
				break;
			case 'R' :
				flag_rom = true;
				flag_add = true;
				flag_fix = true;
				flag_move = true;
				flag_remove_binary = true;
				flag_remove_garbage = true;
				break;
			case 'S' :
				flag_sample = true;
				flag_add = true;
				flag_fix = true;
				flag_move = true;
				flag_remove_binary = true;
				flag_remove_garbage = true;
				break;
			case 'a' :
				flag_add = true;
				break;
			case 'b' :
				flag_fix = true;
				break;
			case 'd' :
				flag_move = true;
				break;
			case 'u' :
				flag_remove_binary = true;
				break;
			case 't' :
				flag_remove_text = true;
				break;
			case 'g' :
				flag_remove_garbage = true;
				break;
			case 'c' :
				cfg_file = optarg;
				break;
			case 'l' :
				flag_bbs = true;
				break;
			case 'e' :
				flag_equal = true;
				break;
			case 'i' :
				flag_ident = true;
				break;
			case 'p' :
				flag_report = true;
				break;
			case 'n' :
				flag_print_only = true;
				break;
			case 'h' :
				usage();
				return;
			case 'V' :
				version();
				return;
			case 'v' :
				flag_verbose = true;
				break;
			default: {
				// not optimal code for g++ 2.95.3
				string opt;
				opt = (char)optopt;
				throw error() << "Unknow option `" << opt << "'";
			}
		} 
	}

	oper.active_set(!flag_print_only);
	oper.output_set(flag_print_only);
	oper.operation_set(flag_move, flag_add, flag_fix, flag_remove_binary, flag_remove_text, flag_remove_garbage);

	// some operation is requested
	bool flag_operation = flag_move || flag_add || flag_fix || flag_remove_binary || flag_remove_text || flag_remove_garbage;

	// some real operation is requested, i.e. not a simulated operation
	bool flag_change = !flag_print_only && flag_operation;

	if ((flag_rom || flag_sample) && !(flag_operation || flag_report)) {
		usage();
		exit(EXIT_FAILURE);
	}

	if (!(flag_rom || flag_sample) && (flag_operation || flag_report)) {
		usage();
		exit(EXIT_FAILURE);
	}

	// set of all game
	gamearchive gar;

	if (!gar.load(cin))
		throw error() << "Failed read at row " << info_row_get()+1 << " at column " << info_col_get()+1 << " on the info file";
	if (gar.begin() == gar.end())
		throw error() << "Empty info file";

	if (flag_ident) {
		for(int i=optind;i<argc;++i)
			ident_file(argv[i],gar,cout);
	}

	if (flag_equal)
		equal(gar,cout);

	if (flag_bbs)
		bbs(gar,cout);

	if (flag_rom || flag_sample) {
		config cfg(cfg_file, flag_rom, flag_sample, flag_change);

		output out(cout);
		analyze ana(gar);

		if (flag_rom) {
			ziparchive zar;

			if (flag_operation) {
				all_rom_load(zar,cfg);
				all_rom_scan(oper,zar,gar,cfg,out,ana);
			} else {
				set_rom_load(zar,cfg);
				set_rom_scan(oper,zar,gar,cfg,out,ana);
			}

			if (flag_report) {
				report_rom_zip(zar,gar,out,flag_verbose,ana);
				report_rom_set(gar,out);
			}

			if (flag_change)
				all_unknow_scan(zar,cfg,out);
		}

		if (flag_sample) {
			filepath_container zar;
			
			set_sample_load(zar,cfg);
			if (flag_operation) {
				all_sample_scan(oper,zar,gar,cfg,out,ana);
			} else {
				set_sample_scan(oper,zar,gar,cfg,out,ana);
			}

			if (flag_report) {
				report_sample_zip(zar,gar,out,flag_verbose,ana);
				report_sample_set(gar,out);
			}
		}
	}
}

int main(int argc, char* argv[]) {

	try {
		run(argc,argv);
	} catch (error& e) {
		cerr << e << endl;
		exit(EXIT_FAILURE);
	} catch (std::bad_alloc) {
		cerr << "Low memory" << endl;
		exit(EXIT_FAILURE);
	} catch (...) {
		cerr << "Unknow error" << endl;
		exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}


