/*
 * This file is part of the Advance project.
 *
 * Copyright (C) 2002 Andrea Mazzoleni
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

#include "conf.h"

#include <iostream>
#include <fstream>

using namespace std;

static void expand_tree(const string& path, filepath_container& ds) throw (error)
{
	DIR* dir = opendir(path.c_str());
	if (!dir)
		throw error() << "Failed open directory '" << path << "'";

	ds.insert(ds.end(), filepath(path));

	struct dirent* ent = readdir(dir);
	while (ent) {
		string subpath = path + "/" + ent->d_name;
		struct stat st;
		if (stat(subpath.c_str(), &st)!=0)
			throw error() << "Failed stat file " << subpath;

		if (S_ISDIR(st.st_mode)) {
			if (strcmp(ent->d_name, ".")!=0 && strcmp(ent->d_name, "..")!=0) {
				expand_tree(subpath.c_str(), ds);
			}
		}

		ent = readdir(dir);
	}

	closedir(dir);
}

// ------------------------------------------------------------------------
// Config

config::config(const string& file, bool need_rom, bool need_sample, bool need_disk, bool need_change)
{
	string cfg;

	if (file.length())
		cfg = file;
	else
		cfg = "advscan.rc";

	ifstream is(cfg.c_str());
	if (!is)
		throw error() << "Failed open of the configuration file " << cfg;

	filepath_container base_romreadonlytree;

	while (!is.eof()) {
		string s;

		getline(is, s, '\n');

		s = strip_space(s);

		if (s.length() == 0)
			continue;

		if (s[0]=='#')
			continue;

		unsigned i = 0;

		string tag = token_get(s, i, " \t");
		token_skip(s, i, " \t");
		string arg = token_get(s, i, "");

		if (tag == "rom") {
			unsigned j = 0;
			while (j < arg.length()) {
				string dir = token_get(arg, j, DIR_SEP);
				if (j < arg.length() && arg[j] == DIR_SEP)
					++j;
				rompath.insert(rompath.end(), filepath(file_adjust(dir)));
			}
		} else if (tag == "sample") {
			unsigned j = 0;
			while (j < arg.length()) {
				string dir = token_get(arg, j, DIR_SEP);
				if (j < arg.length() && arg[j] == DIR_SEP)
					++j;
				samplepath.insert(samplepath.end(), filepath(file_adjust(dir)));
			}
		} else if (tag == "disk") {
			unsigned j = 0;
			while (j < arg.length()) {
				string dir = token_get(arg, j, DIR_SEP);
				if (j < arg.length() && arg[j] == DIR_SEP)
					++j;
				diskpath.insert(diskpath.end(), filepath(file_adjust(dir)));
			}
		} else if (tag == "rom_import") {
			unsigned j = 0;
			while (j < arg.length()) {
				string dir = token_get(arg, j, DIR_SEP);
				if (j < arg.length() && arg[j] == DIR_SEP)
					++j;
				base_romreadonlytree.insert(base_romreadonlytree.end(), filepath(file_adjust(dir)));
			}
		} else if (tag == "rom_unknown") {
			if (romunknownpath.file_get().length())
				throw error() << "Double specification of option `rom_unknown' in file " << cfg;
			if (arg.length() == 0)
				throw error() << "Empty specification of option `rom_unknown' in file " << cfg;
			if (arg.find(DIR_SEP) != string::npos)
				throw error() << "Multiple path specification in option `rom_unknown' in file " << cfg;
			romunknownpath.file_set(file_adjust(arg));
		} else if (tag == "sample_unknown") {
			if (sampleunknownpath.file_get().length())
				throw error() << "Double specification of option `sample_unknown' in file " << cfg;
			if (arg.length() == 0)
				throw error() << "Empty specification of option `sample_unknown' in file " << cfg;
			if (arg.find(DIR_SEP) != string::npos)
				throw error() << "Multiple path specification in option `sample_unknown' in file " << cfg;
			sampleunknownpath.file_set(file_adjust(arg));
		} else if (tag == "disk_unknown") {
			if (diskunknownpath.file_get().length())
				throw error() << "Double specification of option `disk_unknown' in file " << cfg;
			if (arg.length() == 0)
				throw error() << "Empty specification of option `disk_unknown' in file " << cfg;
			if (arg.find(DIR_SEP) != string::npos)
				throw error() << "Multiple path specification in option `disk_unknown' in file " << cfg;
			diskunknownpath.file_set(file_adjust(arg));
		} else if (tag == "rom_new") {
			if (romnewpath.file_get().length())
				throw error() << "Double specification of option `rom_new' in file " << cfg;
			if (arg.length() == 0)
				throw error() << "Empty specification of option `rom_new' in file " << cfg;
			if (arg.find(DIR_SEP) != string::npos)
				throw error() << "Multiple path specification in option `rom_new' in file " << cfg;
			romnewpath.file_set(file_adjust(arg));
		} else {
			throw error() << "Unknown option `" << tag << "' in file " << cfg;
		}
	}

	is.close();

	if (need_rom) {
		if (rompath.empty())
			throw error() << "Missing option `rom' option in file " << cfg;
		for(filepath_container::iterator i=rompath.begin();i!=rompath.end();++i) {
			if (access(i->file_get().c_str(), F_OK) != 0)
				throw error() << "Not existing dir " << i->file_get() << " on `rom' option in file " << cfg;
		}
		if (need_change) {
			for(filepath_container::iterator i=rompath.begin();i!=rompath.end();++i) {
				if (access(i->file_get().c_str(), W_OK) != 0)
					throw error() << "Access denied on dir " << i->file_get() << " on `rom' option in file " << cfg;
			}
			if (!romnewpath.file_get().length())
				throw error() << "Missing `rom_new' option in file " << cfg;
			if (access(romnewpath.file_get().c_str(), F_OK) != 0)
				throw error() << "Not existing dir " << romnewpath.file_get() << " on `rom_new' option in file " << cfg;
			if (access(romnewpath.file_get().c_str(), W_OK) != 0)
				throw error() << "Access denied on dir " << romnewpath.file_get() << " on `rom_new' option in file " << cfg;
			if (!romunknownpath.file_get().length())
				throw error() << "Missing `rom_unknown' option in file " << cfg;
			if (access(romunknownpath.file_get().c_str(), F_OK) != 0)
				throw error() << "Not existing dir " << romunknownpath.file_get() << " on `rom_unknown' option in file " << cfg;
			if (access(romunknownpath.file_get().c_str(), W_OK) != 0)
				throw error() << "Access denied on dir " << romunknownpath.file_get() << " on `rom_unknown' option in file " << cfg;
		}
	}
	if (need_sample) {
		if (samplepath.empty())
			throw error() << "Missing `sample' option in file " << cfg;
		for(filepath_container::iterator i=samplepath.begin();i!=samplepath.end();++i) {
			if (access(i->file_get().c_str(), F_OK) != 0)
				throw error() << "Not existing dir " << i->file_get() << " on `sample' option in file " << cfg;
		}
		if (need_change) {
			for(filepath_container::iterator i=samplepath.begin();i!=samplepath.end();++i) {
				if (access(i->file_get().c_str(), W_OK) != 0)
					throw error() << "Access denied on dir " << i->file_get() << " on `sample' option in file " << cfg;
			}
			if (!sampleunknownpath.file_get().length())
				throw error() << "Missing `sample_unknown' option in file " << cfg;
			if (access(sampleunknownpath.file_get().c_str(), F_OK) != 0)
				throw error() << "Not existing dir " << sampleunknownpath.file_get() << " on `sample_unknown' option in file " << cfg;
			if (access(sampleunknownpath.file_get().c_str(), W_OK) != 0)
				throw error() << "Access denied on dir " << sampleunknownpath.file_get() << " on `sample_unknown' option in file " << cfg;
		}
	}
	if (need_disk) {
		if (diskpath.empty())
			throw error() << "Missing `disk' option in file " << cfg;
		for(filepath_container::iterator i=diskpath.begin();i!=diskpath.end();++i) {
			if (access(i->file_get().c_str(), F_OK) != 0)
				throw error() << "Not existing dir " << i->file_get() << " on `disk' option in file " << cfg;
		}
		if (need_change) {
			for(filepath_container::iterator i=diskpath.begin();i!=diskpath.end();++i) {
				if (access(i->file_get().c_str(), W_OK) != 0)
					throw error() << "Access denied on dir " << i->file_get() << " on `disk' option in file " << cfg;
			}
			if (!diskunknownpath.file_get().length())
				throw error() << "Missing `disk_unknown' option in file " << cfg;
			if (access(diskunknownpath.file_get().c_str(), F_OK) != 0)
				throw error() << "Not existing dir " << diskunknownpath.file_get() << " on `disk_unknown' option in file " << cfg;
			if (access(diskunknownpath.file_get().c_str(), W_OK) != 0)
				throw error() << "Access denied on dir " << diskunknownpath.file_get() << " on `disk_unknown' option in file " << cfg;
		}
	}

	// expand romreadonly recursively
	filepath_container expand_romreadonlytree;
	for(filepath_container::iterator i=base_romreadonlytree.begin();i!=base_romreadonlytree.end();++i) {
		expand_tree(i->file_get(), expand_romreadonlytree);
	}
	
	// remove paths already in rompath
	for(filepath_container::iterator i=expand_romreadonlytree.begin();i!=expand_romreadonlytree.end();++i) {
		filepath_container::iterator j;
		for(j=rompath.begin();j!=rompath.end();++j) {
			if (file_compare(j->file_get(), i->file_get())==0)
				break;
		}
		if (j==rompath.end()) {
			romreadonlytree.insert(romreadonlytree.end(), *i);
		}
	}
}

config::~config()
{
}

