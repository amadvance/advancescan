/*
 * This file is part of the Advance project.
 *
 * Copyright (C) 2002, 2004 Andrea Mazzoleni
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

#ifndef __TOKEN_H
#define __TOKEN_H

#include <string>

std::string token_get(const std::string& s, unsigned& ptr, const char* sep);
void token_skip(const std::string& s, unsigned& ptr, const char* sep);
std::string token_get(const std::string& s, unsigned& ptr, char sep);
void token_skip(const std::string& s, unsigned& ptr, char sep);
std::string strip_space(const std::string& s);

#endif
