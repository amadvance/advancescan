Name
	advpng - The AdvancePNG compressor

Synopsis
	:advpng [-l, --list] [-z, --recompress] [-0, --shrink-0]
	:[-1, --shrink-1] [-2, --shrink-2] [-3, --shrink-3]
	:[-f, --force] [-q, --quiet] [-h, --help] [-V, --version]
	:FILES...

Description
	The main purpose of this utility is to recompress png
	files to get the smallest possible size.

Options
	-l, --list FILES...
		List the content of the specified files.

	-z, --recompress FILES...
		Recompress the specified files. If the -1, -2, -3
		options are specified it's used the smallest file
		choice from the previous compressed data and the
		new compression. If the -0 option is specified the
		file is always rewritten without any compression.

	-0, --shrink-0
		Set the compression level to "none". The file are
		only stored and not compressed. The file is always
		rewritten also if it's bigger.

	-1, --shrink-1
		Set the compression level to "normal". This is the
		default level of compression.

	-2, --shrink-2
		Set the compression level to "extra". It's SLOW.

	-3, --shrink-3
		Set the compression level to "extreme". It's VERY
		SLOW.

	-f, --force
		Force the use of the new file also if it's bigger.

Bugs
	All the ancillary chuncks are ignored and silently removed.
	Only the transparency tRNS chunck is processed.

Copyright
	This file is Copyright (C) 2002 Andrea Mazzoleni, Filipe Estima

See Also
	advzip(1), advmng(1)
