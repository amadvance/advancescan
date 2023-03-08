/*
 * This file is part of the Advance project.
 *
 * Copyright (C) 2002, 2003 Andrea Mazzoleni
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

#include "output.h"

#include <iomanip>

using namespace std;

#define FIELD_OP_WIDTH 12 // Operation tag width
#define FIELD_TOTAL_WIDTH 32 // Total tag width
#define FIELD_CMD_WIDTH 12 // Command tag width
#define FIELD_NAME_WIDTH 8 // Rom name width
#define FIELD_ZIPSIZE_WIDTH 6 // Zip size width
#define FIELD_FILE_WIDTH 12 // File name width
#define FIELD_CRC_WIDTH 8 // Crc width
#define FIELD_SIZE_WIDTH 8 // File size width
#define FIELD_BIGSIZE_WIDTH 12 // Big file size width
#define FIELD_COUNT_WIDTH 6 // Counter width

ostream& output::op(const string& op)
{
	// .c_str() is required by g++ 2.95.3
	os << setw(FIELD_OP_WIDTH) << setfill(' ') << op.c_str();
	os << " ";

	return os;
}

ostream& output::total(const string& op)
{
	// .c_str() is required by g++ 2.95.3
	os << setw(FIELD_TOTAL_WIDTH) << setfill(' ') << op.c_str();
	os << " ";

	return os;
}

ostream& output::free(const string& op)
{
	os << op;
	os << " ";

	return os;
}

ostream& output::cmd(const string& op, const string& cmd)
{
	string cmd_square = "[" + cmd + "]";

	// .c_str() is required by g++ 2.95.3
	os << setw(FIELD_CMD_WIDTH) << setfill(' ') << cmd_square.c_str();
	os << " ";

	os << setw(FIELD_OP_WIDTH) << setfill(' ') << op.c_str();
	os << " ";

	return os;
}

ostream& output::zip(const string& op, const string& zip)
{
	free(op);

	os << zip << endl;

	return os;
}

ostream& output::ziptag(const string& op, const string& zip, const string& tag)
{
	free(op);
	os << zip << " " << tag << endl;

	return os;
}

ostream& output::title(const string& op, bool& title, const string& path)
{
	if (!title) {
		title = true;
		zip(op, path);
	}

	return os;
}

ostream& output::c(const string& tag, unsigned count)
{
	total(tag);

	os.setf(ios::right, ios::adjustfield);

	os << setw(FIELD_COUNT_WIDTH) << dec << setfill(' ') << count;

	os.setf(ios::left, ios::adjustfield);

	os << endl;

	return os;
}

ostream& output::cs(const string& tag, unsigned count, unsigned long long size)
{
	total(tag);

	os.setf(ios::right, ios::adjustfield);

	os << setw(FIELD_COUNT_WIDTH) << dec << setfill(' ') << count;

	os << "  ";

	os << setw(FIELD_BIGSIZE_WIDTH) << dec << setfill(' ') << size;

	os.setf(ios::left, ios::adjustfield);

	os << endl;

	return os;
}

ostream& output::cp(const string& tag, double v)
{
	total(tag);

	os.setf(ios::left, ios::adjustfield);

	os << dec << v;

	os << endl;

	return os;
}


ostream& output::csz(const string& tag, unsigned count, unsigned long long size, unsigned long long sizezip)
{
	total(tag);

	os.setf(ios::right, ios::adjustfield);

	os << setw(FIELD_COUNT_WIDTH) << dec << setfill(' ') << count;

	os << "  ";

	os << setw(FIELD_BIGSIZE_WIDTH) << dec << setfill(' ') << size;

	os << "  ";

	os << setw(FIELD_BIGSIZE_WIDTH) << dec << setfill(' ') << sizezip;

	os.setf(ios::left, ios::adjustfield);

	os << endl;

	return os;
}

ostream& output::cz(const string& tag, unsigned count, unsigned long long sizezip)
{
	total(tag);

	os.setf(ios::right, ios::adjustfield);

	os << setw(FIELD_COUNT_WIDTH) << dec << setfill(' ') << count;

	os << "  ";

	os << setw(FIELD_BIGSIZE_WIDTH) << dec << setfill(' ') << sizezip;

	os.setf(ios::left, ios::adjustfield);

	os << endl;

	return os;
}

ostream& output::state_gamesample(const string& tag, const game& g)
{
	free(tag);

	// .c_str() is required by g++ 2.95.3
	os << setw(FIELD_NAME_WIDTH) << setfill(' ') << g.name_get().c_str();

	os << "  ";

	os << g.description_get();

	os << "\n";

	return os;
}

ostream& output::state_gamedisk(const string& tag, const game& g, bool name)
{
	free(tag);

	// .c_str() is required by g++ 2.95.3
	os << setw(FIELD_NAME_WIDTH) << setfill(' ') << g.name_get().c_str();

	os << "  ";

	os << g.description_get();

	if (name && !g.ds_get().empty()) {
		os << " [";
		os << g.ds_get().begin()->name_get();
		os << ".chd]";
	}

	os << "\n";

	return os;
}

ostream& output::state_gamerom(const string& tag, const game& g, const gamearchive& gar, bool onecrc)
{
	free(tag);

	// .c_str() is required by g++ 2.95.3
	os << setw(FIELD_NAME_WIDTH) << setfill(' ') << g.name_get().c_str();

	os.setf(ios::right, ios::adjustfield);

	os << " " << setw(FIELD_ZIPSIZE_WIDTH) << dec << g.size_get()/1024;

	os.setf(ios::left, ios::adjustfield);

	os << " " << g.description_get();

	if (g.cloneof_get().length()) {
		os << " [cloneof ";
		os << g.cloneof_get();
		gamearchive::const_iterator parent = gar.find(g.cloneof_get());
		if (parent != gar.end()) {
			os << " " << dec << (*parent).size_get()/1024;
		}
		os << "]";
	}

	if (!g.working_subset_get()) {
		os << " [preliminary]";
	}

	if (g.resource_get()) {
		os << " [resource]";
	}

	if (onecrc) {
		unsigned crc = 0;
		unsigned size = 0;

		for(rom_by_name_set::const_iterator i=g.rs_get().begin();i!=g.rs_get().end();++i) {
			if (i->crc_get() != 0 && i->size_get() >= size) {
				crc = i->crc_get();
				size = i->size_get();
			}
		}

		if (crc) {
			os << " [onecrc ";

			os << setw(FIELD_CRC_WIDTH) << hex << setfill('0') << crc;

			os << "]";
		}
	}


	os << "\n";

	return os;
}

ostream& output::pair(unsigned size, crc_t crc)
{
	os.setf(ios::right, ios::adjustfield);

	os << setw(FIELD_SIZE_WIDTH) << dec << setfill(' ') << size << " " << setw(FIELD_CRC_WIDTH) << hex << setfill('0') << crc;

	os.setf(ios::left, ios::adjustfield);

	return os;
}

ostream& output::cmd_rom(const string& tag, const string& c, const string& name, unsigned size, crc_t crc)
{
	cmd(tag, c);

	pair(size, crc);

	os << " ";

	// .c_str() is required by g++ 2.95.3
	os << setw(FIELD_FILE_WIDTH) << setfill(' ') << name.c_str();

	return os;
}

ostream& output::cmd_rom(const string& tag, const string& cmd, const rom& r)
{

	cmd_rom(tag, cmd, r.name_get(), r.size_get(), r.crc_get());

	return os;
}

ostream& output::state_rom(const string& tag, const string& name, unsigned size, crc_t crc)
{
	op(tag);

	pair(size, crc);

	os << " ";

	// .c_str() is required by g++ 2.95.3
	os << setw(FIELD_FILE_WIDTH) << setfill(' ') << name.c_str();

	return os;
}

ostream& output::state_rom(const string& tag, const rom& r)
{

	state_rom(tag, r.name_get(), r.size_get(), r.crc_get());

	return os;
}

ostream& output::state_rom_real(const string& tag, const rom& r, unsigned real_size, unsigned real_crc)
{
	state_rom(tag, r);

	os << " [";
	pair(real_size, real_crc);
	os << "]";

	return os;
}

ostream& output::cmd_sample(const string& tag, const string& c, const sample& s)
{
	cmd(tag, c);

	// .c_str() is required by g++ 2.95.3
	os << setw(FIELD_FILE_WIDTH) << setfill(' ') << s.name_get().c_str();

	return os;
}

std::ostream& output::cmd_disk(const std::string& tag, const std::string& c, const std::string& name)
{
	cmd(tag, c);

	// .c_str() is required by g++ 2.95.3
	os << name.c_str();

	return os;
}

ostream& output::state_sample(const string& tag, const sample& s)
{
	op(tag);

	// .c_str() is required by g++ 2.95.3
	os << setw(FIELD_FILE_WIDTH) << setfill(' ') << s.name_get().c_str();

	return os;
}

ostream& output::state_disk(const string& tag, const disk& r)
{
	op(tag);

	os << r.sha1_get();

	os << " ";

	// .c_str() is required by g++ 2.95.3
	os << setw(FIELD_FILE_WIDTH) << setfill(' ') << r.name_get().c_str();

	return os;
}

ostream& output::state_disk_real(const string& tag, const disk& r, sha1 real_hash)
{
	state_disk(tag, r);

	os << " [";
	os << real_hash;
	os << "]";

	return os;
}

