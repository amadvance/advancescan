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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "output.h"

#include <iomanip>

using namespace std;

#define OP_WIDTH 12 // Operation tag width
#define TOTAL_WIDTH 26 // Total tag width
#define CMD_WIDTH 12 // Command tag width
#define NAME_WIDTH 8 // Rom name width
#define ZIPSIZE_WIDTH 6 // Zip size width
#define FILE_WIDTH 12 // File name width
#define CRC_WIDTH 8 // Crc width
#define SIZE_WIDTH 8 // File size width
#define BIGSIZE_WIDTH 10 // Big file sie width
#define COUNT_WIDTH 6 // Counter width

ostream& output::op(const string& op) {
	// .c_str() is required by g++ 2.95.3
	os << setw(OP_WIDTH) << setfill(' ') << op.c_str();
	os << " ";

	return os;
}

ostream& output::total(const string& op) {
	// .c_str() is required by g++ 2.95.3
	os << setw(TOTAL_WIDTH) << setfill(' ') << op.c_str();
	os << " ";

	return os;
}

ostream& output::free(const string& op) {
	os << op;
	os << " ";

	return os;
}

ostream& output::cmd(const string& op, const string& cmd) {
	string cmd_square = "[" + cmd + "]";

	// .c_str() is required by g++ 2.95.3
	os << setw(CMD_WIDTH) << setfill(' ') << cmd_square.c_str();
	os << " ";

	os << setw(OP_WIDTH) << setfill(' ') << op.c_str();
	os << " ";

	return os;
}

ostream& output::zip(const string& op, const string& zip) {
	free(op);

	os << zip << endl;

	return os;
}

ostream& output::ziptag(const string& op, const string& zip, const string& tag) {
	free(op);
	os << zip << " " << tag << endl;

	return os;
}

ostream& output::title(const string& op, bool& title, const string& path) {
	if (!title) {
		title = true;
		zip(op,path);
	}

	return os;
}

ostream& output::c(const string& tag, unsigned count) {
	total(tag);

	os.setf(ios::right, ios::adjustfield);

	os << setw(COUNT_WIDTH) << dec << setfill(' ') << count;

	os.setf(ios::left, ios::adjustfield);

	os << endl;

	return os;
}

ostream& output::cs(const string& tag, unsigned count, unsigned long long size) {
	total(tag);

	os.setf(ios::right, ios::adjustfield);

	os << setw(COUNT_WIDTH) << dec << setfill(' ') << count;

	os << "  ";

	os << setw(BIGSIZE_WIDTH) << dec << setfill(' ') << size;

	os.setf(ios::left, ios::adjustfield);

	os << endl;

	return os;
}

ostream& output::csz(const string& tag, unsigned count, unsigned long long size, unsigned long long sizezip) {
	total(tag);

	os.setf(ios::right, ios::adjustfield);

	os << setw(COUNT_WIDTH) << dec << setfill(' ') << count;

	os << "  ";

	os << setw(BIGSIZE_WIDTH) << dec << setfill(' ') << size;

	os << "  ";

	os << setw(BIGSIZE_WIDTH) << dec << setfill(' ') << sizezip;

	os.setf(ios::left, ios::adjustfield);

	os << endl;

	return os;
}

ostream& output::cz(const string& tag, unsigned count, unsigned long long sizezip) {
	total(tag);

	os.setf(ios::right, ios::adjustfield);

	os << setw(COUNT_WIDTH) << dec << setfill(' ') << count;

	os << "  ";

	os << setw(BIGSIZE_WIDTH) << dec << setfill(' ') << sizezip;

	os.setf(ios::left, ios::adjustfield);

	os << endl;

	return os;
}

ostream& output::state_gamesample(const string& tag, const game& g) {
	free(tag);

	// .c_str() is required by g++ 2.95.3
	os << setw(NAME_WIDTH) << setfill(' ') << g.name_get().c_str();
	
	os << "  ";

	os << g.description_get();

	os << endl;

	return os;
}

ostream& output::state_gamerom(const string& tag, const game& g, const gamearchive& gar) {
	free(tag);

	// .c_str() is required by g++ 2.95.3
	os << setw(NAME_WIDTH) << setfill(' ') << g.name_get().c_str();

	os.setf(ios::right, ios::adjustfield);

	os << " " << setw(ZIPSIZE_WIDTH) << dec << g.size_get()/1024;

	os.setf(ios::left, ios::adjustfield);

	os << " " << g.description_get();

	gamearchive::const_iterator parent = gar.find( game( g.cloneof_get() ));
	if (parent != gar.end()) {
		os << " [cloneof " << (*parent).description_get();
		os << " " << dec << (*parent).size_get()/1024 << "]";
	}

	os << endl;

	return os;
}

ostream& output::pair(unsigned size, crc_t crc) {
	os.setf(ios::right, ios::adjustfield);

	os << setw(SIZE_WIDTH) << dec << setfill(' ') << size << " " << setw(CRC_WIDTH) << hex << setfill('0') << crc;

	os.setf(ios::left, ios::adjustfield);

	return os;
}

ostream& output::cmd_rom(const string& tag, const string& c, const string& name, unsigned size, crc_t crc) {
	cmd(tag,c);

	pair(size, crc);

	os << " ";

	// .c_str() is required by g++ 2.95.3
	os << setw(FILE_WIDTH) << setfill(' ') << name.c_str();

	return os;
}

ostream& output::cmd_rom(const string& tag, const string& cmd,const rom& r) {

	cmd_rom(tag, cmd, r.name_get(), r.size_get(), r.crc_get());

	return os;
}

ostream& output::state_rom(const string& tag, const string& name, unsigned size, crc_t crc) {
	op(tag);

	pair(size, crc);

	os << " ";

	// .c_str() is required by g++ 2.95.3
	os << setw(FILE_WIDTH) << setfill(' ') << name.c_str();

	return os;
}

ostream& output::state_rom(const string& tag, const rom& r) {

	state_rom(tag, r.name_get(), r.size_get(), r.crc_get());

	return os;
}

ostream& output::state_rom_real(const string& tag, const rom& r, unsigned real_size, unsigned real_crc) {
	state_rom(tag,r);

	os << " [";
	pair(real_size, real_crc);
	os << "]";

	return os;
}

ostream& output::cmd_sample(const string& tag, const string& c, const sample& s) {
	cmd(tag,c);

	// .c_str() is required by g++ 2.95.3
	os << setw(FILE_WIDTH) << setfill(' ') << s.name_get().c_str();

	return os;
}


ostream& output::state_sample(const string& tag, const sample& s) {
	op(tag);

	// .c_str() is required by g++ 2.95.3
	os << setw(FILE_WIDTH) << setfill(' ') << s.name_get().c_str();

	return os;
}

