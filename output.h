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

#ifndef __OUTPUT_H
#define __OUTPUT_H

#include "game.h"

class output {
	std::ostream& os;

	std::ostream& op(const std::string& op);
	std::ostream& total(const std::string& op);
	std::ostream& free(const std::string& op);
	std::ostream& cmd(const std::string& op, const std::string& cmd);
	std::ostream& pair(unsigned size, crc_t crc);

public:
	output(std::ostream& Aos) : os(Aos) { os.setf(std::ios::left, std::ios::adjustfield); }

	std::ostream& operator()() { return os; }

	std::ostream& zip(const std::string& op, const std::string& zip);
	std::ostream& ziptag(const std::string& op, const std::string& zip, const std::string& tag);

	std::ostream& title(const std::string& op, bool& title, const std::string& path);

	std::ostream& c(const std::string& op, unsigned count);
	std::ostream& cs(const std::string& op, unsigned count, unsigned long long size);
	std::ostream& csz(const std::string& op, unsigned count, unsigned long long size, unsigned long long sizezip);
	std::ostream& cz(const std::string& op, unsigned count, unsigned long long sizezip);
	
	std::ostream& state_gamesample(const std::string& tag, const game& g);
	std::ostream& state_gamerom(const std::string& tag, const game& g, const gamearchive& gar);

	std::ostream& cmd_rom(const std::string& tag, const std::string& cmd, const rom& r);
	std::ostream& cmd_rom(const std::string& tag, const std::string& cmd, const std::string& name, unsigned size, crc_t crc);
	std::ostream& cmd_sample(const std::string& tag, const std::string& cmd, const sample& s);

	std::ostream& state_rom(const std::string& tag, const rom& r);
	std::ostream& state_rom(const std::string& tag, const std::string& name, unsigned size, crc_t crc);
	std::ostream& state_rom_real(const std::string& tag, const rom& r, unsigned real_size, unsigned real_crc);
	std::ostream& state_sample(const std::string& tag, const sample& s);
};

#endif
