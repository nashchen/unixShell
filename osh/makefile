CC = g++
OBJECTS = osh.o parse.o
#makeup the executable file
osh:$(OBJECTS)
	$(CC) -g $(OBJECTS) -o osh
osh.o parse.o:osh.cpp parse.cpp
	$(CC) -g osh.cpp -c
	$(CC) -g parse.cpp -c
#make clean
clean:
	rm *.o