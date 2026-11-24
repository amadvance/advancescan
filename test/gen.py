#!/usr/bin/env python3
#
# Generate the input data of the advscan regression test run by check.sh:
# the rom information file merge.xml, the initial rom set in rom/
# and the import dir in import/.
#
# Usage: python3 gen.py
#
# The files are written in the directory of this script. They are committed
# in the repository, so this script is needed only to change the test data.
# After a change, also the expected .lst files must be updated.
#

import hashlib, os, zipfile, zlib

T = os.path.dirname(os.path.abspath(__file__))

def data(seed, size):
	out = b""
	n = 0
	while len(out) < size:
		out += hashlib.sha256(("%s-%d" % (seed, n)).encode()).digest()
		n += 1
	return out[:size]

# rom contents
D = {
	"bios": data("bios", 1024),
	"dev": data("dev", 512),
	"g1": data("g1", 2048),
	"g2": data("g2", 2048),
	"g2b": data("g2b", 2048),   # gameb g2.bin, same name of the parent, different crc
	"shared": data("shared", 1024), # in both gameb and gamea
	"a1": data("a1", 4096),
	"a1bad": data("a1bad", 4096),
	"o1": data("o1", 1024),
	"s1": data("s1", 512),
	"s3": data("s3", 3000),     # size not power of 2, classified as text
	"junk": data("junk", 256),
}
README = b"This is a readme file added by someone.\r\n" * 3

def crc(d):
	return "%08x" % zlib.crc32(d)

def rom(name, key):
	d = D[key]
	return '\t\t<rom name="%s" size="%d" crc="%s"/>\n' % (name, len(d), crc(d))

def machine(name, attr, roms, extra=""):
	s = '\t<machine name="%s"%s>\n' % (name, attr)
	s += '\t\t<description>%s</description>\n\t\t<year>2000</year>\n\t\t<manufacturer>Test</manufacturer>\n' % name
	s += "".join(roms) + extra
	s += '\t\t<driver status="good"/>\n\t</machine>\n'
	return s

# the clone gameb is before gamea, to test the information file order
xml = '<?xml version="1.0"?>\n<mame build="test">\n'
xml += machine("testbios", ' isbios="yes"', [rom("bios.bin", "bios")])
xml += machine("testdev", ' isdevice="yes" runnable="no"', [rom("dev.bin", "dev")])
xml += machine("game", ' romof="testbios"', [
	rom("bios.bin", "bios"), rom("g1.bin", "g1"), rom("g2.bin", "g2"),
	'\t\t<rom name="nd.bin" size="1024" status="nodump"/>\n'],
	'\t\t<device_ref name="testdev"/>\n')
xml += machine("gameb", ' cloneof="game" romof="game"', [
	rom("bios.bin", "bios"), rom("g1.bin", "g1"), rom("g2.bin", "g2b"), rom("shared.bin", "shared")])
xml += machine("gamea", ' cloneof="game" romof="game"', [
	rom("bios.bin", "bios"), rom("g1.bin", "g1"), rom("g2.bin", "g2"), rom("a1.bin", "a1"), rom("shared.bin", "shared")])
xml += machine("other", ' cloneof="testbios" romof="testbios"', [
	rom("bios.bin", "bios"), rom("o1.bin", "o1")])
xml += machine("solo", '', [
	rom("s1.bin", "s1"), rom("s2.bin", "s1"), rom("s3.bin", "s3")])
xml += '</mame>\n'

def mkzip(path, files):
	os.makedirs(os.path.dirname(path), exist_ok=True)
	with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as z:
		for name, d in files:
			zi = zipfile.ZipInfo(name, date_time=(2000, 1, 1, 0, 0, 0))
			zi.compress_type = zipfile.ZIP_DEFLATED
			zi.external_attr = 0o644 << 16
			z.writestr(zi, d)

with open(os.path.join(T, "merge.xml"), "w") as f:
	f.write(xml)

# initial rom set in the split format, with errors
mkzip(os.path.join(T, "rom", "testbios.zip"), [("bios.bin", D["bios"])])
mkzip(os.path.join(T, "rom", "game.zip"), [
	("g1.bin", D["g1"]),
	("G2.BAD", D["g2"]),        # misnamed
	("g1copy.bin", D["g1"]),    # duplicate
	("readme.txt", README),     # text
	("bios.bin", D["bios"]),    # rom of the bios, unknown binary
])
mkzip(os.path.join(T, "rom", "gamea.zip"), [
	("a1.bin", D["a1bad"]),     # bad crc
	("a1.good", D["a1"]),       # misnamed, fixes the bad rom
	("shared.bin", D["shared"]),
])
mkzip(os.path.join(T, "rom", "other.zip"), [("o1.bin", D["o1"])])
mkzip(os.path.join(T, "rom", "solo.zip"), [
	("s1.bin", D["s1"]),        # also used for s2.bin with the same crc
	("S3.DAT", D["s3"]),        # misnamed, classified as text
])
mkzip(os.path.join(T, "rom", "junk.zip"), [("junk.bin", D["junk"])]) # unknown zip

# import directory, with the roms of the missing gameb and testdev
mkzip(os.path.join(T, "import", "pack.zip"), [
	("dir/gameb_g2.rom", D["g2b"]),
	("shared.rom", D["shared"]),
	("dev.bin", D["dev"]),
])
