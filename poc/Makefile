
caller: main.o
	gcc -o caller main.o -lopal -lpt -L/usr/local/lib

%.o: %.cxx
	g++ -c $^ -I/usr/local/include/opal
