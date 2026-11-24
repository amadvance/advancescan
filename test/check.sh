#!/bin/sh
#
# Regression test of advscan with a small rom set in the split and merged formats.
#
# Usage: check.sh SRCDIR ADVSCAN
#
# It must be run in the build directory. All the files are created in bench/.
# The rom set is fixed in the split format, then converted to the merged
# format, and then converted back to the split format.
# The output of every step is compared with the expected one in SRCDIR/test.
#

SRC=$1/test
ADVSCAN=$2
BENCH=bench

fail() {
	echo "Failed: $*"
	exit 1
}

# compare an output with the expected one
compare() {
	if ! cmp -s $BENCH/$1 $SRC/$2; then
		diff -u $SRC/$2 $BENCH/$1
		fail "$BENCH/$1 differs from $SRC/$2"
	fi
}

# run advscan, saving the output and the log
run() {
	OUT=$1
	shift
	$ADVSCAN "$@" < $SRC/merge.xml > $BENCH/$OUT 2> $BENCH/$OUT.err || fail "advscan $* returned an error"
	# any message that is not a log is an error
	if grep -v '^log: ' $BENCH/$OUT.err > /dev/null; then
		cat $BENCH/$OUT.err
		fail "advscan $* printed an error"
	fi
}

# report of the rom set, without the sizes that depend on the zip compression
report() {
	run $1.tmp -c $SRC/$2.rc -rvp
	# files of the zips with an error
	sed '/^total_game_rom_dup/,$d' $BENCH/$1.tmp > $BENCH/$1
	# status of the games, only the name
	sed -n 's/^\(game_rom_[a-z]*\)  *\([^ ]*\) .*/\1 \2/p' $BENCH/$1.tmp >> $BENCH/$1
	echo >> $BENCH/$1
	run $1.tmp -c $SRC/$2.rc -rP
	cat $BENCH/$1.tmp >> $BENCH/$1
}

# fix the rom set, saving the list of the operations, and checking that
# a second run has nothing to do
fix() {
	run $1 -c $SRC/$2.rc -Rn
	run $1.tmp -c $SRC/$2.rc -R
	run $1.again -c $SRC/$2.rc -Rn
	test -s $BENCH/$1.again && fail "advscan -R didn't fix all the rom set in the $2 format"
	return 0
}

rm -rf $BENCH/rom $BENCH/import $BENCH/unknown $BENCH/empty
mkdir -p $BENCH/rom $BENCH/import $BENCH/unknown $BENCH/empty || fail "mkdir"
cp $SRC/rom/*.zip $BENCH/rom || fail "cp"
cp $SRC/import/*.zip $BENCH/import || fail "cp"
# the source dir may be read-only, like in make distcheck, and advscan doesn't modify read-only zips
chmod u+w $BENCH/rom/*.zip $BENCH/import/*.zip || fail "chmod"

# initial rom set in the split format, with errors
report split_dirty.lst split
compare split_dirty.lst split_dirty.lst

# fix in the split format
fix split_fix.lst split
compare split_fix.lst split_fix.lst
report split.lst split
compare split.lst split.lst

# convert to the merged format
fix merged_fix.lst merged
compare merged_fix.lst merged_fix.lst
report merged.lst merged
compare merged.lst merged.lst

# the files removed are saved in the unknown dir, without overwriting
# the ones with the same name saved before
report merged_unknown.lst unknown
compare merged_unknown.lst merged_unknown.lst

# convert back to the split format, it must be equal at the first fix
fix split_back_fix.lst split
compare split_back_fix.lst split_back_fix.lst
report split_back.lst split
compare split_back.lst split.lst

exit 0
