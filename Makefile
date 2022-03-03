addr2line:addr2line.c
	gcc -c addr2line.c -std=c99
	ar rcs libaddr2line.a addr2line.o
	rm addr2line.o

.PHONY:test
test:
	gcc test.c -laddr2line -lbfd -liberty -ldl -lz  -o test -std=c99 -g

.PHONY:clean
clean:
	rm -rf {*.o,*.a}
	rm test

.PHONY:install
install:
	cp addr2line.h /usr/local/include/
	cp libaddr2line.a /usr/local/lib/

