#include "game.h"

#include "lib/readinfo.h"

using namespace std;

extern "C" int info_ext_get(void* _arg)
{
	istream* arg = static_cast<istream*>(_arg);
	return arg->get();
}

extern "C" void info_ext_unget(void* _arg, char c)
{
	istream* arg = static_cast<istream*>(_arg);
	arg->putback(c);
}

bool gamearchive::load_info_internal() {
	info_t token = info_token_get();
	while (token!=info_eof) {
		if (token != info_symbol) return false;
		if (strcmp(info_text_get(),"game")==0 || strcmp(info_text_get(),"resource")==0 || strcmp(info_text_get(),"machine")==0) {
			if (info_token_get() != info_open) return false;
			game g;
			token = info_token_get();
			while (token != info_close) {
				if (token != info_symbol)
					return false;
				if (strcmp(info_text_get(),"name")==0) {
					if (info_token_get() != info_symbol) return false;
					g.name_set( info_text_get() );
				} else if (strcmp(info_text_get(),"description")==0) {
					if (info_token_get() != info_string) return false;
					g.description_set( info_text_get() );
				} else if (strcmp(info_text_get(),"manufacturer")==0) {
					if (info_token_get() != info_string) return false;
					g.manufacturer_set( info_text_get() );
				} else if (strcmp(info_text_get(),"year")==0) {
					if (info_token_get() != info_symbol) return false;
					g.year_set( info_text_get() );
				} else if (strcmp(info_text_get(),"cloneof")==0) {
					if (info_token_get() != info_symbol) return false;
					g.cloneof_set( info_text_get() );
				} else if (strcmp(info_text_get(),"romof")==0) {
					if (info_token_get() != info_symbol) return false;
					g.romof_set( info_text_get() );
				} else if (strcmp(info_text_get(),"sampleof")==0) {
					if (info_token_get() != info_symbol) return false;
					g.sampleof_set( info_text_get() );
				} else if (strcmp(info_text_get(),"rom")==0) {
					if (info_token_get() != info_open)  return false;
					rom r;
					token = info_token_get();
					while (token != info_close) {
						if (token != info_symbol) return false;
						if (strcmp(info_text_get(),"name")==0) {
							if (info_token_get() != info_symbol) return false;
							r.name_set( info_text_get() );
						} else if (strcmp(info_text_get(),"size")==0) {
							const char* e;
							if (info_token_get() != info_symbol) return false;
							r.size_set( strdec( info_text_get(), &e ) );
							if (*e != 0)
								return false;
						} else if (strcmp(info_text_get(),"crc")==0) {
							const char* e;
							if (info_token_get() != info_symbol) return false;
							r.crc_set( strhex( info_text_get(), &e ) );
							if (*e != 0)
								return false;
						} else if (strcmp(info_text_get(),"flags")==0) {
							if (info_token_get() != info_symbol) return false;
							if (strcmp(info_text_get(),"nodump")==0)
								r.nodump_set(true);
						} else {
							if (info_skip_value() == info_error) return false;
						}
						token = info_token_get();
					}
					g.rs_get().insert( r );
				} else if (strcmp(info_text_get(),"driver")==0) {
					if (info_token_get() != info_open)  return false;
					token = info_token_get();
					while (token != info_close) {
						if (token != info_symbol) return false;
						if (strcmp(info_text_get(),"status")==0) {
							if (info_token_get() != info_symbol) return false;
							g.working_set( strcmp( info_text_get(), "good") == 0 );
						} else {
							if (info_skip_value() == info_error) return false;
						}
						token = info_token_get();
					}
				} else if (strcmp(info_text_get(),"sample")==0) {
					if (info_token_get() != info_symbol) return false;
					sample s( info_text_get() );
					g.ss_get().insert( s );
				} else {
					if (info_skip_value() == info_error) return false;
				}
				token = info_token_get();
			}
			map.insert( g );
		} else {
			if (info_skip_value() == info_error) return false;
		}
		token = info_token_get();
	}

	return true;
}

void gamearchive::load_info(istream& f) {
	info_init(info_ext_get, info_ext_unget, &f);

	bool r = load_info_internal();

	if (!r) {
		unsigned row = info_row_get()+1;
		unsigned col = info_col_get()+1;
		info_done();
		throw error() << "Invalid data at row " << row << " at column " << col << ".";
	}

	info_done();
}

