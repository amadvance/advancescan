/*
 * This file is part of the Advance project.
 *
 * Copyright (C) 1998, 1999, 2000, 2001, 2002 Andrea Mazzoleni
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
#include "zip.h"

#include <iostream>

using namespace std;

rom::rom()
{
	size = 0;
	crc = 0;
	nodump = false;
}

rom::rom(const string& Aname, unsigned Asize, crc_t Acrc, bool Anodump)
	: name(Aname), size(Asize), crc(Acrc), nodump(Anodump)
{
}

rom::rom(const rom& A)
	: name(A.name), size(A.size), crc(A.crc), nodump(A.nodump)
{
}

rom::~rom()
{
}

void rom::name_set(const string& Aname)
{
	name = Aname;
}

void rom::crc_set(crc_t Acrc)
{
	crc = Acrc;
}

void rom::size_set(unsigned Asize)
{
	size = Asize;
}

void rom::nodump_set(bool Anodump)
{
	nodump = Anodump;
}

bool rom::operator==(const rom& A) const
{
	return file_compare(name_get(), A.name_get())==0 && size_get()==A.size_get() && crc_get()==A.crc_get();
}

gamerom::gamerom()
{
}

gamerom::gamerom(const string& Agame, const string& Aname, const unsigned Asize, const crc_t Acrc, bool Anodump)
	: rom(Aname, Asize, Acrc, Anodump), game(Agame)
{
}

gamerom::gamerom(const std::string& Agame, const rom& Arom)
	: rom(Arom), game(Agame)
{
}

gamerom::gamerom(const gamerom& A)
	: rom(A), game(A.game)
{
}

gamerom::~gamerom()
{
}

void gamerom::game_set(const string& Agame)
{
	game = Agame;
}

