Name{number}
	advdiff - AdvanceSCAN Diff Utility

Synopsis
	:advdiff [-i, --info] ORIG_INFO DEST_INFO

Description
	This utility prints all the games contained in the
	DEST_INFO file and not contained in ORIG_INFO file.
	A game is considered contained if all his roms are present
	in a game with the same name.

	The DEST_INFO and ORIG_INFO files are the rom information
	files generated with the -listinfo/-listxml or -gameinfo
	options of the emulators.

	You can use this utility to create differential romset.
	For example you can use this command:

		:advdiff mame.lst raine.lst > raine_diff.lst

	to create the list of roms needed by Raine, which are not
	already included in the standard MAME set.

	You can also use this utility to create the list of roms,
	which need to be updated between two emulator releases.
	For example you can use this command:

		:advdiff mame79.xml mame80.xml > mame79-80.xmlt

	to create the list of roms needed to upgrade the MAME
	romset from the version 0.79 version to the version 0.80.

Options
	-i, --info
		Output in the old info format instead of the XML format.

Copyright
	This file is Copyright (C) 2002, 2004 Andrea Mazzoleni, Filipe Estima

See Also
	advscan(1)

