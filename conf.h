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

#ifndef __CONFIG_H
#define __CONFIG_H

#include "utility.h"

// ------------------------------------------------------------------------
// Config

class config {
	filepath_container rompath;
	filepath_container romreadonlytree;
	filepath_container samplepath;
	filepath romunknownpath;
	filepath sampleunknownpath;
	filepath romnewpath;
public:
	config(const std::string& file, bool need_rom, bool need_sample, bool need_change);
	~config();

	const filepath_container& rompath_get() const { return rompath; }
	const filepath_container& romreadonlytree_get() const { return romreadonlytree; }
	const filepath& romunknownpath_get() const { return romunknownpath; }
	const filepath& romnewpath_get() const { return romnewpath; }

	const filepath_container& samplepath_get() const { return samplepath; }
	const filepath& sampleunknownpath_get() const { return sampleunknownpath; }
};

#endif
