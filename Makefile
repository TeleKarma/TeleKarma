PREFIX=/usr/local/
CXXFLAGS=-I$(PREFIX)/include/opal -g
CPPFLAGS=-D__LINUX__ -DDISABLE_VIDEO

telekarma: main.o pcss.o sip.o state.o telekarma.o telephony.o action.o
	gcc -o $@ $^ -lopal -lpt -L$(PREFIX)/lib
	@echo "Please copy the telekarma executable to the bin directory."

