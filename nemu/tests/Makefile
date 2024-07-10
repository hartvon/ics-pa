all: test
%: %.c
	gcc -ggdb -O0 $^ -o $@

.PHONY: run debug clean

run:
	./test
debug:
	gdb test
clean:
	rm -f test
