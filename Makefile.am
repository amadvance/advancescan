## Process this file with automake to produce Makefile.in

bin_PROGRAMS = advscan advdiff advzip advpng advmng

advdiff_SOURCES = \
	diff.cc \
	gamediff.cc \
	readinfo.c \
	portable.c

advzip_SOURCES = \
	rezip.cc \
	zip.cc \
	utility.cc \
	siglock.cc \
	compress.cc \
	zipsh.cc \
	portable.c \
	7z/7zdeflate.cc \
	7z/7zlzma.cc \
	7z/AriBitCoder.cc \
	7z/CRC.cc \
	7z/DeflateDecoder.cc \
	7z/DeflateEncoder.cc \
	7z/HuffmanEncoder.cc \
	7z/IInOutStreams.cc \
	7z/InByte.cc \
	7z/LSBFDecoder.cc \
	7z/LSBFEncoder.cc \
	7z/LZMA.cc \
	7z/LZMADecoder.cc \
	7z/LZMAEncoder.cc \
	7z/LenCoder.cc \
	7z/LiteralCoder.cc \
	7z/OutByte.cc \
	7z/WindowIn.cc \
	7z/WindowOut.cc

advscan_SOURCES =  \
	scan.cc \
	readinfo.c \
	rom.cc \
	utility.cc \
	conf.cc \
	ziprom.cc \
	game.cc \
	zip.cc \
	output.cc \
	analyze.cc \
	siglock.cc \
	portable.c

advpng_SOURCES = \
	repng.cc \
	fz.c \
	png.c \
	pngex.cc \
	utility.cc \
	siglock.cc \
	compress.cc \
	portable.c \
	7z/7zdeflate.cc \
	7z/7zlzma.cc \
	7z/AriBitCoder.cc \
	7z/CRC.cc \
	7z/DeflateDecoder.cc \
	7z/DeflateEncoder.cc \
	7z/HuffmanEncoder.cc \
	7z/IInOutStreams.cc \
	7z/InByte.cc \
	7z/LSBFDecoder.cc \
	7z/LSBFEncoder.cc \
	7z/LZMA.cc \
	7z/LZMADecoder.cc \
	7z/LZMAEncoder.cc \
	7z/LenCoder.cc \
	7z/LiteralCoder.cc \
	7z/OutByte.cc \
	7z/WindowIn.cc \
	7z/WindowOut.cc

advmng_SOURCES = \
	remng.cc \
	fz.c \
	png.c \
	mng.c \
	pngex.cc \
	utility.cc \
	siglock.cc \
	compress.cc \
	portable.c \
	7z/7zdeflate.cc \
	7z/7zlzma.cc \
	7z/AriBitCoder.cc \
	7z/CRC.cc \
	7z/DeflateDecoder.cc \
	7z/DeflateEncoder.cc \
	7z/HuffmanEncoder.cc \
	7z/IInOutStreams.cc \
	7z/InByte.cc \
	7z/LSBFDecoder.cc \
	7z/LSBFEncoder.cc \
	7z/LZMA.cc \
	7z/LZMADecoder.cc \
	7z/LZMAEncoder.cc \
	7z/LenCoder.cc \
	7z/LiteralCoder.cc \
	7z/OutByte.cc \
	7z/WindowIn.cc \
	7z/WindowOut.cc

EXTRA_DIST = advscan.rc.linux advscan.rc.dos \
	README AUTHORS HISTORY INSTALL COPYING \
	doc/advscan.d doc/advdiff.d doc/advzip.d doc/advpng.d doc/advmng.d doc/history.d doc/readme.d doc/authors.d doc/install.d \
	doc/advscan.1 doc/advdiff.1 doc/advzip.1 doc/advpng.1 doc/advmng.1 doc/history.1 doc/readme.1 doc/authors.1 doc/install.1 \
	doc/advscan.txt doc/advdiff.txt doc/advzip.txt doc/advpng.txt doc/advmng.txt doc/history.txt doc/readme.txt doc/authors.txt doc/install.txt \
	doc/advscan.html doc/advdiff.html doc/advzip.html doc/advpng.html doc/advmng.html doc/history.html doc/readme.html doc/authors.html doc/install.html \
	configure.msdos configure.windows autogen.sh noautomake.sh \
	7z/README \
	check.lst

noinst_HEADERS = \
	png.h \
	mng.h \
	fz.h \
	endianrw.h \
	readinfo.h \
	pngex.h \
	compress.h \
	rom.h \
	utility.h \
	conf.h \
	ziprom.h \
	game.h \
	gamediff.h \
	zip.h \
	error.h \
	output.h \
	operatio.h \
	analyze.h \
	analyze.dat \
	siglock.h \
	portable.h \
	7z/7z.h \
	7z/AriBitCoder.h \
	7z/AriConst.h \
	7z/AriPrice.h \
	7z/BinTree.h \
	7z/BinTree2.h \
	7z/BinTree2Main.h \
	7z/BinTree3.h \
	7z/BinTree3Main.h \
	7z/BinTree3Z.h \
	7z/BinTree3ZMain.h \
	7z/BinTree4.h \
	7z/BinTree4Main.h \
	7z/BinTree4b.h \
	7z/BinTree4bMain.h \
	7z/BinTreeMF.h \
	7z/BinTreeMFMain.h \
	7z/BinTreeMain.h \
	7z/BitTreeCoder.h \
	7z/CRC.h \
	7z/Const.h \
	7z/DeflateDecoder.h \
	7z/DeflateEncoder.h \
	7z/HuffmanDecoder.h \
	7z/HuffmanEncoder.h \
	7z/IInOutStreams.h \
	7z/InByte.h \
	7z/LSBFDecoder.h \
	7z/LSBFEncoder.h \
	7z/LZMA.h \
	7z/LZMADecoder.h \
	7z/LZMAEncoder.h \
	7z/LenCoder.h \
	7z/LiteralCoder.h \
	7z/OutByte.h \
	7z/Portable.h \
	7z/RCDefs.h \
	7z/RangeCoder.h \
	7z/WindowIn.h \
	7z/WindowOut.h

man_MANS = doc/advscan.1 doc/advdiff.1 doc/advzip.1 doc/advpng.1 doc/advmng.1

clean-local:
	rm -f *~
	rm -f new.lst check.zip
	rm -f advscan.exe advscan.rc advdiff.exe advzip.exe advpng.exe advmng.exe
	rm -f README AUTHORS HISTORY INSTALL doc/copying.txt
	rm -f doc/*.hh

# Docs

%.1 : %.d
	advd2 man < $(srcdir)/$< > $@

%.txt : %.d
	advd2 txt < $(srcdir)/$< | todos > $@

%.html : %.d
	advd2 html < $(srcdir)/$< > $@

%.hh : %.d
	advd2 frame < $(srcdir)/$< > $@

# Archives

advscan.rc: advscan.rc.dos
	cp $(srcdir)/advscan.rc.dos advscan.rc

README: doc/readme.txt
	cat $< | fromdos > $@

AUTHORS: doc/authors.txt
	cat $< | fromdos > $@

INSTALL: doc/install.txt
	cat $< | fromdos > $@

HISTORY: doc/history.txt
	cat $< | fromdos > $@

doc/copying.txt: COPYING
	cat $< | todos > $@

check-local: advzip check.lst
	@./advzip -a -1 check.zip $(srcdir)/COPYING
	@./advzip -l check.zip > new.lst
	@./advzip -t -p check.zip
	@./advzip -a -2 check.zip $(srcdir)/COPYING
	@./advzip -l check.zip >> new.lst
	@./advzip -t -p check.zip
	@./advzip -a -3 check.zip $(srcdir)/COPYING
	@./advzip -l check.zip >> new.lst
	@./advzip -t -p check.zip
	@./advzip -a -1 -N check.zip $(srcdir)/COPYING
	@./advzip -l check.zip >> new.lst
	@./advzip -t -p check.zip
	@./advzip -a -2 -N check.zip $(srcdir)/COPYING
	@./advzip -l check.zip >> new.lst
	@./advzip -t -p check.zip
	@./advzip -a -3 -N check.zip $(srcdir)/COPYING
	@./advzip -l check.zip >> new.lst
	@./advzip -t -p check.zip
	@cmp new.lst $(srcdir)/check.lst
	@echo Success!

DISTDOS_ROOT = \
	doc/readme.txt doc/authors.txt doc/history.txt doc/copying.txt \
	advscan.exe advdiff.exe advzip.exe advpng.exe advmng.exe \
	advscan.rc

DISTDOS_DOC = \
	doc/advscan.txt doc/advdiff.txt doc/advzip.txt doc/advpng.txt doc/advmng.txt \
	doc/readme.txt doc/authors.txt doc/history.txt \
	doc/advscan.html doc/advdiff.html doc/advzip.html doc/advpng.html doc/advmng.html \
	doc/readme.html doc/authors.html doc/history.html 

distdos: $(DISTDOS_ROOT) $(DISTDOS_DOC)
	-upx -q -q advscan.exe
	-upx -q -q advdiff.exe
	-upx -q -q advzip.exe
	-upx -q -q advpng.exe
	-upx -q -q advmng.exe
	rm -f $(PACKAGE)-$(VERSION)-dos-i586.zip
	mkdir tmp
	cp $(DISTDOS_ROOT) tmp
	mkdir tmp/doc
	cp $(DISTDOS_DOC) tmp/doc
	cd tmp && zip -r ../$(PACKAGE)-$(VERSION)-dos-i586.zip *
	rm -r tmp

distwindows: $(DISTDOS_ROOT) $(DISTDOS_DOC)
	-upx -q -q advscan.exe
	-upx -q -q advdiff.exe
	-upx -q -q advzip.exe
	-upx -q -q advpng.exe
	-upx -q -q advmng.exe
	rm -f $(PACKAGE)-$(VERSION)-windows-i586.zip
	mkdir tmp
	cp $(DISTDOS_ROOT) tmp
	mkdir tmp/doc
	cp support/zlib.dll tmp
	cp $(DISTDOS_DOC) tmp/doc
	cd tmp && zip -r ../$(PACKAGE)-$(VERSION)-windows-i586.zip *
	rm -r tmp

DISTWEB = \
	doc/advscan.hh doc/advdiff.hh doc/advzip.hh doc/advpng.hh doc/advmng.hh doc/history.hh

web: $(DISTWEB)
