CXXFLAGS=-I/usr/local/include/opal
CPPFLAGS=-D__LINUX__ -DDISABLE_VIDEO

telekarma: main.o pcss.o sip.o state.o telekarma.o telephony.o
	gcc -o $@ $^ -lopal -lpt -L/usr/local/lib

