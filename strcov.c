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

#include "portable.h"

#include "strcov.h"

/**
 * Match one string with a shell pattern expression.
 * \param pattern Pattern with with the globbing * and ? chars.
 * \param str String to compare,
 * \return
 *  - ==0 match
 *  - !=0 don't match
 */
int striwildcmp(const char* pattern, const char* str)
{
	while (*str && *pattern) {
		if (*pattern == '*') {
			++pattern;
			while (*str) {
				if (striwildcmp(pattern, str)==0) return 0;
				++str;
			}
		} else if (*pattern == '?') {
			++str;
			++pattern;
		} else {
			if (toupper(*str) != toupper(*pattern))
				return 1;
			++str;
			++pattern;
		}
	}
	while (*pattern == '*')
		++pattern;
	if (!*str && !*pattern)
		return 0;
	return 1;
}

/**
 * Convert a string to a unsigned.
 * \param s String to convert.
 * \param e First not numerical char detected.
 * \return 0 if string contains a non digit char.
 */
unsigned strdec(const char* s, const char** e)
{
	unsigned v = 0;
	while (*s) {
		if (!isdigit(*s)) {
			*e = s;
			return 0;
		}
		v *= 10;
		v += *s - '0';
		++s;
	}
	*e = s;
	return v;
}

/**
 * Convert a hex string to a unsigned.
 * \param s String to convert.
 * \param e First not numerical char detected.
 * \return converted value.
 */
unsigned strhex(const char* s, const char** e)
{
	unsigned v = 0;
	while (*s) {
		if (!isxdigit(*s)) {
			break;
		}
		v *= 16;
		if (*s>='0' && *s<='9')
			v += *s - '0';
		else
			v += toupper(*s) - 'A' + 10;
		++s;
	}
	if (e)
		*e = s;
	return v;
}

/**
 * Convert a hex string to a vector of bytes.
 * \param dst Destination vector.
 * \param s String to convert.
 * \param e First not numerical char detected.
 */
void strvhex(unsigned char* dst, const char* s, const char** e)
{
	unsigned i;
	i = 0;
	while (*s) {
		unsigned v;
		if (!isxdigit(*s)) {
			break;
		}
		if (*s>='0' && *s<='9')
			v = *s - '0';
		else
			v = toupper(*s) - 'A' + 10;
		dst[i] = v * 16;
		++s;

		if (!isxdigit(*s)) {
			break;
		}
		if (*s>='0' && *s<='9')
			v = *s - '0';
		else
			v = toupper(*s) - 'A' + 10;
		dst[i] += v;
		++i;
		++s;
	}
	if (e)
		*e = s;
}


