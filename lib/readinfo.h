/*
 * This file is part of the Advance project.
 *
 * Copyright (C) 1999-2002 Andrea Mazzoleni
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

/** \file
 * INFO file support.
 */

#ifndef __READINFO_H
#define __READINFO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

/** \addtogroup Info */
/*@{*/

enum info_t {
	info_error,      /* generic error in reading */
	info_eof,        /* end of file got at valid position */
	info_symbol,     /* symbol */
	info_open,       /* ( */
	info_close,      /* ) */
	info_string      /* c string automatically converted */
};

void info_init(void);
void info_done(void);

const char* info_text_get(void);
enum info_t info_token_get(FILE* f);
enum info_t info_skip_value(FILE* f);

unsigned info_row_get(void);
unsigned info_col_get(void);
unsigned info_pos_get(void);

/*@}*/

#ifdef __cplusplus
}
#endif

#endif
