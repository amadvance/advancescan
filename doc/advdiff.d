Name
	advdiff - The AdvanceDIFF roms diff

Synopsis
	:advdiff ORIG_INFO DEST_INFO

Description
	This utility prints all the games contained in the
	DEST_INFO file and not contained in ORIG_INFO file.
	A game is considerated contained if all his roms are present
	in a game with the same name.

	The DEST_INFO and ORIG_INFO files are the rom information
	files generated with the -listinfo or -gameinfo options of
	the emulators.

	You can use this utility to create differential romset.
	For example you can use this command:

		:advdiff mame.lst raine.lst > raine_diff.lst

	to create the list of roms needed by Raine, which are not
	already included in the standard MAME set.

	You can also use this utility to create the list of roms,
	which need to be updated between two emulator releases.
	For example you can use this command:

		:advdiff mame59.lst mame60.lst > mame59-60.lst

	to create the list of roms needed to upgrade the MAME rom
	set from the 0.59 version to the 0.60 version.

Copyright
	This file is Copyright (C) 2002 Andrea Mazzoleni

See Also
	advscan(1)

