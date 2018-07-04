DEPS = src/main.c src/quic_engine_holder.c src/address_finder.c src/engine_structs.c src/socket_handler.c src/connection_establisher.c \
	src/connection_init_handler.c src/connection_read_handler.c src/connection_write_handler.c 
TEST_DEPS = src/test.c src/test/address_finder_test.c src/address_finder.c 
ALL_DEPS = $(TEST_DEPS)
INCLUDE = -I/home/sergei/Documents/git/lsquic-client -I/home/sergei/Documents/git/lsquic-client/include -I/home/sergei/Documents/git/boringssl/include
LINKS = -L/usr/local/lib -rdynamic /home/sergei/Documents/git/lsquic-client/src/liblsquic/liblsquic.a -levent -lpthread -Wl,-Bstatic -lssl -lcrypto -Wl,-Bdynamic -lz -lm -Wl,-rpath,/usr/local/lib
DEPS_OBJECTS=$(DEPS:.c=.o)
TEST_OBJECTS = $(TEST_DEPS:.c=.o)

all: main test

default: main

main: $(DEPS_OBJECTS)
	gcc -g $(DEPS_OBJECTS) $(INCLUDE) $(LINKS) -o main
	
test: $(TEST_OBJECTS)
	gcc -g $(TEST_OBJECTS) -o test

.c.o:
	gcc -g -c $< $(INCLUDE) -o $@

clean: 
	rm main test src/*.o
