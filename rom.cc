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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "rom.h"
#include "zip.h"

#include <iostream>

using namespace std;

rom::rom() {
	size = 0;
	crc = 0;
}

rom::rom(const string& Aname, unsigned Asize, crc_t Acrc) :
	name(Aname), size(Asize), crc(Acrc) {
}

rom::rom(const rom& A) :
	name(A.name), size(A.size), crc(A.crc) {
}

rom::~rom() {
}

void rom::name_set(const string& Aname) {
	name = Aname;
}

void rom::crc_set(crc_t Acrc) {
	crc = Acrc;
}

void rom::size_set(unsigned Asize) {
	size = Asize;
}

bool rom::operator==(const rom& A) const {
	return file_compare(name_get(),A.name_get())==0 && size_get()==A.size_get() && crc_get()==A.crc_get();
}

// gamerom

gamerom::gamerom() {
}

gamerom::gamerom(const string& Agame, const string& Aname, const unsigned Asize, const crc_t Acrc) :
	rom(Aname,Asize,Acrc), game(Agame) {
}

gamerom::gamerom(const gamerom& A) :
	rom(A), game(A.game) {
}

gamerom::~gamerom() {
}

void gamerom::game_set(const string& Agame) {
	game = Agame;
}

// gameromdata

gameromdata::gameromdata(const string& Agame, const string& Aname, const unsigned Asize, const crc_t Acrc, void* Adata) :
	gamerom(Agame,Aname,Asize,Acrc), data(Adata) {
}

gameromdata::gameromdata(const gameromdata& A) :
	gamerom(A),data(A.data) {
	A.data = 0;
}

gameromdata::~gameromdata() {
	if (data)
		operator delete(data);
}

void gameromdata::data_set(void* Adata) const {
	if (data)
		operator delete(data);
	data = Adata;
}

// sample

sample::sample(const string& Aname) {
	name = Aname;
}

sample::sample(const sample& A) {
	name = A.name;
}

sample::~sample() {
}


