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
 *
 * In addition, as a special exception, Andrea Mazzoleni
 * gives permission to link the code of this program with
 * the MAME library (or with modified versions of MAME that use the
 * same license as MAME), and distribute linked combinations including
 * the two.  You must obey the GNU General Public License in all
 * respects for all of the code used other than MAME.  If you modify
 * this file, you may extend this exception to your version of the
 * file, but you are not obligated to do so.  If you do not wish to
 * do so, delete this exception statement from your version.
 */

#include "extra.h"

/** \def USE_ERROR_SILENT
 * If defined USE_ERROR_SILENT the error functions don't show the error in the log.
 * This macro can be used to reduce the dependencies of the library.
 */

#ifndef USE_ERROR_SILENT
#include "log.h"
#endif

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

/****************************************************************************/
/* Error */

/**
 * Max length of the error description.
 */
#define ERROR_DESC_MAX 2048

/**
 * Last error description.
 */
static char error_buffer[ERROR_DESC_MAX];

/**
 * Flag set if an unsupported PNG feature is found.
 */
static adv_bool error_unsupported_flag;

/**
 * Get the current error description.
 */
const char* error_get(void) {
	/* remove the trailing \n */
	while (error_buffer[0] && isspace(error_buffer[strlen(error_buffer)-1]))
		error_buffer[strlen(error_buffer)-1] = 0;
	return error_buffer;
}

/**
 * Set the description of the last error.
 * The previous description is overwritten.
 * \note The description IS logged.
 */
void error_set(const char* text, ...)
{
	va_list arg;

	error_unsupported_flag = 0;

	va_start(arg,text);
	vsprintf(error_buffer,text,arg);

#ifndef USE_ERROR_SILENT
	log_std(("advance: set_error_description \""));
	log_va(text,arg);
	log_std(("\"\n"));
#endif

	va_end(arg);
}

/**
 * Set the description of the last error due unsupported feature.
 * \note The description IS logged.
 */
void error_unsupported_set(const char* text, ...)
{
	va_list arg;

	error_unsupported_flag = 1;

	va_start(arg,text);
	vsprintf(error_buffer,text,arg);

#ifndef USE_ERROR_SILENT
	log_std(("advance: set_error_description \""));
	log_va(text,arg);
	log_std(("\"\n"));
#endif

	va_end(arg);
}

/**
 * Check if a unsupported feature is found.
 * This function can be called only if another function returns with error.
 * \return
 *  - ==0 Not found.
 *  - !=0 Unsupported feature found.
 */
adv_bool error_unsupported_get(void)
{
	return error_unsupported_flag;
}

/**
 * Set the error description.
 * The previous description is overwritten.
 * The description is not logged.
 */
void error_nolog_set(const char* text, ...)
{
	va_list arg;

	error_unsupported_flag = 0;

	va_start(arg,text);
	vsprintf(error_buffer,text,arg);
	va_end(arg);
}

/**
 * Add some text at the description of the last error.
 * \note The description IS NOT logged.
 */
void error_nolog_cat(const char* text, ...)
{
	va_list arg;
	char buffer[ERROR_DESC_MAX];

	va_start(arg,text);
	vsprintf(buffer,text,arg);

	strncat(error_buffer,buffer,ERROR_DESC_MAX);
	error_buffer[ERROR_DESC_MAX-1] = 0;

	va_end(arg);
}


