Name{number}
	advscan - AdvanceSCAN Roms Manager

Synopsis
	:advscan [-c, --cfg CONFIG] [-r, --rom] [-s, --sample]
	:	[-k, --disk] [-a, --add-zip] [-b, --add-bin]
	:	[-d, --del-zip] [-u, --del-unknown]
	:	[-g, --del-garbage] [-t, --del-text]
	:	[-n, --print-only] [-p, --report]
	:	[-f, --filter FILTER]
	:	[-v, --verbose] < info.xml

	:advscan [-R, --rom-std] [-S, --sample-std]
	:	[-K, --disk-std] < info.xml

	:advscan [-i, --ident] files... < info.xml

	:advscan [-l, --bbs] < info.xml

	:advscan [-e, --equal] < info.xml

	:advscan [-h, --help] [-V, --version]

Description
	advscan is a command line utility for maintaining a zipped
	archive of roms and samples for AdvanceMAME, AdvanceMESS
	and any other MAME derivative emulator.

	The goal of advscan is to maintain a complete and accurate
	rom and sample collection using the split or the merged format.

	Split format means that each game has its own zip archive,
	containing only the rom files that are not already present in
	the parent zip archive (if one exists).

	Merged format means that each parent game has its own zip
	archive, containing the rom files of the parent and of all
	its clones. Clones don't have their own zip archive.
	See the `rom_format' option for details.

	advscan has these features:

	* Directly reads and writes zip archives without decompressing
		and recompressing them for best performance.
	* Adds, copies, moves, and renames files in zip
		archives. Any rom you have is placed
		automatically into the correct zip archive.
	* Recognizes advertising text files added by rom sites and
		deletes them.
	* Recognizes text files added by rom dumpers
		and lets you choose whether to keep or delete them.
	* It's safe. During all zip operations, any file
		removed or overwritten is saved in the
		`rom_unknown' and `sample_unknown' directories and kept
		for future use. This prevents any accidental
		data loss.

	but also has these limitations:

	* Supports only zipped roms and samples.
	* Supports only .chd files in game subdirectories.

Getting Started
	To use AdvanceSCAN, you must first create and edit its text
	configuration file named 'advscan.rc'. An example advscan.rc file
	is provided in the distribution archive, but you can also write
	one from scratch.

	Open your preferred text editor and create the configuration
	file advscan.rc with these options:

		:rom DIR_OF_YOUR_ROMS
		:rom_new DIR_OF_YOUR_ROMS
		:rom_unknown DIR_OF_REJECTED_ROMS
		:rom_import DIR_OF_ROMS_TO_IMPORT

	The most important configuration option is the directory where
	your roms are stored. This directory must be specified in both the
	'rom' and 'rom_new' options.
	The 'rom_unknown' option specifies the directory where unused
	roms are stored. AdvanceSCAN never deletes roms for safety,
	so you need to specify this holding directory.
	With the 'rom_import' option you can specify directories containing
	roms to import (for example, update packs you want to integrate
	into your rom set). If you don't have any, omit this option.

	To analyze the roms, AdvanceSCAN also requires the rom
	information file from the emulator. This file can be created
	with the command:

		:mame -listxml > info.xml

	Finally, you can run AdvanceSCAN to fix your roms with
	the command:

		:advscan -R < info.xml

	You can also generate a report on the state of your roms
	by running:

		:advscan -r -p < info.xml > report.txt

Options
	-c, --cfg CONFIG
		Select the configuration file. If not specified, the
		file `advscan.rc' in the current directory is used.

	=< info.xml
		advscan always requires a rom information file as
		input. It can generally be created with the -listxml
		option of the emulator.

	-r, --rom
		Operate on roms (.zip files). Subsequent action
		options will apply to roms.

	-s, --sample
		Operate on samples. Subsequent action options
		will apply to samples.

	-k, --disk
		Operate on disks (.chd files). Subsequent action
		options will apply to disks.

	-a, --add-zip
		Add missing rom zip archives. Any missing zip archive
		for which at least one rom is available will be created.

	-b, --add-bin
		Add, rename and replace necessary files in existing
		zip archives. No new zip archives are created; only
		already present zip files are modified. Files may be
		overwritten. Any overwritten file is saved to the
		`rom_unknown' or `sample_unknown' directory in a zip
		archive with the same name as the original.
		A file with the CRC and size of a rom, but with a
		different name, is renamed. Any additional copy of
		a rom is deleted, because it's identical to the rom.

	-d, --del-zip
		Remove any unknown zip or chd archive. Any removed
		archive is saved to the `rom_unknown',
		`sample_unknown' or `disk_unknown' directory.

	-u, --del-unknown
		Remove any unknown file from existing zip archives.
		Any removed file is saved to the `rom_unknown' or
		`sample_unknown' directory in a zip archive with the
		same name as the original. Only binary files are
		removed; all text files are kept.
		A file with the CRC and size of a rom of the zip archive
		is never considered unknown, even if it has a different
		name.

	-g, --del-garbage
		Remove any garbage files from zip archives. A garbage
		file is an advertising text file commonly added by rom
		download sites. Removed files are permanently deleted
		and not saved.

	-t, --del-text
		Remove any text file from zip archives. Any removed
		file is saved to the `rom_unknown' or `sample_unknown'
		directory in a zip archive with the same name as the
		original.

	-R, --rom-std
		Shortcut for options -rabdug. Performs all standard
		operations on roms except removing text files.

	-S, --sample-std
		Shortcut for options -sabdug. Performs all standard
		operations on samples except removing text files.

	-K, --disk-std
		Shortcut for options -kd. Moves any unknown disk file
		to the disk_unknown directory.

	-n, --print-only
		Dry-run mode: do not modify anything. Operations are
		only printed and NOT executed.

	-p, --report
		Write an extensive text report listing good, bad and
		missing roms and/or samples. The contents of any zip
		archive with missing files are printed. You must also
		specify the -r and/or -s options.

	-P, --report-file
		Write a concise text report with information about
		present files, listing incomplete, duplicate or
		missing required zips.

	-f, --filter FILTER
		Apply a specific filter to the rom list. See the
		FILTERS section for a detailed list of available filters.

	-v, --verbose
		Print a more verbose report. The contents of any zip
		archive are printed if it contains at least one
		unknown or bad rom file.

Information Options
	The following options are used only to print information.
	These options do not require a configuration file and do not
	access your roms.

	-i, --ident files...
		Identify the specified files using only the
		information present in the info file.

	-l, --bbs
		Print a standard `.bbs' file with descriptions
		of all roms in the info file.

	-e, --equal
		Print a list of all duplicate roms present in
		the info file. Only information present in the
		info file is used.

	-h, --help
		Print a short help screen.

	-V, --version
		Print the program version.

Identification
	Rom files are identified by their CRC and size. Roms are
	not actually decompressed; instead, the CRC values stored
	directly in the zip archives are read. If a rom has an
	incorrect CRC or size, but its name matches, it is kept
	if no valid alternative exists.

	Sample files are identified only by their names. This
	limits possible operations; essentially advscan can only
	report missing samples.

	Garbage files are identified by their size and CRC.

	All other files are identified using this algorithm:

	* If the name matches *.sam or *.wav, it is considered an
		unknown binary file.
	* If the name matches *.doc, *.txt, *.nfo,
		*.diz or readme.*, it is considered a text file.
	* If the file size is a power of 2, it is considered an
		unknown binary file.
	* Otherwise, it is considered a text file.

Configuration
	Running advscan requires two files: the rom information
	file and the configuration file.

	The rom information file contains details on all roms used
	by the emulator. It can be created with the command:

		:mame -listxml > info.xml

	This file is passed to advscan via standard input:

		:advscan [options] < info.xml

	Or by piping the two commands together:

		:mame -listxml | advscan [options]

	The configuration file is a text file that describes your
	directory structure. You can use absolute or relative paths.
	A relative path is relative to the current directory from
	which you run advscan.

	In Unix the PATH separator is `:'. In DOS and Windows the PATH
	separator is `;'. The following options are shown using the
	Unix format.

	=rom PATH:PATH...
		List of paths where roms are located. These are
		the zip archives that will be modified and fixed.
		Zip archives without write permission are never
		modified, but they are still used as a source of roms.
		If an operation is required on them, a warning is
		printed and the operation is skipped.

	=rom_new PATH
		Single directory where newly created zip archives are
		placed. It is STRONGLY recommended to include this path
		ALSO in the `rom' option. Otherwise, newly created
		archives will not be found on subsequent runs and will
		be recreated.

	=rom_import PATH:PATH...
		List of directory trees where other rom files are
		located. These are used to import rom files missing
		from the main rom directories. These files are only
		read and never modified in any way. This is very useful
		for specifying rom directories of other arcade
		emulators. When a new game is supported, the rom
		archive will be created automatically.

	=rom_unknown PATH
		Single directory where unknown rom zip archives are
		moved. Any rom file removed from the rom zip archives
		is placed in this directory. However, a rom file is
		automatically deleted by advscan if a duplicate exists
		in an archive listed in the `rom' or `rom_import' options.
		A file already saved in this directory is never
		overwritten. If a file with the same CRC and size is
		already saved, the file is not saved again. If a different
		file with the same name is already saved, the file is
		saved in a directory named as its CRC, like `crc/name'.

	=sample PATH:PATH...
		List of paths where samples are located. These
		are the zip archives that will be modified and fixed.
		As for roms, zip archives without write permission
		are never modified.

	=sample_unknown PATH
		Single path where unknown sample zip archives are
		moved. Any sample file removed from the sample zip
		archives is placed in this directory.

	=disk PATH:PATH...
		List of paths where disks are located. These
		are the chd files.

	=disk_unknown PATH
		Single path where unknown disk chd files will
		be moved.

	=rom_format split | merged
		Format of the rom zip archives. The default is `split'.
		See the MERGED FORMAT section for details.

	If the -c option is not specified, the configuration file
	is read from ./advscan.rc.

	The files advscan.rc.linux and advscan.rc.dos provide two
	example configuration files.

Merged Format
	By default, advscan uses the split format. In the `split'
	format each game has its own zip archive, containing only the
	rom files that are not already present in the parent, or in the
	bios, zip archive.

	With the `rom_format merged' option, advscan uses the merged
	format. In the merged format each parent game has its own zip
	archive, containing the rom files of the parent and of all its
	clones. The zip archive is named as the parent.
	Bios and devices keep their own zip archive, and their
	roms are not stored in the zip archive of the parent.
	The zip archives of the clones are considered unknown,
	and with the -d option they are moved to the `rom_unknown'
	directory, after their roms have been copied in the zip archive
	of the parent.

	In the merged zip archive the file names are always the following:

	* The roms of the parent are stored with their name
		and without any directory.
	* The roms of a clone are stored in a directory with
		the same name of the clone, like `clone/rom'.
	* A clone rom with the same CRC and size of a rom of the
		parent, or of a previous clone, is not stored again.
		Clones are processed in the same order of the rom
		information file. So, the rom is stored only once
		with the name used by the first clone.
	* Two or more roms of the parent with the same CRC and
		size are stored with all their names.

	The `rom_format' option applies only to roms. Samples
	and disks are always managed in the same way. Disks
	are always stored in the directory of the game, as in
	the split format, because the emulator searches them
	only in the directories of the game and of its parents.

	To convert a rom collection from the split format to the
	merged format, or vice versa, change the `rom_format' option
	and run:

		:advscan -R < info.xml

Damaged Files
	If a damaged .zip or .chd file is detected, it is renamed
	by appending the extension ".damaged".
	For a .chd file, this may happen if the file is a disk image
	in an older or unsupported format.

	If the rename operation is not possible, the program aborts.

	Please note that the program does not perform an
	exhaustive test to detect damaged files.

Filters
	By default, advscan uses all rom definitions, also including
	unplayable games. If you prefer, you can use only a subset
	of roms by specifying the --filter option.

	The available filters are:
		working - Use only working roms. A working rom is one
			reported as working or required by another working
			game. A rom is considered working if it is not marked
			with the "preliminary" flag in driver, video or
			sound. This is the recommended filter to keep only
			playable games.
		preliminary - Use only roms that are not in the "working"
			subset.
		working_parent - Use only working parent roms from the
			"working" subset, excluding clones. If a parent rom
			is not working but has a working clone, both
			the parent and the clone are included.
			This is the preferred filter if you want only one
			working game for each parent/clone set.
		working_clone - Use only roms that are in the "working"
			subset but not in the "working_parent" subset.

	In the merged format, if a clone is selected by the filter,
	its parent is always included, because the parent zip archive
	contains the roms of the clone. The roms of the clones not
	selected by the filter are instead considered unknown files
	of the parent zip archive.
	The `working_clone' filter is not useful in the merged format.

	For example, if you want to keep preliminary and working roms in
	different directories, you can set up two different advscan.rc files
	pointing to different directories but sharing the `rom_unknown' dir
	to allow automatic transfer between the two sets.
	In this configuration, updating your romset requires running
	advscan three times:

		on working set - To export preliminary roms to the
			rom_unknown dir.
		on preliminary set - To import preliminary roms and export
			working roms to the rom_unknown dir.
		on working set - To import working roms from the
			rom_unknown dir.

	For example:
		advscan -R -c advscan-wrk.rc -f working < advmame.xml > rom_wrk.log
		advscan -R -c advscan-pre.rc -f preliminary < advmame.xml > rom_pre.log
		advscan -R -c advscan-wrk.rc -f working < advmame.xml > rom_wrk.log

Report Format
	The report generated with the -p option uses the following
	status tags:

	=rom_good
		A recognized good rom. The rom is recognized by its
		name, CRC and size.

	=rom_bad
		A recognized bad rom with an incorrect size or CRC.
		The rom is recognized by its name.

	=rom_miss
		A missing rom.

	=rom_misnamed
		A file with the CRC and size of a rom, but stored
		with a different name. The rom is considered present,
		and with the -b option the file is renamed.

	=rom_duplicate
		An additional copy of a rom, already present with the
		correct name, or with another `rom_misnamed' file.
		With the -b option the file is deleted.

	=disk_bad
		A recognized bad disk with an incorrect hash.
		The disk is recognized by its name.

	=nodump_good
		A fake "NO GOOD DUMP KNOWN" rom. The rom is
		recognized by its name, size and CRC.

	=nodump_miss
		A missing "NO GOOD DUMP KNOWN" rom. This is the
		expected state, as a "no dump" rom should be missing.

	=nodump_bad
		A recognized bad "NO GOOD DUMP KNOWN" rom. The rom is
		recognized by its name.

	=sound_good
		A recognized good sound sample. The sample is
		recognized by its name.

	=sound_miss
		A missing sound sample.

	=text
		An unknown text file.

	=binary
		An unknown binary file.

	=garbage
		A recognized garbage file. A garbage file is an
		advertising text file commonly added by rom
		sites. The file is recognized by its name, size and
		CRC.

Examples
	For typical usage, run advscan with:

		:advscan -R < info.xml

	This command will fix your rom collection (without removing
	accompanying text files).

	To preview all operations in advance without modifying files:

		:advscan -R -n < info.xml

	which only displays the planned operations.

	To generate a comprehensive report of your rom set:

		:advscan -r -p < info.xml > report.txt

	To increase the verbosity of the printed information,
	add the -v switch.

Target Set
	The target set is the list of files that advscan expects to
	find for each game. It's computed from the rom information
	file, and it depends on the `rom_format' option.

	These rules are common to the split and merged formats:

	* A rom is identified by its CRC and size, not by its name.
		A rom is present if a file with its CRC and size is in
		the zip archive, with any name. Such a file is never
		unknown. If the name is wrong it's reported as
		`rom_misnamed', and renamed by the -b option.
		Any additional copy is reported as `rom_duplicate', and
		deleted by the -b option. If there are more copies with
		a wrong name, they are used in name order: the first ones
		are renamed, one for each rom with that CRC and size,
		and the others are deleted.
	* The `romof' attribute defines the chain of a game: its parent,
		and then the bios or device of the parent. A rom with the
		same CRC and size of a rom of a game in the chain is merged,
		and it's not part of the target set, because it's stored
		by the other game.
	* Bios and devices always have their own zip archive. Their roms
		are never stored in the zip archive of another game.
	* A rom with size zero, or marked as `nodump', is never required.
		A game with only such roms doesn't require a zip archive.
	* Samples follow the same rules, using the `sampleof' chain
		and identifying the samples by name.
	* Disks follow the same rules, identifying the disks by name
		and SHA1. A clone disk with the same name of a parent disk,
		but with a different SHA1, is part of the target set.
		Disks are always stored in a directory named as the game.
	* A missing or circular definition in the `romof' or `cloneof'
		chain is reported, and the chain ends there.
	* The target set is computed after the filter selection, see the
		FILTERS section.

	A file in a zip archive that doesn't match any rom of the target
	set is unknown. Unknown files are classified with these rules,
	common to the split and merged formats, and for roms and samples:

	* Binary, if the name matches *.sam or *.wav. Otherwise text,
		if the name matches *.txt, *.doc, *.nfo, *.diz or readme.*.
		Otherwise binary, if the size is zero or a power of 2.
		Otherwise text. The names are matched case insensitive
		and including any directory.
	* Garbage, if it's text, and its name, size and CRC match
		a known advertising file.
	* A binary file can be used as a rom of another zip archive
		by the -a and -b options. It's removed by the -u option,
		and saved in the `rom_unknown' directory.
	* A text file is never used as a rom. It's removed only by
		the -t option, and saved in the `rom_unknown' directory.
	* A garbage file is never used as a rom. It's removed only by
		the -g option, and permanently deleted.
	* Unknown files are reported with the -v option, with the
		tags `binary', `text' and `garbage'.
	* A disk that doesn't match any disk of the target set is
		unknown, and it's moved to the `disk_unknown' directory
		by the -d option.

	The differences between the two formats are the following:

	* In the split format each game has its own zip archive,
		containing its target set. The rom names don't contain
		any directory.
	* In the merged format only the games that are not clones
		have their own zip archive. It contains the target set of
		the game, and of all its clones, stored as `clone/rom'.
		A rom with the same CRC and size of a rom already stored
		is not stored again. Bios and devices are never merged in
		the zip archive of the parent, and a game with a `cloneof'
		of a bios or device keeps its own zip archive.
	* In the merged format a filter selecting a clone also
		selects its parent.

Copyright
	This file is Copyright (C) 2003, 2004, 2008 Andrea Mazzoleni, Filipe Estima

See Also
	advdiff(1)

