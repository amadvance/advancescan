Name
        advscan - History For AdvanceSCAN

AdvanceSCAN Version 1.6 2003/03
	) Updated with autoconf 2.57 and automake 1.7.2

AdvanceSCAN Version 1.5 2003/02
	) Removed the recompression utilities. Now they are in the
		AdvanceCOMP package.

AdvanceSCAN Version 1.4 2002/12
	) Fixed a bug in the advmng utility when it was called with
		more than one file in the command line. The program
		was incorrectly adding a PLTE chunk at rgb images.

AdvanceSCAN Version 1.3 2002/11
	) Fixed a bug in the advscan utility. When a .zip file was completly
		removed the program was aborting with a "Failed stat" message
		because it was trying to reread the just deleted file.
	) Added the support for the transparency tRNS chunk at the
		advpng utility.
	) Improved the garbage detector for zipped roms.
	) Upgraded at the lastest Advance Library.
	) Fixes at the docs. [by Filipe Estima]
	) Minor changes at the autoconf/automake scripts.

AdvanceSCAN Version 1.2 2002/08
	) Added the advpng utility to compress the PNG files.
	) Added the advmng utility to compress the MNG files.
	) Added a Windows version.
	) Other minor fixes.

AdvanceSCAN Version 1.1 2002/06
	) Fixed an infinite loop bug testing some small damaged zips.
	) Removed some warning compiling with gcc 3.1.

AdvanceSCAN Version 1.0 2002/05
	) First public release.
	) Fixed the compression percentage computation on big files.
	) The unknow .zip files are now deleted if they contains only
		empty directories.
	) Added the --pedantic option at the advzip utility. These
		tests are only done if requested.
	) Renamed all the "unknow" options in "unknown".
	) Fixed the reading of configuration file containing the \r char.
	) Documentation fixes.
	) Other minor fixes.

AdvanceSCAN Version 0.6-beta 2002/05
	) Major revision.
	) Renamed AdvanceSCAN.
	) Updated to the last C++ standard.
	) General cleanup of the code.
	) Added the AdvanceDIFF and AdvanceZIP utility.

MAMESCAN Version 0.5 2001/08
	) Removed the options -fix, -cat, -collision.

MAMESCAN Version 0.4 2000/02
	) Added configure for msdos.

MAMESCAN Version 0.3
	) Minor revison.

MAMESCAN Version 0.2
	) Minor revison.

MAMESCAN Version 0.1 1999/07
	) First version.

MAMESCAN Version 0.0 1998/11
	) Posted in the MAME list the new listinfo command.

