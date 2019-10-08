mkdir -p bin
for f in linux/*.c; do
	out="bin/`basename "$f" .c`"
	if test -f "$out"; then
		continue
	fi
	echo $f
	flags=""
	if grep "__NR_mmap2" $f; then
		flags="-m32"
	fi
	gcc "$f" -static -pthread $flags -o $out
done
