#ifndef __7Z_H
#define __7Z_H

bool compress_deflate_7z(const char* in_data, unsigned in_size, char* out_data, unsigned& out_size, unsigned num_passes, unsigned num_fast_bytes) throw ();
bool decompress_deflate_7z(const char* in_data, unsigned in_size, char* out_data, unsigned out_size) throw ();

bool compress_lzma_7z(const char* in_data, unsigned in_size, char* out_data, unsigned& out_size, unsigned algo, unsigned dictionary_size, unsigned num_fast_bytes) throw ();
bool decompress_lzma_7z(const char* in_data, unsigned in_size, char* out_data, unsigned out_size) throw ();

#endif

