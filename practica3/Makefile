CC           = gcc

CFLAGSFUSE   = `pkg-config fuse --cflags`
LLIBSFUSE    = `pkg-config fuse --libs`
LLIBSOPENSSL = -lcrypto

CFLAGS = -c -g -Wall -Wextra
LFLAGS = -g -Wall -Wextra

FUSE_EXAMPLES = fuse201504429 

.PHONY: all fuse-examples clean

all: fuse-examples

fuse-examples: $(FUSE_EXAMPLES)

fuse201504429: fuse201504429.o
	$(CC) $(LFLAGS) $^ -o $@ $(LLIBSFUSE)

fuse201504429.o: fuse201504429.c
	$(CC) $(CFLAGS) $(CFLAGSFUSE) $<

clean:
	rm -f $(FUSE_EXAMPLES)
	rm -f $(XATTR_EXAMPLES)
	rm -f $(OPENSSL_EXAMPLES)
	rm -f *.o
	rm -f *~
	rm -f handout/*~
	rm -f handout/*.log
	rm -f handout/*.aux
	rm -f handout/*.out



