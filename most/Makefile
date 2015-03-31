ECHO_CFLAGS=$(CFLAGS) -fPIC -Wall -std=c++0x -I ../extension_common -I .. -I ../ -I ../..
SOURCES=src/AudioMost.cpp src/ControlDesc.cpp src/Most.cpp \
 src/Optolyzer.cpp src/most_extension.cpp src/most_instance.cpp src/most_api.cc ../extension_common/extension.cc


all: libmost.so MOSTinit

most_api.cc:
	python ../extension_tools/generate_api.py src/most_api.js kSource_most_api src/most_api.cc

libmost.so: prepare  most_api.cc
	$(CC) $(ECHO_CFLAGS) -shared -o build/libmost.so \
	$(SYSROOT_FLAGS) -I./ $(SOURCES)

MOSTinit: build/Optolyzer.o
	g++ -std=c++0x -I .. -I ./ -I ../ -I ../.. -I ../extension_common src/MOSTinit.cpp -o build/MOSTinit build/Optolyzer.o $(SYSROOT_FLAGS) 

build/Optolyzer.o:
	g++ -std=c++0x -I .. -I ./ -I ../ -I ../.. -I ../extension_common -c src/Optolyzer.cpp -o build/Optolyzer.o

prepare:
	mkdir -p build

install: libmost.so MOSTinit
	install -D build/libmost.so \
	$(DESTDIR)/$(PREFIX)/lib/tizen-extensions-crosswalk/libmost.so
	install -D build/MOSTinit $(DESTDIR)/$(PREFIX)/bin/MOSTinit

clean:
	rm -Rf build
	rm src/most_api.cc

.PHONY: all prepare clean
