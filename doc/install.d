Name
	advscan - AdvanceSCAN Installation

	As root or user type:

		:./configure
		:make

	To check the generated executable type:

		:make check

	and as root to install the programs and the documentation
	type:

		:make install-strip

	then change the example configuration file advscan.rc for
	AdvanceSCAN to your requirements.

	The documentation is in the man pages:

		:man advscan
		:man advdiff
		:man advzip
		:man advpng
		:man advmng

Requirements
	To build AdvanceSCAN you need a C/C++ compiler.

	Tested with the following compilers on a GNU/Linux
	environment :

		GNU gcc 2.95.3
		GNU gcc 3.0.4
		GNU gcc 3.1

	The build process on the DOS platform isn't tested. The DOS
	and Windows binaries are generated in Linux cross compiling.
	If you have the cross compiler installed, you can try the
	configure.msdos and configure.windows configuration scripts.

