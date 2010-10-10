CXXFLAGS=-I/usr/local/include/opal
CPPFLAGS=-D__LINUX__ -DDISABLE_VIDEO

telekarma: main.o TkCentralManager.o TkOpalManager.o TkPCSSEndPoint.o TkSIPEndPoint.o
	gcc -o $@ $^ -lopal -lpt -L/usr/local/lib

