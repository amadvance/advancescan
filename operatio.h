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

#ifndef __OPERATION_H
#define __OPERATION_H

class operation {
	bool flag_active; // execute the command
	bool flag_output; // print the command
	bool flag_move; // remove unknown zip
	bool flag_add; // additive change on not existing zips
	bool flag_fix; // additive change on existing zips
	bool flag_remove_binary; // remove unknown binary files
	bool flag_remove_text; // remove unknown text files
	bool flag_remove_garbage; // remove garbage files
public:
	void active_set(bool A) {
		flag_active = A;
	}

	void output_set(bool A) {
		flag_output = A;
	}

	void operation_set(bool move, bool add, bool fix, bool remove_binary, bool remove_text, bool remove_garbage) {
		flag_move = move;
		flag_add = add;
		flag_fix = fix;
		flag_remove_binary = remove_binary;
		flag_remove_text = remove_text;
		flag_remove_garbage = remove_garbage;
	}

	bool active_add() const { return flag_active && flag_add; }
	bool active_fix() const { return flag_active && flag_fix; }
	bool active_remove_binary() const { return flag_active && flag_remove_binary; }
	bool active_remove_text() const { return flag_active && flag_remove_text; }
	bool active_move() const { return flag_active && flag_move; }
	bool active_remove_garbage() const { return flag_active && flag_remove_garbage; }

	bool output_add() const { return flag_output && flag_add; }
	bool output_fix() const { return flag_output && flag_fix; }
	bool output_remove_binary() const { return flag_output && flag_remove_binary; }
	bool output_remove_text() const { return flag_output && flag_remove_text; }
	bool output_move() const { return flag_output && flag_move; }
	bool output_remove_garbage() const { return flag_output && flag_remove_garbage; }
};

#endif
