/*
 * This file is part of the Advance project.
 *
 * Copyright (C) 2003, 2004 Andrea Mazzoleni
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

#include "game.h"
#include "strcov.h"
#include "expat/expat.h"

#include <string>
#include <iostream>

using namespace std;

/**
 * Max depth checked.
 */
#define DEPTH_MAX 5

enum token_t {
	token_open,
	token_close,
	token_data
};

struct state_t;

typedef void (process_t)(struct state_t* state, enum token_t t, const char* s, unsigned len, const char** attributes);

/**
 * State for every depth level.
 */
struct level_t {
	const char* tag; /**< Tag name. */
	char* data; /**< Accumulative data. */
	unsigned len; /**< Length of the data. */
	process_t* process; /**< Processing function. */
};

/**
 * Global parsing state.
 */
struct state_t {
	XML_Parser parser; /**< Parser. */
	int depth; /**< Current depth. */
	struct level_t level[DEPTH_MAX]; /**< Level state. */
	int error; /**< Error flag. */
	string error_desc;
	rom* r;
	disk* d;
	game* g; /**< Curren game in the loading process. */
	game_by_name_set* a; /**< Game archive. */

};

static void process_error(struct state_t* state, const char* tag, const char* msg)
{
	ostringstream o;

	if (tag)
		o << "Error reading at line " << XML_GetCurrentLineNumber(state->parser) << " for element/attribute `" << tag << "' for " << msg << ".";
	else
		o << "Error reading at line " << XML_GetCurrentLineNumber(state->parser) << " for " << msg << ".";

	state->error_desc = o.str();
	state->error = 1;
}

static void process_game(struct state_t* state, enum token_t t, const char* s, unsigned len, const char** attributes)
{
	if (t == token_open) {
		state->g = new game;
	} else if (t == token_close) {
		state->a->insert(*state->g);
		delete state->g;
		state->g = 0;
	}
}

static void process_runnable(struct state_t* state, enum token_t t, const char* s, unsigned len, const char** attributes)
{
	if (t == token_data) {
		string v = string(s, len);
		if (!state->g) {
			process_error(state, 0, "invalid state");
			return;
		}
		state->g->resource_set(v == "no");
	}
}

static void process_name(struct state_t* state, enum token_t t, const char* s, unsigned len, const char** attributes)
{
	if (t == token_data) {
		if (!state->g) {
			process_error(state, 0, "invalid state");
			return;
		}
		state->g->name_set(string(s, len));
	}
}

static void process_description(struct state_t* state, enum token_t t, const char* s, unsigned len, const char** attributes)
{
	if (t == token_data) {
		if (!state->g) {
			process_error(state, 0, "invalid state");
			return;
		}
		state->g->description_set(string(s, len));
	}
}

static void process_manufacturer(struct state_t* state, enum token_t t, const char* s, unsigned len, const char** attributes)
{
	if (t == token_data) {
		if (!state->g) {
			process_error(state, 0, "invalid state");
			return;
		}
		state->g->manufacturer_set(string(s, len));
	}
}

static void process_year(struct state_t* state, enum token_t t, const char* s, unsigned len, const char** attributes)
{
	if (t == token_data) {
		if (!state->g) {
			process_error(state, 0, "invalid state");
			return;
		}
		state->g->year_set(string(s, len));
	}
}

static void process_cloneof(struct state_t* state, enum token_t t, const char* s, unsigned len, const char** attributes)
{
	if (t == token_data) {
		if (!state->g) {
			process_error(state, 0, "invalid state");
			return;
		}
		state->g->cloneof_set(string(s, len));
	}
}

static void process_romof(struct state_t* state, enum token_t t, const char* s, unsigned len, const char** attributes)
{
	if (t == token_data) {
		if (!state->g) {
			process_error(state, 0, "invalid state");
			return;
		}
		state->g->romof_set(string(s, len));
	}
}

static void process_sampleof(struct state_t* state, enum token_t t, const char* s, unsigned len, const char** attributes)
{
	if (t == token_data) {
		if (!state->g) {
			process_error(state, 0, "invalid state");
			return;
		}
		state->g->sampleof_set(string(s, len));
	}
}

static void process_rom(struct state_t* state, enum token_t t, const char* s, unsigned len, const char** attributes)
{
	if (t == token_open) {
		state->r = new rom;
	} else if (t == token_close) {
		if (!state->g) {
			process_error(state, 0, "invalid state");
			return;
		}
		state->g->rs_get().insert(*state->r);
		delete state->r;
		state->r = 0;
	}
}

static void process_romsize(struct state_t* state, enum token_t t, const char* s, unsigned len, const char** attributes)
{
	if (t == token_data) {
		string v = string(s, len);
		if (!state->r) {
			process_error(state, 0, "invalid state");
			return;
		}
		const char* e;
		unsigned z = strdec(v.c_str(), &e);
		if (*e != 0) {
			process_error(state, 0, "invalid size value");
			return;
		}
		state->r->size_set(z);
	}
}

static void process_romcrc(struct state_t* state, enum token_t t, const char* s, unsigned len, const char** attributes)
{
	if (t == token_data) {
		string v = string(s, len);
		if (!state->r) {
			process_error(state, 0, "invalid state");
			return;
		}
		const char* e;
		unsigned z = strhex(v.c_str(), &e);
		if (*e != 0) {
			process_error(state, 0, "invalid crc value");
			return;
		}
		state->r->crc_set(z);
	}
}

static void process_romname(struct state_t* state, enum token_t t, const char* s, unsigned len, const char** attributes)
{
	if (t == token_data) {
		if (!state->r) {
			process_error(state, 0, "invalid state");
			return;
		}
		state->r->name_set(string(s, len));
	}
}

static void process_romstatus(struct state_t* state, enum token_t t, const char* s, unsigned len, const char** attributes)
{
	if (t == token_data) {
		if (!state->r) {
			process_error(state, 0, "invalid state");
			return;
		}
		if (string(s, len) == "nodump")
			state->r->nodump_set(true);
	}
}

static void process_driverstatus(struct state_t* state, enum token_t t, const char* s, unsigned len, const char** attributes)
{
	if (t == token_data) {
		if (!state->g) {
			process_error(state, 0, "invalid state");
			return;
		}
		if (string(s, len) == "preliminary")
			state->g->working_set(false);
	}
}

static void process_samplename(struct state_t* state, enum token_t t, const char* s, unsigned len, const char** attributes)
{
	if (t == token_data) {
		if (!state->g) {
			process_error(state, 0, "invalid state");
			return;
		}
		sample s(string(s, len));
		state->g->ss_get().insert(s);
	}
}

static void process_disk(struct state_t* state, enum token_t t, const char* s, unsigned len, const char** attributes)
{
	if (t == token_open) {
		state->d = new disk;
	} else if (t == token_close) {
		if (!state->g) {
			process_error(state, 0, "invalid state");
			return;
		}
		state->g->ds_get().insert(*state->d);
		delete state->d;
		state->d = 0;
	}
}

static void process_diskname(struct state_t* state, enum token_t t, const char* s, unsigned len, const char** attributes)
{
	if (t == token_data) {
		if (!state->d) {
			process_error(state, 0, "invalid state");
			return;
		}
		state->d->name_set(string(s, len));
	}
}

static void process_disksha1(struct state_t* state, enum token_t t, const char* s, unsigned len, const char** attributes)
{
	if (t == token_data) {
		if (!state->d) {
			process_error(state, 0, "invalid state");
			return;
		}
		state->d->sha1_set(sha1(string(s, len)));
	}
}

static char* match_mamemessraine = "mame|mess|raine";
static char* match_gamemachine = "game|machine";

/**
 * Conversion table.
 * Any element/attribute not in this table is ignored.
 */
struct conversion_t {
	unsigned depth;
	const char* name[DEPTH_MAX];
	process_t* process;
};

static struct conversion_t CONV1[] = {
	{ 1, { match_mamemessraine, match_gamemachine, 0, 0, 0 }, process_game },
	{ 0, { 0, 0, 0, 0, 0 }, 0 }
};

static struct conversion_t CONV2[] = {
	{ 2, { match_mamemessraine, match_gamemachine, "runnable", 0, 0 }, process_runnable },
	{ 2, { match_mamemessraine, match_gamemachine, "name", 0, 0 }, process_name },
	{ 2, { match_mamemessraine, match_gamemachine, "description", 0, 0 }, process_description },
	{ 2, { match_mamemessraine, match_gamemachine, "manufacturer", 0, 0 }, process_manufacturer },
	{ 2, { match_mamemessraine, match_gamemachine, "year", 0, 0 }, process_year },
	{ 2, { match_mamemessraine, match_gamemachine, "cloneof", 0, 0 }, process_cloneof },
	{ 2, { match_mamemessraine, match_gamemachine, "romof", 0, 0 }, process_romof },
	{ 2, { match_mamemessraine, match_gamemachine, "sampleof", 0, 0 }, process_sampleof },
	{ 2, { match_mamemessraine, match_gamemachine, "rom", 0, 0 }, process_rom },
	{ 2, { match_mamemessraine, match_gamemachine, "disk", 0, 0 }, process_disk },
	{ 0, { 0, 0, 0, 0, 0 }, 0 }
};

static struct conversion_t CONV3[] = {
	{ 3, { match_mamemessraine, match_gamemachine, "rom", "name", 0 }, process_romname },
	{ 3, { match_mamemessraine, match_gamemachine, "rom", "size", 0 }, process_romsize },
	{ 3, { match_mamemessraine, match_gamemachine, "rom", "crc", 0 }, process_romcrc },
	{ 3, { match_mamemessraine, match_gamemachine, "rom", "status", 0 }, process_romstatus },
	{ 3, { match_mamemessraine, match_gamemachine, "driver", "status", 0 }, process_driverstatus },
	{ 3, { match_mamemessraine, match_gamemachine, "sample", "name", 0 }, process_samplename },
	{ 3, { match_mamemessraine, match_gamemachine, "disk", "name", 0 }, process_diskname },
	{ 3, { match_mamemessraine, match_gamemachine, "disk", "sha1", 0 }, process_disksha1 },
	{ 0, { 0, 0, 0, 0, 0 }, 0 }
};

/**
 * Identify the specified element/attribute.
 */
static struct conversion_t* identify(unsigned depth, const struct level_t* level)
{
	struct conversion_t* conv;

	switch (depth) {
	case 1 : conv = CONV1; break;
	case 2 : conv = CONV2; break;
	case 3 : conv = CONV3; break;
	default:
		return 0;
	}

	for(unsigned i=0;conv[i].name[0];++i) {
		bool equal = true;

		// check all the item, backward
		for(int j=depth;equal && j>=0;--j) {
			if (conv[i].name[j] == match_mamemessraine) {
				if (strcmp(level[j].tag, "mame") != 0 && strcmp(level[j].tag, "mess") != 0 && strcmp(level[j].tag, "raine") != 0)
					equal = false;
			} else if (conv[i].name[j] == match_gamemachine) {
				if (strcmp(level[j].tag, "game") != 0 && strcmp(level[j].tag, "machine") != 0)
					equal = false;
			} else {
				if (strcmp(level[j].tag, conv[i].name[j]) != 0)
					equal = false;
			}
		}

		// if match return
		if (equal)
			return &conv[i];
	}

	return 0;
}

/**
 * End Handler for the Expat parser.
 */
static void end_handler(void* data, const XML_Char* name)
{
	struct state_t* state = (struct state_t*)data;

	if (state->depth < DEPTH_MAX) {
		if (state->error == 0) {
			if (state->level[state->depth].process) {
				if (state->level[state->depth].data != 0) {
					state->level[state->depth].process(state, token_data, state->level[state->depth].data, state->level[state->depth].len, 0);
				} else {
					state->level[state->depth].process(state, token_data, "", 0, 0);
				}
				state->level[state->depth].process(state, token_close, 0, 0, 0);
			}
		}
		free(state->level[state->depth].data);
	}

	--state->depth;
}

/**
 * Data Handler for the Expat parser.
 */
void data_handler(void* data, const XML_Char* s, int len)
{
	struct state_t* state = (struct state_t*)data;

	if (state->depth < DEPTH_MAX) {
		if (state->error == 0) {
			/* accumulate the data */
			unsigned new_len = state->level[state->depth].len + len;
			state->level[state->depth].data = (char*)realloc(state->level[state->depth].data, new_len);
			if (!state->level[state->depth].data) {
				process_error(state, state->level[state->depth].tag, "low memory");
				return;
			}
			memcpy(state->level[state->depth].data + state->level[state->depth].len, s, len);
			state->level[state->depth].len += len;
		}
	}
}

/**
 * Start Handler for the Expat parser.
 */
static void start_handler(void* data, const XML_Char* name, const XML_Char** attributes)
{
	struct state_t* state = (struct state_t*)data;
	struct conversion_t* c;
	unsigned i;

	++state->depth;

	if (state->depth < DEPTH_MAX) {
		state->level[state->depth].tag = name;
		state->level[state->depth].data = 0;
		state->level[state->depth].len = 0;

		if (state->error == 0) {
			c = identify(state->depth, state->level);
			if (c) {
				state->level[state->depth].process = c->process;
				state->level[state->depth].process(state, token_open, 0, 0, attributes);
			} else {
				state->level[state->depth].process = 0;
			}

			for(i=0;attributes[i];i+=2) {
				const char* null_atts[1] = { 0 };
				start_handler(data, attributes[i], null_atts);
				data_handler(data, attributes[i+1], strlen(attributes[i+1]));
				end_handler(data, attributes[i]);
			}
		} else {
			state->level[state->depth].process = 0;
		}
	}
}

void gamearchive::load_xml(istream& is)
{
	struct state_t state;
	char buf[16384];

	state.parser = XML_ParserCreate(NULL);
	if (!state.parser) {
		throw error() << "Error creating the XML parser";
	}

	state.depth = -1;
	state.error = 0;
	state.error_desc = "";
	state.r = 0;
	state.d = 0;
	state.g = 0;
	state.a = &map;

	XML_SetUserData(state.parser, &state);
	XML_SetElementHandler(state.parser, start_handler, end_handler);
	XML_SetCharacterDataHandler(state.parser, data_handler);

	while (1) {
		int done;
		int len;

		is.read(buf, sizeof(buf));
		if (is.bad()) {
			process_error(&state, "", "read error");
			break;
		}

		len = is.gcount();

		done = is.eof();

		if (XML_Parse(state.parser, buf, len, done) == XML_STATUS_ERROR) {
			process_error(&state, "", XML_ErrorString(XML_GetErrorCode(state.parser)));
			break;
		}

		if (done)
			break;
	}

	XML_ParserFree(state.parser);

	if (state.error) {
		throw error() << state.error_desc;
	}
}

