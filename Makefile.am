## Process this file with automake to produce Makefile.in

bin_PROGRAMS = advscan advzip advdiff

advdiff_SOURCES = \
	diff.cc \
	gamediff.cc \
	readinfo.c

advzip_SOURCES = \
	rezip.cc \
	zip.cc \
	utility.cc \
	siglock.cc \
	zipsh.cc \
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
	siglock.cc

EXTRA_DIST = advscan.rc.linux advscan.rc.dos \
	README AUTHORS HISTORY COPYING BUILD INSTALL \
	advscan.1 advzip.1 advdiff.1 \
	configure.msdos \
	7z/README \
	check.lst

noinst_HEADERS = \
	readinfo.h \
	rom.h \
	types.h \
	utility.h \
	conf.h \
	ziprom.h \
	game.h \
	gamediff.h \
	zip.h \
	ziperr.h \
	output.h \
	operatio.h \
	analyze.h \
	siglock.h \
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

man_MANS = advscan.1 advzip.1 advdiff.1

clean-local:
	rm -f *~
	rm -f new.lst check.zip
	rm -f advscan.exe advscan.txt advscan.html advscan.rc
	rm -f advzip.exe advzip.txt advzip.html
	rm -f advdiff.exe advdiff.txt advdiff.html

# DOS archive

%.txt : %.1
	$(GROFF) -man -Tascii $(srcdir)/$< | $(COL) -bx > $@

%.html : %.1
	$(GROFF) -man -Thtml $(srcdir)/$< > $@

advscan.txt: advscan.1
advzip.txt: advzip.1
advdiff.txt: advdiff.1

advscan.html: advscan.1
advzip.html: advzip.1
advdiff.html: advdiff.1

advscan.rc: advscan.rc.dos
	cp $(srcdir)/advscan.rc.dos advscan.rc

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

DISTDOS = \
	README AUTHORS HISTORY \
	advscan.txt advzip.txt advdiff.txt \
	advscan.exe advzip.exe advdiff.exe \
	advscan.rc

distdos: $(DISTDOS)
	-upx -q -q advscan.exe
	-upx -q -q advzip.exe
	-upx -q -q advdiff.exe
	rm -f $(PACKAGE)-$(VERSION)-dos-i586.zip
	zip $(PACKAGE)-$(VERSION)-dos-i586.zip $(DISTDOS)



