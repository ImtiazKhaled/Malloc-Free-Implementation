CC=       	gcc
CFLAGS= 	-g -gdwarf-2 -std=gnu99 -Wall
LDFLAGS=
LIBRARIES=      lib/libmalloc-ff.so \
		lib/libmalloc-nf.so \
		lib/libmalloc-bf.so \
		lib/libmalloc-wf.so

TESTS=		tests/test1 \
                tests/test2 \
                tests/test3 \
                tests/test4 \
                tests/bfwf \
                tests/ffnf 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all:    $(LIBRARIES) $(TESTS)

lib/libmalloc-ff.so:     src/malloc.c
	$(CC) -shared -fPIC $(CFLAGS) -DFIT=0 -o $@ $< $(LDFLAGS)

lib/libmalloc-nf.so:     src/malloc.c
	$(CC) -shared -fPIC $(CFLAGS) -DNEXT=0 -o $@ $< $(LDFLAGS)

lib/libmalloc-bf.so:     src/malloc.c
	$(CC) -shared -fPIC $(CFLAGS) -DBEST=0 -o $@ $< $(LDFLAGS)

lib/libmalloc-wf.so:     src/malloc.c
	$(CC) -shared -fPIC $(CFLAGS) -DWORST=0 -o $@ $< $(LDFLAGS)

run_tests:	
	env LD_PRELOAD=lib/libmalloc-ff.so tests/test1 > outfile
	env LD_PRELOAD=lib/libmalloc-ff.so tests/test2 >> outfile
	env LD_PRELOAD=lib/libmalloc-ff.so tests/test3 >> outfile 
	env LD_PRELOAD=lib/libmalloc-ff.so tests/test4 >> outfile 	
	env LD_PRELOAD=lib/libmalloc-nf.so tests/test1 >> outfile 
	env LD_PRELOAD=lib/libmalloc-nf.so tests/test2 >> outfile 
	env LD_PRELOAD=lib/libmalloc-nf.so tests/test3 >> outfile 
	env LD_PRELOAD=lib/libmalloc-nf.so tests/test4 >> outfile 
	env LD_PRELOAD=lib/libmalloc-bf.so tests/test1 >> outfile 
	env LD_PRELOAD=lib/libmalloc-bf.so tests/test2 >> outfile 
	env LD_PRELOAD=lib/libmalloc-bf.so tests/test3 >> outfile 
	env LD_PRELOAD=lib/libmalloc-bf.so tests/test4 >> outfile 
	env LD_PRELOAD=lib/libmalloc-wf.so tests/test1 >> outfile 
	env LD_PRELOAD=lib/libmalloc-wf.so tests/test2 >> outfile 
	env LD_PRELOAD=lib/libmalloc-wf.so tests/test3 >> outfile 
	env LD_PRELOAD=lib/libmalloc-wf.so tests/test4 >> outfile 

clean:
	rm -f $(LIBRARIES) $(TESTS)

.PHONY: all clean