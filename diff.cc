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

#include "rom.h"
#include "game.h"
#include "except.h"

#include <iostream>
#include <fstream>
#include <iomanip>

using namespace std;

/**
 * Check if A include B.
 */
bool include(const rom_by_name_set& A, const rom_by_name_set& B) {
	rom_by_crc_set Ac;

	for(rom_by_name_set::const_iterator i=A.begin();i!=A.end();++i)
		Ac.insert(*i);

	for(rom_by_name_set::const_iterator i=B.begin();i!=B.end();++i) {
		rom_by_crc_set::const_iterator j = Ac.find(*i);
		if (j==Ac.end())
			return false;
	}

	return true;
}

std::ostream& output_info(std::ostream& os, const rom& A) {
	os << "rom ( name " << A.name_get() << " size " << std::dec << A.size_get();
	if (A.nodump_get())
		os << " flags nodump";
	else
		os << " crc " << std::hex << setw(8) << setfill('0') << A.crc_get();
	os << " )";
	return os;
}

std::ostream& output_info(std::ostream &os, const game& A) {
	os << "game (\n";
	os << "\tname " << A.name_get() << "\n";
	for(rom_by_name_set::const_iterator i=A.rs_get().begin();i!=A.rs_get().end();++i) {
		os << "\t";
		output_info(os, *i);
		os  << "\n";
	}
	os << ")\n";
	return os;
}

std::ostream& output_xml(std::ostream& os, const rom& A) {
	os << "<rom name=\"" << A.name_get() << "\" size=\"" << std::dec << A.size_get() << "\"";
	if (A.nodump_get())
		os << " status=\"nodump\"";
	else
		os << " crc=\"" << std::hex << setw(8) << setfill('0') << A.crc_get() << "\"";
	os << "/>";
	return os;
}

std::ostream& output_xml(std::ostream &os, const game& A) {
	os << "\t<game name=\"" << A.name_get() << "\">\n";
	for(rom_by_name_set::const_iterator i=A.rs_get().begin();i!=A.rs_get().end();++i) {
		os << "\t\t";
		output_xml(os, *i);
		os  << "\n";
	}
	os << "\t</game>\n";
	return os;
}

void version() {
	std::cout << PACKAGE " v" VERSION " by Andrea Mazzoleni" << std::endl;
}

void usage() {
	version();

	cout << "Usage: advdiff [-i] info1.lst/xml info2.lst/xml" << endl;
	cout << endl;
	cout << "Options:" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-i, --info  ", "-i") "  Output in info format" << endl;
}

#if HAVE_GETOPT_LONG
struct option long_options[] = {
	{"info", 0, 0, 'i'},
	{"help", 0, 0, 'h'},
	{"version", 0, 0, 'V'},
	{0, 0, 0, 0}
};
#endif

#define OPTIONS "ihV"

void process(int argc, char* argv[]) {
	gamearchive g0;
	gamearchive g1;
	bool opt_info;

	opt_info = false;

	if (argc <= 1) {
		usage();
		return;
	}

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
			case 'i' :
				opt_info = true;
				break;
			case 'h' :
				usage();
				return;
			case 'V' :
				version();
				return;
			default: {
				// not optimal code for g++ 2.95.3
				string opt;
				opt = (char)optopt;
				throw error() << "Unknown option `" << opt << "'";
			}
		} 
	}

	if (argc - optind < 2)
		throw error() << "Missing input files";
	if (argc - optind > 2)
		throw error() << "Too many input files";

	string f0 = argv[optind+0];
	string f1 = argv[optind+1];

	ifstream ff0(f0.c_str(), ios::in | ios::binary);
	if (!ff0)
		throw error() << "Failed open of " << f0;
	g0.load(ff0);
	ff0.close();

	ifstream ff1(f1.c_str(), ios::in | ios::binary);
	if (!ff1)
		throw error() << "Failed open of " << f1;
	g1.load(ff1);
	ff1.close();

	if (!opt_info)
		std::cout << "<mame>\n";
	gamearchive::const_iterator i;
	for(i=g1.begin();i!=g1.end();++i) {
		gamearchive::const_iterator j = g0.find(*i);
		if (j==g0.end() || !include(j->rs_get(), i->rs_get())) {
			if (opt_info)
				output_info(std::cout, *i) << "\n";
			else
				output_xml(std::cout, *i);
		}
	}
	if (!opt_info)
		std::cout << "</mame>\n";
}

int main(int argc, char* argv[]) {

	try {
		process(argc,argv);
	} catch (error& e) {
		cerr << e << endl;
		exit(EXIT_FAILURE);
	} catch (std::bad_alloc) {
		cerr << "Low memory" << endl;
		exit(EXIT_FAILURE);
	} catch (...) {
		cerr << "Unknown error" << endl;
		exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}

