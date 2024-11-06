OBJECTS = obj/ini.o obj/util.o

build: FLAGS = -ggdb 
build: build-lib build-test

build-objects:
	gcc -c $(FLAGS) -Isrc/include/ src/ini.c -o obj/ini.o
	gcc -c $(FLAGS) -Isrc/include/ src/util.c -o obj/util.o

build-lib: build-objects
	ar rcs bin/libinip.a $(OBJECTS)

build-test:
	gcc -o test $(FLAGS)test.c -L./bin -linip -Isrc/include/

shared: FLAGS = -fPIC -ggdb
shared: build-objects shared-lib install shared-test

shared-lib:
	gcc -shared $(OBJECTS) -o bin/shared/libinip.so

shared-test:
	gcc shared-test.c -o shared-test -linip -Isrc/include/

install: 
	sudo cp bin/shared/libinip.so /usr/lib
	sudo cp bin/libinip.a /usr/lib
	sudo cp -r src/include/* /usr/lib/include/
	sudo chmod 755 /usr/lib/include/ 
	sudo chmod 755 /usr/lib/libinip.so
	sudo chmod 755 /usr/lib/libinip.a

valgrind-clean:
	rm -f vgcore.*
