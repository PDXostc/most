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
# These command install the MOSTinit executable and the systemd framework to execute it at
# boot time so that MOST will be initialized by the time a POC app. needs it.
	install -D build/MOSTinit $(DESTDIR)/$(PREFIX)/bin/MOSTinit
	install -D ../systemd/MOST.service $(DESTDIR)/$(PREFIX)/lib/systemd/system/MOST.service
	install -D ../systemd/MOST-ready.path $(DESTDIR)/$(PREFIX)/lib/systemd/system/MOST-ready.path
	install -D ../systemd/conf.NDIS $(DESTDIR)/$(PREFIX)/../etc/most/conf.NDIS
	install -D ../systemd/conf.NUC $(DESTDIR)/$(PREFIX)/../etc/most/conf.NUC
	install -D ../systemd/conf.VTC1010 $(DESTDIR)/$(PREFIX)/../etc/most/conf.VTC1010

clean:
	rm -Rf build
	rm src/most_api.cc

.PHONY: all prepare clean
