#include "7z.h"

#include "DeflateEncoder.h"
#include "DeflateDecoder.h"

bool compress_deflate_7z(const char* in_data, unsigned in_size, char* out_data, unsigned& out_size, unsigned num_passes, unsigned num_fast_bytes) throw ()
{
	try {
		NDeflate::NEncoder::CCoder cc;

		if (cc.SetEncoderNumPasses(num_passes) != S_OK)
			return false;

		if (cc.SetEncoderNumFastBytes(num_fast_bytes) != S_OK)
			return false;

		ISequentialInStream in(in_data, in_size);
		ISequentialOutStream out(out_data, out_size);

		UINT64 in_size_l = in_size;

		if (cc.Code(&in, &out, &in_size_l) != S_OK)
			return false;

		out_size = out.size_get();

		if (out.overflow_get())
			return false;

		return true;
	} catch (...) {
		return false;
	}
}

bool decompress_deflate_7z(const char* in_data, unsigned in_size, char* out_data, unsigned out_size) throw () {
	try {
		NDeflate::NDecoder::CCoder cc;

		ISequentialInStream in(in_data, in_size);
		ISequentialOutStream out(out_data, out_size);

		UINT64 in_size_l = in_size;
		UINT64 out_size_l = out_size;

		if (cc.Code(&in, &out, &in_size_l, &out_size_l) != S_OK)
			return false;

		if (out.size_get() != out_size || out.overflow_get())
			return false;

		return true;
	} catch (...) {
		return false;
	}
}
