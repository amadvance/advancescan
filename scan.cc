/*
 * This file is part of the Advance project.
 *
 * Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Andrea Mazzoleni
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

using namespace std;

void read_dir(const string& path, filepath_container& ds, bool recursive, const string& ext) {
	DIR* dir = opendir(path.c_str());
	if (!dir)
		throw error() << "Failed open on dir " << path;

	struct dirent* ent = readdir(dir);
	while (ent) {
		string subpath = path + "/" + ent->d_name;
		struct stat st;

		if (stat(subpath.c_str(), &st)!=0)
			throw error() << "Failed stat on file " << subpath;

		if (S_ISDIR(st.st_mode)) {
			if (recursive) {
				if (strcmp(ent->d_name, ".")!=0 && strcmp(ent->d_name, "..")!=0) {
					read_dir(subpath.c_str(), ds, recursive, ext);
				}
			}
		} else {
			if (file_compare(file_ext(ent->d_name), ext) == 0) {
				ds.insert(ds.end(), filepath(subpath.c_str()));
			}
		}
		ent = readdir(dir);
	}

	closedir(dir);
}

void read_zip(const string& path, ziparchive& zar, zip_type type, bool ignore_error, bool rename_error) {
	filepath_container ds;

	read_dir(path, ds, false, ".zip");

	for(filepath_container::iterator i=ds.begin();i!=ds.end();++i) {
		ziparchive::iterator j;

		try {
			j = zar.open_and_insert(ziprom(i->file_get(), type, true));
		} catch (error_invalid& e) {
			if (ignore_error) {
				cerr << "warning: damaged zip " << i->file_get() << "\n";
				cerr << "warning: " << e << "\n";
				cerr << "warning: ignoring it and resuming\n";
			} else if (rename_error) {
				cerr << "warning: damaged zip " << i->file_get() << "\n";
				cerr << "warning: " << e << "\n";

#if HAVE_LONG_FNAME
				string reject = i->file_get() + ".damaged";
#else
				string reject = file_basepath(i->file_get()) + ".bad";
#endif

				cerr << "warning: renaming it to " << reject << " and resuming\n";

				file_move(i->file_get(), reject);
			} else {
				throw e << " opening zip " << i->file_get();
			}
		}
	}
}

// ---------------------------------------------------------------------------
// statistics

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
		rom_by_name_set::iterator i = b.find(rom(z->name_get(), 0, 0, false));
		if (i == b.end()) { // if name unknown
			// rom to insert
			rom r(z->name_get(), z->uncompressed_size_get(), z->crc_get(), false);

			analyze_type t = ana(z->name_get(), z->uncompressed_size_get(), z->crc_get());
			switch (t) {
				case analyze_text :
					result.unk_text.insert(r);
					break;
				case analyze_binary :
					result.unk_binary.insert(r);
					break;
				case analyze_garbage :
					result.unk_garbage.insert(r);
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
					result.rom_equal.insert(*i);
				} else {
					// rom is wrong
					rom_bad r;
					r.r = *i;
					r.bad_size = z->uncompressed_size_get();
					r.bad_crc = z->crc_get();
					result.rom_bad.insert(result.rom_bad.end(), r);
				}
			}

			// remove from original bag
			b.erase(i);
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
		sample_by_name_set::iterator i = result.sample_miss.find(sample(z->name_get()));
		if (i == result.sample_miss.end()) { // if name unknown
			sample s(z->name_get());

			analyze_type t = ana(z->name_get(), z->uncompressed_size_get(), z->crc_get());
			switch (t) {
				case analyze_text :
					result.unk_text.insert(s);
					break;
				case analyze_binary :
					result.unk_binary.insert(s);
					break;
				case analyze_garbage :
					result.unk_garbage.insert(s);
					break;
			}
		} else { // if name know
			result.sample_equal.insert(*i);
			// remove from original bag
			result.sample_miss.erase(i);
		}
	}
}

struct disk_stat_t {
	sha1 hash;
	disk_by_name_set disk_equal;
	disk_by_name_set disk_bad;
	disk_by_name_set unk_binary;
};

void disk_stat(
	const string& z,
	const game& gam,
	disk_stat_t& result,
	const analyze& ana)
{
	string name = file_basename(z);

	result.hash = disk_sha1(z);

	disk_by_name_set::iterator i=gam.ds_get().find(disk(name));
	if (i == gam.ds_get().end()) {
		result.unk_binary.insert(disk(name));
	} else {
		if (!(result.hash == i->sha1_get())) {
			result.disk_bad.insert(disk(name));
		} else {
			result.disk_equal.insert(disk(name));
		}
	}
}

// ----------------------------------------------------------------------------
// add a new rom set

void rom_add(
	const operation& oper,
	ziprom& z,
	const game& g, 
	const ziparchive& zar,
	gamerom_by_crc_multiset& rcb,
	output& out) 
{
	bool title = false; // true if is printed the zip file name

	rom_by_name_set good;
	rom_by_name_set miss_unique;
	rom_by_name_set miss_shared;

	// split the roms in shared and unique
	for(rom_by_name_set::iterator i=g.rs_get().begin();i!=g.rs_get().end();++i) {
		pair<gamerom_by_crc_multiset::iterator,gamerom_by_crc_multiset::iterator> range;

		range = rcb.equal_range(gamerom("",*i));

		if (range.first != range.second)
			++range.first; // one element is always present and it's the rom itself

		if (range.first != range.second)
			miss_shared.insert(*i);
		else
			miss_unique.insert(*i);
	}

	// for any unique missig rom
	rom_by_name_set tmp_miss;
	for(rom_by_name_set::iterator i=miss_unique.begin();i!=miss_unique.end();++i) {
		bool found = false;
		bool added = false;

		// skip nodump
		if (i->nodump_get())
			continue;

		// check if is in another zip
		if (!found) {
			ziprom::const_iterator k;
			ziparchive::const_iterator j = zar.find_exclude(z, i->size_get(), i->crc_get(), k);
			if (j!=zar.end()) {
				if (oper.active_add() && !z.is_readonly()) {
					z.add(k, i->name_get());
					good.insert(*i);
					added = true;
				} 
				if (oper.output_add()) {
					out.title("rom_zip", title, z.file_get());
					out.cmd_rom("rom_good", "add", *i) << " " << k->parentname_get() << "/" << k->name_get() << "\n";
				}
				found = true;
			}
		}

		if (!added)
			// if not found insert in missing rom
			tmp_miss.insert(*i);
	}
	miss_unique = tmp_miss;

	// if all the roms are shared or at least one unique rom was found
	if (miss_unique.empty() || !good.empty()) {

		// for any shared missig rom
		rom_by_name_set tmp_miss;
		for(rom_by_name_set::iterator i=miss_shared.begin();i!=miss_shared.end();++i) {
			bool found = false;
			bool added = false;

			// skip nodump
			if (i->nodump_get())
				continue;

			// check if is in another zip
			if (!found) {
				ziprom::const_iterator k;
				ziparchive::const_iterator j = zar.find_exclude(z, i->size_get(), i->crc_get(), k);
				if (j!=zar.end()) {
					if (oper.active_add() && !z.is_readonly()) {
						z.add(k, i->name_get());
						good.insert(*i);
						added = true;
					}
					if (oper.output_add()) {
						out.title("rom_zip", title, z.file_get());
						out.cmd_rom("rom_good", "add", *i) << " " << k->parentname_get() << "/" << k->name_get() << "\n";
					}
					found = true;
				}
			}

			if (!added)
				// if not found insert in missing rom
				tmp_miss.insert(*i);
		}
		miss_shared = tmp_miss;
	}

	if (title)
		out() << "\n";

	// if zip created with almost one good rom
	if (!good.empty()) {
		// update the zip
		z.save();
		z.unload();

		if (!z.empty()) {
			// get file information
			struct stat fst;
			string path = z.file_get();
			if (stat(path.c_str(), &fst) != 0)
				throw error() << "Failed stat file " << z.file_get();

			// if no rom is missing
			if (miss_unique.empty() && miss_shared.empty()) {
				// add zip to directory list of game as good
				g.rzs_add(infopath(z.file_get(), true, fst.st_size, z.is_readonly()));
			} else {
				// add zip to directory list of game as bad
				g.rzs_add(infopath(z.file_get(), false, fst.st_size, z.is_readonly()));
			}
		}
	}
}

// ----------------------------------------------------------------------------
// scan an existing rom set

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
					out.title("rom_zip", title, z.file_get());
					out.cmd_rom(s_add, s_cmd, s_name, s_size, s_crc) << " " << j->name_get() << "\n";
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
					out.title("rom_zip", title, z.file_get());
					out.cmd_rom(s_add, s_cmd, s_name, s_size, s_crc) << " " << name << "\n";
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
					out.title("rom_zip", title, z.file_get());
					out.cmd_rom(s_add, s_cmd, s_name, s_size, s_crc) << " " << reject.file_get() << "/" << name << "\n";
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
		ziparchive::const_iterator j = zar.find_exclude(z, s_size, s_crc, k);
		if (j!=zar.end()) {
			if (oper.active_fix() && !z.is_readonly()) {
				z.add(k, s_name, reject);
				added = true;
			}
			if (oper.output_fix()) {
				out.title("rom_zip", title, z.file_get());
				out.cmd_rom(s_add, s_cmd, s_name, s_size, s_crc) << " " << k->parentname_get() << "/" << k->name_get() << "\n";
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
	stat_rom_zip(z, gam, st, ana);

	// for any missing rom
	rom_by_name_set tmp_rom_miss; // missing rom
	for(rom_by_name_set::iterator i=st.rom_miss.begin();i!=st.rom_miss.end();++i) {
		bool found = false;
		bool added = false;

		if (rom_scan_add(
			oper,
			"rom_good", "add",
			z, reject, found, title,
			i->name_get(), i->size_get(), i->crc_get(), 
			st.unk_binary, zar, out)) {

			st.rom_equal.insert(*i);
			added = true;
		}

		if (!added) {
			// rom is missing
			tmp_rom_miss.insert(*i);
		}
	}
	st.rom_miss = tmp_rom_miss;

	// for any wrong rom (rom with corret name but bad crc or size)
	rom_bad_container tmp_rom_bad;
	for(rom_bad_container::iterator i=st.rom_bad.begin();i!=st.rom_bad.end();++i) {
		bool found = false;
		bool added = false;

		if (rom_scan_add(
			oper,
			"rom_good", "add",
			z, reject, found, title,
			i->name_get(), i->size_get(), i->crc_get(), 
			st.unk_binary, zar, out)) {

			st.rom_equal.insert(i->r);
			added = true;
		}

		if (!added)
			// if not found insert in wrong rom
			tmp_rom_bad.insert(tmp_rom_bad.end(), *i);
	}
	st.rom_bad = tmp_rom_bad;

	for(rom_by_name_set::iterator i=st.unk_binary.begin();i!=st.unk_binary.end();++i) {
		if (oper.active_remove_binary() && !z.is_readonly()) {
			z.remove(i->name_get(), reject);
		}
		if (oper.output_remove_binary()) {
			out.title("rom_zip", title, z.file_get());
			out.cmd_rom("binary", "remove", *i) << "\n";
		}
	}

	for(rom_by_name_set::iterator i=st.unk_text.begin();i!=st.unk_text.end();++i) {
		if (oper.active_remove_text() && !z.is_readonly()) {
			z.remove(i->name_get(), reject);
		}
		if (oper.output_remove_text()) {
			out.title("rom_zip", title, z.file_get());
			out.cmd_rom("text", "remove", *i) << "\n";
		}
	}

	for(rom_by_name_set::iterator i=st.unk_garbage.begin();i!=st.unk_garbage.end();++i) {
		if (oper.active_remove_garbage() && !z.is_readonly()) {
			z.remove(i->name_get());
		}
		if (oper.output_remove_garbage()) {
			out.title("rom_zip", title, z.file_get());
			out.cmd_rom("garbage", "remove", *i) << "\n";
		}
	}

	if (title)
		out() << "\n";

	// update the zip
	z.save();
	z.unload();

	if (!z.empty()) {
		// get file information
		struct stat fst;
		string path = z.file_get();
		if (stat(path.c_str(), &fst) != 0)
			throw error() << "Failed stat file " << z.file_get();

		// if no rom is missing or wrong. Ignore nodump.
		if (st.rom_miss.empty()
		    && st.rom_bad.empty()
		  ) {
			// add zip to directory list of game as good
			gam.rzs_add(infopath(z.file_get(), true, fst.st_size, z.is_readonly()));
		} else {
			// add zip to directory list of game as bad
			gam.rzs_add(infopath(z.file_get(), false, fst.st_size, z.is_readonly()));
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
	sample_stat(z, gam, st, ana);
	
	// for any miss sample
	sample_by_name_set tmp_rom_miss;
	for(sample_by_name_set::iterator i=st.sample_miss.begin();i!=st.sample_miss.end();++i) {
		bool found = false;
		bool added = false;

		if (!found) {
			// check if is present in remove bag with directory
			for(sample_by_name_set::iterator j=st.unk_binary.begin();j!=st.unk_binary.end();++j) {
				// compare name without dir
				if (file_compare(i->name_get(), file_name(j->name_get()))==0) {
					// found rom with different name
					if (oper.active_fix() && !z.is_readonly()) {
						z.add(j->name_get(), i->name_get());
						st.sample_equal.insert(*i);
						added = true;
					} 
					if (oper.output_fix()) {
						out.title("sample_zip", title, z.file_get());
						out.cmd_sample("sound", "add", *i) << " " << j->name_get() << "\n";
					}
					found = true;
					break;
				}
			}
		}

		if (!added)
			// if not found insert in missing sample
			tmp_rom_miss.insert(*i);
	}
	st.sample_miss = tmp_rom_miss;

	for(sample_by_name_set::iterator i=st.unk_binary.begin();i!=st.unk_binary.end();++i) {
		if (oper.active_remove_binary() && !z.is_readonly()) {
			z.remove(i->name_get(), reject);
		}
		if (oper.output_remove_binary()) {
			out.title("sample_zip", title, z.file_get());
			out.cmd_sample("binary", "remove", *i) << "\n";
		}
	}

	for(sample_by_name_set::iterator i=st.unk_text.begin();i!=st.unk_text.end();++i) {
		if (oper.active_remove_text() && !z.is_readonly()) {
			z.remove(i->name_get(), reject);
		}
		if (oper.output_remove_text()) {
			out.title("sample_zip", title, z.file_get());
			out.cmd_sample("text", "remove", *i) << "\n";
		}
	}

	for(sample_by_name_set::iterator i=st.unk_garbage.begin();i!=st.unk_garbage.end();++i) {
		if (oper.active_remove_garbage() && !z.is_readonly()) {
			z.remove(i->name_get());
		}
		if (oper.output_remove_garbage()) {
			out.title("sample_zip", title, z.file_get());
			out.cmd_sample("garbage", "remove", *i) << "\n";
		}
	} 

	if (title)
		out() << "\n";

	// update the zip
	z.save();
	z.unload();

	if (!z.empty()) {
		// get file information
		struct stat fst;
		string path = z.file_get();
		if (stat(path.c_str(), &fst) != 0)
			throw error() << "Failed stat file " << z.file_get();

		// if no sample is missing
		if (st.sample_miss.empty()) {
			gam.szs_add(infopath(z.file_get(), true, fst.st_size, z.is_readonly()));
		} else {
			gam.szs_add(infopath(z.file_get(), false, fst.st_size, z.is_readonly()));
		}
	}

	// update the reject zip
	reject.save();
	reject.unload();
}

void disk_scan(
	const operation& oper,
	const string& z,
	const game& gam,
	output& out,
	const analyze& ana)
{
	disk_stat_t st;
	disk_stat(z, gam, st, ana);

	// get file information
	struct stat fst;
	if (stat(z.c_str(), &fst) != 0)
		throw error() << "Failed stat file " << z;

	if (!st.disk_equal.empty()) {
		gam.dzs_add(infopath(z, true, fst.st_size, false));
	} else {
		gam.dzs_add(infopath(z, false, fst.st_size, false));
	}
}

void unknown_scan(
	ziprom& z,
	const ziparchive& zar)
{
	// create the list of all the roms to remove
	rom_by_name_set remove;
	for(ziprom::const_iterator i=z.begin();i!=z.end();++i) {
		rom r(i->name_get(), i->uncompressed_size_get(), i->crc_get(), false);
		remove.insert(r);
	}

	for(rom_by_name_set::iterator i=remove.begin();i!=remove.end();++i) {
		ziprom::const_iterator k;

		// search if the rom is present in a file in the own set
		ziparchive::const_iterator j = zar.find(i->size_get(), i->crc_get(), zip_own, k);
		if (j != zar.end()) {
			// if present remove the duplicate in the unknown set
			z.remove(i->name_get());
		}
	}

	// update the zip
	z.save();
	z.unload();
}

// ----------------------------------------------------------------------------
// move a rom set

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
		remove.insert(r);
	}

	for(rom_by_name_set::iterator i=remove.begin();i!=remove.end();++i) {
		if (oper.active_move() && !z.is_readonly()) {
			z.remove(i->name_get(), reject);
		}
		if (oper.output_move()) {
			out.title("rom_zip", title, z.file_get());
			out.cmd_rom("binary", "remove", *i) << "\n";
		}
	}

	if (title)
		out() << "\n";

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
		remove.insert(r);
	}

	for(rom_by_name_set::iterator i=remove.begin();i!=remove.end();++i) {
		if (oper.active_move() && !z.is_readonly()) {
			z.remove(i->name_get(), reject);
		}
		if (oper.output_move()) {
			out.title("sample_zip", title, z.file_get());
			out.cmd_rom("binary", "remove", *i) << "\n";
		}
	}

	if (title)
		out() << "\n";

	// save the reject zip. First save and only after delete the data.
	reject.save();
	reject.unload();

	// delete the empty zip.
	z.save();
	z.unload();
}

void disk_move(
	const operation& oper,
	const string& z,
	const string& reject,
	output& out)
{
	bool title = false;

	if (oper.active_move()) {
		cerr << "log: " << "move " << z << " to " << reject << endl;
		file_move(z, reject);
	}

	if (oper.output_move()) {
		out.title("disk_chd", title, z);
		out.cmd_disk("binary", "remove", file_name(z)) << "\n";
	}

	if (title)
		out() << "\n";
}

// ----------------------------------------------------------------------------
// report

void rom_report(
	const ziprom& z,
	const game& gam,
	output& out,
	bool verbose,
	const analyze& ana)
{
	// if empty don't report
	if (z.empty())
		return;

	// get rom status
	rom_stat_t st;
	stat_rom_zip(z, gam, st, ana);

	// print the report
	if (!st.rom_miss.empty() || !st.rom_bad.empty()
		|| (verbose && (!st.nodump_miss.empty() || !st.nodump_bad.empty() || !st.nodump_equal.empty() || !st.unk_text.empty() || !st.unk_binary.empty() || !st.unk_garbage.empty()))
	) {
		bool title = false;

		out.title("rom_zip", title, z.file_get());

		for(rom_by_name_set::iterator i=st.rom_miss.begin();i!=st.rom_miss.end();++i) {
			out.state_rom("rom_miss", *i) << "\n";
		}

		for(rom_by_name_set::iterator i=st.nodump_miss.begin();i!=st.nodump_miss.end();++i) {
			out.state_rom("nodump_miss", *i) << "\n";
		}

		for(rom_bad_container::iterator i=st.rom_bad.begin();i!=st.rom_bad.end();++i) {
			out.state_rom_real("rom_bad", i->r, i->bad_size_get(), i->bad_crc_get()) << "\n";
		}

		for(rom_bad_container::iterator i=st.nodump_bad.begin();i!=st.nodump_bad.end();++i) {
			out.state_rom_real("nodump_bad", i->r, i->bad_size_get(), i->bad_crc_get()) << "\n";
		}

		for(rom_by_name_set::iterator i=st.unk_text.begin();i!=st.unk_text.end();++i) {
			out.state_rom("text", *i) << "\n";
		}

		for(rom_by_name_set::iterator i=st.unk_binary.begin();i!=st.unk_binary.end();++i) {
			out.state_rom("binary", *i) << "\n";
		}

		for(rom_by_name_set::iterator i=st.unk_garbage.begin();i!=st.unk_garbage.end();++i) {
			out.state_rom("garbage", *i) << "\n";
		}

		for(rom_by_name_set::iterator i=st.nodump_equal.begin();i!=st.nodump_equal.end();++i) {
			out.state_rom("nodump_good", i->name_get(), i->size_get(), i->crc_get()) << "\n";
		}

		for(rom_by_name_set::iterator i=st.rom_equal.begin();i!=st.rom_equal.end();++i) {
			out.state_rom("rom_good", *i) << "\n";
		}

		out() << "\n";
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
	if (z.empty())
		return;

	// get rom status
	sample_stat_t st;
	sample_stat(z, gam, st, ana);

	// print the report
	if (!st.sample_miss.empty()
		|| (verbose && (!st.unk_text.empty() || !st.unk_binary.empty() || !st.unk_garbage.empty()))
	) {
		bool title = false;

		out.title("sample_zip", title, z.file_get());

		for(sample_by_name_set::iterator i=st.sample_miss.begin();i!=st.sample_miss.end();++i) {
			out.state_sample("sound_miss", *i) << "\n";
		}

		for(sample_by_name_set::iterator i=st.unk_text.begin();i!=st.unk_text.end();++i) {
			out.state_sample("text", *i) << "\n";
		}
		
		for(sample_by_name_set::iterator i=st.sample_equal.begin();i!=st.sample_equal.end();++i) {
			out.state_sample("sound_good", *i) << "\n";
		}

		out() << "\n";
	}
}

void disk_report(
	const string& z,
	const game& gam,
	output& out,
	bool verbose,
	const analyze& ana)
{
	disk_stat_t st;
	disk_stat(z, gam, st, ana);

	// print the report
	if (!st.disk_bad.empty() || !st.unk_binary.empty()) {
		bool title = false;

		out.title("disk_chd", title, z);

		for(disk_by_name_set::iterator i=st.unk_binary.begin();i!=st.unk_binary.end();++i) {
			out.state_disk("binary", *i) << "\n";
		}

		for(disk_by_name_set::iterator i=st.disk_bad.begin();i!=st.disk_bad.end();++i) {
			out.state_disk_real("disk_bad", *i, st.hash) << "\n";
		}
		
		for(disk_by_name_set::iterator i=st.disk_equal.begin();i!=st.disk_equal.end();++i) {
			out.state_disk("disk_good", *i) << "\n";
		}

		out() << "\n";
	}
}

// ---------------------------------------------------------------------------
// load

void all_rom_load(ziparchive& zar, const config& cfg)
{
	// read own zip
	for(filepath_container::const_iterator i=cfg.rompath_get().begin();i!=cfg.rompath_get().end();++i) {
		read_zip(i->file_get(), zar, zip_own, false, true);
	}

	// read unknown zip
	read_zip(cfg.romunknownpath_get().file_get(), zar, zip_unknown, false, true);

	// read import zip
	for(filepath_container::const_iterator i=cfg.romreadonlytree_get().begin();i!=cfg.romreadonlytree_get().end();++i) {
		read_zip(i->file_get(), zar, zip_import, true, false);
	}
}

void set_rom_load(ziparchive& zar, const config& cfg)
{
	for(filepath_container::const_iterator i=cfg.rompath_get().begin();i!=cfg.rompath_get().end();++i) {
		read_zip(i->file_get(), zar, zip_own, false, true);
	}
}

void set_sample_load(filepath_container& zar, const config& cfg)
{
	filepath_container ds;

	for(filepath_container::const_iterator i=cfg.samplepath_get().begin();i!=cfg.samplepath_get().end();++i) {
		read_dir(i->file_get(), ds, false, ".zip");
	}

	for(filepath_container::const_iterator i=ds.begin();i!=ds.end();++i) {
		try {
			zip z(i->file_get());

			z.open(); // detect damaged archives

			zar.insert(zar.end(), *i);

		} catch (error_invalid& e) {
			cerr << "warning: damaged zip " << i->file_get() << "\n";
			cerr << "warning: " << e << "\n";

#if HAVE_LONG_FNAME
			string reject = i->file_get() + ".damaged";
#else
			string reject = file_basepath(i->file_get()) + ".bad";
#endif

			cerr << "warning: renaming it to " << reject << " and resuming\n";

			file_move(i->file_get(), reject);
		}
	}
}

void set_disk_load(filepath_container& zar, const config& cfg)
{
	filepath_container ds;

	for(filepath_container::const_iterator i=cfg.diskpath_get().begin();i!=cfg.diskpath_get().end();++i) {
		read_dir(i->file_get(), ds, false, ".chd");
	}

	for(filepath_container::const_iterator i=ds.begin();i!=ds.end();++i) {
		try {
			disk_sha1(i->file_get()); // detect damaged archives

			zar.insert(zar.end(), *i);

		} catch (error_invalid& e) {
			cerr << "warning: damaged chd " << i->file_get() << "\n";
			cerr << "warning: " << e << "\n";

#if HAVE_LONG_FNAME
			string reject = i->file_get() + ".damaged";
#else
			string reject = file_basepath(i->file_get()) + ".bad";
#endif

			cerr << "warning: renaming it to " << reject << " and resuming\n";

			file_move(i->file_get(), reject);
		}
	}
}

// ----------------------------------------------------------------------------
// scan

void all_rom_scan(const operation& oper, ziparchive& zar, gamearchive& gar, gamerom_by_crc_multiset& rcb, const config& cfg, output& out, const analyze& ana)
{
	filepath_container unknown; // container of unknown zip

	// scan zips
	for(ziparchive::iterator i=zar.begin();i!=zar.end();++i) {
		assert(i->is_open());

		if (i->type_get() == zip_own) {
			gamearchive::iterator g = gar.find(game(file_basename(i->file_get())));

			if (g == gar.end() || !g->is_romset_required()) {
				// insert in the unknown set, processed later
				unknown.insert(unknown.end(), filepath(i->file_get()));
			} else {
				ziprom reject(cfg.romunknownpath_get().file_get() + "/" + g->name_get() + ".zip", zip_unknown, false);

				try {
					rom_scan(oper, *i, reject, *g, zar, out, ana);
				} catch (error& e) {
					throw e << " scanning rom " << i->file_get();
				}

				zar.update(reject);
			}
		}
	}

	// add zips
	if (oper.active_add() || oper.output_add()) {
		for(gamearchive::const_iterator i=gar.begin();i!=gar.end();++i) {
			if (i->is_romset_required() && !i->has_usable_rom()) {
				string path = cfg.romnewpath_get().file_get() + "/" + i->name_get() + ".zip";

				ziparchive::iterator j;

				// create zip and insert in container
				try {
					j = zar.open_and_insert(ziprom(path, zip_own, false));
				} catch (error &e) {
					throw e << " creating zip " << path;
				}

				try {
					rom_add(oper, *j, *i, zar, rcb, out);
				} catch (error& e) {
					throw e << " scanning rom " << j->file_get();
				}
			}
		}
	}

	// move zips
	if (oper.active_move() || oper.output_move()) {
		for(filepath_container::const_iterator i=unknown.begin();i!=unknown.end();++i) {

			ziparchive::iterator j = zar.find(i->file_get());
			if (j == zar.end())
				throw error() << "Failed internal check on moving the unknown zip " << i->file_get();

			ziprom reject(cfg.romunknownpath_get().file_get() + "/" + file_name(j->file_get()), zip_unknown, false);

			try {
				rom_move(oper, *j, reject, out);
			} catch (error& e) {
				throw e << " moving zip " << j->file_get() << " to " << reject.file_get();
			}

			zar.update(reject);
		}
	}
}

void set_rom_scan(const operation& oper, ziparchive& zar, gamearchive& gar, const config& cfg, output& out, const analyze& ana)
{
	// scan zips
	for(ziparchive::iterator i=zar.begin();i!=zar.end();++i) {
		assert(i->is_open());

		if (i->type_get() == zip_own) {
			gamearchive::iterator g = gar.find(game(file_basename(i->file_get())));

			if (g == gar.end() || !g->is_romset_required()) {
				// ignored
			} else {
				ziprom reject(cfg.romunknownpath_get().file_get() + "/" + g->name_get() + ".zip", zip_unknown, false);

				try {
					rom_scan(oper, *i, reject, *g, zar, out, ana);
				} catch (error& e) {
					throw e << " scanning rom " << i->file_get();
				}

				zar.update(reject);
			}
		}
	}
}

void all_sample_scan(const operation& oper, filepath_container& zar, gamearchive& gar, config& cfg, output& out, const analyze& ana)
{
	filepath_container unknown;

	// scan zips
	for(filepath_container::iterator i=zar.begin();i!=zar.end();++i) {
		gamearchive::iterator g = gar.find(game(file_basename(i->file_get())));
		if (g == gar.end() || !g->is_sampleset_required()) {
			unknown.insert(unknown.end(), filepath(i->file_get()));
		} else {
			ziprom z(i->file_get(), zip_own, false);

			z.open();

			ziprom reject(cfg.sampleunknownpath_get().file_get() + "/" + file_name(i->file_get()), zip_unknown, false);

			try {
				sample_scan(oper, z, reject, *g, out, ana);
			} catch (error& e) {
				throw e << " scanning sample " << i->file_get();
			}
		}
	}

	// move zips
	if (oper.active_move() || oper.output_move()) {
		for(filepath_container::const_iterator i=unknown.begin();i!=unknown.end();++i) {

			ziprom z(i->file_get(), zip_own, false);

			z.open();

			ziprom reject(cfg.sampleunknownpath_get().file_get() + "/" + file_name(i->file_get()), zip_unknown, false);

			try {
				sample_move(oper, z, reject, out);
			} catch (error& e) {
				throw e << " moving zip " << i->file_get() << " to " << reject.file_get();
			}
		}
	}
}

void set_sample_scan(const operation& oper, filepath_container& zar, gamearchive& gar, config& cfg, output& out, const analyze& ana)
{
	// scan zips
	for(filepath_container::iterator i=zar.begin();i!=zar.end();++i) {
		gamearchive::iterator g = gar.find(game(file_basename(i->file_get())));
		if (g == gar.end() || !g->is_sampleset_required()) {
			// ignore
		} else {
			ziprom z(i->file_get(), zip_own, false);

			z.open();

			ziprom reject(cfg.sampleunknownpath_get().file_get() + "/" + file_name(i->file_get()), zip_unknown, false);

			try {
				sample_scan(oper, z, reject, *g, out, ana);
			} catch (error& e) {
				throw e << " scanning sample " << i->file_get();
			}
		}
	}
}

void all_disk_scan(const operation& oper, filepath_container& zar, gamearchive& gar, config& cfg, output& out, const analyze& ana)
{
	filepath_container unknown;

	// scan zips
	for(filepath_container::iterator i=zar.begin();i!=zar.end();++i) {
		string name = file_basename(i->file_get());
		gamearchive::iterator g;
		for(g=gar.begin();g!=gar.end();++g) {
			if (g->is_diskset_required()) {
				disk_by_name_set::iterator i = g->ds_get().find(disk(name));
				if (i != g->ds_get().end())
					break;
			}
		}
		if (g == gar.end()) {
			unknown.insert(unknown.end(), filepath(i->file_get()));
		} else {
			try {
				disk_scan(oper, i->file_get(), *g, out, ana);
			} catch (error& e) {
				throw e << " scanning disk " << i->file_get();
			}
		}
	}

	// move chd
	if (oper.active_move() || oper.output_move()) {
		for(filepath_container::const_iterator i=unknown.begin();i!=unknown.end();++i) {
			string reject = cfg.diskunknownpath_get().file_get() + "/" + file_name(i->file_get());
			try {
				disk_move(oper, i->file_get(), reject, out);
			} catch (error& e) {
				throw e << " moving disk " << i->file_get() << " to " << reject;
			}
		}
	}
}

void set_disk_scan(const operation& oper, filepath_container& zar, gamearchive& gar, config& cfg, output& out, const analyze& ana)
{
	filepath_container unknown;

	// scan zips
	for(filepath_container::iterator i=zar.begin();i!=zar.end();++i) {
		string name = file_basename(i->file_get());
		gamearchive::iterator g;
		for(g=gar.begin();g!=gar.end();++g) {
			if (g->is_diskset_required()) {
				disk_by_name_set::iterator i = g->ds_get().find(disk(name));
				if (i != g->ds_get().end())
					break;
			}
		}
		if (g == gar.end()) {
			unknown.insert(unknown.end(), filepath(i->file_get()));
		} else {
			try {
				disk_scan(oper, i->file_get(), *g, out, ana);
			} catch (error& e) {
				throw e << " scanning disk " << i->file_get();
			}
		}
	}
}

void all_unknown_scan(ziparchive& zar, const config& cfg, output& out)
{

	// scan unknown zips
	for(ziparchive::iterator i=zar.begin();i!=zar.end();++i) {
		assert(i->is_open());

		if (i->type_get() == zip_unknown && !i->is_readonly()) {
			try {
				unknown_scan(*i, zar);
			} catch (error& e) {
				throw e << " scanning zip " << i->file_get();
			}
		}
	}
}

// ----------------------------------------------------------------------------
// report_zip

void report_rom_zip(const ziparchive& zar, gamearchive& gar, output& out, bool verbose, const analyze& ana)
{
	for(ziparchive::const_iterator i=zar.begin();i!=zar.end();++i) {
		if (i->type_get() == zip_own) {
			gamearchive::iterator g = gar.find(game(file_basename(i->file_get())));

			if (g == gar.end() || !g->is_romset_required()) {
				// ignore
			} else {
				rom_report(*i, *g, out, verbose, ana);
			}
		}
	}
}

void report_sample_zip(const filepath_container& zar, gamearchive& gar, output& out, bool verbose, const analyze& ana)
{
	for(filepath_container::const_iterator i=zar.begin();i!=zar.end();++i) {
		gamearchive::iterator g = gar.find(game(file_basename(i->file_get())));

		if (g == gar.end() || !g->is_sampleset_required()) {
			// ignore
		} else {
			ziprom z(i->file_get(), zip_own, true);

			z.open();

			sample_report(z, *g, out, verbose, ana);

			z.close();
		}
	}
}

void report_disk_zip(const filepath_container& zar, gamearchive& gar, output& out, bool verbose, const analyze& ana)
{
	for(filepath_container::const_iterator i=zar.begin();i!=zar.end();++i) {
		string name = file_basename(i->file_get());
		gamearchive::iterator g;
		for(g=gar.begin();g!=gar.end();++g) {
			if (g->is_diskset_required()) {
				disk_by_name_set::iterator i = g->ds_get().find(disk(name));
				if (i != g->ds_get().end())
					break;
			}
		}
		if (g == gar.end()) {
			// ignore
		} else {
			disk_report(i->file_get(), *g, out, verbose, ana);
		}
	}
}

// ----------------------------------------------------------------------------
// report_set

void report_rom_set(const gamearchive& gar, output& out)
{
	unsigned duplicate = 0;
	for(gamearchive::const_iterator i=gar.begin();i!=gar.end();++i) {
		if (i->rzs_get().size()>1) {
			++duplicate;
			out.state_gamerom("game_rom_dup", *i, gar, false);
			for(zippath_container::const_iterator j=i->rzs_get().begin();j!=i->rzs_get().end();++j) {
				if (j->good_get())
					out.ziptag("zip_rom_dup", j->file_get(), "good");
				else
					out.ziptag("zip_rom_dup", j->file_get(), "bad");
			}
			out() << "\n";
		}
	}

	out.c("total_game_rom_dup", duplicate);
	out() << "\n";

	unsigned ok_parent = 0;
	unsigned long long ok_parent_size = 0;
	unsigned long long ok_parent_size_zip = 0;
	unsigned ok_clone = 0;
	unsigned long long ok_clone_size = 0;
	unsigned long long ok_clone_size_zip = 0;
	for(gamearchive::const_iterator i=gar.begin();i!=gar.end();++i) {
		if (!i->is_romset_required() || i->has_good_rom()) {
			if (i->cloneof_get().length()) {
				++ok_clone;
				if (i->is_romset_required()) {
					ok_clone_size += i->size_get();
					ok_clone_size_zip += i->good_rom_size();
				}
			} else {
				++ok_parent;
				if (i->is_romset_required()) {
					ok_parent_size += i->size_get();
					ok_parent_size_zip += i->good_rom_size();
				}
			}
			out.state_gamerom("game_rom_good", *i, gar, false);
		}
	}
	out() << "\n";

	out.csz("total_game_rom_good_parent", ok_parent, ok_parent_size, ok_parent_size_zip);
	out.csz("total_game_rom_good_clone", ok_clone, ok_clone_size, ok_clone_size_zip);
	out.csz("total_game_rom_good", ok_parent+ok_clone, ok_clone_size+ok_parent_size, ok_clone_size_zip+ok_parent_size_zip);
	out() << "\n";

	unsigned wrong_clone = 0;
	unsigned long long wrong_clone_size = 0;
	unsigned wrong_parent = 0;
	unsigned long long wrong_parent_size = 0;
	for(gamearchive::const_iterator i=gar.begin();i!=gar.end();++i) {
		if (i->is_romset_required() && !i->has_good_rom() && i->has_bad_rom()) {
			if (i->cloneof_get().length()) {
				++wrong_clone;
				wrong_clone_size += i->size_get();
			} else {
				++wrong_parent;
				wrong_parent_size += i->size_get();
			}
			out.state_gamerom("game_rom_bad", *i, gar, false);
		}
	}
	out() << "\n";

	out.cs("total_game_rom_bad_parent", wrong_parent, wrong_parent_size);
	out.cs("total_game_rom_bad_clone", wrong_clone, wrong_clone_size);
	out.cs("total_game_rom_bad", wrong_parent+wrong_clone, wrong_clone_size+wrong_parent_size);
	out() << "\n";

	unsigned miss_parent = 0;
	unsigned long long miss_parent_size = 0;
	unsigned miss_clone = 0;
	unsigned long long miss_clone_size = 0;
	for(gamearchive::const_iterator i=gar.begin();i!=gar.end();++i) {
		// only if have almost on rom and not exist any zip
		if (i->is_romset_required() && i->rzs_get().empty()) {
			if (i->cloneof_get().length()) {
				++miss_clone;
				miss_clone_size += i->size_get();
			} else {
				++miss_parent;
				miss_parent_size += i->size_get();
			}
			out.state_gamerom("game_rom_miss", *i, gar, true);
		}
	}
	out() << "\n";

	out.cs("total_game_rom_miss_parent", miss_parent, miss_parent_size);
	out.cs("total_game_rom_miss_clone", miss_clone, miss_clone_size);
	out.cs("total_game_rom_miss", miss_parent+miss_clone, miss_clone_size+miss_parent_size);
	out() << "\n";

	out.cp("total_percentage", (double)(ok_clone_size+ok_parent_size) / (miss_clone_size+miss_parent_size+wrong_clone_size+wrong_parent_size+ok_clone_size+ok_parent_size));
	out() << "\n";
}

void report_rom_set_zip(const gamearchive& gar, output& out)
{
	bool has_duplicate = false;
	bool has_preliminary = false;
	bool has_incomplete = false;
	bool has_missing = false;

	for(gamearchive::const_iterator i=gar.begin();i!=gar.end();++i) {
		if (i->is_romset_required() && !i->rzs_get().empty()) {
			if (i->rzs_get().size() > 1) {
				has_duplicate = true;
				out() << "Rom '" << i->name_get() << "' has duplicate zips";
				for(zippath_container::const_iterator j=i->rzs_get().begin();j!=i->rzs_get().end();++j) {
					out() << " '" << j->file_get() << "'";
				}
				out() << ".\n\n";
			}

			string romof = i->romof_get();
			while (romof.length()) {
				gamearchive::const_iterator j = gar.find(romof);
				if (j == gar.end()) {
					has_missing = true;
					out() << "Rom '" << i->name_get() << "' requires missing rom '" << romof << "'.\n\n";
					romof = "";
				} else {
					if (j->rzs_get().empty()) {
						has_missing = true;
						out() << "Rom '" << i->name_get() << "' requires missing rom '" << romof << "'.\n\n";
					}
					romof = j->romof_get();
				}
			}

			if (!i->working_get()) {
				if (!gar.has_working_clone_with_rom(*i)) {
					has_preliminary = true;
					out() << "Rom '" << i->name_get() << "' has a preliminary driver";

					// search for a working clone
					string_container clone = gar.find_working_clones(*i);
					if (!clone.empty()) {
						if (clone.size() == 1)
							out() << " but has also a working clone";
						else
							out() << " but has also working clones";
						for(string_container::const_iterator j=clone.begin();j!=clone.end();++j) {
							out() << " '" << *j << "'";
						}
					}
					out() << ".\n\n";
				}
			}

			for(zippath_container::const_iterator j=i->rzs_get().begin();j!=i->rzs_get().end();++j) {
				if (!j->good_get()) {
					has_incomplete = true;
					out() << "Rom '" << i->name_get() << "' has an incomplete zip '" << j->file_get() << "'.\n\n";
				}
			}
		}
	}

	if (!has_preliminary) {
		out() << "All the roms are for working drivers.\n";
	}
	if (!has_incomplete) {
		out() << "All the roms are complete.\n";
	}
	if (!has_missing) {
		out() << "All the dependencies are satisfied.\n";
	}
	if (!has_duplicate) {
		out() << "All the roms are without duplicates.\n";
	}
	if (has_preliminary) {
		out() << "Roms with a preliminary driver are unplayable, you can remove them.\n";
	}
	if (has_incomplete) {
		out() << "Roms with an incomplete zip must be redownloaded.\n";
	}
	if (has_missing) {
		out() << "Missing roms must be added, otherwise other roms may not work.\n";
	}
	if (has_duplicate) {
		out() << "Duplicate roms can be removed.\n";
	}
	out() << "\n";
}

void report_sample_set(const gamearchive& gar, output& out)
{
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
			out() << "\n";
		}
	}
	
	out.c("total_game_sample_dup", duplicate);
	out() << "\n";

	unsigned ok = 0;
	unsigned long long ok_size_zip = 0;
	for(gamearchive::const_iterator i=gar.begin();i!=gar.end();++i) {
		if (i->has_good_sample()) {
			++ok;
			ok_size_zip += i->good_sample_size();
			out.state_gamesample("game_sample_good", *i);
		}
	}
	out() << "\n";

	out.cz("total_game_sample_good", ok, ok_size_zip);
	out() << "\n";

	unsigned wrong = 0;
	for(gamearchive::const_iterator i=gar.begin();i!=gar.end();++i) {
		if (!i->has_good_sample() && i->has_bad_sample()) {
			++wrong;
			out.state_gamesample("game_sample_bad", *i);
		}
	}
	out() << "\n";

	out.c("total_game_sample_bad", wrong);
	out() << "\n";

	unsigned miss = 0;
	for(gamearchive::const_iterator i=gar.begin();i!=gar.end();++i) {
		// only if have almost on sample and not exist any zip
		if (!i->ss_get().empty() && i->szs_get().empty()) {
			// increase counter
			++miss;
			out.state_gamesample("game_sample_miss", *i);
		}
	}
	out() << "\n";

	out.c("total_game_sample_miss", miss);
	out() << "\n";

	out.cp("total_percentage", (double)(ok) / (ok+wrong+miss));
	out() << "\n";
}

void report_disk_set(const gamearchive& gar, output& out)
{
	unsigned duplicate = 0;
	for(gamearchive::const_iterator i=gar.begin();i!=gar.end();++i) {
		if (i->dzs_get().size()>1) {
			++duplicate;
			out.state_gamedisk("game_disk_dup", *i, false);
			for(zippath_container::const_iterator j=i->dzs_get().begin();j!=i->dzs_get().end();++j) {
				if (j->good_get()) 
					out.ziptag("chd_disk_dup", j->file_get(), "good");
				else
					out.ziptag("chd_disk_dup", j->file_get(), "bad");
			}
			out() << "\n";
		}
	}
	
	out.c("total_game_disk_dup", duplicate);
	out() << "\n";

	unsigned ok = 0;
	unsigned long long ok_size_zip = 0;
	for(gamearchive::const_iterator i=gar.begin();i!=gar.end();++i) {
		if (i->is_diskset_required() && i->has_good_disk()) {
			++ok;
			ok_size_zip += i->good_disk_size();
			out.state_gamedisk("game_disk_good", *i, false);
		}
	}
	out() << "\n";

	out.cz("total_game_disk_good", ok, ok_size_zip);
	out() << "\n";

	unsigned wrong = 0;
	for(gamearchive::const_iterator i=gar.begin();i!=gar.end();++i) {
		if (i->is_diskset_required() && !i->has_good_disk() && i->has_bad_disk()) {
			++wrong;
			out.state_gamedisk("game_disk_bad", *i, false);
		}
	}
	out() << "\n";

	out.c("total_game_disk_bad", wrong);
	out() << "\n";

	unsigned miss = 0;
	for(gamearchive::const_iterator i=gar.begin();i!=gar.end();++i) {
		if (i->is_diskset_required() && !i->has_good_disk() && !i->has_bad_disk()) {
			++miss;
			out.state_gamedisk("game_disk_miss", *i, true);
		}
	}
	out() << "\n";

	out.c("total_game_disk_miss", miss);
	out() << "\n";

	out.cp("total_percentage", (double)(ok) / (ok+wrong+miss));
	out() << "\n";

}

// ----------------------------------------------------------------------------
// filter

bool filter_preliminary(const game& g)
{
	return !g.working_subset_get();
}

bool filter_working(const game& g)
{
	return g.working_subset_get();
}

bool filter_working_parent(const game& g)
{
	return g.working_subset_get() && g.working_parent_subset_get();
}

bool filter_working_clone(const game& g)
{
	return g.working_subset_get() && !g.working_parent_subset_get();
}

void filt(gamearchive& gar, const string& filter)
{
	filter_proc* p;
	if (filter == "preliminary")
		p = filter_preliminary;
	else if (filter == "working")
		p = filter_working;
	else if (filter == "working_parent")
		p = filter_working_parent;
	else if (filter == "working_clone")
		p = filter_working_clone;
	else if (filter == "")
		return;
	else {
		throw error() << "Invalid filter " << filter;
	}

	gar.filter(p);
}

// ----------------------------------------------------------------------------
// command

void equal(const gamearchive& gar, gamerom_by_crc_multiset& rcb, ostream& out)
{
	unsigned equal = 0;
	
	gamerom_by_crc_multiset::const_iterator start = rcb.begin();
	while (start != rcb.end()) {
		unsigned count = 1;
		gamerom_by_crc_multiset::const_iterator end = start;
		++end;
		while (end!=rcb.end() && start->crc_get()==end->crc_get() && start->size_get()==end->size_get()) {
			++end;
			++count;
		}

		if (count>1) {
			++equal; 
			out << "group " << dec << start->size_get();
			out << " " << hex << setw(8) << setfill('0') << start->crc_get();
			out << "\n";
			while (start != end) {
				out << "rom " << start->game_get().c_str() << "/" << start->name_get();
				gamearchive::const_iterator g = gar.find(start->game_get());
				// print parent game also if not really exists
				if (g!=gar.end() && g->cloneof_get().length()) {
					out << " cloneof " << g->cloneof_get();
				}
				out << "\n";
				++start;
			}
			out << "\n";
		}
		start = end;
	}

	out.setf(ios::right, ios::adjustfield);

	out << "total_group " << setw(8) << setfill(' ') << dec << equal << "\n";

	out.setf(ios::left, ios::adjustfield);

	out << "\n";
}

void ident_data(const string& file, crc_t crc, unsigned size, const gamearchive& gar, ostream& out)
{
	out << file << "\n";

	for(gamearchive::const_iterator i=gar.begin();i!=gar.end();++i) {
		for(rom_by_name_set::const_iterator j=i->rs_get().begin();j!=i->rs_get().end();++j) {
			if (j->crc_get()==crc && j->size_get()==size) {
				out << "\t" << setw(8) << i->name_get().c_str() << " " << setw(12) << j->name_get().c_str() << " " << i->description_get() << "\n";
			}
		}
	}
}

void ident_zip(const string& file, const gamearchive& gar, ostream& out)
{
	zip z(file);

	z.open();

	for(zip::const_iterator i=z.begin();i!=z.end();++i) {
		ident_data(z.file_get() + "/" + i->name_get(), i->crc_get(), i->uncompressed_size_get(), gar, out);
	}
}

void ident_file(const string& file, const gamearchive& gar, ostream& out)
{
	struct stat st;
	if (stat(file.c_str(), &st) != 0)
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
	} else if (file_compare(file_ext(file), ".zip")==0) {
		ident_zip(file, gar, out);
	} else {
		unsigned size = file_size(file);
		crc_t crc = file_crc(file);
		ident_data(file, crc, size, gar, out);
	}
}

void bbs(const gamearchive& gar, ostream& out)
{
	for(gamearchive::const_iterator i=gar.begin();i!=gar.end();++i) {
		out << i->name_get() << ".zip";
		out << " " << i->size_get();
		out << " " << i->description_get();
		if (i->cloneof_get().length()) {
			out << " [clone]";
		}
		out << ", " << i->manufacturer_get() << ", " << i->year_get() << "\n";
	}
}

void version()
{
	cout << PACKAGE " v" VERSION " by Andrea Mazzoleni\n";
}

void usage()
{
	version();

	cout << "Usage: advscan [options] < info.txt\n";
	cout << "\n";
	cout << "Modes:\n";
	cout << "  " SWITCH_GETOPT_LONG("-r, --rom        ", "-r") "  Operate on rom sets\n";
	cout << "  " SWITCH_GETOPT_LONG("-s, --sample     ", "-s") "  Operate on sample sets\n";
	cout << "  " SWITCH_GETOPT_LONG("-k, --disk       ", "-k") "  Operate on disk files\n";
	cout << "Operations:\n";
	cout << "  " SWITCH_GETOPT_LONG("-a, --add-zip    ", "-a") "  Add missing zips\n";
	cout << "  " SWITCH_GETOPT_LONG("-b, --add-bin    ", "-b") "  Add missing files in zips\n";
	cout << "  " SWITCH_GETOPT_LONG("-d, --del-zip    ", "-d") "  Delete unknown zips\n";
	cout << "  " SWITCH_GETOPT_LONG("-u, --del-unknown", "-u") "  Delete unknown files in zips\n";
	cout << "  " SWITCH_GETOPT_LONG("-g, --del-garbage", "-g") "  Delete garbage files\n";
	cout << "  " SWITCH_GETOPT_LONG("-t, --del-text   ", "-t") "  Delete unused text files\n";
	cout << "Shortcuts:\n";
	cout << "  " SWITCH_GETOPT_LONG("-R, --rom-std    ", "-R") "  Shortcut for -rabdug\n";
	cout << "  " SWITCH_GETOPT_LONG("-S, --sample-std ", "-S") "  Shortcut for -sabdug\n";
	cout << "Commands:\n";
	cout << "  " SWITCH_GETOPT_LONG("-e, --equal      ", "-e") "  Print list of roms with equal crc+size\n";
	cout << "  " SWITCH_GETOPT_LONG("-i, --ident FILE ", "-i") "  Identify roms by crc and size\n";
	cout << "  " SWITCH_GETOPT_LONG("-b, --bbs        ", "-b") "  Output a 'files.bbs' for roms .zip\n";
	cout << "  " SWITCH_GETOPT_LONG("-h, --help       ", "-h") "  Help of the program\n";
	cout << "  " SWITCH_GETOPT_LONG("-V, --version    ", "-V") "  Version of the program\n";
	cout << "Options:\n";
	cout << "  " SWITCH_GETOPT_LONG("-c, --cfg FILE   ", "-c") "  Select a configuration file\n";
	cout << "  " SWITCH_GETOPT_LONG("-f, --filter FILT", "-f") "  Filter the game list\n";
	cout << "  " SWITCH_GETOPT_LONG("-p, --report     ", "-p") "  Write a rom based report\n";
	cout << "  " SWITCH_GETOPT_LONG("-P, --report-zip ", "-P") "  Write a zip based report\n";
	cout << "  " SWITCH_GETOPT_LONG("-n, --print-only ", "-n") "  Only print operations, do nothing\n";
	cout << "  " SWITCH_GETOPT_LONG("-v, --verbose    ", "-v") "  Verbose output\n";
}

#if HAVE_GETOPT_LONG
struct option long_options[] = {
	{"rom", 0, 0, 'r'},
	{"rom-std", 0, 0, 'R'},
	{"sample", 0, 0, 's'},
	{"sample-std", 0, 0, 'S'},
	{"disk", 0, 0, 'k'},
	{"disk-std", 0, 0, 'K'},

	{"add-zip", 0, 0, 'a'},
	{"add-bin", 0, 0, 'b'},
	{"del-zip", 0, 0, 'd'},
	{"del-unknown", 0, 0, 'u'},
	{"del-text", 0, 0, 't'},
	{"del-garbage", 0, 0, 'g'},

	{"filter", 1, 0, 'f'},
	{"cfg", 1, 0, 'c'},

	{"bbs", 0, 0, 'l'},
	{"equal", 0, 0, 'e'},
	{"ident", 0, 0, 'i'},

	{"report", 0, 0, 'p'},
	{"report-file", 0, 0, 'P'},

	{"print-only", 0, 0, 'n'},

	{"verbose", 0, 0, 'v'},
	{"help", 0, 0, 'h'},
	{"version", 0, 0, 'V'},
	{0, 0, 0, 0}
};
#endif

#define OPTIONS "rRsSkKabdutgf:c:leipPnvhV"

void run(int argc, char* argv[])
{
	if (argc<=1) {
		usage();
		exit(EXIT_FAILURE);
	}

	// read options
	bool flag_rom = false;
	bool flag_sample = false;
	bool flag_disk = false;
	bool flag_equal = false;
	bool flag_bbs = false;
	bool flag_report = false;
	bool flag_report_zip = false;
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
	string filter;

	int c = 0;

	opterr = 0; // don't print errors

	while ((c =
#if HAVE_GETOPT_LONG
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
			case 'k' :
				flag_disk = true;
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
			case 'K' :
				flag_disk = true;
				flag_move = true;
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
			case 'f' :
				filter = optarg;
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
			case 'P' :
				flag_report_zip = true;
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
				throw error() << "Unknown option `" << opt << "'";
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

	if ((flag_rom || flag_sample || flag_disk) && !(flag_operation || flag_report || flag_report_zip)) {
		usage();
		exit(EXIT_FAILURE);
	}

	if (!(flag_rom || flag_sample || flag_disk) && (flag_operation || flag_report || flag_report_zip)) {
		usage();
		exit(EXIT_FAILURE);
	}

	// set of all game and roms
	gamearchive gar;

	// load the rom set
	gar.load(cin);

	// filter the rom set
	filt(gar, filter);

	if (gar.begin() == gar.end())
		throw error() << "Empty information file";

	// build the crc/size rom set used to detect unique roms
	gamerom_by_crc_multiset rcb;
	for(gamearchive::const_iterator i=gar.begin();i!=gar.end();++i) {
		for(rom_by_name_set::const_iterator j=i->rs_get().begin();j!=i->rs_get().end();++j) {
			rcb.insert(gamerom(i->name_get(), j->name_get(), j->size_get(), j->crc_get(), j->nodump_get()));
		}
	}

	if (flag_ident) {
		for(int i=optind;i<argc;++i)
			ident_file(argv[i], gar, cout);
	}

	if (flag_equal)
		equal(gar, rcb, cout);

	if (flag_bbs)
		bbs(gar, cout);

	if (flag_rom || flag_sample || flag_disk) {
		config cfg(cfg_file, flag_rom, flag_sample, flag_disk, flag_change);

		output out(cout);
		analyze ana(gar);

		if (flag_rom) {
			ziparchive zar;

			if (flag_operation) {
				all_rom_load(zar, cfg);
				all_rom_scan(oper, zar, gar, rcb, cfg, out, ana);
			} else {
				set_rom_load(zar, cfg);
				set_rom_scan(oper, zar, gar, cfg, out, ana);
			}

			if (flag_report) {
				report_rom_zip(zar, gar, out, flag_verbose, ana);
				report_rom_set(gar, out);
			}

			if (flag_report_zip) {
				report_rom_set_zip(gar, out);
			}

			if (flag_change)
				all_unknown_scan(zar, cfg, out);
		}

		if (flag_sample) {
			filepath_container zar;
			
			set_sample_load(zar, cfg);
			if (flag_operation) {
				all_sample_scan(oper, zar, gar, cfg, out, ana);
			} else {
				set_sample_scan(oper, zar, gar, cfg, out, ana);
			}

			if (flag_report) {
				report_sample_zip(zar, gar, out, flag_verbose, ana);
				report_sample_set(gar, out);
			}
		}

		if (flag_disk) {
			filepath_container zar;
			
			set_disk_load(zar, cfg);
			if (flag_operation) {
				all_disk_scan(oper, zar, gar, cfg, out, ana);
			} else {
				set_disk_scan(oper, zar, gar, cfg, out, ana);
			}

			if (flag_report) {
				report_disk_zip(zar, gar, out, flag_verbose, ana);
				report_disk_set(gar, out);
			}
		}
	}
}

int main(int argc, char* argv[])
{
	try {
		run(argc, argv);
	} catch (error& e) {
		cerr << e << "\n";
		exit(EXIT_FAILURE);
	} catch (std::bad_alloc) {
		cerr << "Low memory\n";
		exit(EXIT_FAILURE);
	} catch (...) {
		cerr << "Unknown error\n";
		exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}

