PREFIX=/usr/local/
CXXFLAGS=-I$(PREFIX)/include/opal -I$(PREFIX)/include -I$(PREFIX)/ptlib -g
CPPFLAGS=-D__LINUX__ -DDISABLE_VIDEO

telekarma: main.o pcss.o sip.o state.o telekarma.o telephony.o cliview.o view.o model.o
	gcc -o $@ $^ -lopal -lpt -L$(PREFIX)/lib
	@echo "Please copy the telekarma executable to the bin directory."

