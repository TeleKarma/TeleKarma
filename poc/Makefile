
caller: main.o
	gcc -o caller main.o -lopal -lpt -L/usr/local/lib

%.o: %.cpp
	g++ -c $^ -I/usr/local/include/opal
