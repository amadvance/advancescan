Name
	advzip - The AdvanceZIP compressor

Synopsis
	:advzip [-a, --add] [-x, --extract] [-l, --list]
	:[-z, --recompress] [-t, --test] [-0, --shrink-0]
	:[-1, --shrink-1] [-2, --shrink-2] [-3, --shrink-3]
	:[-N, --not-zip] [-p, --pedantic] [-q, --quiet]
	:[-h, --help] [-V, --version] ARCHIVES... [FILES...]

Description
	The main purpose of this utility is to recompress and test
	the zip archives to get the smallest possible size.

	For recompression the 7-Zip (www.7-zip.com) Deflate
	implementation is used. This implementation generally
	gives 5-10% more compression than the zLib Deflate
	implementation.

	For experimental purpose also the 7-Zip LZMA algorithm is
	available with the -N option. In this case, the generated
	zips WILL NOT BE USEABLE by any other program. To make
	them useable you need to recompress them without the -N
	option. Generally this algorithm gives 10-20% more
	compression than the 7-Zip Deflate implementation.

Options
	-a, --add ARCHIVE FILES...
		Create the specified archive with the specified
		files. You must specify only one archive.

	-x, --extract ARCHIVE
		Extract all the files on the specified archive. You
		must specify only one archive.

	-l, --list ARCHIVES...
		List the content of the specified archives.

	-z, --recompress ARCHIVES...
		Recompress the specified archives. If the -1, -2,
		-3 options are specified, it's used the smallest file
		choice from: the previous compressed data, the new
		compression and the uncompressed format. If the -0
		option is specified the archive is always rewritten
		without any compression.

	-t, --test ARCHIVES...
		Test the specified archives. The tests may be
		extended with the -p option.

	-N, --not-zip
		Use the LZMA algorithm when compressing. The
		generated zips will not be readable by any other
		application!

	-p, --pedantic
		Be pedantic on the zip tests. If this flag is
		enabled some more extensive tests on the zip
		integrity are done. These tests are generally not
		done by other zip utilities.

	-0, --shrink-0
		Set the compression level to "none". The file are
		only stored and not compressed. This option is
		very useful to expand the archives of .png and .mp3
		files. These files are already strongly compressed,
		trying to compress them another time is really a
		waste of time and resource. This is specifically
		true for the frontends that need to access them a
		lot of times.

	-1, --shrink-1
		Set the compression level to "normal". This is the
		default level of compression.

	-2, --shrink-2
		Set the compression level to "extra". It's SLOW.

	-3, --shrink-3
		Set the compression level to "extreme". It's VERY
		SLOW.

Copyright
	This file is Copyright (C) 2002 Andrea Mazzoleni, Filipe Estima

See Also
	advpng(1), advmng(1)
